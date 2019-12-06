#include "TROOT.h"
#include "TLatex.h"
#include "STPIDCutTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "STVector.hh"

ClassImp(STPIDCutTask);

STPIDCutTask::STPIDCutTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  fPDGLists = new STVectorI();
  fMassLists = new STVectorF();
}

STPIDCutTask::~STPIDCutTask()
{}

InitStatus STPIDCutTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  for(int i = 0; i < fNPitches; ++i)
  {
    std::vector<TH2F*> yawHists;
    for(int j = 0; j < fNYaw; ++j)
      yawHists.push_back(new TH2F(TString::Format("Pitch%dYaw%d", i, j), "", 
                                  fMomBins, fMinMom, fMaxMom, fdEdXBins, fMindEdX, fMaxdEdX));
    fPIDHists.push_back(yawHists);
  }

  auto namelist = ioMan -> GetBranchNameList();
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  ioMan -> Register("PDG", "ST", fPDGLists, fIsPersistence);
  ioMan -> Register("Mass", "ST", fMassLists, fIsPersistence);
  return kSUCCESS;
}

void
STPIDCutTask::SetParContainers()
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

void STPIDCutTask::Exec(Option_t *opt)
{
  fPDGLists -> fElements.clear();
  fMassLists -> fElements.clear();
  auto data = (STData*) fData -> At(0);
  int npart = data -> multiplicity;

  auto pdg_ary = fPDGLists;
  auto mass_ary = fMassLists;

  for(int part = 0; part < npart; ++part)
  {
    int nClus = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
    double poca = data ->recodpoca[part].Mag();
    const auto& mom = data -> vaMom[part];
    const auto& charge = data -> recoCharge[part];
    const auto& dedx = data -> vadedx[part];

    double momMag = mom.Mag()/charge;
    int pdg = 0;
    double mass = 0;
    int pitchId = this->_ToPitchId(mom);
    int yawId = this->_ToYawId(mom);

    for(int icut = 0; icut < fCuts.size(); ++icut)
      if(auto cut = fCuts[icut][{pitchId, yawId}])
        if(cut -> IsInside(momMag, dedx))
        { 
          pdg = fPDG[icut];
          mass = fMass[icut];
          break;
        }
    pdg_ary -> fElements.push_back(pdg); 
    mass_ary -> fElements.push_back(mass);
 
    if(nClus > fMinNClus && poca < fMaxDPOCA)
      if(pitchId >= 0 && yawId >= 0)
        fPIDHists[pitchId][yawId] -> Fill(momMag, dedx);
  }
}

void STPIDCutTask::FinishTask()
{
  FairRootManager::Instance() -> GetOutFile();
  for(auto& yaws : fPIDHists)
    for(auto hist : yaws)
      hist -> Write();

  gROOT -> SetBatch(true);
  TCanvas c1;
  OrgainzePIDs(&c1, fPIDHists, fNYaw, fNPitches);
  c1.Write("PIDCanvas");
  gROOT -> SetBatch(false);
}

void STPIDCutTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STPIDCutTask::SetNPitch(int pitches)                                                  { fNPitches = pitches; }
void STPIDCutTask::SetNYaw(int yaws)                                                         { fNYaw = yaws; }
void STPIDCutTask::SetCutConditions(int minNClus, double maxPOCA)                            { fMinNClus = minNClus; fMaxDPOCA = maxPOCA; }
void STPIDCutTask::SetCutFile(const std::string& cutfile)
{
  fCutFile = new TFile(cutfile.c_str());
  if(!fCutFile -> IsOpen())
  {
    fLogger -> Error(MESSAGE_ORIGIN, TString::Format("Cutfile %s cannot be opened", cutfile.c_str()));
    return;
  }
  else fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Loading cuts from file %s", cutfile.c_str()));

  //for(const auto& particle : fParticleNames)
  //{
  //  auto cutg = static_cast<TCutG*>(fCutFile -> Get(particle.c_str()));
  //  if(cutg) 
  //    fCuts.push_back(static_cast<TCutG*>(fCutFile -> Get(particle.c_str())));
  //  else
  //  {
  //    fCuts.push_back(nullptr);
  //    fLogger -> Error(MESSAGE_ORIGIN, TString::Format("Particle cut for %s is not found", particle.c_str())); 
  //  }
  //}
  for(const auto& particle : fParticleNames)
  {
    std::map<std::pair<int, int>, TCutG*> cutForParticle;
    for(int pitchId = 0; pitchId < fNPitches; ++pitchId)
      for(int yawId = 0; yawId < fNYaw; ++yawId)
      {
        auto cutg = static_cast<TCutG*>(fCutFile -> Get(TString::Format("%sPitch%dYaw%d", particle.c_str(), pitchId, yawId)));
        cutForParticle[{pitchId, yawId}] = cutg;
      }
    fCuts.push_back(cutForParticle);
  }
}


int STPIDCutTask::_ToPitchId(const TVector3& vec)
{
  double pitch = TMath::ATan2(vec.y(), vec.z());
  double dPitch = TMath::Pi()/fNPitches;
  if(std::fabs(pitch) < TMath::Pi()/2.)
    return (pitch + TMath::Pi()/2.)/dPitch;
  else
    return -1;
}

int STPIDCutTask::_ToYawId(const TVector3& vec)
{
  double yaw = TMath::ATan2(vec.x(), vec.z());
  double dYaw = TMath::Pi()/fNYaw;
  if(std::fabs(yaw) < TMath::Pi()/2.)
    return (yaw + TMath::Pi()/2.)/dYaw;
  else
    return -1;
}

void OrgainzePIDs(TCanvas *c1, std::vector<std::vector<TH2F*>>& hists, int nYaw, int nPitch)
{
  c1 -> cd();
  c1 -> SetFillStyle(4000);
  double lMargin = 0.10;
  double rMargin = 0.10;
  double bMargin = 0.15;
  double tMargin = 0.05;
  auto pads = CanvasPartition(c1, nYaw, nPitch, lMargin,
                              rMargin, bMargin, tMargin,
                              "Momentum (MeV/c^{2})", "dE/dx (channels / mm)");
  for(int yawId = 0; yawId < nYaw; ++yawId)
    for(int pitchId = 0; pitchId < nPitch; ++pitchId)
    {
      c1 -> cd();
      auto pad = pads[yawId][pitchId];
      pad -> Draw();
      pad -> SetFillStyle(4000);
      pad -> SetFrameFillStyle(4000);

      pad -> cd();
      hists[pitchId][yawId] -> Draw("col");
      auto pt = new TPaveText(1000, 600, 2900, 950, "NB");
      pt -> SetFillStyle(0);
      GetCutCaption(pt, yawId, pitchId, nYaw, nPitch);
      pt ->Draw();
    }
}

void GetCutCaption(TPaveText *pt, int yawId, int pitchId, int nYaw, int nPitch)
{
  double dPitch = 180./nPitch;
  double dYaw = 180./nYaw;
  pt -> AddText(TString::Format("%.1f^{o} < yaw < %.1f^{o}",
                                yawId*dYaw - 90., (yawId + 1)*dYaw - 90));
  pt -> AddText(TString::Format("%.1f^{o} < pitch < %.1f^{o}",
                                pitchId*dPitch - 90, (pitchId + 1)*dPitch - 90));
}

std::vector<std::vector<TPad*>> CanvasPartition(TCanvas *C,const Int_t Nx,const Int_t Ny,
                                                Float_t lMargin, Float_t rMargin,
                                                Float_t bMargin, Float_t tMargin,
                                                const std::string& xtitle, const std::string& ytitle)
{
   C -> SetLeftMargin(0);
   C -> SetRightMargin(0);
   C -> SetBottomMargin(0);
   C -> SetTopMargin(0);

   // Setup Pad layout:
   Float_t vSpacing = 0.0;
   Float_t vStep  = (1.- bMargin - tMargin - (Ny-1) * vSpacing) / Ny;

   Float_t hSpacing = 0.0;
   Float_t hStep  = (1.- lMargin - rMargin - (Nx-1) * hSpacing) / Nx;

   Float_t vposd,vposu,vmard,vmaru,vfactor;
   Float_t hposl,hposr,hmarl,hmarr,hfactor;

   std::vector<std::vector<TPad*>> pads;
   for (Int_t i=0;i<Nx;i++) {
      pads.push_back(std::vector<TPad*>());
      if (i==0) {
         hposl = 0.0;
         hposr = lMargin + hStep;
         hfactor = hposr-hposl;
         hmarl = lMargin / hfactor;
         hmarr = 0.0;
      } else {
         hposl = hposr + hSpacing;
         hposr = hposl + hStep;
         hfactor = hposr-hposl;
         hmarl = 0.0;
         hmarr = 0.0;
         if(i == Nx - 1) hmarr = 0.01;
      }

      for (Int_t j=0;j<Ny;j++) {

         if (j==0) {
            vposd = 0.0;
            vposu = bMargin + vStep;
            vfactor = vposu-vposd;
            vmard = bMargin / vfactor;
         } else {
            vposd = vposu + vSpacing;
            vposu = vposd + vStep;
            vfactor = vposu-vposd;
            vmard = 0.0;
            vmaru = 0.0;
         }

         C->cd(0);

         char name[16];
         sprintf(name,"pad_%i_%i",i,j);
         TPad *pad = (TPad*) gROOT->FindObject(name);
         if (pad) delete pad;
         pad = new TPad(name,"",hposl,vposd,hposr,vposu);
         pad->SetLeftMargin(hmarl);
         pad->SetRightMargin(hmarr);
         pad->SetBottomMargin(vmard);
         pad->SetTopMargin(vmaru);

         pad->SetFrameBorderMode(0);
         pad->SetBorderMode(0);
         pad->SetBorderSize(0);

         pad->SetLogz();
         pad->Draw();
         pads.back().push_back(pad);
      }
   }

   TLatex *xtex = new TLatex();
   xtex -> SetTextAlign(22);
   xtex -> SetTextSize(0.03); 
   xtex -> DrawLatexNDC(0.5, 0.1, xtitle.c_str());

   TLatex *ytex = new TLatex();
   ytex -> SetTextAlign(22);
   ytex -> SetTextSize(0.03);
   ytex -> SetTextAngle(90);
   ytex -> DrawLatexNDC(0.05, 0.5, ytitle.c_str());

   C->cd(0);
   xtex -> Draw();
   ytex -> Draw();
   return pads;
}
  
