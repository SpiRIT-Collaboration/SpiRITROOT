#ifndef STSAMPLEPOINT
#define STSAMPLEPOINT

#include "Rtypes.h"
#include "TString.h"
#include <string>
#include <sstream>

class STSamplePoint
{
  public:
    STSamplePoint();
    STSamplePoint(STSamplePoint &sample);
    STSamplePoint(Double_t v, Double_t rms = 0, Double_t w = 1);
    ~STSamplePoint();

    void Print();
    void Init(Double_t v, Double_t rms = 0, Double_t w = 1);
    void Init(std::string line);
    void Update(Double_t v, Double_t w = 1);

    TString GetSummary();

    Double_t fValue;      ///< Value of the sample point
    Double_t fRMS;        ///< RMS of the value
    Double_t fWeightSum;  ///< Sum of weights from the updates

  ClassDef(STSamplePoint, 1)
};

#endif
