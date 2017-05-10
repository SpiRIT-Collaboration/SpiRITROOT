#ifndef STPID_HH
#define STPID_HH

#define NUMSTPID 6

#include "TObject.h"
#include "TString.h"
#include <map>

class STPID : public TObject {
  public:
    enum PID { 
      kNon = -1,
      kPion = 0,
      kProton = 1,
      kDeuteron = 2,
      kTriton = 3,
      k3He = 4,
      k4He = 5,
    };

    friend std::ostream& operator<<(std::ostream& out, const PID value) {
      static std::map<PID, std::string> particleNames;
      if (particleNames.size() == 0) {
        particleNames[kNon]      = "Non";
        particleNames[kPion]     = "Pion";
        particleNames[kProton]   = "Proton";
        particleNames[kDeuteron] = "Deuteron";
        particleNames[kTriton]   = "Triton";
        particleNames[k3He]      = "3He";
        particleNames[k4He]      = "4He";
      }   
      return out << particleNames[value];
    }

    static TString GetName(PID pid) {
      static std::map<PID, std::string> particleNames;
      if (particleNames.size() == 0) {
        particleNames[kNon]      = "Non";
        particleNames[kPion]     = "Pion";
        particleNames[kProton]   = "Proton";
        particleNames[kDeuteron] = "Deuteron";
        particleNames[kTriton]   = "Triton";
        particleNames[k3He]      = "3He";
        particleNames[k4He]      = "4He";
      }
      return TString(particleNames[pid].c_str());
    }

    static Int_t GetPDG(PID pid) {
      static std::map<PID, Int_t> particlePDG;
      if (particlePDG.size() == 0) {
        particlePDG[kNon]      = 0;
        particlePDG[kPion]     = 211;
        particlePDG[kProton]   = 2212;
        particlePDG[kDeuteron] = 1000010020;
        particlePDG[kTriton]   = 1000010030;
        particlePDG[k3He]      = 1000020030;
        particlePDG[k4He]      = 1000020040;
      }
      return particlePDG[pid];
    }

    static Int_t GetNUMSTPID() { return NUMSTPID; } 

  ClassDef(STPID, 1)
};

#endif
