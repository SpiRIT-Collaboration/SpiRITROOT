void createPionEvent() {
  Int_t pdg = 211;
  Int_t nEvents = 2500;
  Int_t nTracks = 2;
  Double_t rad = TMath::Pi()/180.;

  for (Int_t p : {50,100,150,200,300,400,500}) {

    std::ofstream macro_theta(Form("macro_pion_p%d_theta.sh",p));
    macro_theta << "source ../build/config.sh" << endl;

    for (Int_t theta = -70; theta <= 70; theta+=20) {
      Double_t pMag = p * 0.001;

      TString name = "pion_p"+TString::Itoa(p,10)+"_theta"+TString::Itoa(theta,10);
      TString egen = name+".egen";

      macro_theta << "root -q -l -b 'run_mc.C(\""+name+"\",\""+egen+"\")'; root -q -l -b 'run_digi.C(\""+name+"\")'; root -q -l -b 'run_reco_mc.C(\""+name+"\")'" << endl;
      std::ofstream file(egen);
      file << nEvents << std::endl;

      for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
        file << iEvent << " " << nTracks << endl;
        for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
          Double_t phi = gRandom -> Uniform(-70*rad, 70*rad);
          TVector3 momentum(TMath::Sin(phi), TMath::Cos(phi)*TMath::Sin(theta*rad), TMath::Cos(phi)*TMath::Cos(theta*rad));
          momentum.SetMag(pMag);
          file << pdg << " " << momentum.X() << " " << momentum.Y() << " " << momentum.Z() << std::endl;
        }
      }
    }

    std::ofstream macro_phi(Form("macro_pion_p%d_phi.sh",p));
    macro_phi << "source ../build/config.sh" << endl;

    for (Int_t phi = -70; phi <= 70; phi+=20) {
      Double_t pMag = p * 0.001;

      TString name = "pion_p"+TString::Itoa(p,10)+"_phi"+TString::Itoa(phi,10);
      TString egen = name+".egen";

      macro_phi << "root -q -l -b 'run_mc.C(\""+name+"\",\""+egen+"\")'; root -q -l -b 'run_digi.C(\""+name+"\")'; root -q -l -b 'run_reco_mc.C(\""+name+"\")'" << endl;
      std::ofstream file(egen);
      file << nEvents << std::endl;

      for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
        file << iEvent << " " << nTracks << endl;
        for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
          Double_t theta = gRandom -> Uniform(-70*rad, 70*rad);
          TVector3 momentum(TMath::Sin(phi), TMath::Cos(phi)*TMath::Sin(theta*rad), TMath::Cos(phi)*TMath::Cos(theta*rad));
          momentum.SetMag(pMag);
          file << pdg << " " << momentum.X() << " " << momentum.Y() << " " << momentum.Z() << std::endl;
        }
      }
    }
  }
}
