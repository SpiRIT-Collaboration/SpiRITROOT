#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "STPID.hh"
#include "STPIDAnalysisTask.hh"
#include "TSystem.h"
#include "STVector.hh"
#include "TString.h"

STPIDAnalysisTask::STPIDAnalysisTask() : fMassCalH("EmpiricalBB"), 
                                         fMassCalHe("EmpiricalBB")
{}

void STPIDAnalysisTask::SetBeamA(int t_beamA)
{ 
  auto calFile = TFile::Open(Form(TString(gSystem -> Getenv("VMCWORKDIR")) + "/input/db/PIDCalib_%dSn.root", t_beamA));
  TH2D *h2ParamH[7], *h2ParamHe[7];
  if( calFile  ) {
    for(int i = 0; i < 7; ++i){
      calFile->GetObject(Form("h2InterpolateNM_%dSn_Par%d"  ,t_beamA,i),h2ParamH[i]);
      calFile->GetObject(Form("h2InterpolateHeNM_%dSn_Par%d",t_beamA,i),h2ParamHe[i]);
    }
    fMassCalH.AddParameters(h2ParamH);
    fMassCalHe.AddParameters(h2ParamHe);
  }
  else
    fLogger -> Fatal(MESSAGE_ORIGIN, Form("Mass calibration file PIDCalib_%dSn.root is not found.", t_beamA));
}

void
STPIDAnalysisTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

InitStatus
STPIDAnalysisTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  LOG(DEBUG) << "Initialization of STPIDAnalysisTaskTask" << FairLogger::endl;

  fData = (TClonesArray*) ioMan -> GetObject("STData");

  fRecoPIDTight = new TClonesArray("STVectorI");
  fRecoPIDLoose = new TClonesArray("STVectorI");
  fRecoPIDNorm = new TClonesArray("STVectorI");
  fVAPIDTight = new TClonesArray("STVectorI");
  fVAPIDLoose = new TClonesArray("STVectorI");
  fVAPIDNorm = new TClonesArray("STVectorI");

  ioMan -> Register("RecoPIDTight", "ST", fRecoPIDTight, fIsPersistence);
  ioMan -> Register("RecoPIDLoose", "ST", fRecoPIDLoose, fIsPersistence);
  ioMan -> Register("RecoPIDNorm", "ST", fRecoPIDNorm, fIsPersistence);

  ioMan -> Register("VAPIDTight", "ST", fVAPIDTight, fIsPersistence);
  ioMan -> Register("VAPIDLoose", "ST", fVAPIDLoose, fIsPersistence);
  ioMan -> Register("VAPIDNorm", "ST", fVAPIDNorm, fIsPersistence);

}

void STPIDAnalysisTask::Exec(Option_t *opt)
{
  fRecoPIDTight -> Delete(); 
  fRecoPIDLoose -> Delete();
  fRecoPIDNorm -> Delete();
  fVAPIDTight -> Delete();
  fVAPIDLoose -> Delete();
  fVAPIDNorm -> Delete(); 

  LOG(INFO) << "Identifying particle" << FairLogger::endl;

  auto RecoPIDTight = new((*fRecoPIDTight)[0]) STVectorI();
  auto RecoPIDLoose = new((*fRecoPIDLoose)[0]) STVectorI();
  auto RecoPIDNorm = new((*fRecoPIDNorm)[0]) STVectorI();
  auto VAPIDTight = new((*fVAPIDTight)[0]) STVectorI();
  auto VAPIDLoose = new((*fVAPIDLoose)[0]) STVectorI();
  auto VAPIDNorm = new((*fVAPIDNorm)[0]) STVectorI();

  auto data = static_cast<STData*>(fData -> At(0));
  auto mult = data -> multiplicity;

  // beam rotation
  TVector3 beamDirection(TMath::Tan(data->proja/1000.), TMath::Tan(data->projb/1000.), 1);
  beamDirection = beamDirection.Unit();
  auto rotationAxis = beamDirection.Cross(TVector3(0, 0, 1));
  auto rotationAngle = beamDirection.Angle(TVector3(0, 0, 1));

  for(int i = 0; i < mult; ++i)
  {
    auto VMom = data -> recoMom[i];
    auto dEdx = data -> recodedx[i];
    VMom.Rotate(rotationAngle, rotationAxis);
    //--- Set MassFitter      
    Double_t mass[2] = {0.,0.};
    mass[0] = fMassCalH.CalcMass(1., VMom, dEdx);
    mass[1] = fMassCalHe.CalcMass(2., VMom, dEdx);
    RecoPIDTight->fElements.push_back(this -> GetPIDTight_(mass, dEdx));
    RecoPIDNorm->fElements.push_back(this -> GetPIDNorm_(mass, dEdx));
    RecoPIDLoose->fElements.push_back(this -> GetPIDLoose_(mass, dEdx));
  }

  for(int i = 0; i < mult; ++i)
  {
    auto VMom = data -> vaMom[i];
    auto dEdx = data -> vadedx[i];
    VMom.Rotate(rotationAngle, rotationAxis);
    //--- Set MassFitter      
    Double_t mass[2] = {0.,0.};
    mass[0] = fMassCalH.CalcMass(1., VMom, dEdx);
    mass[1] = fMassCalHe.CalcMass(2., VMom, dEdx);

    VAPIDTight->fElements.push_back(this -> GetPIDTight_(mass, dEdx));
    VAPIDNorm->fElements.push_back(this -> GetPIDNorm_(mass, dEdx));
    VAPIDLoose->fElements.push_back(this -> GetPIDLoose_(mass, dEdx));
  }
}

Int_t STPIDAnalysisTask::GetPID_(Double_t mass[2], Double_t dedx)
{
  // p, d, t                                                                                                                               
  if( mass[0] == 0 )
    return 0;

  else if( mass[1] < 2500 && mass[0] > 0 ) {

    for(UInt_t i = 0; i < 4; i++) {
      Double_t mass_low = MassRegion[i][0]-MassRegion[i][1]*MassRegion[i][2] ;
      Double_t mass_up  = MassRegion[i][0]+MassRegion[i][1]*MassRegion[i][3] ;

      if( mass[0] >= mass_low && mass[0] <= mass_up ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }

  // He3, He4, He6                                                                                                                          
  else if( mass[0] >= 3100 && dedx <= 700) {
    for( UInt_t i = 4; i < 7; i++ ){
      Double_t mass_low = MassRegion[i][0]-MassRegion[i][1]*MassRegion[i][2] ;
      Double_t mass_up  = MassRegion[i][0]+MassRegion[i][1]*MassRegion[i][3] ;

      if( mass[1] >= mass_low && mass[1] <= mass_up ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  return 0;
}

Int_t STPIDAnalysisTask::GetPIDLoose_(Double_t mass[2], Double_t dedx)
{
  if( mass[0] == 0 )
    return 0;

  // p, d, t   
  else if( mass[1] < MassRegionLU_L[4][0] ) {
    for(UInt_t i = 0; i < 4; i++) {
      if( mass[0] >= MassRegionLU_L[i][0] && mass[0] < MassRegionLU_L[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  // He3, He4, He6
  else if( mass[0] >= 3100 && dedx <= 700) {
    for( UInt_t i = 4; i < 7; i++ ){
      if( mass[1] >= MassRegionLU_L[i][0] && mass[1] < MassRegionLU_L[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  return 0;
}

Int_t STPIDAnalysisTask::GetPIDTight_(Double_t mass[2], Double_t dedx)
{
  if( mass[0] == 0 )
    return 0;

  // p, d, t   
  else if( mass[1] < MassRegionLU_T[4][0]  ) {
    for(UInt_t i = 0; i < 4; i++) {
      if( mass[0] >= MassRegionLU_T[i][0] && mass[0] < MassRegionLU_T[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  // He3, He4, He6
  else if( mass[0] >= 3100 && dedx <= 700) {
    for( UInt_t i = 4; i < 7; i++ ){
      if( mass[1] >= MassRegionLU_T[i][0] && mass[1] < MassRegionLU_T[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  return 0;
}

Int_t STPIDAnalysisTask::GetPIDNorm_(Double_t mass[2], Double_t dedx)
{
  if( mass[0] == 0 )
    return 0;

  // p, d, t   
  else if( mass[1] < MassRegionLU_N[4][0] ) {
    for(UInt_t i = 0; i < 4; i++) {
      if( mass[0] >= MassRegionLU_N[i][0] && mass[0] < MassRegionLU_N[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  // He3, He4, He6
  else if( mass[0] >= 3100 && dedx <= 700) {
    for( UInt_t i = 4; i < 7; i++ ){
      if( mass[1] >= MassRegionLU_N[i][0] && mass[1] < MassRegionLU_N[i][1] ) {
	STPID::PID pid = static_cast<STPID::PID>(i);
        return STPID::GetPDG(pid);
      }
    }
  }
  return 0;
}

void STPIDAnalysisTask::SetPersistence(Bool_t value) { fIsPersistence = value; }


