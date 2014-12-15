void calculateEnergyLoss(TString   mcFileNameTag = "test",
                         Double_t  &dEdxVal,
                         Double_t  &travelDistanceVal)
{
  TString mcFileNameHead = "data/spirit_";
  TString mcFileNameTail = ".mc.root";
  TString mcFileName     = mcFileNameHead + mcFileNameTag + mcFileNameTail;

  cout << "[calculateEnergyLoss ] Calculating " << mcFileName << endl;

  Double_t zLength;        // traveled length [cm]
  Double_t zLengthMean;    // traveled length [cm] - mean value
  Double_t energyLoss;     // energy loss
  Double_t energyLossSum;  // energy loss sum
  Double_t energyLossMean; // energy loss - mean value

  STMCPoint* point; 
  TClonesArray *pointArray;

  TFile* file = new TFile(mcFileName.Data(),"READ");
  TTree* tree = (TTree*) file -> Get("cbmsim");
         tree -> SetBranchAddress("STMCPoint", &pointArray);

  cout << " - " << "i "<< "zLength[mm]  " << "energyLossSum[MeV]" << endl;
  Int_t nEvents = tree -> GetEntriesFast();
  for(Int_t iEvent=0; iEvent<nEvents; iEvent++) 
  {
    tree -> GetEntry(iEvent);

    energyLossSum=0;
    Int_t n = pointArray -> GetEntries();
    for(Int_t i=0; i<n; i++) {
      point = (STMCPoint*) pointArray -> At(i);
      if((point -> GetTrackID())!= 0) continue;

      energyLoss = ( point -> GetEnergyLoss() )*1000; // [GeV] to [MeV]
      energyLossSum += energyLoss; 
      if(zLength<(point->GetLength()*10)) zLength = point -> GetLength()*10; // [cm] to [mm]
    }
    cout << "   " << iEvent << " " << zLength << " " << energyLossSum << endl;

    if(zLength==0) continue;
    energyLossMean = ( (Double_t)iEvent/(iEvent+1) * energyLossMean )  +  ((energyLossSum/zLength)/(iEvent+1));
    zLengthMean    = ( (Double_t)iEvent/(iEvent+1) *    zLengthMean )  +  (               zLength /(iEvent+1));
  }

  dEdxVal           = energyLossMean;
  travelDistanceVal = zLengthMean;
}
