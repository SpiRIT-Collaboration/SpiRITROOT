#include "STClusterizerScan.hh"
#include "STLinearTrack.hh"

#include "TMath.h"

#include <iostream>
#include <vector>

#define DEBUG

STClusterizerScan::STClusterizerScan()
{
  fHitArray = new std::vector<STHit *>;
  fHitArray -> reserve(5000);
  fHitClusterArray = new std::vector<STHitCluster *>;
  fHitClusterArray -> reserve(100);

  SetVerticalCutTbUnit(3);
  //SetVerticalCutTbUnit(0.8);
  SetHorizontalCutPadUnit(4);
  SetProximityCutInUnit(1.5, 2.5, 0.5);
  SetSigmaCutInUnit(4.0, 4.0, 0.8);

  fClusterTemp = new STHitCluster();

#ifdef DEBUG
  fCvs = new TCanvas("cvs","",1200,800);
  //fFrame = new TH2D("hist",";z;x;", 100,0,1500, 100,-500,500);
  fFrame = new TH2D("hist",";z;x;", 100,100,800, 100,-100,100);
  fFrame -> Draw();
  fGraphHit = new TGraph();
  fGraphHit -> SetMarkerColor(kGray+1);
  fGraphHit -> SetLineColor(kGray);
  fGraphHit -> SetMarkerStyle(20);
  fGraphHit -> SetMarkerSize(0.5);
  fGraphHit -> SetLineStyle(2);
  fGraphAddedHit = new TGraph();
  fGraphAddedHit -> SetMarkerColor(kRed);
  fGraphAddedHit -> SetMarkerStyle(20);
  fGraphAddedHit -> SetMarkerSize(0.5);
  fGraphCurrentCluster = new TGraphErrors();
  fGraphCurrentCluster -> SetFillStyle(3002);
  fGraphCurrentCluster -> SetFillColor(kRed);
  fGraphCurrentCluster -> SetMarkerColor(kRed);
  fGraphCurrentCluster -> SetLineColor(kRed);
  fGraphCurrentCluster -> SetMarkerStyle(24);
  fGraphCurrentCluster -> SetMarkerSize(1.5);
  fGraphCluster = new TGraphErrors();
  fGraphCluster -> SetMarkerColor(kBlack);
  fGraphCluster -> SetFillStyle(3002);
  fGraphCluster -> SetFillColor(kGray);
  fGraphCluster -> SetMarkerStyle(24);
  fGraphCluster -> SetMarkerSize(1.5);
  fGraphFinishedCluster = new TGraphErrors();
  fGraphFinishedCluster -> SetMarkerColor(kBlack);
  fGraphFinishedCluster-> SetFillStyle(3002);
  fGraphFinishedCluster-> SetFillColor(kGray);
  fGraphFinishedCluster -> SetMarkerStyle(20);
  fGraphFinishedCluster -> SetMarkerSize(1.5);
#endif
}

STClusterizerScan::~STClusterizerScan()
{
}

void STClusterizerScan::SetVerticalCut(Double_t vCut)            { fVerticalCut = vCut; }
void STClusterizerScan::SetVerticalCutTbUnit(Double_t vCut)      { fVerticalCut = vCut*fYTb; }
void STClusterizerScan::SetHorizontalCut(Double_t hCut)          { fHorizontalCut = hCut; }
void STClusterizerScan::SetHorizontalCutPadUnit(Double_t hCut)   { fHorizontalCut = hCut*fPadSizeX; }

void STClusterizerScan::SetParameters(Double_t *par)
{
  SetHorizontalCutPadUnit(par[1]);
  SetVerticalCutTbUnit(par[2]);
}

void 
STClusterizerScan::AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut)
{
  Int_t numTracks = trackArray -> GetEntries();

  fHitClusterArray -> clear();
  for (Int_t iTrack = 0; iTrack < numTracks; iTrack++)
  {
    STLinearTrack *track = (STLinearTrack*) trackArray -> At(iTrack);
    std::vector<STHit*> *hitArrayFromTrack = track -> GetHitPointerArray();

    fHitArray -> clear();

    Int_t nHits = hitArrayFromTrack -> size();
    for(Int_t iHit=0; iHit<nHits; iHit++) 
    {
      STHit *hit = hitArrayFromTrack -> at(iHit);
      hit -> SetPosition(hit -> GetPosition() - fPrimaryVertex);
      fHitArray -> push_back(hit);
    }

    AnalyzeHitArray(eventOut);
  }
}

void 
STClusterizerScan::Analyze(STEvent* eventIn, STEvent* eventOut)
{
  fRCut = sqrt(  fZCut*fZCut
              + fVerticalCut*fVerticalCut 
              + fHorizontalCut*fHorizontalCut );

  fLogger -> Debug(MESSAGE_ORIGIN,Form("Z-Cut: %f",fZCut));
  fLogger -> Debug(MESSAGE_ORIGIN,Form("Vertical-Cut: %f",fVerticalCut));
  fLogger -> Debug(MESSAGE_ORIGIN,Form("Horizontal-Cut: %f",fHorizontalCut));



  fHitArray -> clear();
  fHitClusterArray -> clear();


  Int_t nHits = eventIn -> GetNumHits();
  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventIn -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() - fPrimaryVertex);
    fHitArray -> push_back(hit);
  }

  STHitSortRInv sortR;
  std::sort(fHitArray -> begin(), fHitArray -> end(), sortR);

  AnalyzeHitArray(eventOut);

  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventIn -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() + fPrimaryVertex);
  }
}

void 
STClusterizerScan::AnalyzeHitArray(STEvent* eventOut)
{
  Int_t nHits = fHitArray -> size();

#ifdef DEBUG
  //for(Int_t iHit=0; iHit<nHits; iHit++) 
  //{
  //  STHit *hit = fHitArray -> at(nHits-iHit-1);
  //  fGraphHit -> SetPoint(iHit, (hit -> GetPosition()).Z(), (hit -> GetPosition()).X() );
  //}
#endif

  Int_t nClusters = 0;

  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
#ifdef DEBUG
    fGraphHit -> Set(0);
    fGraphHit -> Clear(0);
    for(Int_t jHit=0; jHit<nHits-iHit; jHit++) 
    {
      STHit *hitTemp = fHitArray -> at(jHit);
      fGraphHit -> SetPoint(jHit, (hitTemp -> GetPosition()).Z(), (hitTemp -> GetPosition()).X() );
    }
#endif
    STHit *hit = fHitArray -> back();
    fHitArray -> pop_back();

    Double_t xHit = (hit -> GetPosition()).X();
    Double_t yHit = (hit -> GetPosition()).Y();
    Double_t zHit = (hit -> GetPosition()).Z();

    Double_t rHit = sqrt(xHit*xHit + yHit*yHit);
    Double_t theta = TMath::ATan2(rHit,zHit);

    /*
    if (zHit > 200 )
      SetZCutPadUnit(1.2);
    else 
      SetZCutPadUnit(0.8);
      */

    /*
    if (theta > TMath::Pi()*25./180)
    {
      SetZCutPadUnit(0.1);
      SetVerticalCutTbUnit(4);
      SetHorizontalCutPadUnit(4);
    }
    else
    {
      SetZCutPadUnit(0.8);
      SetVerticalCutTbUnit(3);
      SetHorizontalCutPadUnit(3);
    }
    */



    // remove clusters out of reach
    nClusters = fHitClusterArray -> size();
    Double_t rhoHit = (hit -> GetPosition()).Mag();
#ifdef DEBUG
    fGraphCluster -> Clear();
    fGraphCluster -> Set(0);
#endif
    for(Int_t iCluster=nClusters-1; iCluster>=0; iCluster--)
    {
      STHitCluster *cluster = fHitClusterArray -> at(iCluster);
      
      Double_t rCluster = (cluster -> GetPosition()).Mag();
      if( rCluster - rhoHit > fRCut ) 
      {
        AddClusterToEvent(eventOut, cluster);
        fHitClusterArray -> erase(fHitClusterArray -> begin() + iCluster);
      }
#ifdef DEBUG
      else
      {
        fGraphCluster -> SetPoint(iCluster, (cluster -> GetPosition()).Z(), (cluster -> GetPosition()).X() );
        fGraphCluster -> SetPointError(iCluster, (cluster -> GetPosSigma()).Z(), (cluster -> GetPosSigma()).X() );
      }
#endif
    }

#ifdef DEBUG
    Int_t number; 
    std::cin >> number; 
    if (number == -1) 
      break;
    fGraphAddedHit -> SetPoint(0, (hit -> GetPosition()).Z(), (hit -> GetPosition()).X() );
#endif

    // correlate hit with clusters 
    Bool_t addHitToCluster = kFALSE;
    nClusters = fHitClusterArray -> size();
    for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      STHitCluster *cluster = fHitClusterArray -> at(iCluster);

#ifdef DEBUG
      std::cout << std::endl;
      std::cout << "===  DEBUGING  " << iCluster << "-th cluster! =========" << std::endl;
#endif
      addHitToCluster = CorrelateHC(hit, cluster);
      if (addHitToCluster) 
      {
#ifdef DEBUG
        fGraphCurrentCluster -> SetPoint(0, (cluster -> GetPosition()).Z(), (cluster -> GetPosition()).X() );
        fGraphCurrentCluster -> SetPointError(0, (cluster -> GetPosSigma()).Z(), (cluster -> GetPosSigma()).X() );
#endif
        cluster -> AddHit(hit);
        break;
      }
    }


    // make new cluster if hit is not added to any clusters 
    // or there are no clusters in fHitClusterArray
    if (!addHitToCluster)
    {
#ifdef DEBUG
      std::cout << "!!! New Cluster !!!" << std::endl;
#endif
      STHitCluster* cluster = new STHitCluster();
      cluster -> AddHit(hit);
      fHitClusterArray -> push_back(cluster);
#ifdef DEBUG
      fGraphCurrentCluster -> SetPoint(0, (cluster -> GetPosition()).Z(), (cluster -> GetPosition()).X() );
      fGraphCurrentCluster -> SetPointError(0, (cluster -> GetPosSigma()).Z(), (cluster -> GetPosSigma()).X() );
#endif
    }
#ifdef DEBUG
    fGraphHit -> Draw("samepl");
    fGraphCluster -> Draw("samep");
    fGraphFinishedCluster -> Draw("samepz");
    fGraphAddedHit -> Draw("samepz");
    fGraphCurrentCluster -> Draw("samepz");
    fCvs -> Modified();
    fCvs -> Update();
#endif
  }

  nClusters = fHitClusterArray -> size();
  for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster *cluster = fHitClusterArray -> at(iCluster);
    AddClusterToEvent(eventOut, cluster);
  }
}

Bool_t 
STClusterizerScan::CorrelateHC(STHit* hit, STHitCluster* cluster)
{
  Double_t phi = 0;

  Double_t xHit = (hit -> GetPosition()).X();
  Double_t yHit = (hit -> GetPosition()).Y();
  Double_t zHit = (hit -> GetPosition()).Z();

  Double_t xCluster = (cluster -> GetPosition()).X();
  Double_t yCluster = (cluster -> GetPosition()).Y();
  Double_t zCluster = (cluster -> GetPosition()).Z();

  Double_t xSigma = (cluster -> GetPosSigma()).X();
  Double_t ySigma = (cluster -> GetPosSigma()).Y();

  Double_t charge = hit -> GetCharge();
  Double_t chargeCluster = cluster -> GetCharge();





  Double_t dX = xHit - xCluster;
  Double_t dY = yHit - yCluster;
  Double_t dZ = zHit - zCluster;
#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "phi:              " << phi << std::endl;
  std::cout << "number of hits:   " << cluster -> GetNumHits() << std::endl;
  std::cout << "hit position:     " << xHit << "  " << yHit << "  " << zHit << std::endl;
  std::cout << "cluster position: " << xCluster << "  " << yCluster << "  " << zCluster << std::endl;
  std::cout << "cluster sigma:    " << xSigma << "  " << ySigma << std::endl;
  std::cout << "position diff:    " << dX << "  " << dY << " " << dZ << std::endl;
  std::cout << "charge h,c:       " << charge << "  " << chargeCluster << std::endl;
#endif
  if (dZ < 0)
    dZ *= -1;

  Double_t dR          = sqrt( dX*dX + dY*dY );
  Double_t dHorizontal = TMath::Cos(phi)*dX + TMath::Sin(phi)*dY;
  Double_t dVertical   = sqrt( dR * dR - dHorizontal * dHorizontal );

  Double_t sigmaR          = sqrt( xSigma * xSigma + ySigma * ySigma );
  Double_t sigmaHorizontal = TMath::Cos(phi) * xSigma + TMath::Sin(phi) * ySigma;
  Double_t sigmaVertical   = sqrt( sigmaR * sigmaR - sigmaHorizontal * sigmaHorizontal);

  //Double_t chargeHorizontal = chargeCluster/(2*sqrt(TMath::Pi())*sigmaHorizontal);

#ifdef DEBUG
  std::cout << "horizontal diff:  " << dHorizontal << std::endl;
  std::cout << "horizontal sigma: " << sigmaHorizontal << std::endl;
  //std::cout << "horizontal chrg:  " << chargeHorizontal << std::endl;
  std::cout << "vertical diff:    " << dVertical   << std::endl;
  std::cout << "vertical sigma:   " << sigmaVertical   << std::endl;
  //std::cout << "charge sig:       charge*TMath::Gaus(d,0,sigma,kTRUE) " << std::endl;
  //std::cout << "charge sig hori:  " << chargeHorizontal*TMath::Gaus(dHorizontal,0,sigmaHorizontal,kTRUE) << std::endl;
  std::cout << std::endl;
#endif
 


#ifdef DEBUG
  std::cout << ">>> dZ CUT:" << std::endl;
  std::cout << "    " << dZ << " < " << fZCut << " ? " << std::endl;
#endif
  if (dZ > fZCut)
    return kFALSE;
#ifdef DEBUG
  std::cout << "    PASSED!" << std::endl;
#endif

  Double_t sigmaConstant = 1.0;

#ifdef DEBUG
  std::cout << ">>> dHorizontal CUT:" << std::endl;
  std::cout << "    " << dHorizontal - sigmaConstant*sigmaHorizontal << " < " << fHorizontalCut << " ? " << std::endl;
  //std::cout << "    " << charge << " < " << 2*chargeHorizontal*TMath::Gaus(dHorizontal,0,sigmaHorizontal,kTRUE) << " ? " << std::endl;
#endif
  if (dHorizontal + sigmaConstant*sigmaHorizontal > fHorizontalCut)
      //charge > 2*chargeHorizontal*TMath::Gaus(dHorizontal,0,sigmaHorizontal,kTRUE))
    return kFALSE;
#ifdef DEBUG
  std::cout << "    PASSED!" << std::endl;
#endif

#ifdef DEBUG
  std::cout << ">>> dVertical CUT:" << std::endl;
  std::cout << "    " << dVertical - sigmaConstant*sigmaVertical << " < " << fVerticalCut << " ? " << std::endl;
#endif
  if (dVertical + sigmaConstant*sigmaVertical > fVerticalCut)
    return kFALSE;
#ifdef DEBUG
  std::cout << "    PASSED!" << std::endl;

  std::cout << std::endl;
  std::cout << "Merging hit and cluster!" << std::endl;
#endif




  fClusterTemp -> SetPosition(cluster -> GetPosition());
  fClusterTemp -> SetPosSigma(cluster -> GetPosSigma());
  fClusterTemp -> SetCovMatrix(cluster -> GetCovMatrix());
  fClusterTemp -> SetCharge(chargeCluster);

  fClusterTemp -> AddHit(hit);

  if ( (fClusterTemp -> GetPosSigma()).X() > fSigmaXCut )
    return kFALSE;
  if ( (fClusterTemp -> GetPosSigma()).Y() > fSigmaYCut)
    return kFALSE;

#ifdef DEBUG
  std::cout << ">>> merged sigma cut passed!" << std::endl;
#endif





  return kTRUE;
}


void STClusterizerScan::AddClusterToEvent(STEvent* eventOut, STHitCluster* cluster)
{
  Int_t clusterID = eventOut -> GetNumClusters();
  cluster -> SetClusterID(clusterID);
#ifdef DEBUG
  fGraphFinishedCluster -> SetPoint(clusterID, (cluster -> GetPosition()).Z(), (cluster -> GetPosition()).X() );
  fGraphFinishedCluster -> SetPointError(clusterID, (cluster -> GetPosSigma()).Z(), (cluster -> GetPosSigma()).X() );
#endif
  cluster -> SetPosition(cluster -> GetPosition() + fPrimaryVertex);
  eventOut -> AddCluster(cluster);
}
