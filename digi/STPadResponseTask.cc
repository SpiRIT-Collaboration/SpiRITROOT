/**
 * @brief Process drifting of electron from created position to anode wire
 * plane. 
 *
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail See header file or detail.
 */

// Fair class header
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ST header
#include "STPadResponseTask.hh"
#include "STProcessManager.hh"

// C/C++ class headers
#include <iostream>
#include <math.h>

using namespace std;

STPadResponseTask::STPadResponseTask()
:FairTask("STPadResponseTask"),
 fEventID(0),
 fGainMatchingDataScale(112, std::vector<double>(108, 1)),
 fAssumeGausPRF(kFALSE)
{
  fIsPersistence = kFALSE;

  fLogger->Debug(MESSAGE_ORIGIN,"Defaul Constructor of STPadResponseTask");
}

STPadResponseTask::~STPadResponseTask()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STPadResponseTask");
}

void 
STPadResponseTask::SetParContainers()
{
  fLogger->Debug(MESSAGE_ORIGIN,"SetParContainers of STPadResponseTask");

  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

void STPadResponseTask::SetGainMatchingData(TString filename)
{
  auto fGainMatchingData = filename;
  if (!(fGainMatchingData.IsNull())) {
    cout<< "== [STPadResponseTask] Low anode gain file set!" <<endl;  
    std::ifstream matchList(fGainMatchingData.Data());
    Int_t layer = 0;
    Int_t row = 0;
    Double_t relativeGain = 0;
    std::string line;
    while(std::getline(matchList, line))
    {
      double col1, col2, col3;
      std::stringstream ss(line);
      ss >> col1 >> col2;
      layer = int(col1 + 0.5);
      if(ss >> col3)
      {
        row = int(col2 + 0.5);
        fGainMatchingDataScale[layer][row] = col3;
      }
      else 
        for(int iRow = 0; iRow < 108; ++iRow)
          fGainMatchingDataScale[layer][iRow] = col2;

    }
  }
  
}

InitStatus 
STPadResponseTask::Init()
{
  fLogger->Debug(MESSAGE_ORIGIN,"Initilization of STPadResponseTask");

  FairRootManager* ioman = FairRootManager::Instance();

  fElectronArray = (TClonesArray*) ioman->GetObject("STDriftedElectron");
  fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");
  ioman->Register("MCEventHeader.", "ST", fFairMCEventHeader, fIsPersistence);
  fRawEventArray = new TClonesArray("STRawEvent"); 
  ioman->Register("PPEvent", "ST", fRawEventArray, fIsPersistence);

  fRawEvent = new ((*fRawEventArray)[0]) STRawEvent();

  fTBTime    = fPar -> GetTBTime();
  fNTbs      = fPar -> GetNumTbs();
  fXPadPlane = fPar -> GetPadPlaneX();
  fZPadPlane = fPar -> GetPadPlaneZ(); 

  fPadSizeRow   = 8;
  fPadSizeLayer = 12;

  fNRows   = fXPadPlane/fPadSizeRow; // 108
  fNLayers = fZPadPlane/fPadSizeLayer; // 112

  fTbOffset = fPar->GetAnodeWirePlaneY()/(fPar->GetDriftVelocity()/100.);

  fElectronicsJitter.clear();
  for(int i = 0; i < fNLayers; ++i)
    fElectronicsJitter.push_back(std::vector<double>(fNRows, 0));
  if(fElectronicsJitterFilename.empty()) fLogger->Info(MESSAGE_ORIGIN, "Electronics jittering is disabled");
  else
  {
    ifstream calfile(fElectronicsJitterFilename.c_str());
    if(calfile.is_open())
    {
      fLogger -> Info(MESSAGE_ORIGIN, ("Loading electronics jittering from " + fElectronicsJitterFilename).c_str());
      Int_t layer, row, n;
      Double_t amp, off, err;
      while (calfile >> layer >> row >> n >> amp >> off >> err) {
        if (abs(off) > 4 || err > 2 || n < 20) off = 0;
        fElectronicsJitter[layer][row] = off;
      }
    }
    else fLogger->Info(MESSAGE_ORIGIN, ("Electronics jitter is disabled as file " + fElectronicsJitterFilename + " cannot be loaded").c_str());
  }

  fDriftVelocity = fPar->GetDriftVelocity() / 100; // cm/us to mm/ns

  InitDummy();
  InitPRF();

  return kSUCCESS;
}

void 
STPadResponseTask::Exec(Option_t* option)
{
  fLogger->Debug(MESSAGE_ORIGIN,"Exec of STPadResponseTask");

  fRawEvent -> Clear();

  if(!fRawEventArray) 
    fLogger->Fatal(MESSAGE_ORIGIN,"No RawEventArray!");

  fEventID = fFairMCEventHeader -> GetEventID();
  fRawEvent->SetEventID(fEventID);

  ReInitDummy();

  Int_t nElectrons = fElectronArray -> GetEntries();
  for(Int_t iElectron=0; iElectron<nElectrons; iElectron++)
  {
    fElectron = (STDriftedElectron*) fElectronArray -> At(iElectron);

    Double_t xEl = fElectron->GetX() + fElectron->GetDiffusedX();
    Double_t tEl = fElectron->GetHitTime() 
                  +fElectron->GetDriftTime()
                  +fElectron->GetDiffusedTime();
                  +fTbOffset; 
    Int_t iWire  = fElectron->GetIWire();
    Int_t gain   = fElectron->GetGain();

    Int_t row   = (xEl+fXPadPlane/2)/fPadSizeRow;
    /** 
     * We can find layer number with /3 because there are just 3 cases for 
     * for position of wire relative to the pad. These cases will be denoted
     * with 'type'.
     *
     * type \n
     * - 0 : wire with 1/6 across (z) the pad.
     * - 1 : wire at the center (z) of the pad 
     * - 2 : wire with 5/6 across (z) the pad.
     */
    Int_t layer = iWire/3;
    tEl += fElectronicsJitter[layer][row]/fDriftVelocity; // subtraction is performed on MC data. It will be added back is PSA in reco task

    Int_t type  = iWire%3; //< %3 : same reason as above
    Int_t iTb   = tEl/fTBTime;
    if(iTb>fNTbs) continue;
    if(iTb < 0) iTb = 0;

    gain /= fGainMatchingDataScale[layer][row];

    // Covering 5x5(25 in total) pads cover 99.97 % of all the charges.
    for(Int_t iLayer=0; iLayer<5; iLayer++){ 
      Int_t jLayer = layer+iLayer-2;
      if(jLayer<0 || jLayer>=fNLayers) continue;

      for(Int_t iRow=0; iRow<5; iRow++)  { 
        Int_t jRow = row+iRow-2;
        if(jRow<0 || jRow>=fNRows) continue;

        STPad* pad = fRawEventDummy -> GetPad(jRow*fNLayers+jLayer);
        if(!pad) continue;

        Double_t x1 = jRow*fPadSizeRow - fXPadPlane/2;     // pad x-range lower edge
        Double_t x2 = (jRow+1)*fPadSizeRow - fXPadPlane/2; // pad x-range higth edge

	Double_t sigma = 3.4;
	Double_t x1_p = (x1-xEl)/(sqrt(2)*sigma);
	Double_t x2_p = (x2-xEl)/(sqrt(2)*sigma);

        Double_t content;
        if(!fAssumeGausPRF){
          content = gain*fFillRatio[type][iLayer]
                   *(fPRIRow->Eval(x2-xEl)-fPRIRow->Eval(x1-xEl));
        }
        else{
          content = gain*fFillRatio[type][iLayer]* .5*(TMath::Erf(x2_p) - TMath::Erf(x1_p));
        }
        Double_t content0 = pad->GetADC(iTb);
        pad -> SetADC(iTb, content0+content);
        fIsActivePad[jRow*fNLayers+jLayer] = kTRUE;
      }
    }
  }

  CloneEvent();

  Int_t nPads = fRawEvent -> GetNumPads();
  fLogger->Info(MESSAGE_ORIGIN, 
                Form("Event #%d : Active pads (%d) created.",
                     fEventID, nPads));

  return;
}

void
STPadResponseTask::InitDummy()
{
  fRawEventDummy = new STRawEvent();
  fRawEventDummy -> SetName("RawEventDummy");

  for(Int_t iRow=0; iRow<fNRows; iRow++){ 
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){ 

      STPad* pad = new STPad(iRow,iLayer);
      pad -> SetPedestalSubtracted(kTRUE);

      fRawEventDummy -> SetPad(pad);
      delete pad;
    }
  }
}

void
STPadResponseTask::ReInitDummy()
{
  for(Int_t iRow=0; iRow<fNRows; iRow++) {
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){
      STPad* pad = fRawEventDummy -> GetPad(iRow*fNLayers+iLayer);
      for(int iTb=0; iTb<fNTbs; iTb++) pad -> SetADC(iTb, 0);
      fIsActivePad[iRow*fNLayers+iLayer] = kFALSE;
    }
  }
}

void 
STPadResponseTask::CloneEvent()
{
  for(Int_t iRow=0; iRow<fNRows; iRow++){
    for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){
      if(!fIsActivePad[iRow*fNLayers+iLayer]) continue;
      STPad* pad = fRawEventDummy -> GetPad(iRow*fNLayers+iLayer);
      fRawEvent -> SetPad(pad);
    }
  }
}

Double_t 
STPadResponseTask::PRFunction(Double_t *x, Double_t *par)
{
  Double_t K1 = 0.2699; // K2*sqrt(K3)/(4TMath::ATan(sqrt(K3)))
  Double_t K2 = 0.8890; // TMath::Pi()/2*(1 - sqrt(K3)/2);
  Double_t K3 = 0.7535;

  Double_t w  = par[0]; // [mm] pad length in direction, perpendicular to the wire
  Double_t h  = 4;      // [mm] anode cathode saparation

  Double_t val = TMath::ATan(sqrt(K3)*TMath::TanH(K2*(x[0]/h+w/2/h)))
                -TMath::ATan(sqrt(K3)*TMath::TanH(K2*(x[0]/h-w/2/h)));

  return val;
}

void
STPadResponseTask::InitPRF()
{
  // Effective range of pad response function
  Double_t effRangePR = 80; 

  fPRRow = new TF1("PRRow",this,&STPadResponseTask::PRFunction,
                   -effRangePR/2, effRangePR/2,1,
                   "STPadResponseTask","PRFunction");
  fPRRow -> SetParameter(0,8);
  fPRLayer = new TF1("PRLayer",this,&STPadResponseTask::PRFunction,
                     -effRangePR/2, effRangePR/2,1,
                     "STPadResponseTask","PRFunction");
  fPRLayer -> SetParameter(0,12);

  Double_t spacingWire = 4.;

  Double_t sigma = 3.5;
  Double_t x1 = -effRangePR/2.;
  Double_t x2 = effRangePR/2.;

  Double_t x1_p = x1/(sqrt(2)*sigma);
  Double_t x2_p = x2/(sqrt(2)*sigma);
  Double_t totL =  .5*(TMath::Erf(x2_p) - TMath::Erf(x1_p));

  for(Int_t iType=0; iType<3; iType++){
    for(Int_t iPad=0; iPad<5; iPad++){

      x1 = -fPadSizeLayer/2+(iPad-2)*fPadSizeLayer-(iType-1)*spacingWire;
      x2 =  fPadSizeLayer/2+(iPad-2)*fPadSizeLayer-(iType-1)*spacingWire;
      //      x1_p = (x1 -(iType-1)*spacingWire)/(sqrt(2)*sigma);
      //      x2_p = (x2 -(iType-1)*spacingWire)/(sqrt(2)*sigma);
      x1_p = (x1)/(sqrt(2)*sigma);
      x2_p = (x2)/(sqrt(2)*sigma);

      Double_t val = .5*(TMath::Erf(x2_p) - TMath::Erf(x1_p));

      fFillRatio[iType][iPad] = val/totL;
    }
  }

  Int_t nPoints = 500;
  Double_t dx = effRangePR/nPoints;
  Double_t totR = fPRRow->Integral(-effRangePR/2,effRangePR/2);
  fPRIRow = new TGraph();
  fPRIRow->SetPoint(0,-fXPadPlane,-0.5);
  for(Int_t iPoint=0; iPoint<nPoints; iPoint++){
    fPRIRow->SetPoint(iPoint+1,
                      (iPoint+0.5)*dx-effRangePR/2,
                      fPRRow->Integral(0,(iPoint+0.5)*dx-effRangePR/2)/totR);
  }
  fPRIRow->SetPoint(nPoints+1,fXPadPlane,0.5);

  TF1* fitPRI = new TF1("f","0.5*TMath::Erf(x/[0])",-effRangePR/2,effRangePR/2);
       fitPRI -> SetParameter(0,5.83944);
  fPRIRow -> Fit(fitPRI,"Q");
  fPRIPar0 = fitPRI -> GetParameter(0);
}

void STPadResponseTask::SetPersistence(Bool_t value)  { fIsPersistence = value; }

ClassImp(STPadResponseTask);
