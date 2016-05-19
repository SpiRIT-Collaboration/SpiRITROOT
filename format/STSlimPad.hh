//
// STSlimPad container
//
// This structure is for storing zero suppressed pad data.
//
//        Author: Genie Jhang
// Start writing: 2016/05/18
//

#ifndef STSLIMPAD
#define STSLIMPAD

#include "Rtypes.h"

#include <vector>

typedef struct STSlimPad {
  Short_t id;
  Float_t baseline;
  Float_t sigma;
  std::vector<Short_t> tb;
  std::vector<Float_t> adc;
} STSlimPad;

#endif
