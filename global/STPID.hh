#ifndef STPID_HH
#define STPID_HH

#define NUMSTPID 6

#include "TObject.h"
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
      static std::map<PID, std::string> strings;
      if (strings.size() == 0) {
        strings[kNon]      = "Non";
        strings[kPion]     = "Pion";
        strings[kProton]   = "Proton";
        strings[kDeuteron] = "Deuteron";
        strings[kTriton]   = "Triton";
        strings[k3He]      = "3He";
        strings[k4He]      = "4He";
      }   
      return out << strings[value];
    }

    static Int_t GetNUMSTPID() { return NUMSTPID; } 

  ClassDef(STPID, 1)
};

#endif
