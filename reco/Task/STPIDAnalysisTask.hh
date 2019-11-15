#ifndef STPIDANALYSISTASK_H
#define STPIDANALYSISTASK_H
#include "STMassCalculator.hh"
#include "STRecoTrack.hh"
#include "STVertex.hh"
#include "STParticle.hh"
#include "STRecoTask.hh"
#include "STBeamInfo.hh"

#include "TClonesArray.h"

class STPIDAnalysisTask : public STRecoTask
{
public:
  STPIDAnalysisTask();

  void SetBeamA(int t_beamA);
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  void SetVerbose(Bool_t value = kTRUE);
private:
  Bool_t fVerbose;
  TClonesArray *fRecoTrack = nullptr;
  TClonesArray *fVATrack = nullptr;
  TClonesArray *fTPCVertex = nullptr;
  TClonesArray *fBDCVertex = nullptr;
  STBeamInfo *fBeamInfo = nullptr;

  TClonesArray *fSTParticle = nullptr;
  TClonesArray *fVAParticle = nullptr;

  void SetData(STParticle* aParticle, STRecoTrack *t_track, STVertex* t_vertex, double ProjA, double ProjB);
  STMassCalculator fMassCal;
  Double_t MassRegion[7][4] ={{ 127.2,   21.3,      4.,  4.},            //pi  
			      { 911.044, 68.4656,   2.,  2.},            //p  685.3 to 1,165.9
			      { 1874.76, 129.962,   1.5, 1.5},           //d  1,552.3 to 
			      { 2870.62, 212.896,   1.,  1.},            //t 2463
			      { 2760.47, 196.659,   1.,  1.},            //He3 
			      { 3720.77, 255.71,    1.,  1.},            //He4 
			      { 5751.97, 673.339,   0.5, 0.5}};      //! //He6  // read from fitted function
 
  Double_t MassRegionLU_L[7][2] = { {   0.0,  400.0},     // pi
				    { 500.0, 1300.0},     // p
				    {1400.0, 2300.0},     // d
				    {2350.0, 3300.0},     // t
				    {2200.0, 3050.0},     // fBBMassHe  3He
				    {3100.0, 4200.0},     // fBBMassHe  4He
				    {4200.0, 7000.0}};    //!           6He

  Double_t MassRegionLU_N[7][2] = { {   0.0,  350.0},     // pi
				    { 700.0, 1200.0},     // p
				    {1500.0, 2200.0},     // d
				    {2450.0, 3200.0},     // t
				    {2300.0, 2950.0},     // fBBMassHe  3He
				    {3200.0, 4100.0},     // fBBMassHe  4He
				    {4100.0, 7000.0}};    //!           6He


  Double_t MassRegionLU_T[7][2] = { {   0.0,  250.0},     // pi
				    { 800.0, 1050.0},     // p
				    {1600.0, 2100.0},     // d
				    {2550.0, 3100.0},     // t
				    {2400.0, 2850.0},     // fBBMassHe  3He
				    {3300.0, 4000.0},     // fBBMassHe  4He
				    {4000.0, 7000.0}};    //!           6He

  Int_t GetPID_(Double_t mass[2], Double_t dedx);
  Int_t GetPIDLoose_(Double_t mass[2], Double_t dedx);
  Int_t GetPIDTight_(Double_t mass[2], Double_t dedx);
  Int_t GetPIDNorm_(Double_t mass[2], Double_t dedx);

  ClassDef(STPIDAnalysisTask,1);
};

#endif
