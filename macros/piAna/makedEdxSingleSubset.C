auto mpi = 139.57018;
auto me = 0.5109989461;
auto mp = 938.2720813;
auto mn = 939.565346;
auto md = 1875.612762;
auto mt = 2808.921112;
auto mhe3 = 2808.39132;
auto mal = 3727.379378;

const Double_t particleMass = mpi; // Select particle you'll going to analyze
const Bool_t useReco = kFALSE;
const Char_t *postfix = "pion";   // change this accordingly
const Char_t *sliceFit = "sliceFit.txt";

Double_t momRange[300][2] = {{0}};
Double_t gausParam[300][9] = {{0}};
Int_t numSlices = 0;

void ReadFile(TString filename) {
  ifstream inFile(filename.Data());
  if (!inFile.is_open()) {
    cerr << "Cannot find the file! - " << filename.Data() << endl;

    exit(0);
  }

  Int_t momIndex = 0;
  Int_t numTotalGaus = 3;

  Double_t dummy;
  while (1) { 
    inFile >> dummy >> momRange[momIndex][0] >> momRange[momIndex][1] >> dummy;
    for (auto iGaus = 0; iGaus < numTotalGaus; iGaus++)
      inFile >> gausParam[momIndex][iGaus*numTotalGaus + 0] >> gausParam[momIndex][iGaus*numTotalGaus + 1] >> gausParam[momIndex][iGaus*numTotalGaus + 2];

    if (inFile.eof())
      break;

    momIndex++;
  }

  numSlices = momIndex;

  inFile.close();
}

Int_t SetParameters(Double_t mom, Double_t dedx, TF1 *totalFunc, TF1 *piFunc, TF1 *bgFunc) {
  Int_t momIndex = 0;
  for (momIndex = 0; momIndex < numSlices; momIndex++)
    if (momRange[momIndex][1] >= mom && momRange[momIndex][0] < mom)
      break;

  if (momIndex >= numSlices) {
    totalFunc -> SetParameters(0, 0, 0, 0, 0, 0, 0, 0, 0);
    piFunc -> SetParameters(0, 0, 0, 0, 0, 0);
    bgFunc -> SetParameters(0, 0, 0);

    return 0;
  } else {
    Double_t dedxMin = std::min(gausParam[momIndex][1] - 5*gausParam[momIndex][2], gausParam[momIndex][7] - 2*gausParam[momIndex][8]);
    Double_t dedxMax = std::max(gausParam[momIndex][1] + 5*gausParam[momIndex][2], gausParam[momIndex][7] + 2*gausParam[momIndex][8]);

    if (dedxMax < dedx)
      return 0;

    if (dedxMin > dedx)
      return 2;

    totalFunc -> SetParameters(gausParam[momIndex][0], gausParam[momIndex][1], gausParam[momIndex][2],
                               gausParam[momIndex][3], gausParam[momIndex][4], gausParam[momIndex][5],
                               gausParam[momIndex][6], gausParam[momIndex][7], gausParam[momIndex][8]);
    piFunc -> SetParameters(gausParam[momIndex][0], gausParam[momIndex][1], gausParam[momIndex][2],
                            gausParam[momIndex][6], gausParam[momIndex][7], gausParam[momIndex][8]);
    bgFunc -> SetParameters(gausParam[momIndex][3], gausParam[momIndex][4], gausParam[momIndex][5]);

    return 1;
  }
}

Double_t fpFunc(Double_t *x, Double_t *p) {
  if (x[0] < 0)
    x[0] = -x[0];

  const Double_t m  = particleMass;
  Double_t totalE = TMath::Sqrt(m*m + x[0]*x[0]);
  Double_t beta = x[0]/totalE;
  Double_t gamma = 1./TMath::Sqrt(1 - beta*beta);

  Double_t betaP3 = TMath::Power(beta, p[3]);

  return p[0]/betaP3*(p[1] - betaP3 + TMath::Log(p[2] + TMath::Power(1./(beta*gamma), p[4])));
}

void makedEdxSingleSubset(Int_t set) {
  gStyle -> SetOptStat(0);
  gStyle -> SetPalette(55);
  gStyle -> SetNumberContours(100);
  gStyle -> SetPadRightMargin(0.12);
  gStyle -> SetPadTopMargin(0.05);
  gStyle -> SetPadBottomMargin(0.15);
  gStyle -> SetPadLeftMargin(0.13);
  gStyle -> SetTitleOffset(1.15, "x");
  gStyle -> SetTitleOffset(1.05, "y");
  gStyle -> SetTitleSize(0.06, "x");
  gStyle -> SetTitleSize(0.06, "y");
  gStyle -> SetTitleSize(0.06, "z");
  gStyle -> SetLabelSize(0.06, "x");
  gStyle -> SetLabelSize(0.06, "y");
  gStyle -> SetLabelSize(0.06, "z");
  gStyle -> SetLegendTextSize(0.06);
  gStyle -> SetOptLogz();

  /*
  auto eGrCut = new TCutG();
  eGrCut -> SetName("eGrCut");
  eGrCut -> SetPoint(0, -136.263, -0.364003);
  eGrCut -> SetPoint(1, -91.5627, -0.390111);
  eGrCut -> SetPoint(2, -64.5433, -0.833861);
  eGrCut -> SetPoint(3, 5.16656, -0.477136);
  eGrCut -> SetPoint(4, 96.7663, -0.268275);
  eGrCut -> SetPoint(5, 143.665, -0.337896);
  eGrCut -> SetPoint(6, 143.665, -10);
  eGrCut -> SetPoint(7, -149.454, -10);
  eGrCut -> SetPoint(8, -156.311, -0.723101);
  eGrCut -> SetPoint(9, -136.263, -0.364003);
  */

  auto time = new TDatime();

  Double_t pitchBinSize = 20;
  Double_t yawBinSize = 180;

  Int_t numPitch = 200/pitchBinSize;
  Int_t numPhi = 360/yawBinSize;
  Int_t numHists = numPitch*numPhi;

  auto grECutFile = new TFile("grECut.root");
  auto eGrCut = (TCutG *) grECutFile -> Get("grE");

  auto grPiCutFile = new TFile("pidFitCut_pion.root");
  auto grPiCut = new TCutG*[numHists];
  for (auto iHist = 0; iHist < numHists; iHist++)
    grPiCut[iHist] = (TCutG *) grPiCutFile -> Get(Form("pidCut_%d", iHist));

  auto fitFunc = new TF1("fiveParameterFitFunction", fpFunc, 0., 700., 5, 1); // GetLV("fitFunc", -700, 0);

  Bool_t sigma10, sigma15, sigma20, sigma20z, sigma25, sigma30, ontarget, ontarget20, goodproj;// rough;
  Int_t eventid, nrc, nlc, vaCharge, evestat, recoCharge, g4nc;
  Double_t dedx, dedxs05, recodist, vadist, recoypicm, vaypicm, ybeamcm, beta, projx, projy, projz;
  TVector3 *recoMom = nullptr;
  TVector3 *recoMomAC = nullptr;
  TVector3 *recoMomTarget = nullptr;
  TVector3 *recoMomTargetAC = nullptr;
  TVector3 *vaMom = nullptr;
  TVector3 *vaMomAC = nullptr;
  Int_t mult[5], vamult[5];
  auto tree = new TChain("tree");
  tree -> AddFile(Form("dedxROOT/dedxSn132-%d.root", set));
  tree -> SetBranchAddress("eventid", &eventid);
  tree -> SetBranchAddress("evestat", &evestat);
  //  tree -> SetBranchAddress("rough", &rough);
  tree -> SetBranchAddress("sigma10", &sigma10);
  tree -> SetBranchAddress("sigma15", &sigma15);
  tree -> SetBranchAddress("sigma20", &sigma20);
  tree -> SetBranchAddress("sigma20z", &sigma20z);
  tree -> SetBranchAddress("sigma25", &sigma25);
  tree -> SetBranchAddress("sigma30", &sigma30);
  tree -> SetBranchAddress("ontarget", &ontarget);
  tree -> SetBranchAddress("ontarget20", &ontarget20);
  tree -> SetBranchAddress("goodproj", &goodproj);
  tree -> SetBranchAddress("nlc", &nlc);
  tree -> SetBranchAddress("nrc", &nrc);
  tree -> SetBranchAddress("g4nc", &g4nc);
  tree -> SetBranchAddress("dedx", &dedx);
  tree -> SetBranchAddress("dedxs05", &dedxs05);
  tree -> SetBranchAddress("recoMom", &recoMom);
  tree -> SetBranchAddress("recoMomAC", &recoMomAC);
  tree -> SetBranchAddress("recoMomTarget", &recoMomTarget);
  tree -> SetBranchAddress("recoMomTargetAC", &recoMomTargetAC);
  tree -> SetBranchAddress("recoCharge", &recoCharge);
  tree -> SetBranchAddress("vaMom", &vaMom);
  tree -> SetBranchAddress("vaMomAC", &vaMomAC);
  tree -> SetBranchAddress("vaCharge", &vaCharge);
  tree -> SetBranchAddress("recodist", &recodist);
  tree -> SetBranchAddress("vadist", &vadist);
  tree -> SetBranchAddress("ybeamcm", &ybeamcm);
  tree -> SetBranchAddress("beta", &beta);
  tree -> SetBranchAddress("mult", &mult);
  tree -> SetBranchAddress("vamult", &vamult);
  tree -> SetBranchAddress("projx", &projx);
  tree -> SetBranchAddress("projy", &projy);
  tree -> SetBranchAddress("projz", &projz);

  ifstream inFile(Form("fitParam_%s.txt", postfix));
  
  Double_t pidFitParam[36][5] = {{0}};
  if (inFile.is_open()) {
    Int_t index;
    Double_t ptemp[5] = {0};

    while (true) {
      inFile >> index;
      for (auto i = 0; i < 5; i++)
        inFile >> ptemp[i];

      if (inFile.eof())
        break;

      for (auto i = 0; i < 5; i++)
        pidFitParam[index][i] = ptemp[i];
    }

    inFile.close();
  }

  Int_t pitchIndex;
  Bool_t grE, pseudoPi, grPi;
  Double_t flatdedxs05, pitch, recokecme, recokecmpi, vakecme, vakecmpi, ppi, pbg;
  Double_t vaPhiCM, vaThetaCM, recoPhiCM, recoThetaCM;
  TVector3 recoMomACCMe, vaMomACCMe,recoMomACCMpi, vaMomACCMpi;
  auto writeFile = new TFile(Form("dedxROOT/dedxSn132-subset-%d.root", set), "recreate");
  auto writeTree = new TTree("subTree", "");
  writeTree -> Branch("eventid", &eventid);
  writeTree -> Branch("evestat", &evestat);
  //  writeTree -> Branch("rough", &rough);
  writeTree -> Branch("sigma10", &sigma10);
  writeTree -> Branch("sigma15", &sigma15);
  writeTree -> Branch("sigma20", &sigma20);
  writeTree -> Branch("sigma20z", &sigma20z);
  writeTree -> Branch("sigma25", &sigma25);
  writeTree -> Branch("sigma30", &sigma30);
  writeTree -> Branch("ontarget", &ontarget);
  writeTree -> Branch("ontarget20", &ontarget20);
  writeTree -> Branch("goodproj", &goodproj);
  writeTree -> Branch("nlc", &nlc);
  writeTree -> Branch("nrc", &nrc);
  writeTree -> Branch("g4nc", &g4nc);
  writeTree -> Branch("dedx", &dedx);
  writeTree -> Branch("dedxs05", &dedxs05);
  writeTree -> Branch("flatdedxs05", &flatdedxs05);
  writeTree -> Branch("recoMom", &recoMom);
  writeTree -> Branch("recoMomAC", &recoMomAC);
  writeTree -> Branch("recoMomTarget", &recoMomTarget);
  writeTree -> Branch("recoMomTargetAC", &recoMomTargetAC);
  writeTree -> Branch("recoMomACCMpi", &recoMomACCMpi);
  writeTree -> Branch("recoCharge", &recoCharge);
  writeTree -> Branch("recoPhiCM", &recoPhiCM);
  writeTree -> Branch("recoThetaCM", &recoThetaCM);
  writeTree -> Branch("recoMom", &recoMom);
  writeTree -> Branch("vaMomAC", &vaMomAC);
  writeTree -> Branch("vaMomACCMe", &vaMomACCMe);
  writeTree -> Branch("vaMomACCMpi", &vaMomACCMpi);
  writeTree -> Branch("vaPhiCM", &vaPhiCM);
  writeTree -> Branch("vaThetaCM", &vaThetaCM);
  writeTree -> Branch("recokecme", &recokecme);
  writeTree -> Branch("recokecmpi", &recokecmpi);
  writeTree -> Branch("vakecme", &vakecme);
  writeTree -> Branch("vakecmpi", &vakecmpi);
  writeTree -> Branch("vaCharge", &vaCharge);
  writeTree -> Branch("pitch", &pitch);
  writeTree -> Branch("pitchIndex", &pitchIndex);
  writeTree -> Branch("recodist", &recodist);
  writeTree -> Branch("vadist", &vadist);
  writeTree -> Branch("ppi", &ppi);
  writeTree -> Branch("pbg", &pbg);
  writeTree -> Branch("pseudoPi", &pseudoPi);
  writeTree -> Branch("grE", &grE);
  writeTree -> Branch("grPi", &grPi);
  writeTree -> Branch("recoypicm", &recoypicm);
  writeTree -> Branch("vaypicm", &vaypicm);
  writeTree -> Branch("ybeamcm", &ybeamcm);
  writeTree -> Branch("mult", &mult, "mult[5]/I");
  writeTree -> Branch("vamult", &vamult, "vamult[5]/I");
  writeTree -> Branch("projx", &projx);
  writeTree -> Branch("projy", &projy);
  writeTree -> Branch("projz", &projz);

  TF1 *totalFunc = new TF1("totalFunc", "gaus(0)+gaus(3)+gaus(6)", -3., 3.);
  TF1 *piFunc = new TF1("piFunc", "gaus(0)+gaus(3)", -3., 3.);
  TF1 *bgFunc = new TF1("bgFunc", "gaus(0)", -3., 3.);

  totalFunc -> SetNpx(10000);
  piFunc -> SetNpx(10000);
  bgFunc -> SetNpx(10000);

  ReadFile(sliceFit);

  Int_t thisCharge = 1;
  TVector3 *thisMom;
  Double_t thisDist, thisDistCut;
  Int_t numEvents = 0;
  Int_t prevEventID = -1;
  
  auto numEntries = tree -> GetEntries();
  for (auto iEntry = 0; iEntry < numEntries; iEntry++) {
    if (iEntry%1000 == 0)
      cout << "[" << time -> AsString() << "] Event ID: " << setw(6) << iEntry << "/" << numEntries << "(" << Form("%.0f", iEntry/(Double_t)numEntries*100) << "%)" << endl;

    tree -> GetEntry(iEntry);

    if (useReco) {
      thisCharge = recoCharge;
      thisMom = recoMomTargetAC;
      thisDist = recodist;
      thisDistCut = 20;
    } else {
      thisCharge = vaCharge;
      thisMom = vaMomAC;
      thisDist = recodist;
      thisDistCut = 20;
    }

    //    if (!sigma20 && !rough)
    if (!sigma20)
      continue;

    if (!ontarget)
      continue;

    if (evestat == 7)
      continue;

    if (thisDist > thisDistCut)
      continue;

    if (nlc + nrc < 20)
      continue;

    cout<<"mult "<<mult[2]<<endl;

    //    if ( !(mult[2] >= 51) )
    //      continue;

    numEvents++;
//    if ((Double_t)(nlc + nrc)/(Double_t)g4nc < 0.51)
//      continue;

    if (!(thisCharge == 1 || thisCharge == -1))
      continue;

    auto momCharge = thisMom -> Mag()/thisCharge;

    pitch = 90 + TMath::ATan2(recoMom -> Y(), recoMom -> Z())*TMath::RadToDeg();
    if (pitch < 0 || pitch > 180)
      continue;

    auto yaw = 90 + TMath::ATan2(recoMom -> X(), recoMom -> Z())*TMath::RadToDeg();
    if (yaw < 0 || yaw > 180)
      continue;

    Int_t pitchBin = pitch/pitchBinSize;
    Int_t yawBin = yaw/yawBinSize;

    pitchIndex = yawBin*numPitch + pitchBin;

    if (pitchIndex >= 0 && pitchIndex < numHists) {
      for (auto i = 0; i < 5; i++)
        fitFunc -> SetParameter(i, pidFitParam[pitchIndex][i]);

      flatdedxs05 = TMath::Log(dedxs05/fitFunc -> Eval(momCharge));

      TVector3 betaVec(0., 0., beta);

      TLorentzVector loVec;
      loVec.SetXYZM(recoMomTargetAC -> X(), recoMomTargetAC -> Y(), recoMomTargetAC -> Z(), me);
      loVec.Boost(betaVec);
      recoMomACCMe = loVec.Vect();
      recokecme = (loVec.Gamma() - 1)*me;

      loVec.SetXYZM(recoMomTargetAC -> X(), recoMomTargetAC -> Y(), recoMomTargetAC -> Z(), mpi);
      loVec.Boost(betaVec);
      recoMomACCMpi = loVec.Vect();
      recoPhiCM = recoMomACCMpi.Phi()*TMath::RadToDeg();
      if (recoPhiCM < 0) recoPhiCM += 360;
      recoThetaCM = recoMomACCMpi.Theta()*TMath::RadToDeg();
      recokecmpi = (loVec.Gamma() - 1)*mpi;
      recoypicm = loVec.Rapidity();

      loVec.SetXYZM(vaMomAC -> X(), vaMomAC -> Y(), vaMomAC -> Z(), mpi);
      loVec.Boost(betaVec);
      vaMomACCMpi = loVec.Vect();
      vakecmpi = (loVec.Gamma() - 1)*mpi;
      vaypicm = loVec.Rapidity();
      vaPhiCM = vaMomACCMpi.Phi()*TMath::RadToDeg();
      if (vaPhiCM < 0) vaPhiCM += 360;
      vaThetaCM = vaMomACCMpi.Theta()*TMath::RadToDeg();

      Int_t ready = SetParameters(thisMom -> Mag()/thisCharge, flatdedxs05, totalFunc, piFunc, bgFunc);
      grE = eGrCut -> IsInside(thisMom -> Mag()/thisCharge, flatdedxs05);

      Double_t x = thisMom -> Mag()/thisCharge;
      Double_t y = flatdedxs05;
      if ((x > 0 && y > -0.90/662.54*x + 0.90) || (x < 0 && y > 0.918/700.*x + 1.081))
        ready = 0;

      ppi = 0;
      pbg = 0;
      if (ready == 1) {
        ppi = piFunc -> Eval(flatdedxs05)/totalFunc -> Eval(flatdedxs05);
        pbg = bgFunc -> Eval(flatdedxs05)/totalFunc -> Eval(flatdedxs05);
        if (std::isnan(ppi)) ppi = 0;
        if (std::isnan(pbg)) pbg = 0;
      }

      pseudoPi = kFALSE;
      if (ppi > 0 || grE)
        pseudoPi = kTRUE;

      grPi = kFALSE;
      if (grPiCut[pitchIndex])
        grPi = grPiCut[pitchIndex] -> IsInside(thisMom -> Mag(), dedxs05);

      writeTree -> Fill();
    }
  }
  cout<<"Number of Events "<<numEvents<<endl;

  writeFile -> cd();
  writeTree -> Write();
  writeFile->Close();
}
