#include "STPIDCorrelatorTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "STTrack.hh"

#include <iostream>
using namespace std;

ClassImp(STPIDCorrelatorTask)

STPIDCorrelatorTask::STPIDCorrelatorTask()
: STRecoTask("PID hypothesis Task", 1, false)
{
}

STPIDCorrelatorTask::STPIDCorrelatorTask(Bool_t persistence)
: STRecoTask("PID hypothesis Task", 1, persistence)
{
}

STPIDCorrelatorTask::~STPIDCorrelatorTask()
{
}

InitStatus STPIDCorrelatorTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRecoTrackArray = (TClonesArray*) fRootManager -> GetObject("STTrackPre");
  if (fRecoTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find RecoTrack array!" << FairLogger::endl;
    return kERROR;
  }

  //  fPIDProbArray = new TClonesArray("STPIDCorrelator", 100);
  //  fRootManager -> Register("STPIDCorrelator", "SpiRIT", fPIDProbArray, fIsPersistence);
  
  return kSUCCESS;
}

void STPIDCorrelatorTask::Exec(Option_t *opt)
{
  //std::cout << "Inside STPIDCorrelatorTask" << std::endl;
    
  if (fEventHeader -> IsBadEvent())
    return;

  //  fPIDProbArray->Clear();
  
  auto numRecoTracks = fRecoTrackArray -> GetEntries();
  //std::cout << numRecoTracks << std::endl;
  for (auto iTrack = 0; iTrack < numRecoTracks; iTrack++)
  {
    //cout << "======================================" << endl;
    auto recoTrack = (STTrack *) fRecoTrackArray -> At(iTrack);
    auto nCands = recoTrack -> GetNumTrackCandidates();

    Double_t pi_prob = 0;
    Double_t p_prob = 0;
    Double_t d_prob = 0;
    Double_t t_prob = 0;
    Double_t he3_prob = 0;
    Double_t he4_prob = 0;
    Double_t tot_prob = 0;

    Int_t bestIndex = 0;
    Double_t bestProb = 0;

    //cout << "SIZE OF DEDX ARRAY " << recoTrack -> GetdEdxArray() -> size() << endl;
    for (auto iCand = 0; iCand < nCands; iCand++) {
      auto candTrack = recoTrack -> GetTrackCandidate(iCand);

      auto momReco = candTrack -> GetP();
      //cout << "size of dedx array " << candTrack -> GetdEdxArray() -> size() << endl;
      auto dedxReco = candTrack -> GetdEdxWithCut(0,0.7);
      // conversion ADC -> MeV/mm
      dedxReco = dedxReco*5.353e-06;

      auto pid = candTrack -> GetPID();

      Double_t prob = -1;
      TString name = "";
      if (pid == 211 || pid == -211) name = "pion";
      else if (pid == 2212)          name = "proton";
      else if (pid == 1000010020)    name = "deuterium";
      else if (pid == 1000010030)    name = "triton";
      else if (pid == 1000020030)    name = "3He";
      else if (pid == 1000020040)    name = "4He";

      auto mu = GetMu(name,momReco);
      auto sigma = GetSigma(name,momReco);
      prob = GetPdf(dedxReco,mu,sigma);

      if (pid == 211 && momReco > 500)
        prob = 0;

      if (bestProb < prob) {
        bestProb = prob;
        bestIndex = iCand;
      }
    }
    recoTrack -> SelectTrackCandidate(bestIndex);
  }
  
  LOG(INFO) << Space() << "STPIDCorrelatorTask done." << FairLogger::endl;
}

Double_t STPIDCorrelatorTask::fitdEdxPion(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<260)*(2.02484e-05 + 0.000460201/TMath::Log(xv) + 1.7545/(xv*xv)) + (xv>=260)*(0.00018498 - 3.10648e-07*xv + 3.52378e-10*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::fitdEdxProton(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<490)*(0.000170506 -56764.6/(xv*xv*xv*xv) + 76.1793/(xv*xv)) +
    (xv>=490)*(0.00218582 -6.96452e-06*xv + 1.01583e-08*xv*xv -7.91095e-12*xv*xv*xv +
	       3.41111e-15*xv*xv*xv*xv -7.67179e-19*xv*xv*xv*xv*xv + 7.01878e-23*xv*xv*xv*xv*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::fitdEdxDeuterium(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<490)*(0.000375736 + 271.322/(xv*xv) +  3.43658e-08*TMath::Log(xv)) +
    (xv>=490 && xv< 1000)*(0.0176734 -8.42645e-05*xv + 1.62116e-07*xv*xv -1.40286e-10*xv*xv*xv + 4.52213e-14*xv*xv*xv*xv)+
    (xv>=1000)*(0.00250854 -4.23881e-06*xv + 3.05739e-09*xv*xv -1.01574e-12*xv*xv*xv + 1.28167e-16*xv*xv*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::fitdEdxTriton(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<700)*(0.000373171 + 609.022/(xv*xv) +  2.4014e-06*TMath::Log(xv)) +
    (xv>=700 && xv< 1500)*(0.0173069 -5.47368e-05*xv + 6.99956e-08*xv*xv -4.02975e-11*xv*xv*xv + 8.64625e-15*xv*xv*xv*xv) +
    (xv>=1500)*(0.00927863 -1.61959e-05*xv + 1.1119e-08*xv*xv -3.42519e-12*xv*xv*xv + 3.95728e-16*xv*xv*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::fitdEdx3He(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<550)*(0.00176161 + 2411.02/(xv*xv) +  7.83326e-06*TMath::Log(xv)) +
    (xv>=550 && xv< 1500)*(0.0647784 -0.000200808*xv + 2.53075e-07*xv*xv -1.43952e-10*xv*xv*xv + 3.05516e-14*xv*xv*xv*xv) +
    (xv>=1500)*(0.0371145 -6.47836e-05*xv + 4.4476e-08*xv*xv -1.37008e-11*xv*xv*xv + 1.58291e-15*xv*xv*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::fitdEdx4He(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t f = (xv<800)*(0.00162818 + 4297.73/(xv*xv) +  1.8015e-05*TMath::Log(xv)) +
    (xv>=800 && xv< 1800)*(0.0680528 -0.00016068*xv + 1.53621e-07*xv*xv -6.61668e-11*xv*xv*xv + 1.06244e-14*xv*xv*xv*xv) +
    (xv>=1800)*(0.00558405 + 7.55158e-06*xv -1.23558e-08*xv*xv + 5.24654e-12*xv*xv*xv -7.15741e-16*xv*xv*xv*xv);
  return f;
}

Double_t STPIDCorrelatorTask::GetMu(TString particle, Double_t p)
{
  TF1* f;
  if (particle == "pion")
    f = new TF1("f",fitdEdxPion,40,490);
  else if (particle == "proton")
    f = new TF1("f",fitdEdxProton,60,2500);
  else if (particle == "deuterium")
    f = new TF1("f",fitdEdxDeuterium,60,2500);
  else if (particle == "triton")
    f = new TF1("f",fitdEdxTriton,60,2500);
  else if (particle == "3He")
    f= new TF1("f",fitdEdx3He,60,2500);
  else if (particle == "4He")
    f = new TF1("f",fitdEdx4He,60,2500);
  else
    std::cout << "The particle doesn't exist" << std::endl;

  Double_t val = f->Eval(p);
  delete f;
  return val;
}

Double_t STPIDCorrelatorTask::GetSigma(TString particle, Double_t p)
{
  TF1* f;
  Double_t coeff;
  if (particle == "pion"){
    f = new TF1("f",fitdEdxPion,40,490);
    coeff = 0.40;
  }
  else if (particle == "proton"){
    f = new TF1("f",fitdEdxProton,60,2500);
    coeff = 0.40;
  }
  else if (particle == "deuterium"){
    f = new TF1("f",fitdEdxDeuterium,60,2500);
    coeff = 0.30;
  }
  else if (particle == "triton"){
    f = new TF1("f",fitdEdxTriton,60,2500);
    coeff = 0.20;
  }
  else if (particle == "3He"){
    f= new TF1("f",fitdEdx3He,60,2500);
    coeff = 0.15;
  }
  else if (particle == "4He"){
    f = new TF1("f",fitdEdx4He,60,2500);
    coeff = 0.15;
  }
  else
    std::cout << "The particle doesn't exist" << std::endl;
  
  Double_t val = f->Eval(p);
  delete f;
  return val*coeff;
}

Double_t STPIDCorrelatorTask::GetPdf(Double_t dedx, Double_t mu, Double_t sigma){
  TF1 *f = new TF1("f","gaus",40,2500);
  f->SetParameter(0,1);
  f->SetParameter(1,mu);
  f->SetParameter(2,sigma);
  Double_t val = f->Eval(dedx);
  delete f;
  return val;
}
