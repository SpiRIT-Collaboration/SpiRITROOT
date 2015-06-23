#include "STClusterizerScan.hh"

#include "TMath.h"

STClusterizerScan::STClusterizerScan()
{
  fHitArray = new std::vector<STHit *>;
  fHitArray -> reserve(5000);
  fHitClusterArray = new std::vector<STHitCluster *>;
  fHitClusterArray -> reserve(100);

  fYTb = fTBTime*fDriftVelocity/100.;

  fPrimaryVertex = TVector3(0., -213.3, -35.2);

  SetYCutTbUnit(5);
  SetPerpCutPadUnit(3,1.2);
}

STClusterizerScan::~STClusterizerScan()
{
}

void STClusterizerScan::SetPrimaryVertex(TVector3 vertex) { fPrimaryVertex = vertex; }

void STClusterizerScan::SetYCut(Double_t yCut)        { fYCut = yCut; }
void STClusterizerScan::SetYCutTbUnit(Double_t tbCut) { fYCut = tbCut*fYTb; }
void STClusterizerScan::SetPerpCut(Double_t perpCut)  { fPerpCut = perpCut; }
void STClusterizerScan::SetPerpCutPadUnit(Double_t xCut, Double_t zCut)
{
  fPerpCut = sqrt(fPadSizeX*xCut*fPadSizeX*xCut + fPadSizeZ*zCut*fPadSizeZ*zCut);
  //fPerpCut = fPadSizeZ*zCut;
  fXCut = fPadSizeX*xCut;
  fZCut = fPadSizeZ*zCut;
}

void 
STClusterizerScan::Analyze(STEvent* eventH, STEvent* eventHC)
{
  fRCut = sqrt(fYCut*fYCut + fPerpCut*fPerpCut);

  fHitArray -> clear();
  fHitClusterArray -> clear();


  Int_t nHits = eventH -> GetNumHits();
  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventH -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() - fPrimaryVertex);
    fHitArray -> push_back(hit);
  }


  STHitSortR sortR;
  std::sort(fHitArray -> begin(), fHitArray -> end(), sortR);


  Int_t nClusters = 0;

  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = fHitArray -> back();
    fHitArray -> pop_back();


    // store clusters out of reach
    nClusters = fHitClusterArray -> size();
    Double_t rHit = (hit -> GetPosition()).Mag();
    for(Int_t iCluster=nClusters-1; iCluster>=0; iCluster--)
    {
      STHitCluster *cluster = fHitClusterArray -> at(iCluster);
      
      Double_t rCluster = (cluster -> GetPosition()).Mag();
      if( rCluster - rHit > fRCut ) 
      {
        AddClusterToEvent(eventHC, cluster);
        fHitClusterArray -> erase(fHitClusterArray -> begin() + iCluster);
      }
    }
    

    // correlate hit with clusters 
    Bool_t addHitToCluster = kFALSE;
    nClusters = fHitClusterArray -> size();
    for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      STHitCluster *cluster = fHitClusterArray -> at(iCluster);

      addHitToCluster = CorrelateHC(hit, cluster);
      if (addHitToCluster) 
      {
        cluster -> AddHit(hit);
        break;
      }
    }


    // make new cluster if hit is not added to any clusters 
    // or there are no clusters in fHitClusterArray
    if (!addHitToCluster)
    {
      STHitCluster* cluster = new STHitCluster();
      cluster -> AddHit(hit);
      fHitClusterArray -> push_back(cluster);
    }
  }

  nClusters = fHitClusterArray -> size();
  for(Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitCluster *cluster = fHitClusterArray -> at(iCluster);
    AddClusterToEvent(eventHC, cluster);
  }

  for(Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventH -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() + fPrimaryVertex);
  }
}

Bool_t 
STClusterizerScan::CorrelateHC(STHit* hit, STHitCluster* cluster)
{
  Double_t zHit = (hit -> GetPosition()).Z(); 
  Double_t zCluster = (cluster -> GetPosition()).Z(); 

  Double_t dZ = zHit - zCluster; 
  if (dZ < 0) dZ *= -1;

  if (dZ > fZCut) 
    return kFALSE;


  Double_t xHit = (hit -> GetPosition()).X(); 
  Double_t xCluster = (cluster -> GetPosition()).X(); 

  Double_t dX = xHit - xCluster; 
  if (dX < 0) dX *= -1;

  if (dX > fXCut) 
    return kFALSE;


  Double_t yHit = (hit -> GetPosition()).Y();
  Double_t yCluster = (cluster -> GetPosition()).Y();

  Double_t dY = yHit - yCluster;
  if (dY < 0) dY *= -1;

  // first correlation : dY
  if (dY > fYCut) 
    return kFALSE;


  /*
  Double_t xHit = (hit -> GetPosition()).X(); 
  Double_t zHit = (hit -> GetPosition()).Z();
  Double_t xCluster = (cluster -> GetPosition()).X(); 
  Double_t zCluster = (cluster -> GetPosition()).Z();

  Double_t dPerp = sqrt( (xHit-xCluster)*(xHit-xCluster) 
                        +(zHit-zCluster)*(zHit-zCluster) );

  // second correlation : dPerp
  if (dPerp > fPerpCut) 
    return kFALSE;
    */



  return kTRUE;
}

void STClusterizerScan::SetAngle(STHit* hit)
{
  TVector3 pos = hit -> GetPosition();
}

void STClusterizerScan::AddClusterToEvent(STEvent* eventHC, STHitCluster* cluster)
{
  Int_t clusterID = eventHC -> GetNumClusters();
  cluster -> SetClusterID(clusterID);
  cluster -> SetPosition(cluster -> GetPosition() + fPrimaryVertex);
  eventHC -> AddCluster(cluster);
}
