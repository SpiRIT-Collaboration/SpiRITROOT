void calculateEnergyLoss(TString   mcFileNameTag = "test",
                         Double_t  &dEdxVal,
                         Double_t  &travelDistanceVal)
{
  TString mcFileNameHead = "data/spirit_";
  TString mcFileNameTail = ".mc.root";
  TString mcFileName     = mcFileNameHead + mcFileNameTag + mcFileNameTail;

  Double_t zLength;
  Double_t zLengthX;
  Double_t energyLoss;
  Double_t energyLossSum;
  Double_t energyLossX; 

  STMCPoint* point; 
  TClonesArray *pointArray;

  TFile* file = new TFile(mcFileName.Data(),"READ");
  TTree* tree = (TTree*) file -> Get("cbmsim");
         tree -> SetBranchAddress("STMCPoint", &pointArray);

  Int_t nEvents = tree -> GetEntriesFast();
  for(Int_t iEvent=0; iEvent<nEvents; iEvent++) {
    tree -> GetEntry(iEvent);

    energyLossSum=0;
    Int_t n = pointArray -> GetEntries();
    for(Int_t i=0; i<n; i++) {
      point = (STMCPoint*) pointArray -> At(i);
      if((point -> GetTrackID())!= 0) continue;

      energyLoss = ( point -> GetEnergyLoss() )*1000; // [GeV] to [MeV]
      energyLossSum += energyLoss; 
      zLength = point -> GetLength()*10; // [cm] to [mm]
    }
    if(zLength==0) continue;
    energyLossX = ((Double_t)iEvent/(iEvent+1))*energyLossX + ((energyLossSum/zLength)/(iEvent+1));
    zLengthX = ((Double_t)iEvent/(iEvent+1))*zLengthX + (zLength/(iEvent+1));
    //cout << iEvent << " / " << nEvents << " : " 
    //     << energyLossX << "(+" << energyLossSum/zLength << "),\t" 
    //     << zLengthX    << "(+" << zLength       << ")" << endl;
  }

  dEdxVal           = energyLossX;
  travelDistanceVal = zLengthX;

  //cout << mcFileNameTag << "\t" << energyLossX << "\t" << zLengthX << endl;
}
