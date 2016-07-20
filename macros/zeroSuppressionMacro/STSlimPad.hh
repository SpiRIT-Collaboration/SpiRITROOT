//
// STSlimPad container (portable)
//
// This structure is for storing zero suppressed pad data.
//
//        Author: Genie Jhang
// Start writing: 2016/05/18
//
// Usage: .L STSlimPad.hh+
//

#include "Rtypes.h"

#include <vector>

typedef struct STSlimPad {
  Short_t id;
  Float_t baseline;
  Float_t sigma;
  std::vector<Short_t> tb;
  std::vector<Float_t> adc;
} STSlimPad;

#ifdef __CLING__
#pragma link C++ class STSlimPad+;
#pragma link C++ class std::vector<Short_t>+;
#pragma link C++ class std::vector<Float_t>+;
#pragma link C++ class std::vector<STSlimPad>+;
#endif
