#include "STPiProbTask.hh"
#include "STVector.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TDatabasePDG.h"
#include "TCanvas.h"

#include <fstream>

ClassImp(STPiProbTask);

STPiProbTask::STPiProbTask() : 
  totalFunc("totalFunc", "gaus(0)+gaus(3)+gaus(6)", -3., 3.),
  piFunc("piFunc", "gaus(0)+gaus(3)", -3., 3.),
  bgFunc("bgFunc", "gaus(0)", -3., 3.),
  fitFunc("fiveParameterFitFunction", [](double *x, double *p)
    {
      if (x[0] < 0) x[0] = -x[0];
      const double m  = TDatabasePDG::Instance()->GetParticle(211) -> Mass()*1000; // replace with pion mass
      double totalE = TMath::Sqrt(m*m + x[0]*x[0]);
      double beta = x[0]/totalE;
      double gamma = 1./TMath::Sqrt(1 - beta*beta);
      double betaP3 = TMath::Power(beta, p[3]);
      return p[0]/betaP3*(p[1] - betaP3 + TMath::Log(p[2] + TMath::Power(1./(beta*gamma), p[4])));
    }, 0, 700., 5)
{
  STAnaParticleDB::EnablePions();
  fLogger = FairLogger::GetLogger();
}

STPiProbTask::~STPiProbTask()
{}

void STPiProbTask::ReadFile(TString filename)
{
  numSlices = 0;
  std::ifstream inFile(filename.Data());
  if (!inFile.is_open())
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find the file! - " + filename);

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
}

int STPiProbTask::SetParameters(double mom, double dedx)
{
  Int_t momIndex = 0;
  for (momIndex = 0; momIndex < numSlices; momIndex++)
    if (momRange[momIndex][1] >= mom && momRange[momIndex][0] < mom)
      break;

  if (momIndex >= numSlices) {
    totalFunc.SetParameters(0, 0, 0, 0, 0, 0, 0, 0, 0);
    piFunc.SetParameters(0, 0, 0, 0, 0, 0);
    bgFunc.SetParameters(0, 0, 0);

    return 0;
  } else {
    Double_t dedxMin = std::min(gausParam[momIndex][1] - 5*gausParam[momIndex][2], gausParam[momIndex][7] - 2*gausParam[momIndex][8]);
    Double_t dedxMax = std::max(gausParam[momIndex][1] + 5*gausParam[momIndex][2], gausParam[momIndex][7] + 2*gausParam[momIndex][8]);

    if (dedxMax < dedx)
      return 0;

    if (dedxMin > dedx)
      return 2;

    totalFunc.SetParameters(gausParam[momIndex][0], gausParam[momIndex][1], gausParam[momIndex][2],
                               gausParam[momIndex][3], gausParam[momIndex][4], gausParam[momIndex][5],
                               gausParam[momIndex][6], gausParam[momIndex][7], gausParam[momIndex][8]);
    piFunc.SetParameters(gausParam[momIndex][0], gausParam[momIndex][1], gausParam[momIndex][2],
                            gausParam[momIndex][6], gausParam[momIndex][7], gausParam[momIndex][8]);
    bgFunc.SetParameters(gausParam[momIndex][3], gausParam[momIndex][4], gausParam[momIndex][5]);

    return 1;
  }
}



InitStatus STPiProbTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  std::ifstream inFile("fitParam_pion.txt");
  if(inFile.is_open())
  {
    int index;
    double ptemp[5] = {0};
    
    while (true) {
      inFile >> index;
      for (auto i = 0; i < 5; i++)
        inFile >> ptemp[i];

      if (inFile.eof())
        break;

      for (auto i = 0; i < 5; i++)
        pidFitParam[index][i] = ptemp[i];
    }
  }

  const auto& pdgList = STAnaParticleDB::GetSupportedPDG();
  fPiPlusID = std::find(pdgList.begin(), pdgList.end(), 211) - pdgList.begin();
  fPiMinusID = std::find(pdgList.begin(), pdgList.end(), -211) - pdgList.begin();

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fSD = (TClonesArray*) ioMan -> GetObject("SD");
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");

  fFlattenPID = new TClonesArray("STVectorF");
  new((*fFlattenPID)[0]) STVectorF();
  ioMan -> Register("FlattenPion", "ST", fFlattenPID, fIsPersistence);
  
  return kSUCCESS;
}

void STPiProbTask::SetParContainers()
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

void STPiProbTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip == 1 indicates event skip

  auto data = static_cast<STData*>(fData -> At(0));
  auto& pip_prob_list = static_cast<STVectorF*>(fProb -> At(fPiPlusID)) -> fElements;
  auto& pip_sd_list = static_cast<STVectorF*>(fSD -> At(fPiPlusID)) -> fElements;
  auto& pim_prob_list = static_cast<STVectorF*>(fProb -> At(fPiMinusID)) -> fElements;
  auto& pim_sd_list = static_cast<STVectorF*>(fSD -> At(fPiMinusID)) -> fElements;
  auto& flat_pid = static_cast<STVectorF*>(fFlattenPID -> At(0)) ->fElements;
  flat_pid.clear();
  for(int i = 0; i < data -> multiplicity; ++i)
  {
    auto& recoMom = data -> vaMom[i];
    auto recoCharge = data -> recoCharge[i];
    auto dedxs05 = data -> vadedx[i];
    auto momCharge = recoMom.Mag()/recoCharge;

    auto pitch = 90. + TMath::ATan2(recoMom.y(), recoMom.z())*TMath::RadToDeg();
    auto yaw = 90 + TMath::ATan2(recoMom.x(), recoMom.z())*TMath::RadToDeg();

    double flatdedxs05 = -9999;
    double pip_prob = 0, pim_prob = 0;


    if(pitch > 0 && pitch < 180 && yaw > 0 && yaw < 180)
    {
      int pitchBin = pitch/pitchBinSize;
      int yawBin = yaw/yawBinSize;

      auto pitchIndex = yawBin*numPitch + pitchBin;
      if (pitchIndex >= 0 && pitchIndex < numHists) 
      {
        for (auto j = 0; j < 5; j++)
          fitFunc.SetParameter(j, pidFitParam[pitchIndex][j]);

        //TCanvas *c1 = new TCanvas;
        //fitFunc.Draw("l");
        //c1->WaitPrimitive();

        flatdedxs05 = TMath::Log(dedxs05/fitFunc.Eval(momCharge));

        int ready = this -> SetParameters(momCharge, flatdedxs05);
        if((momCharge > 0 && flatdedxs05 > -0.90/662.54*momCharge + 0.90) || 
           (momCharge < 0 && flatdedxs05 > 0.918/700.*momCharge + 1.081)) 
          ready = 0;

        if(ready == 1)
        {
          auto ppi = piFunc.Eval(flatdedxs05)/totalFunc.Eval(flatdedxs05);
          auto pbg = bgFunc.Eval(flatdedxs05)/totalFunc.Eval(flatdedxs05);
          if(std::isnan(ppi)) ppi = 0;
          if(std::isnan(pbg)) pbg = 0;
          if(pbg + ppi > 0)
          {
            if(recoCharge > 0) pip_prob = ppi/(pbg + ppi);
            else pim_prob = ppi/(pbg + ppi);
          }
        }
        //flatdedxs05 = fitFunc.Eval(momCharge);
      }
    }
    //std::cout << dedxs05 << " " << recoMom.Mag() << std::endl;
    flat_pid.push_back(flatdedxs05);
    pip_prob_list.push_back(pip_prob);
    pim_prob_list.push_back(pim_prob);
    // SD not implemented
    pip_sd_list.push_back(0);
    pim_sd_list.push_back(0);
  }
}

void STPiProbTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

