#include "STPSALayerOPTICS.hh"
#include "STProcessManager.hh"

// STL
#include <cmath>
#include <cstdlib>

ClassImp(STPSALayerOPTICS)

STClusterizerOPTICS::STClusterizerOPTICS()
{
  fMinPointsForFit = 4;
  fPercPeakMin = 10;
  fPercPeakMax = 90;

  fRowHalfRange = 2; 
  fTbHalfRange  = 8; 
  fNCandInRange = fRowHalfRange*fTbHalfRange - 1; // Removing center bin.

  fThresholdADC = 1;
  fThresholdEps = 100;
}

STClusterizerOPTICS::~STClusterizerOPTICS()
{
}

void STClusterizerOPTICS::SetThresholdADC(Double_t val) { fThresholdADC = val; }
void STClusterizerOPTICS::SetThresholdEps(Double_t val) 
{ 
  fThresholdEps = val; 
  Double_t epsMin = Eps(0,0,fThresholdADC,1,0,fThresholdADC);
  if(fThresholdEps < epsMin)
  {
    cout << "[Warning] Epsilon Threshold cannot be smaller than " << epsMin << ". Adjusting threshold to " << epsMin << "." << endl;
  }
}

void STClusterizerOPTICS::SetSearchRangeFromPeak(Double_t rowHalf, Double_t tbHalf) 
{
  fRowHalfRange = rowHalf; 
  fTbHalfRange = tbHalf; 
  fNCandInRange = fRowHalfRange*fTbHalfRange - 1;
}

Double_t 
STClusterizerOPTICS::Eps(Double_t row1, 
                         Double_t tb1, 
                         Double_t adc1, 
                         Double_t row2, 
                         Double_t tb2, 
                         Double_t adc2)
{
  Double_t rowDiff  = row1 - row2;
  Double_t tbDiff   = (tb1 - tb2)/2.;
  Double_t distance = sqrt(rowDiff*rowDiff + tbDiff*tbDiff);
  Double_t weight   = 1./adc2;
  Double_t eps      = weight * distance * distance;

  return eps;
}

void 
STClusterizerOPTICS::Reset()
{
  fCountClusters = 0;

  fPeakPointArray.clear();

  for(Int_t iRow=0; iRow<fNumRows; iRow++) {
    for(Int_t iTb=0; iTb<fNumTbs; iTb++) {
      fStatus[iRow][iTb].SetStatus(OPTICSPointStatus::kQueue, -1, -1, 0);
    }
  }

  fPeakFinder = new TSpectrum();
}


void
STPSALayerOPTICS::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Reset();

  for(Int_t iLayer=0; iLayer<fPadLayers iLayer++)
  {
    fCurrentLayer = iLayer; 
    SetLayer(rawEvent);
    AnalyzeLayer(event);
  }
}

void
STClusterizerOPTICS::SetLayer(STRawEvent* rawEvent)
{
  for(Int_t iRow=0; iRow<fNumRows; iRow++)
  {
    STPad* pad = rawEvent -> GetPad(iRow,fCurrentLayer);
    if(!pad) continue;

    Double_t *adcDouble = pad -> GetADC(); 
    Float_t adcFloat[512] = {0};
    Float_t dummy[512]    = {0};

    for(Int_t iTb=0; iTb<fNumTbs; iTb++) 
    {
      Double_t adc = adcDouble[iTb];
      adcFloat[iTb] = adc;
      fStatus[iRow][iTb].adc = adc;
    }

    Int_t nPeaks = fPeakFinder -> SearchHighRes(adcFloat, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);

    for(Int_t iPeak=0; iPeak<nPeaks; iPeak++) 
    {
      Int_t tbPeak = (Int_t) ceil((peakFinder->GetPositionX())[iPeak]);
      Double_t adcPeak = fStatus[iRow][tbPeak].adc;
      if(adcPeak < fThresholdADC) continue;

      Double_t tbHit;
      Double_t tbArray[10] = {0};
      Double_t adcArray[10] = {0};

      Int_t countPoints = 0;
      for(Int_t iTb=tbPeak; iTb>tbPeak-10; iTb--) {
        Double_t adc = fStatus[iRow][iTb]

        if (adc < adcPeak*fPercPeakMin/100. || adc > peakValue*fPercPeakMax/100.)
          continue;

        tbArray[countPoints] = iTb;
        adcArray[countPoints] = adc;
        countPoints++;
      }

      if (countPoints < fMinPointsForFit) 
        continue;

      Double_t fitConst = 0;
      Double_t fitSlope = 0;
      LSLFit(countPoints, tbArray, adcArray, fitConst, fitSlope);
      Double_t tbHit = -fitConst/fitSlope;
      Double_t yHit = CalculateY(hitTime);

      OPTICSPoint point;
      point.Set(iRow, tbPeak, -1, adc, yHit);
      fPeakPointArray.push_back(point);
      fStatus[iRow][tbPeak].idxPeak = fPeakPointArray.size() - 1;
    }
  }
}

void 
STClusterizerOPTICS::AnalyzeLayer(STEvent* event)
{
  Int_t nPeaks = fPeakPointArray.size();
  OPTICSSortADC sortADC;
  std::sort(fPeakPointArray.begin(), fPeakPointArray.end(), sortADC);

  for(Int_t iPeak=0; iPeak<nPeaks; iPeak++)
  {
    OPTICSPoint point = fPeakPointArray[iPeak];
    Int_t rowPeak = point.row;
    Int_t tbPeak  = point.tb;

    if(fStatus[rowPeak][tbPeak].status != OPTICSPointStatus::kQueue)
      continue;

    RunOPTICS(rowPeak, tbPeak);

    event -> AddCluster(GetCurrentCluster());

    fCountClusters++;
  }
}

void
STClusterizerOPTICS::RunOPTICS(Int_t rowCenter, Int_t tbCenter)
{
  ResetCluster();
  Int_t idxPeakCenter = fStatus[rowCenter][tbCenter].idxPeak;
  AddPeakToCluster(fPeakPointArray[idxPeakCenter]);

  fStatus[rowCenter][tbCenter].status = OPTICSPointStatus::kCore;

  Double_t adcCenter = fStatus[rowCenter][tbCenter].adc;

  Int_t rowBoundLow  = rowCenter - fRowHalfRange;
  Int_t rowBoundHigh = rowCenter + fRowHalfRange;
  Int_t tbBoundLow   = tbCenter  - fTbHalfRange;
  Int_t tbBoundHigh  = tbCenter  + fTbHalfRange;

  OPTICSSortEps sortEps;
  std::vector<OPTICSPoint> orderingList;

  Double_t epsMax = -1;
  Double_t epsMin = -1;
  Double_t epsOld = -1;
  Double_t epsNew = -1;

  while(1)
  {
    Int_t indexCand = 0;
    while(1)
    {
      Int_t rowCand, tbCand;
      Bool_t inLoop = GetNbBin(1, indexCand, rowCenter, tbCenter, rowCand, tbCand);
      indexCand++;

      if(!inLoop) break;

      Int_t status = fStatus[rowCand][tbCand].status;
      if(status != OPTICSPointStatus::kQueue) 
        continue;

      fStatus[rowCand][tbCand].status = OPTICSPointStatus::kNoise;

      if(rowCand < rowBoundLow || rowCand > rowBoundHigh || 
         tbCand  < tbBoundLow  || tbCand  > tbBoundHigh)
        continue;
      if(rowCand < 0 || rowCand > fNumRows || tbCand < 0 || tbCand > fNumTbs)
        continue;

      Double_t adcCand = fStatus[rowCand][tbCand].adc;
      Double_t eps = Eps(rowCenter, tbCenter, adcCenter, rowCand, tbCand, adcCand);

      if(eps<fThresholdEps)
      {
        fStatus[rowCand][tbCand].status = OPTICSPointStatus::kCore;
        OPTICSPoint point;
        point.Set(rowCand, tbCand, eps);
        orderingList.push_back(point);
      }
    }
    Int_t nSortedPoints = orderingList.size();
    if(nSortedPoints==0) break;

    std::sort(orderingList.begin(), orderingList.end(), sortEps);

    rowCenter = orderingList[0].row;
    tbCenter  = orderingList[0].tb;
    adcCenter = fStatus[rowCenter][tbCenter].adc;

    epsOld = epsNew;
    epsNew = orderingList[0].eps;

    if(epsOld < epsNew) {
      epsMax = epsNew;
      epsMin = epsNew;
    }
    else 
      epsMin = epsNew;

    if(epsMin < .9*epsMax)
      break;

    Int_t idxPeakCand = fStatus[rowCenter][tbCenter].idxPeak;
    if(idxPeakCand != -1)
      AddPeakToCluster(fPeakPointArray[idxPeakCand]);

    orderingList.erase(orderingList.begin());
  }
}

void 
STClusterizerOPTICS::ResetCluster()
{
  fNPointsInCluster = 0;
  fPosCluster.SetXYZ(0,0,CalculateZ(fCurrentLayer));
  fSigmaCluster.SetXYZ(0,0,fPadSizeZ/2.);
  fChargeCluster = 0;
}

void  
STClusterizerOPTICS::AddPeakToCluster(OPTICSPoint point)
{
  Double_t x = CalculateX(point.row);
  Double_t y = point.y;
  Double_t charge = point.adc;

  Double_t chargeSum  = fChargeCluster + sum;

  Double_t xNew  = fPosCluster.X() * fChargeCluster / chargeSum + x / chargeSum;
  Double_t yNew  = fPosCluster.Y() * fChargeCluster / chargeSum + y / chargeSum;

  Double_t xSigmaNew;
  Double_t ySigmaNew;

  if(fNPointsInCluster == 0) 
  {
    xSigmaNew = 0;
    ySigmaNew = 0;
  }
  else
  {
    Double_t xDiff = xNew - x;
    Double_t yDiff = yNew - y;

    xSigmaNew = sqrt( fSigmaCluster.X() * fSigmaCluster.X() * fNPointsInCluster / chargeSum + xDiff * xDiff / fNPointsInCluster );
    ySigmaNew = sqrt( fSigmaCluster.Y() * fSigmaCluster.Y() * fNPointsInCluster / chargeSum + yDiff * yDiff / fNPointsInCluster );
  }

  fPosCluster.SetX();

  fChargeCluster += charge;

  fNPointsInCluster++;
}

STHitCluster*
STClusterizerOPTICS::GetCurrentCluster()
{
  if (fNPointsInCluster == 1)
  {
    fSigmaCluster.SetX(fPadSizeX/2.);
    fSigmaCluster.SetY(0); // TODO : How to set sigma in tb?
  }
  STHitCluster* cluster = new STHitCluster();
  cluster -> SetPosition(fPosCluster);
  cluster -> SetPosSigma(fSigmaCluster);
  cluster -> SetCharge(fChargeCluster);

  return cluster;
}

Bool_t
STClusterizerOPTICS::GetNbBin(Int_t rBox, Int_t index, Int_t row0, Int_t tb0, Int_t &row, Int_t &tb)
{
  Int_t nBinsPerSide = 2*rBox;

  if(index>=nBinsPerSide*4)
    return kFALSE;

  Int_t indexSide = index/nBinsPerSide;
  Int_t indexRemainder = index%nBinsPerSide;

  switch(indexSide)
  {
    case 0 :
      row = rBox - 1 - indexRemainder;
      tb = rBox;
      break;

    case 1 :
      row = -rBox;
      tb = rBox - 1 - indexRemainder;
      break;

    case 2 :
      row = -(rBox - 1 - indexRemainder);
      tb = -rBox;
      break;

    case 3 :
      row = rBox;
      tb = -(rBox - 1 - indexRemainder);
      break;
  }

  row += row0;
  tb += tb0;

  return kTRUE;
}


void
STPSALayerOPTICS::LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope)
{
  Double_t sumXY = 0, sumX = 0, sumY = 0, sumX2 = 0;
  for (Int_t iPoint = 0; iPoint < numPoints; iPoint++) {
    sumXY += x[iPoint]*y[iPoint];
    sumX += x[iPoint];
    sumY += y[iPoint];
    sumX2 += x[iPoint]*x[iPoint];
  }

  slope = (numPoints*sumXY - sumX*sumY)/(numPoints*sumX2 - sumX*sumX);
  constant = (sumX2*sumY - sumX*sumXY)/(numPoints*sumX2 - sumX*sumX);
}
