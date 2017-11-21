#include "AsmFlw_getEvents_v2.h"
//----------------------------------------------------------------------
//   AsmFlw_getEvent.C  succeeded from AsmFlw_getMixing.C
// (c) Mizuki Kurata-Nishimura 
//----------------------------------------
//   Particle selection and mixed events will be done.
//
//----------------------------------------------------------------------
void AsmFlw_getEvents_v2(Long64_t nmax = -1)
{
  SetEnvironment();

  // I/O
  Open();

  OutputTree(nmax);

  SetNumberOfProcess(nmax);

  for(Long64_t ievt = 0; ievt < maxProc; ievt++){

    PrintProcess(ievt);

    Initialize();
    
    Long64_t nTrack = 0;
    if(bMix)
      nTrack = (Long64_t)GetRandomNumTrack(); // get number of track 
    else {
      fTree->GetEntry(ievt);
      if( snbm != 132 && snbm != 108) continue;

      nTrack = aParticleArray->GetEntries();
    }

    TClonesArray &npar = *nParticleArray;

    if(ntrack[2] > 0 || bMix) {
     
      Long64_t nLoop = 0;

      while(kTRUE) {

	if(  bMix && numGoodTrack > nTrack - 1 ) break;  // mGoodTrack-1

	if( !bMix && nLoop >=  nTrack ) break;

	STParticle *aPart1;
	Long64_t    mixEvt = (Long64_t)ievt;
	
	Int_t ntrk = nTrack;

	if( bMix ) {
	  aPart1 = GetMixedTrack(&mixEvt, &ntrk); // Get a track randomly
	}
	else 
	  aPart1 = GetRealTrack(nLoop);


	// Good track and fragments are selected
	if(  CheckPID(aPart1) ){

	  SetPtWeight(aPart1);

	  if( bMix ) {
	    aPart1->SetMixedEventID(mixEvt);
	    event.push_back(mixEvt);
	  }
	  else 
	    event.push_back(ievt);
	  
	  aPart1->SetMixedNtrack(ntrk);

	  trackID.push_back(numGoodTrack);

	  new(npar[numGoodTrack]) STParticle( *aPart1 );	  

	  numGoodTrack++;
	}

	nLoop++;
      }
    }

    mtrack = trackID.size();
    ntrack[3] = mtrack;

    //    cout << " mtrack " << mtrack << endl;

    if(mtrack > 0) {
      mflw->Fill();
      hgtc->Fill(mtrack);

    }
  }


  if(!bMix && mhfile != NULL) {
    mhfile->cd();
    hgtc->Write();
  }


  fout->cd();
  fout->Write();
  std::cout << fout->GetName() << std::endl;

  if(gROOT->IsBatch()) {
    fout->Close();
    exit(0);
  }

}

Bool_t CheckPID(STParticle *apart)
{
  if( apart == NULL ) return kFALSE;

  //  if( !apart->GetBestTrackFlag() ) return kFALSE;

  ResetPID(apart);

  auto pid    =  apart -> GetPID();
  auto prob   =  apart -> GetPIDProbability();
  auto Charge =  apart -> GetCharge();
  auto P      =  apart -> GetRotatedMomentum().Mag();

  Bool_t bsel = kFALSE;
  
  if(pid == 211){
    apart -> SetReactionPlaneFlag(1);
    bsel =  kTRUE;
  }  
  else if(pid > 1000 && Charge > 0 && P < 2500){
    apart -> SetReactionPlaneFlag(10);
    bsel =   kTRUE;
  }

  return bsel;
} 


void SetEnvironment()
{

  sRun = gSystem -> Getenv("RUN");  // RUN number
  sVer = gSystem -> Getenv("VER");  // Version ID
  sMix = gSystem -> Getenv("MIX");


  if(sRun =="" || sVer == "" ||sMix == "" ||!DefineVersion()) {
    cout << " Please type " << endl;
    cout << "$ RUN=#### VER=#.#.# MIX=0(real) or 1(mix)  root AsmFlw_getEvents.C(Number of event) " << endl;
    exit(0);
  }


  // Print Run configuration 
  cout << " ---------- CONFIGURATION ----------  " << endl;
  cout << "RUN = "      << sRun << " Assemble v" << sAsm 
       << " with ver "  << sVer  << " mix = " << sMix 
       << " Flatten ; " << nBin
       << endl;

  cout << "sBinp = " << sBinp << " Flatten RUN = " << sbRun << endl;

  // Real or mixed event 
  if (sMix == "1") bMix = kTRUE;
  else bMix = kFALSE;


  // Set RUN number
  iRun = atoi(sRun);

  if( sbRun == "0" )
    nBin = 0;

}

void PrintProcess(Int_t ievt)
{
  TDatime dtime;
  static TDatime btime(dtime);

  UInt_t eprint = 10000;
  if(bMix) eprint = 100;

  if(ievt%eprint == 0) {
    dtime.Set();
    Int_t ptime = dtime.Get() - btime.Get();
    std::cout << "Process " << setw(8) << ievt << "/"<< maxProc << " = " 
	      << ((Double_t)(ievt)/(Double_t)maxProc)*100. << " % --->"
	      << dtime.AsString() << " ---- "
	      << setw(5) << (Int_t)ptime/60 << " [min] "
	      << std::endl;
  }
}

void SetNumberOfProcess(Int_t nmax)
{
  Long64_t mEvt = GetMultiplicityDistribution();
  nEntry = fTree->GetEntries();

  if( bMix ) {
    if( nmax == -1 )  maxProc = mEvt;
    else if( nmax == -2 ) maxProc = mEvt*10;
    else
      maxProc = nmax;
  }
  else{
    if( nmax == -1 || nmax > nEntry )  maxProc = nEntry;
    else   maxProc = nmax;
  }
      


  std::cout << " Number of Events -->" << nEntry   << std::endl;
  std::cout << " Maximum Events   -->" << maxProc  << std::endl;
  std::cout << " Set     Events   -->" << nmax  << std::endl;

}

void Open()
{
  LoadPIDFile();

  TString fn = Form("run%d_flw_v"+sVer(0,3)+".root",iRun);
  if( !gSystem->FindFile("../data/", fn) ) {
    
    Int_t iver = atoi((TString)sVer(2,1));

    while( iver > -1 ){
      TString ss = Form(".%d",iver);
      fn = Form("run%d_flw_v"+sVer(0,1)+ss+".root",iRun);

      if( !gSystem->FindFile("../data/", fn) ) 
	iver--;
      else 
	break;
    }

  }
  
  auto fFile = new TFile(fn);
  fTree = (TTree*)fFile->Get("flw");

  if(!fTree) {
    cout << " No data was found " << fn << endl;
    exit(0);
  }
  cout << "Input file is " << fn << endl;


  aParticleArray   = new TClonesArray("STParticle",150);

  fTree->SetBranchAddress("STParticle",&aParticleArray);
  fTree->SetBranchAddress("ntrack",ntrack);
  //  fTree->SetBranchAddress("kymult",&kymult);

  if( iVer[0] > 1) {
    fTree->SetBranchAddress("aoq",&aoq);
    fTree->SetBranchAddress("z",&z);
    fTree->SetBranchAddress("ProjA",&ProjA);
    fTree->SetBranchAddress("ProjB",&ProjB);
    fTree->SetBranchAddress("snbm",&snbm);
  }

  p_rot = new TClonesArray("TVector3",150);

  p_org  = new TClonesArray("TVector3",150);
  
}

void Initialize()
{

  trackID.clear();

  mtrack = 0;
  mxntrk = 0;  
  numGoodTrack = 0;
  for(Int_t i = 0; i< 7; i++) ntrack[i];
  
  event.clear();

  aParticleArray->Clear();
  nParticleArray->Clear();


  TDatime dtime;
  rnd.SetSeed(dtime.Get());
}


Long64_t GetMultiplicityDistribution()
{
  // Multplicity histgram
  TString mHistFile = Form("MultRoot/run%d.ngt_v"+sVer(0,3)+".root",iRun);
  TString hf = mHistFile;

  Long64_t nEvt = 0;

  if(bMix) { 
    mhfile = new TFile(mHistFile);
    if( mhfile == NULL ) {
      cout << mHistFile << " is not found." << endl;
      exit(0);
    }
    cout << "Multiplicity : "  << mHistFile << " is loaded." << endl;


    if( (TH1I*)mhfile->FindObject("hgtc") ) 
      exit(0);
    
    histGT_r =  (TH1I*)mhfile->Get("hgtc");
    histGT_r -> SetName("hgtc_r");
    histGT_r -> SetDirectory(gROOT);
    nEvt = (Long64_t)histGT_r -> GetEntries();
  }
  else {
    //    if( !gSystem->FindFile(".",hf) ) {
    mhfile = new TFile(mHistFile,"recreate");
    cout << mHistFile << " is created." << endl;
    //    }
    // else
    //   cout << mHistFile << " is existing." << endl;
  }

  return nEvt;

}


void ResetPID(STParticle *apart)
{

  auto p    = apart->GetP();
  auto dedx = apart->GetdEdx();

  if(gProton && gDeuteron && gTriton && gPip && gPim){
    Int_t gpid  = 0;

    if(gProton->IsInside(p, dedx))
      gpid = 2212;
    else if(gDeuteron->IsInside(p, dedx))
      gpid = 1000010020;
    else if(gTriton->IsInside(p, dedx))
      gpid = 1000010030;
    else if(gPip->IsInside(p, dedx))
      gpid = 211;
    else if(gPim->IsInside(p, dedx))
      gpid = 211;

    else
      gpid = 12212; // temporal

    apart->SetPID(gpid);
  }
}


void LoadPIDFile()
{
  auto gcutFile = new TFile("gcutPID132Sn.root");
  gProton   = (TCutG*)gcutFile->Get("gcutProton132Sn2");
  gDeuteron = (TCutG*)gcutFile->Get("gcutDeutron132Sn");
  gTriton   = (TCutG*)gcutFile->Get("gcutTriton132Sn");
  gPip      = (TCutG*)gcutFile->Get("gcutPip132Sn");
  gPim      = (TCutG*)gcutFile->Get("gcutPim132Sn");


  gcutFile->Close();
}



void OutputTree(Long64_t nmax)
{

  // ROOT out
  TString sdeb = ".s";
  if(nmax < 0)  sdeb = "";
  TString foutname = "../data/run"+sRun+"_";

  if( bMix ) {
    foutname += "mxflw";
    foutname += "_v"+sVer+".root";
  }
  else {
    foutname += "rdflw";
    foutname += "_v"+sVer+sdeb+".root";
  }
  
  TString fo = foutname;

  if( !gROOT->IsBatch() && gSystem->FindFile(".",fo) ) {
    cout << foutname << " is existing. Do you recreate? (y/n)" << endl;
    TString sAns;
    cin >> sAns;
    if(sAns == "y" || sAns == "Y")
      fout  = new TFile(foutname,"recreate");
    else {
      cout << " Retry" << endl;
      exit(0);
    }      
  }
  else
    fout  = new TFile(foutname,"recreate");



  if( bMix ) 
    mflw = new TTree("mflw","FLOW analysis track mixiing");    
  else
    mflw = new TTree("rflw","FLOW analysis");

  cout << "Output file is " << foutname << endl;

  //-- output                                                                                                              
  mflw->Branch("irun",&iRun,"irun/I");
  if( iVer[0] > 1) {
    mflw->Branch("aoq",&aoq,"aoq/D");
    mflw->Branch("z",&z,"z/D");
    // mflw->Branch("aX",&aX,"aX/D");
    // mflw->Branch("bY",&bY,"bY/D");
  }

  nParticleArray = new TClonesArray("STParticle",100);

  mflw->Branch("STParticle",&nParticleArray);

  mflw->Branch("ntrack",ntrack,"ntrack[7]/I");
  mflw->Branch("mtrack",&mtrack,"mtrack/I");
  mflw->Branch("event",&event);
  mflw->Branch("mxntrk",&mxntrk);

  hgtc         = new TH1I("hgtc","number of good fragment",100,0,100);
}

Long64_t GetRandomNumTrack()
{

  TDatime dtime;

  Int_t itime = dtime.Get();

  // gRandom = new TRandom3(0);
  // gRandom->SetSeed(itime);

  if(!histGT_r){
    cout << " no histgram was found for randowm number " << endl;
    return 0;
  }

  Long64_t bcont = (Long64_t)histGT_r->GetRandom();


  return bcont;
}

STParticle *GetRealTrack(Long64_t ival)
{
  STParticle *realPart = NULL;
  realPart = (STParticle*)aParticleArray -> At(ival);

  //  cout << " real part " << realPart << endl;
  return realPart;
}


STParticle *GetMixedTrack(Long64_t *ival, Int_t *kval)
{
  STParticle  *mixPart = NULL;
  static Long64_t mevt = *ival;

  // ---------- shfiting -- search similar multiplcity events
  while(kTRUE){	
    mevt += 2;
    if(mevt > nEntry) 	mevt = 0;
           
    UInt_t kLoop = 0;
    while( kLoop < 5 ){
      fTree->GetEntry(mevt);

      mevt++;
      if(mevt > nEntry) {
	mevt = 0;
	kLoop++;
	if(kLoop > 1) cout << " Too much loops " << kLoop << " kval " << *kval << " mevt " << mevt << " / " << nEntry <<  endl;
      }

      if( snbm != 132 && snbm != 108) continue;



      if( std::abs(ntrack[2] - *kval) < ntr_diff ) {
	//	cout << "ntrack[2] " << ntrack[2] << " kval " << *kval  << endl;
	break;
      }
    }


    Int_t nmixTrack = aParticleArray->GetEntries();

    Int_t imixTrack = (Int_t)pran.Uniform(0,nmixTrack);

    if( imixTrack < nmixTrack ) {
      mixPart = (STParticle*)aParticleArray->At(imixTrack);	        

      if( mixPart->GetBestTrackFlag()) {
	mixPart->SetMixedNtrack(nmixTrack);
	mixPart->SetMixTrackID(imixTrack);

	*kval = ntrack[2];

	*ival = mevt;
	return mixPart;
      }

    }

  }

  *ival = -1;
  return mixPart;
}



void SetPtWeight(STParticle *apart)
{

  Double_t rpd   = apart->GetRapidity();


  if( rpd <  0.75455/2. ) 
    apart->SetRPWeight(-1);
  else
    apart->SetRPWeight(1);

}

Bool_t DefineVersion()
{
  Bool_t bfound = kFALSE;

  TString ver = sVer + ".";
  
  for ( Int_t i = 0; i < 3; i++) {
    if( ver.First(".") < 0 ) break;

    Ssiz_t end = ver.First(".")  ;
    TString ver1 = ver(0, end);

    ver = ver(end+1, ver.Length());

    iVer[i] = atoi(ver1);

    if(i==2) bfound = kTRUE;

  }
  
  if(!bfound)
    cout << " missing version number : " << iVer[0] << "." << iVer[1] << "." << iVer[2] << endl;

  return bfound;

}

