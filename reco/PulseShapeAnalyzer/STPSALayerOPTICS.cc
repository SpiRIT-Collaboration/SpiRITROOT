#include "STPSALayerOPTICS.hh"
#include "STProcessManager.hh"

// STL
#include <cmath>
#include <cstdlib>
#include <iostream>

// ROOT
#include "RVersion.h"

ClassImp(STPSALayerOPTICS)

STPSALayerOPTICS::STPSALayerOPTICS()
{
  fMinPointsForFit = 4;
  fPercPeakMin = 10;
  fPercPeakMax = 90;

  fRowHalfRange = 2; 
  fTbHalfRange  = 4; 
  fNCandInRange = (2*fRowHalfRange+1)*(2*fTbHalfRange+1) - 1; // Removing center bin.

  fThresholdADC = 100;
  fThresholdEps = 10;

#ifdef DEBUG_PLOT
  gStyle -> SetPalette(55);
  gStyle -> SetOptStat(0);

  fCvsFrame = new TCanvas("cvsFrame","",650,650);
  //fCvsFrame -> SetLogz();
  fHistDataSet = new TH2D("histDataSet","",fPadRows,0,fPadRows,fNumTbs,0,fNumTbs);
  fGraphCluster = new TGraph();
  fGraphCluster -> SetMarkerColor(kRed-9);
  fGraphCluster -> SetMarkerStyle(30);

  fGraphClusterPeaks = new TGraph();
  fGraphClusterPeaks -> SetMarkerColor(kRed-9);
  fGraphClusterPeaks -> SetMarkerStyle(30);
  fGraphClusterPoints = new TGraph();
  fGraphClusterPoints -> SetLineColor(17);
  fGraphClusterPoints -> SetLineStyle(3);

  fCvsEps = new TCanvas("cvsEps","",650,0,650,400);
  fHistEps = new TH1D("histEps",";step;#varepsilon",fNCandInRange,0,fNCandInRange);
  fHistEps -> SetFillColor(9);
  //fHistEpsAll = new TH1D("histEpsAll",";step;#varepsilon",fNCandInRange,0,fNCandInRange);
  //fHistEpsAll -> SetLineColor(9);
  //fHistEpsAll -> SetLineStyle(9);

  //fCvsFrameXY = new TCanvas("cvsFrameXY","",650,0,650,650);
  //fHistXY = new TH2D("histDataSetXY","",100,-500,500,100,-500,0);
  //fGraphClusterXY = new TGraph();
  //fGraphClusterXY -> SetMarkerColor(kRed-9);
  //fGraphClusterXY -> SetMarkerStyle(30);
#endif
}

STPSALayerOPTICS::~STPSALayerOPTICS()
{
}

void STPSALayerOPTICS::SetThresholdADC(Double_t val) { fThresholdADC = val; }
void STPSALayerOPTICS::SetThresholdEps(Double_t val) 
{
  fThresholdEps = val; 
  Double_t epsMin = Eps(0,0,fThresholdADC,1,0,fThresholdADC);
  if(fThresholdEps < epsMin)
  {
    LOG(WARNING) << "[Warning] Epsilon Threshold cannot be smaller than " << epsMin << ". Adjusting threshold to " << epsMin << "." << FairLogger::endl;
  }
}

void STPSALayerOPTICS::SetSearchRangeFromPeak(Double_t rowHalf, Double_t tbHalf) 
{
  fRowHalfRange = rowHalf; 
  fTbHalfRange = tbHalf; 
  fNCandInRange = (2*fRowHalfRange+1)*(2*fTbHalfRange+1) - 1;
}

Double_t 
STPSALayerOPTICS::Eps(Double_t row1, 
                         Double_t tb1, 
                         Double_t adc1, 
                         Double_t row2, 
                         Double_t tb2, 
                         Double_t adc2)
{
  Double_t rowDiff  = (row1 - row2);
  Double_t tbDiff   = tb1 - tb2;
  Double_t distance = sqrt(rowDiff*rowDiff + tbDiff*tbDiff);
  Double_t weight   = 1./adc2;
  Double_t eps      = weight * distance * distance;

  return eps;
}

void
STPSALayerOPTICS::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  for(Int_t iLayer=0; iLayer<fPadLayers; iLayer++)
  {
#ifdef DEBUG_PLOT
    if(iLayer != 10) continue;
#endif
    Reset();

    fCurrentLayer = iLayer; 
#ifdef DEBUG_TIME
    TStopwatch timer;
    timer.Start();
#endif
    SetLayer(rawEvent);
#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME] SetLayer()     " << iLayer << " " << timer.RealTime() << std::endl;
#endif
#ifdef DEBUG_TIME
    timer.Start();
#endif
    AnalyzeLayer(event);
#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME] AnalyzeLayer() " << iLayer << " " << timer.RealTime() << std::endl;
    std::cout << std::endl;
#endif
#ifdef DEBUG_PLOT
    /*
    fCvsFrameXY -> cd();
    fHistXY -> Draw();
    fGraphClusterXY -> Draw("psame");
    fCvsFrameXY -> Modified();
    fCvsFrameXY -> Update();

    fCvsFrame -> cd();
    fHistDataSet -> Draw("col");
    fGraphCluster -> Draw("psame");
    fCvsFrame -> Modified();
    fCvsFrame -> Update();

    Int_t number;
    std::cout << "[DEBUG] enter any number... "; std::cin >> number;
    std::cout << std::endl;
    fHistDataSet -> Reset();
    fGraphCluster -> Clear();
    fGraphCluster -> Set(0);
    fGraphClusterXY -> Clear();
    fGraphClusterXY -> Set(0);
    */
#endif
  }
  event -> SetIsClustered(kTRUE);
}

void 
STPSALayerOPTICS::Reset()
{
  fCountClusters = 0;
#ifdef DEBUG_PLOT
  fIdxGraphCluster = 0;
#endif

  fPeakPointArray.clear();

  for(Int_t iRow=0; iRow<fPadRows; iRow++) {
    for(Int_t iTb=0; iTb<fNumTbs; iTb++) {
      fStatus[iRow][iTb].SetStatus(OPTICSPointStatus::kQueue, -1, 0);
    }
  }

  fPeakFinder = new TSpectrum();
}

void
STPSALayerOPTICS::SetLayer(STRawEvent* rawEvent)
{
  for(Int_t iRow=0; iRow<fPadRows; iRow++)
  {
#ifdef DEBUG_TIME
    TStopwatch timer;
    timer.Start();
#endif
    STPad* pad = rawEvent -> GetPad(iRow,fCurrentLayer);
    if(!pad) continue;
#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME]   Get Pad:   " << iRow << " " << timer.RealTime() << std::endl;
#endif

    Double_t *adcDouble = pad -> GetADC(); 
    Float_t adcFloat[512] = {0};

    for(Int_t iTb=0; iTb<fNumTbs; iTb++) 
    {
      Double_t adc = adcDouble[iTb];
      adcFloat[iTb] = adc;
      fStatus[iRow][iTb].adc = adc;
#ifdef DEBUG_PLOT
      fHistDataSet -> Fill(iRow, iTb, adc);
#endif
    }

#ifdef DEBUG_TIME
    timer.Start();
#endif

#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
    Float_t dummy[512]    = {0};
    Int_t nPeaks = fPeakFinder -> SearchHighRes(adcFloat, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);
#else
    Double_t dummy[512]    = {0};
    Int_t nPeaks = fPeakFinder -> SearchHighRes(adcDouble, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);
#endif

#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME]   Find Peak: " << iRow << " " << timer.RealTime() << std::endl;
#endif

#ifdef DEBUG_TIME
    timer.Start();
#endif
    for(Int_t iPeak=0; iPeak<nPeaks; iPeak++) 
    {
      Int_t tbPeakTemp = (Int_t) ceil((fPeakFinder->GetPositionX())[iPeak]);
      Double_t adcTemp = fStatus[iRow][tbPeakTemp].adc;
      Int_t tbPeak = tbPeakTemp;

      for(Int_t i=1; i<3; i++)
      {
        Int_t tbTemp = tbPeak + i;

        if(tbTemp<0 && tbTemp>=fNumTbs) 
          break;

        if(adcTemp < fStatus[iRow][tbTemp].adc)
          tbPeak = tbTemp;
      }

      for(Int_t i=1; i<3; i++)
      {
        Int_t tbTemp = tbPeak - i;

        if(tbTemp<0 && tbTemp>=fNumTbs) 
          break;

        if(adcTemp < fStatus[iRow][tbTemp].adc)
          tbPeak = tbTemp;
      }

      Double_t adcPeak = fStatus[iRow][tbPeak].adc;
      if(adcPeak < fThresholdADC) continue;

      Double_t tbArray[10] = {0};
      Double_t adcArray[10] = {0};

      Int_t countPoints = 0;
      for(Int_t iTb=tbPeak; iTb>tbPeak-10; iTb--) 
      {
        Double_t adc = fStatus[iRow][iTb].adc;

        if (adc < adcPeak*fPercPeakMin/100. || adc > adcPeak*fPercPeakMax/100.)
          continue;

        tbArray[countPoints] = iTb;
        adcArray[countPoints] = adc;
        countPoints++;
      }

      if (countPoints < fMinPointsForFit) 
        continue;

      Double_t fitConst = 0;
      Double_t fitSlope = 0;
      Double_t chi2 = 0;
      LSLFit(countPoints, tbArray, adcArray, fitConst, fitSlope, chi2); // chi2 is not stored.
      Double_t tbHit = -fitConst/fitSlope;
      Double_t yHit = CalculateY(tbHit);
      if (yHit > 0 || yHit < -fMaxDriftLength)
        continue;
#ifdef DEBUG_PLOT
      std::cout << "y: " << tbHit << " " << yHit << std::endl;
#endif

      OPTICSPoint point;
      point.Set(iRow, tbPeak, -1, adcPeak, yHit);
      fPeakPointArray.push_back(point);

#ifdef DEBUG_PLOT
      //fGraphCluster -> SetPoint(fIdxGraphCluster++, iRow+.5, tbPeak+.5);
#endif
    }
#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME]   Ana Peak:  " << iRow << " " << timer.RealTime() << std::endl;
#endif
  }
}

void 
STPSALayerOPTICS::AnalyzeLayer(STEvent* event)
{
  Int_t nPeaks = fPeakPointArray.size();
  OPTICSSortADC sortADC;
  std::sort(fPeakPointArray.begin(), fPeakPointArray.end(), sortADC);

  for(Int_t iPeak=0; iPeak<nPeaks; iPeak++)
  {
    OPTICSPoint point = fPeakPointArray[iPeak];
    Int_t rowPeak = point.row;
    Int_t tbPeak  = point.tb;
    fStatus[rowPeak][tbPeak].idxPeak = iPeak;
#ifdef DEBUG_PLOT
    std::cout << "Peak: " << iPeak << " " << fStatus[rowPeak][tbPeak].adc << std::endl;
#endif
  }

  for(Int_t iPeak=0; iPeak<nPeaks; iPeak++)
  {
    OPTICSPoint point = fPeakPointArray[iPeak];
    Int_t rowPeak = point.row;
    Int_t tbPeak  = point.tb;

    if(fStatus[rowPeak][tbPeak].status != OPTICSPointStatus::kQueue)
      continue;

    if(rowPeak<fPadRows/4 && rowPeak>fPadRows*3/4)
      SetSearchRangeFromPeak(3,6);
    else
      SetSearchRangeFromPeak(2,4);

#ifdef DEBUG_TIME
    TStopwatch timer;
    timer.Start();
#endif
    RunOPTICS(rowPeak, tbPeak);
#ifdef DEBUG_TIME
    timer.Stop();
    std::cout << "[TIME]   RunOPTICS() " << iPeak << " " << timer.RealTime() << std::endl;
#endif
    event -> AddCluster(GetCurrentCluster());

#ifdef DEBUG_PLOT
    fCvsFrame -> cd();
    fHistDataSet -> Draw("colz");
    fGraphClusterPeaks -> Draw("psame");
    fGraphClusterPoints -> Draw("lsame");

    Int_t rowBoundLow  = rowPeak - 10*fRowHalfRange;
    Int_t rowBoundHigh = rowPeak + 10*fRowHalfRange;
    Int_t tbBoundLow   = tbPeak  - 10*fTbHalfRange;
    Int_t tbBoundHigh  = tbPeak  + 10*fTbHalfRange;
    //fHistDataSet -> GetXaxis() -> SetRangeUser(rowBoundLow, rowBoundHigh);
    //fHistDataSet -> GetYaxis() -> SetRangeUser(tbBoundLow, tbBoundHigh);
    fCvsFrame -> Modified();
    fCvsFrame -> Update();

    fCvsEps -> cd();
    fHistEps -> Draw();
    //fHistEpsAll -> Draw("same");
    fCvsEps -> Modified();
    fCvsEps -> Update();

    Int_t number;
    std::cout << "[DEBUG ] enter any number... "; std::cin >> number;
    if(number == -1) break;
    std::cout << std::endl;
    fGraphClusterPeaks -> Clear();
    fGraphClusterPeaks -> Set(0);
    fGraphClusterPoints -> Clear();
    fGraphClusterPoints -> Set(0);
    fHistEps -> Reset();
    //fHistEpsAll -> Reset();
#endif

    fCountClusters++;
  }
}

void
STPSALayerOPTICS::RunOPTICS(Int_t rowCenter, Int_t tbCenter)
{
  ResetCluster();
  Int_t idxPeakCenter = fStatus[rowCenter][tbCenter].idxPeak;
  AddPeakToCluster(fPeakPointArray[idxPeakCenter]);

  fStatus[rowCenter][tbCenter].status = OPTICSPointStatus::kCore;

  Double_t adcCenter = fStatus[rowCenter][tbCenter].adc;

#ifdef DEBUG_PLOT
      std::cout << "Step Peak: " << "-" << " " << rowCenter << " " << tbCenter << " " << fStatus[rowCenter][tbCenter].adc << " " << 0 << std::endl;
#endif

  Int_t rowStart = rowCenter;
  Int_t tbStart = tbCenter;

  Int_t rowBoundLow  = rowCenter - fRowHalfRange;
  Int_t rowBoundHigh = rowCenter + fRowHalfRange;
  Int_t tbBoundLow   = tbCenter  - fTbHalfRange;
  Int_t tbBoundHigh  = tbCenter  + fTbHalfRange;

  OPTICSSortEps sortEps;
  std::vector<OPTICSPoint> orderingList;

  Double_t epsMax = 0;
  Double_t epsMin = 0;
  Double_t epsOld = 0;
  Double_t epsNew = 0;

  Int_t countStep = 0;
  while(1)
  {
    Int_t countCand = 0;
#ifdef DEBUG_PLOT
    fGraphClusterPoints -> SetPoint(countStep, rowCenter+.5, tbCenter+.5);
    fHistEps -> Fill(countStep, epsNew);
#endif
    while(1)
    {
      Int_t rowCand, tbCand;
      Bool_t inLoop = GetNbBin(1, countCand, rowCenter, tbCenter, rowCand, tbCand);
      countCand++;

      if(!inLoop) break;

      Int_t status = fStatus[rowCand][tbCand].status;
      if(status != OPTICSPointStatus::kQueue) 
        continue;

      if(rowCand < rowBoundLow || rowCand > rowBoundHigh || 
         tbCand  < tbBoundLow  || tbCand  > tbBoundHigh)
        continue;
      if(rowCand < 0 || rowCand > fPadRows || tbCand < 0 || tbCand > fNumTbs)
        continue;

      Double_t adcCand = fStatus[rowCand][tbCand].adc;
      Double_t eps = Eps(rowCenter, tbCenter, adcCenter, rowCand, tbCand, adcCand);

      if(eps<fThresholdEps)
      {
        OPTICSPoint point;
        point.Set(rowCand, tbCand, eps);
        orderingList.push_back(point);

        fStatus[rowCand][tbCand].status = OPTICSPointStatus::kCore;
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

    if(epsMin < epsMax)
      break;

    Int_t idxPeakCand = fStatus[rowCenter][tbCenter].idxPeak;
    if(idxPeakCand != -1 && rowCenter != rowStart)
    {
      AddPeakToCluster(fPeakPointArray[idxPeakCand]);
      fStatus[rowCenter][tbCenter].idxPeak = -1;
#ifdef DEBUG_PLOT
      std::cout << "Step Peak: " << countStep << " " << rowCenter << " " << tbCenter << " " << fStatus[rowCenter][tbCenter].adc << " " << epsNew << std::endl;
#endif
    }

    orderingList.erase(orderingList.begin());

    countStep++;
  }

  Int_t nLeftOvers = orderingList.size();
  for(Int_t i=0; i<nLeftOvers; i++)
  {
    OPTICSPoint point = orderingList.back();
    fStatus[point.row][point.tb].status = OPTICSPointStatus::kQueue;
    orderingList.pop_back();
  }
}

void 
STPSALayerOPTICS::ResetCluster()
{
  fNPointsInCluster = 0;
  fPosCluster.SetXYZ(0,0,CalculateZ(fCurrentLayer));
  fSigmaCluster.SetXYZ(0,0,fPadSizeZ/2.);
  fChargeCluster = 0;
}

void  
STPSALayerOPTICS::AddPeakToCluster(OPTICSPoint point)
{
#ifdef DEBUG_PLOT
  fGraphClusterPeaks -> SetPoint(fNPointsInCluster, point.row+.5, point.tb+.5);
#endif
  Double_t x = CalculateX(point.row);
  Double_t y = point.y;
  Double_t charge = point.adc;

  Double_t chargeSum  = fChargeCluster + charge;

  Double_t xNew  = fPosCluster.X() * fChargeCluster / chargeSum + charge * x / chargeSum;
  Double_t yNew  = fPosCluster.Y() * fChargeCluster / chargeSum + charge * y / chargeSum;

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

    xSigmaNew = sqrt( fSigmaCluster.X() * fSigmaCluster.X() * fNPointsInCluster / chargeSum + charge * xDiff * xDiff / fNPointsInCluster );
    ySigmaNew = sqrt( fSigmaCluster.Y() * fSigmaCluster.Y() * fNPointsInCluster / chargeSum + charge * yDiff * yDiff / fNPointsInCluster );
  }

  //std::cout << "[DEBUG]   adding:  " << x << " " << y << " " << charge << std::endl;

  fPosCluster.SetX(xNew);
  fPosCluster.SetY(yNew);

  fSigmaCluster.SetX(xSigmaNew);
  fSigmaCluster.SetY(ySigmaNew);

  fChargeCluster += charge;

  //std::cout << "[DEBUG]   result:  " << fPosCluster.X() << " " << fPosCluster.Y() << " " << fChargeCluster << std::endl;

  fNPointsInCluster++;
}

STHitCluster*
STPSALayerOPTICS::GetCurrentCluster()
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

#ifdef DEBUG_PLOT
  std::cout << "[DEBUG] cluster! " 
            << fCountClusters  << " " 
            << fNPointsInCluster << " : " 
            << fPosCluster.X() << " " 
            << fPosCluster.Y() << " " 
            << fPosCluster.Z() << std::endl;
  //fGraphClusterXY -> SetPoint(fCountClusters, fPosCluster.X(), fPosCluster.Y());
#endif

  return cluster;
}

Bool_t
STPSALayerOPTICS::GetNbBin(Int_t rBox, Int_t index, Int_t row0, Int_t tb0, Int_t &row, Int_t &tb)
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
