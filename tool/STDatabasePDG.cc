#include "STDatabasePDG.hh"

STDatabasePDG* STDatabasePDG::fInstance = nullptr;
STDatabasePDG* STDatabasePDG::Instance() 
{
  if (fInstance == nullptr) fInstance = new STDatabasePDG();
  return fInstance;
}

STDatabasePDG::STDatabasePDG() 
{
  const Double_t kAu2Gev = 0.9314943228;
  const Double_t khSlash = 1.0545726663e-27;
  const Double_t kErg2Gev = 1/1.6021773349e-3;
  const Double_t khShGev = khSlash*kErg2Gev;
  const Double_t kYear2Sec = 3600*24*365.25;

  //AddParticle (const char *Name,
  //             const char *Title,
  //             Double_t Mass,
  //             Bool_t Stable,
  //             Double_t DecayWidth,
  //             Double_t Charge,
  //             const char *ParticleClass,
  //             Int_t PdgCode,
  //             Int_t Anti=-1,
  //             Int_t TrackingCode=0)
  TDatabasePDG *db = TDatabasePDG::Instance();
  db -> AddParticle("Deuteron", "Deuteron", 2*kAu2Gev+8.071e-3,  kTRUE,  0,                         3,  "Ion", 1000010020);
  db -> AddParticle("Triton",   "Triton",   3*kAu2Gev+14.931e-3, kFALSE, khShGev/(12.33*kYear2Sec), 3,  "Ion", 1000010030);
  db -> AddParticle("Alpha",    "Alpha",    4*kAu2Gev+2.424e-3,  kTRUE,  khShGev/(12.33*kYear2Sec), 6,  "Ion", 1000020040);
  db -> AddParticle("HE3",      "HE3",      3*kAu2Gev+14.931e-3, kFALSE, 0,                         6,  "Ion", 1000020030);
  db -> AddParticle("Sn108",    "Sn108",  107.911925*kAu2Gev,    kFALSE, khShGev/(10.30*60),        50, "Ion", 1000501080);
  db -> AddParticle("Sn112",    "Sn112",  111.904818*kAu2Gev,    kTRUE,  0,                         50, "Ion", 1000501120);
  db -> AddParticle("Sn124",    "Sn124",  123.9052739*kAu2Gev,   kTRUE,  0,                         50, "Ion", 1000501240);
  db -> AddParticle("Sn132",    "Sn132",  131.917816*kAu2Gev,    kFALSE, khShGev/(39.7),            50, "Ion", 1000501320);

  fPDGCandidateArray.push_back(2212);
  //fPDGCandidateArray.push_back(11);
  fPDGCandidateArray.push_back(211);
  //fPDGCandidateArray.push_back(-211);
  fPDGCandidateArray.push_back(1000010020);
  fPDGCandidateArray.push_back(1000010030);
  //  fPDGCandidateArray.push_back(1000020030);
  //  fPDGCandidateArray.push_back(1000020040);
}

void STDatabasePDG::AddPDGCandidate(Int_t pdg) { fPDGCandidateArray.push_back(pdg); }
std::vector<Int_t> *STDatabasePDG::GetPDGCandidateArray() { return &fPDGCandidateArray; }


