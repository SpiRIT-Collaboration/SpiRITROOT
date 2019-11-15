#include "STPIDAnalysisTask.hh"
#include "TSystem.h"
#include "TString.h"

STPIDAnalysisTask::STPIDAnalysisTask()
{}

void STPIDAnalysisTask::SetBeamA(int t_beamA)
{ 
  fMassCal.LoadCalibrationParameters(TString(gSystem -> Getenv("VMCWORKDIR")) + "/input/PIDCalib.root", t_beamA); 
}

void STPIDAnalysisTask::SetData(STParticle* aParticle, STRecoTrack *t_track, STVertex* t_vertex, double ProjA, double ProjB)
{
  aParticle -> SetRecoTrack(t_track);
  aParticle -> SetVertex(t_vertex);

  aParticle -> SetVertexAtTargetFlag(1); // always assume vertex is at target 

  // SetupTrackQualityFlags
  if( aParticle -> GetDistanceAtVertex() > 10 )
    aParticle -> SetDistanceAtVertexFlag(0);

  if( aParticle -> GetNDF() < 15) 
    aParticle -> SetNDFFlag(0);

  aParticle -> RotateAlongBeamDirection(ProjA/1000., ProjB/1000.);
  Int_t    Charge   = aParticle -> GetCharge();
  TVector3 VMom     = aParticle -> GetRotatedMomentum();
  Double_t dEdx     = aParticle -> GetdEdx();

  //--- Set MassFitter      
  Double_t mass[2] = {0.,0.};
  if( dEdx > -1 ){
    mass[0]  = fMassCal.CalcMass(0, 1., VMom, dEdx);  // proton fitted
    
    if( mass[0] > 1500. )  // deuteron fitted
      mass[0]  = fMassCal.CalcMass(1, 1., VMom, dEdx);  
    
    mass[1]  = fMassCal.CalcMass(1, 2., VMom, dEdx);
  }
  int PIDTight = this -> GetPIDTight_(mass, dEdx);
  int PIDNormal = this -> GetPIDNorm_(mass, dEdx);
  int PIDLoose = this -> GetPIDLoose_(mass, dEdx);

  aParticle->SetBBMass(mass[0]);      
  aParticle->SetBBMassHe(mass[1]);
  aParticle->SetPID(PIDTight);
  aParticle->SetPIDTight(PIDTight);
  aParticle->SetPIDNorm(PIDNormal);
  aParticle->SetPIDLoose(PIDLoose); 
};

InitStatus
STPIDAnalysisTask::Init()
{
  if(STRecoTask::Init()==kERROR)
    return kERROR;
  
  LOG(DEBUG) << "Initialization of STPIDAnalysisTaskTask" << FairLogger::endl;
  fRecoTrack = (TClonesArray*) fRootManager -> GetObject("STRecoTrack");
  fVATrack = (TClonesArray*) fRootManager -> GetObject("VATracks");
  fBDCVertex = (TClonesArray*) fRootManager -> GetObject("BDCVertex");
  fTPCVertex = (TClonesArray*) fRootManager -> GetObject("STVertex");
  fBeamInfo = (STBeamInfo*) fRootManager -> GetObject("STBeamInfo");

  fSTParticle = new TClonesArray("STParticle");
  fVAParticle = new TClonesArray("STParticle");

  fRootManager -> Register("STParticle", "SpiRIT", fSTParticle, fIsPersistence);
  fRootManager -> Register("VAParticle", "SpiRIT", fVAParticle, fIsPersistence);


}

void STPIDAnalysisTask::Exec(Option_t *opt)
{
  STVertex *stVertex = nullptr;
  STVertex *bdcVertex = nullptr;
  if(fTPCVertex)
    if(fTPCVertex -> GetEntries() > 0) stVertex = static_cast<STVertex*>(fTPCVertex -> At(0));
  if(fBDCVertex)
    if(fBDCVertex -> GetEntries() > 0) bdcVertex = static_cast<STVertex*>(fBDCVertex -> At(0));

  LOG(INFO) << Space() << "Identifying particle PID" << FairLogger::endl;
  for(int i = 0; i < fRecoTrack -> GetEntries(); ++i)
  {
    auto particle = new((*fSTParticle)[i]) STParticle();
    if(stVertex && fBeamInfo)
      this -> SetData(particle, 
                      static_cast<STRecoTrack*>(fRecoTrack -> At(i)), 
                      stVertex, 
                      fBeamInfo -> fRotationAngleTargetPlaneA, 
                      fBeamInfo -> fRotationAngleTargetPlaneB);
  }

  for(int i = 0; i < fVATrack -> GetEntries(); ++i)
  {
    auto particle = new((*fVAParticle)[i]) STParticle();
    if(bdcVertex && fBeamInfo)
      this -> SetData(particle, 
                      static_cast<STRecoTrack*>(fVATrack -> At(i)), 
                      bdcVertex, 
                      fBeamInfo -> fRotationAngleTargetPlaneA, 
                      fBeamInfo -> fRotationAngleTargetPlaneB);
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



