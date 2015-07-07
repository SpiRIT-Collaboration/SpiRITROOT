#include "STClusterizerScan2.hh"
#include <iostream>

STClusterizerScan2::STClusterizerScan2()
{
  fHitArray = new std::vector<STHit *>;
  fHitArray -> reserve(5000);
  fHitClusterFinalArray = new std::vector<STHitClusterRich *>;
  fHitClusterFinalArray -> reserve(100);
  fHitClusterTempArray = new std::vector<STHitClusterRich *>;
  fHitClusterTempArray -> reserve(20);

  fPrimaryVertex = TVector3(0., -213.3, -35.2);

  fClusterTemp = new STHitClusterRich();

  fXCut = fPadSizeX*1.5;
  fYCut = fYTb*2.1;
  //fZCut = fPadSizeZ*1.2;
  fZCut = fPadSizeZ*0.5;

  fSigmaXCut = fPadSizeX*1.8;
  fSigmaYCut = fYTb*1.8;
  fSigmaZCut = fPadSizeZ*0.8;
}

STClusterizerScan2::~STClusterizerScan2()
{
}

void STClusterizerScan2::SetPrimaryVertex(TVector3 vertex) { fPrimaryVertex = vertex; }

void STClusterizerScan2::SetParameters(Double_t *par)
{
}

void 
STClusterizerScan2::Analyze(STEvent* eventH, STEvent* eventHC)
{
  // Reset arrays
  fHitArray -> clear();
  fHitClusterFinalArray -> clear();
  fHitClusterTempArray -> clear();



  // Store arry and sort in R.
  Int_t nHits = eventH -> GetNumHits();
  for (Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventH -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() - fPrimaryVertex);
    fHitArray -> push_back(hit);
  }
  STHitSortR2 sortR;
  std::sort(fHitArray -> begin(), fHitArray -> end(), sortR);



  Double_t rSizeTempArray = fZCut*4;
  Double_t rCurrentTempArray = (fHitArray -> back() -> GetPosition()).Mag();



  // Loop through hits
  for (Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = fHitArray -> back();
    fHitArray -> pop_back();

    // Merge and move clusters in temp array to final array
    if (rCurrentTempArray - rSizeTempArray > (hit -> GetPosition()).Mag())
    {
      rCurrentTempArray -= rSizeTempArray/2.;

      Int_t nClusters = fHitClusterTempArray -> size();
      for (Int_t iCluster=nClusters-1; iCluster>=0; iCluster--)
      {
        STHitClusterRich *cluster1 = fHitClusterTempArray -> at(iCluster);
        for (Int_t jCluster=iCluster-1; jCluster>=0; jCluster--)
        {
          STHitClusterRich *cluster2 = fHitClusterTempArray -> at(jCluster);
          Bool_t mergeCC = CorrelateCC(eventH, cluster1, cluster2);
          if (mergeCC) 
          {
            MergeCC(eventH, cluster1, cluster2);
            fHitClusterTempArray -> erase(fHitClusterTempArray -> begin() + iCluster);
            break;
          }
        }
      }

      nClusters = fHitClusterTempArray -> size();
      for (Int_t iCluster=nClusters-1; iCluster>=0; iCluster--)
      {
        STHitClusterRich *cluster = fHitClusterTempArray -> at(iCluster);
        if ((cluster -> GetPosition()).Mag() > rCurrentTempArray)
        {
          fHitClusterFinalArray -> push_back(cluster);
          fHitClusterTempArray -> erase(fHitClusterTempArray -> begin() + iCluster);
        }
      }
    }


    // correlate hit with clusters 
    Bool_t mergeHC = kFALSE;
    Int_t nClusters = fHitClusterTempArray -> size();
    for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
    {
      STHitClusterRich *cluster = fHitClusterTempArray -> at(iCluster);

      mergeHC = CorrelateHC(hit, cluster);
      if (mergeHC) 
      {
        //std::cout << "MERGE" << std::endl;
        cluster -> AddHit(hit);
        break;
      }
    }


    // make new cluster if hit is not added to any clusters 
    // or there are no clusters in fHitClusterTempArray
    if (!mergeHC)
    {
      //std::cout << "NEW CLUSTER" << std::endl;
      STHitClusterRich* cluster = new STHitClusterRich();
      cluster -> AddHit(hit);
      fHitClusterTempArray -> push_back(cluster);
    }
  }



  Int_t nClusters = fHitClusterTempArray -> size();
  for (Int_t iCluster=nClusters-1; iCluster>=0; iCluster--)
  {
    STHitClusterRich *cluster1 = fHitClusterTempArray -> at(iCluster);
    for (Int_t jCluster=iCluster-1; jCluster>=0; jCluster--)
    {
      STHitClusterRich *cluster2 = fHitClusterTempArray -> at(jCluster);
      Bool_t mergeCC = CorrelateCC(eventH, cluster1, cluster2);
      if (mergeCC) 
      {
        MergeCC(eventH, cluster1, cluster2);
        fHitClusterTempArray -> erase(fHitClusterTempArray -> begin() + iCluster);
        break;
      }
    }
  }



  // Push left over clusters in to Final cluster array
  nClusters = fHitClusterTempArray -> size();
  for (Int_t iCluster=0; iCluster<nClusters; iCluster++)
  {
    STHitClusterRich *cluster = fHitClusterTempArray -> at(iCluster);
    fHitClusterFinalArray -> push_back(cluster);
  }




  // Add Final cluster in to event;
  Int_t nClustersFinal  = fHitClusterFinalArray -> size(); 
  for (Int_t iClusterFinal=0; iClusterFinal<nClustersFinal; iClusterFinal++)
  {
    STHitClusterRich *clusterFinal = fHitClusterFinalArray -> at(iClusterFinal);
    AddClusterToEvent(eventHC, clusterFinal);
  }




  // Reset hits back to it's original position
  for (Int_t iHit=0; iHit<nHits; iHit++) 
  {
    STHit *hit = (STHit *) eventH -> GetHit(iHit);
    hit -> SetPosition(hit -> GetPosition() + fPrimaryVertex);
  }
}

Bool_t 
STClusterizerScan2::CorrelateCC(STEvent* eventH,
                                STHitClusterRich* cluster1, 
                                STHitClusterRich* cluster2)
{
  Double_t xMax1 = cluster1 -> GetXMax();
  Double_t yMax1 = cluster1 -> GetYMax();
  Double_t zMax1 = cluster1 -> GetZMax();
  Double_t xMin1 = cluster1 -> GetXMin();
  Double_t yMin1 = cluster1 -> GetYMin();
  Double_t zMin1 = cluster1 -> GetZMin();

  Double_t xMax2 = cluster2 -> GetXMax();
  Double_t yMax2 = cluster2 -> GetYMax();
  Double_t zMax2 = cluster2 -> GetZMax();
  Double_t xMin2 = cluster2 -> GetXMin();
  Double_t yMin2 = cluster2 -> GetYMin();
  Double_t zMin2 = cluster2 -> GetZMin();

  Double_t dZ1 = TMath::Abs(zMin1 - zMax2);
  Double_t dZ2 = TMath::Abs(zMax1 - zMin2);
  Double_t dZ  = (dZ1 < dZ2) ? dZ1 : dZ2;
  //std::cout << "merge-z: " << dZ1 << " " << dZ2  << " : " << dZ << " <? " << fZCut << std::endl;
  if (dZ > fZCut)
    return kFALSE;

  Double_t dY1 = TMath::Abs(yMin1 - yMax2);
  Double_t dY2 = TMath::Abs(yMax1 - yMin2);
  Double_t dY  = (dY1 < dY2) ? dY1 : dY2;
  //std::cout << "merge-y: " << dY1 << " " << dY2  << " : " << dY << " <? " << fYCut << std::endl;
  if (dY > fYCut)
    return kFALSE;

  Double_t dX1 = TMath::Abs(xMin1 - xMax2);
  Double_t dX2 = TMath::Abs(xMax1 - xMin2);
  Double_t dX  = (dX1 < dX2) ? dX1 : dX2;
  //std::cout << "merge-x: " << dX1 << " " << dX2  << " : " << dX << " <? " << fXCut << std::endl;
  if (dX > fXCut)
    return kFALSE;


  fClusterTemp -> SetPosition(cluster2 -> GetPosition());
  fClusterTemp -> SetPosSigma(cluster2 -> GetPosSigma());
  fClusterTemp -> SetCovMatrix(cluster2 -> GetCovMatrix());
  fClusterTemp -> SetCharge(cluster2 -> GetCharge());

  std::vector<Int_t> *hitIDs = cluster1 -> GetHitIDs();
  Int_t nHits = hitIDs -> size();
  for (Int_t iHit=0; iHit<nHits; iHit++)
  {
    Int_t hitID = hitIDs -> at(iHit);
    STHit *hit = (STHit *) eventH -> GetHit(hitID);
    fClusterTemp -> AddHit(hit);
  }

  if ( (fClusterTemp -> GetPosSigma()).X() > fSigmaXCut )
    return kFALSE;
  if ( (fClusterTemp -> GetPosSigma()).Y() > fSigmaYCut)
    return kFALSE;
  if ( (fClusterTemp -> GetPosSigma()).Z() > fSigmaZCut)
    return kFALSE;


  return kTRUE;
}

Bool_t 
STClusterizerScan2::CorrelateHC(STHit* hit, STHitClusterRich* cluster)
{
  Double_t xHit = (hit -> GetPosition()).X();
  Double_t yHit = (hit -> GetPosition()).Y();
  Double_t zHit = (hit -> GetPosition()).Z();

  Double_t xSigma = (cluster -> GetPosSigma()).X();
  Double_t ySigma = (cluster -> GetPosSigma()).Y();

  Double_t xMax = cluster -> GetXMax();
  Double_t yMax = cluster -> GetYMax();
  Double_t zMax = cluster -> GetZMax();
  Double_t xMin = cluster -> GetXMin();
  Double_t yMin = cluster -> GetYMin();
  Double_t zMin = cluster -> GetZMin();

  Double_t charge = hit -> GetCharge();
  Double_t chargeCluster = cluster -> GetCharge();



  Double_t dZ1 = TMath::Abs(zHit - zMax);
  Double_t dZ2 = TMath::Abs(zHit - zMin);
  Double_t dZ  = (dZ1 < dZ2) ? dZ1 : dZ2;
  //std::cout << "z: " << zHit << " " << zMax << " " << zMin << " : " << dZ << " <? " << fZCut << std::endl;
  if (dZ > fZCut)
    return kFALSE;

  Double_t dY1 = TMath::Abs(yHit - yMax);
  Double_t dY2 = TMath::Abs(yHit - yMin);
  Double_t dY  = (dY1 < dY2) ? dY1 : dY2;
  //std::cout << "y: " << yHit << " " << yMax << " " << yMin << " : " << dY << " <? " << fYCut << std::endl;
  if (dY > fYCut)
    return kFALSE;

  Double_t dX1 = TMath::Abs(xHit - xMax);
  Double_t dX2 = TMath::Abs(xHit - xMin);
  Double_t dX  = (dX1 < dX2) ? dX1 : dX2;
  //std::cout << "x: " << xHit << " " << xMax << " " << xMin << " : " << dX << " <? " << fXCut << std::endl;
  if (dX > fXCut)
    return kFALSE;



  fClusterTemp -> SetPosition(cluster -> GetPosition());
  fClusterTemp -> SetPosSigma(cluster -> GetPosSigma());
  fClusterTemp -> SetCovMatrix(cluster -> GetCovMatrix());
  fClusterTemp -> SetCharge(cluster -> GetCharge());

  fClusterTemp -> AddHit(hit);

  //std::cout << (fClusterTemp -> GetPosSigma()).X() << " " << fSigmaXCut << std::endl;
  //std::cout << (fClusterTemp -> GetPosSigma()).y() << " " << fSigmaYCut << std::endl;
  if ( (fClusterTemp -> GetPosSigma()).X() > fSigmaXCut )
    return kFALSE;
  if ( (fClusterTemp -> GetPosSigma()).Y() > fSigmaYCut)
    return kFALSE;
  if ( (fClusterTemp -> GetPosSigma()).Z() > fSigmaZCut)
    return kFALSE;



  return kTRUE;
}


void STClusterizerScan2::MergeCC(STEvent* eventH,
                                 STHitClusterRich* cluster1,
                                 STHitClusterRich* cluster2)
{
  std::vector<Int_t> *hitIDs = cluster1 -> GetHitIDs();
  Int_t nHits = hitIDs -> size();
  for (Int_t iHit=0; iHit<nHits; iHit++)
  {
    Int_t hitID = hitIDs -> at(iHit);
    STHit *hit = (STHit *) eventH -> GetHit(hitID);
    cluster2 -> AddHit(hit);
  }
}

void STClusterizerScan2::AddClusterToEvent(STEvent* eventHC, STHitClusterRich* clusterRich)
{
  STHitCluster* cluster = new STHitCluster();

  cluster -> SetPosition(clusterRich -> GetPosition()+ fPrimaryVertex);
  cluster -> SetPosSigma(clusterRich -> GetPosSigma());
  cluster -> SetCovMatrix(clusterRich -> GetCovMatrix());
  cluster -> SetCharge(clusterRich -> GetCharge());
  cluster -> SetClusterID(eventHC -> GetNumClusters());

  eventHC -> AddCluster(cluster);
}
