#include "STAnalyzeG4StepTask.hh"
#include "STProcessManager.hh"
#include "STMCPoint.hh"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <cmath>
#include <iostream>
#include <iomanip>

#include "TLorentzVector.h"
#include "TString.h"
#include "TMath.h"
#include "TRandom.h"
#include "Math/Interpolator.h"

using namespace std;

STAnalyzeG4StepTask::STAnalyzeG4StepTask()
   :	FairTask("STAnalyzeG4StepTask"),
   fIsPersistence(kFALSE),
   fIsTAPersistence(kFALSE),
   fEventID(0),
   fIsSetGainMatchingData(kFALSE)
{
   fLogger->Debug(MESSAGE_ORIGIN,"Default Constructor of STAnalyzeG4StepTask");
}

STAnalyzeG4StepTask::~STAnalyzeG4StepTask()
{
   fLogger->Debug(MESSAGE_ORIGIN,"Destructor of STAnalyzeG4StepTask");
}

void STAnalyzeG4StepTask::SetParContainers()
{
   fLogger->Debug(MESSAGE_ORIGIN,"SetParContainwes of STAnalyzeG4StepTask");

   FairRunAna* ana = FairRunAna::Instance();
   FairRuntimeDb* rtdb = ana->GetRuntimeDb();
   fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");
}

InitStatus STAnalyzeG4StepTask::Init()
{
   fLogger->Debug(MESSAGE_ORIGIN,"Initialization of STAnalyzeG4StepTask");

   FairRootManager* ioman = FairRootManager::Instance();
   fMCPointArray = (TClonesArray*) ioman->GetObject("STMCPoint");	// input mc points array

   fRawEventArray = new TClonesArray("STRawEvent");		// pad response by drifted electrons
   fTAMCPointArray = new TClonesArray("STMCPoint");		// mc step in trigger array
   ioman->Register("PPEvent","ST",fRawEventArray,fIsPersistence);
   ioman->Register("TAMCPoint","ST",fTAMCPointArray,fIsTAPersistence);

   fRawEvent = new ((*fRawEventArray)[0]) STRawEvent();

   fYAnodeWirePlane = fPar->GetAnodeWirePlaneY(); // [mm] 
   fZWidthPadPlane  = fPar->GetPadPlaneZ(); // [mm] 
   fNumWires        = 363;
   fZSpacingWire    = 4;
   fZOffsetWire     = 2;
   fZCenterWire = fZWidthPadPlane/2+fZOffsetWire;
   fZFirstWire  = fZCenterWire - fNumWires/2*fZSpacingWire;
   fZLastWire   = fZCenterWire + fNumWires/2*fZSpacingWire;
   fZCritWire   = fZOffsetWire; // = 0*fZSpacingWire + fZOffsetWire
   fICritWire   = (fZCritWire-fZOffsetWire)/fZSpacingWire; // == 0
   fIFirstWire  = (fZFirstWire-fZOffsetWire)/fZSpacingWire;
   fILastWire   = (fZLastWire-fZOffsetWire)/fZSpacingWire;

   fEIonize  = fPar->GetEIonize()*1.E6; // [MeV] to [eV]
   fVelDrift = fPar->GetDriftVelocity()/100.; // [cm/us] to [mm/ns]
   fCoefT    = fPar->GetCoefDiffusionTrans()*sqrt(10.); // [cm^(-1/2)] to [mm^(-1/2)]
   fCoefL    = fPar->GetCoefDiffusionLong()*sqrt(10.);  // [cm^(-1/2)] to [mm^(-1/2)]
   fGain     = fPar->GetGain();
   
   if(fIsSetGainMatchingData){
      TString fileName = TString(gSystem->Getenv("VMCWORKDIR")) + "/parameters/RelativeGain.list";
      std::ifstream matchList(fileName.Data());
      Int_t layer = 0;
      Double_t relativeGain = 0;
      for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
         matchList >> layer >> relativeGain;
         fGainMatchingDataScale[layer] = relativeGain;
      }
   }

   // STPadResponseTask part
   fTBTime    = fPar -> GetTBTime();
   fNTbs      = fPar -> GetNumTbs();
   fXPadPlane = fPar -> GetPadPlaneX();
   fZPadPlane = fPar -> GetPadPlaneZ();

   fPadSizeRow   = 8;
   fPadSizeLayer = 12;

   fNRows   = fXPadPlane/fPadSizeRow; // 108
   fNLayers = fZPadPlane/fPadSizeLayer; // 112

   fTbOffset = fYAnodeWirePlane/fVelDrift;

   InitDummy();
   InitPRF();

   return kSUCCESS; 
}

void STAnalyzeG4StepTask::Exec(Option_t* option)
{
   fLogger->Debug(MESSAGE_ORIGIN, "Exec of STAnalyzeG4StepTask");

   Int_t nMCPoints = fMCPointArray->GetEntries();
   if(nMCPoints<10){
      fLogger->Warning(MESSAGE_ORIGIN, "Not enough hits for digitization!");
      return;
   }

   fRawEvent -> Clear();

   if(!fRawEventArray)
      fLogger->Fatal(MESSAGE_ORIGIN, "No RawEventArray!");

   fRawEvent->SetEventID(fEventID);
   ReInitDummy();
   fTAMCPointArray->Delete();

   for(Int_t iPoint=0; iPoint<nMCPoints; iPoint++){
      auto mcpoint = (STMCPoint*) fMCPointArray->At(iPoint);

      Int_t detID	= mcpoint->GetDetectorID();
      if(detID!=3){
	 Int_t nStep = fTAMCPointArray->GetEntries();
	 new((*fTAMCPointArray)[nStep]) STMCPoint(mcpoint->GetTrackID(),detID,
	       TVector3(mcpoint->GetX(),mcpoint->GetY(),mcpoint->GetZ()),
	       TVector3(mcpoint->GetPx(),mcpoint->GetPy(),mcpoint->GetPz()),
	       mcpoint->GetTime(),mcpoint->GetLength(),
	       mcpoint->GetEnergyLoss(),mcpoint->GetPDG());
      }

      if(detID!=3) continue;

      Double_t eLoss = mcpoint->GetEnergyLoss()*1.E9;
      if(eLoss<=0.) continue;

      Double_t lDrift = fYAnodeWirePlane -10.*mcpoint->GetY();
      if(lDrift<0.||lDrift>1000.) continue;
      Double_t tDrift = lDrift/fVelDrift;
      Double_t sigmaL = fCoefL*TMath::Sqrt(lDrift);
      Double_t sigmaT = fCoefT*TMath::Sqrt(lDrift);

      Int_t nElectrons = (Int_t)eLoss/fEIonize;
      for(Int_t iElectron=0; iElectron<nElectrons; iElectron++){
	 Int_t gain = gRandom->Gaus(fGain,20);
	 if(gain<=0)continue;

	 Double_t dr    = gRandom->Gaus(0,sigmaT); // displacement in radial direction
	 Double_t angle = gRandom->Uniform(2*TMath::Pi()); // random angle

	 Double_t dx = dr*TMath::Cos(angle); // displacement in x-direction
	 Double_t dz = dr*TMath::Sin(angle); // displacement in y-direction
	 Double_t dt = gRandom->Gaus(0,sigmaL)/fVelDrift; // displacement in time

	 Int_t iWire = (Int_t)floor((mcpoint->GetZ()*10+dz)/fZSpacingWire); //The index of the anode wire is from 0 to 112*3-1
	 if(iWire < 0 || iWire > 112*3-1) continue; //the anode wire number wont change for the SpiRIT experiment
	 Int_t zWire = iWire*fZSpacingWire+fZOffsetWire;

	 Double_t xEl = mcpoint->GetX()*10. + dx;
	 Double_t tEl = mcpoint->GetTime() + tDrift + dt + fTbOffset;

	 Int_t row   = (xEl+fXPadPlane/2)/fPadSizeRow;
	 Int_t layer = iWire/3;
	 Int_t type  = iWire%3; //< %3 : same reason as above
	 Int_t iTb   = tEl/fTBTime;
	 if(iTb<0||iTb>fNTbs) continue;

	 // Covering 5x5(25 in total) pads cover 99.97 % of all the charges.
	 for(Int_t iLayer=0; iLayer<5; iLayer++){ 
	    Int_t jLayer = layer+iLayer-2;
	    if(jLayer<0 || jLayer>=fNLayers) continue;

	    for(Int_t iRow=0; iRow<5; iRow++)  { 
	       Double_t relGain = (Double_t)gain;
	       Int_t jRow = row+iRow-2;
	       if(jRow<0 || jRow>=fNRows) continue;

	       STPad* pad = fRawEventDummy -> GetPad(jRow*fNLayers+jLayer);
	       if(!pad) continue;

	       Double_t x1 = jRow*fPadSizeRow - fXPadPlane/2;     // pad x-range lower edge
	       Double_t x2 = (jRow+1)*fPadSizeRow - fXPadPlane/2; // pad x-range higth edge
	       Double_t content;
               if(fIsSetGainMatchingData)
                 relGain /= fGainMatchingDataScale[jLayer];
	       if(!fAssumeGausPRF){
		  content = relGain*fFillRatio[type][iLayer]
		     *(fPRIRow->Eval(x2-xEl)-fPRIRow->Eval(x1-xEl));
	       }
	       else{
		  content = relGain*fFillRatio[type][iLayer]
		     *( (0.5*TMath::Erf((x2-xEl)/fPRIPar0)) 
			   -(0.5*TMath::Erf((x1-xEl)/fPRIPar0)) );
	       }
	       Double_t content0 = pad->GetADC(iTb);
	       pad -> SetADC(iTb, content0+content);
	       fIsActivePad[jRow*fNLayers+jLayer] = kTRUE;
	    }
	 }
      }
   }
   CloneEvent();

   fLogger->Info(MESSAGE_ORIGIN,
	 Form("Event #%d : MCPoints (%d) found. Active pads (%d) created.",
	    fEventID++, nMCPoints, fRawEvent->GetNumPads()));

   return;
}

void STAnalyzeG4StepTask::InitDummy()
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

void STAnalyzeG4StepTask::ReInitDummy()
{
   for(Int_t iRow=0; iRow<fNRows; iRow++) {
      for(Int_t iLayer=0; iLayer<fNLayers; iLayer++){
	 STPad* pad = fRawEventDummy -> GetPad(iRow*fNLayers+iLayer);
	 for(int iTb=0; iTb<fNTbs; iTb++) pad -> SetADC(iTb, 0);
	 fIsActivePad[iRow*fNLayers+iLayer] = kFALSE;
      }
   }
}

void STAnalyzeG4StepTask::CloneEvent()
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
STAnalyzeG4StepTask::PRFunction(Double_t *x, Double_t *par)
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
STAnalyzeG4StepTask::InitPRF()
{
   // Effective range of pad response function
   Double_t effRangePR = 80; 

   fPRRow = new TF1("PRRow",this,&STAnalyzeG4StepTask::PRFunction,
	 -effRangePR/2, effRangePR/2,1,
	 "STAnalyzeG4StepTask","PRFunction");
   fPRRow -> SetParameter(0,8);
   fPRLayer = new TF1("PRLayer",this,&STAnalyzeG4StepTask::PRFunction,
	 -effRangePR/2, effRangePR/2,1,
	 "STAnalyzeG4StepTask","PRFunction");
   fPRLayer -> SetParameter(0,12);

   Double_t spacingWire = 4;
   Double_t totL = fPRLayer->Integral(-effRangePR/2,effRangePR/2);
   for(Int_t iType=0; iType<3; iType++){
      for(Int_t iPad=0; iPad<5; iPad++){
	 Double_t val 
	    = fPRLayer->Integral(-fPadSizeLayer/2+(iPad-2)*fPadSizeLayer-(iType-1)*spacingWire,
		  fPadSizeLayer/2+(iPad-2)*fPadSizeLayer-(iType-1)*spacingWire);
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




void STAnalyzeG4StepTask::SetPersistence(Bool_t value)  { fIsPersistence = value; }
void STAnalyzeG4StepTask::SetTAPersistence(Bool_t value){ fIsTAPersistence = value; }


ClassImp(STAnalyzeG4StepTask);
