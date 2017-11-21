#ifndef STBOOTSTRAP_HH
#define STBOOTSTRAP_HH


#include "TRandom3.h"
#include "TVector2.h"
#include "TMath.h"
#include <iostream>

class STBootStrap : public TObject {

public:
  STBootStrap(){};
  STBootStrap(UInt_t ival1, UInt_t ival2, Double_t *sample);
  STBootStrap(UInt_t ival1, UInt_t ival2, TVector2 *sample);
  STBootStrap(UInt_t ival1, std::vector<TVector2> *sample);
  ~STBootStrap(){};

  void clear();
  void Initialize();
  
  Double_t GetPhiOriginal() {return TVector2::Phi_mpi_pi( phi_mean );}

  Double_t GetMean(UInt_t idx);
  Double_t GetCosMean(UInt_t idx);
  Double_t GetStdDev(UInt_t idx);
  Double_t GetStdDevError(UInt_t idx);
  Double_t GetStdDev2(UInt_t idx);
  Double_t GetNElem(){return (Double_t)numElements;}

  const UInt_t nidx = 3;

  std::vector< Double_t > GetMeanVector(UInt_t idx)      {if(idx < nidx) return resMean[idx]; else return resMean[0];}
  std::vector< Double_t > GetStdDevVector(UInt_t idx)    {if(idx < nidx) return resStdv[idx]; else return resStdv[0];}

  std::vector< Double_t > GetMeanCnvVector(UInt_t idx)   {if(idx < nidx) return cnvMean[idx];    else return cnvMean[0];}
  std::vector< Double_t > GetCosMeanVector(UInt_t idx)   {if(idx < nidx) return cnvCosMean[idx]; else return cnvCosMean[0];}
  std::vector< Double_t > GetStdDevCnvVector(UInt_t idx) {if(idx < nidx) return cnvStdv[idx];    else return cnvStdv[0];}
  std::vector< Double_t > GetStdDev2CnvVector(UInt_t idx){if(idx < nidx) return cnvStdv2[idx];   else return cnvStdv2[0];}


  void StoreResults(UInt_t idx = 0, Double_t off = 0 );

  std::vector< UInt_t > Resampling(UInt_t ival);
  void SumUpVector();

private:


  std::vector< Double_t > elements;
  std::vector< TVector2 > elementsTV2;

  UInt_t numElements;
  Double_t     phi_mean;

  UInt_t nboot = 0;
  std::vector< std::vector< Double_t > >   replace;    // resampling event
  std::vector< std::vector< Double_t > >   resMean;    // <Phi> / resampling event
  std::vector< std::vector< Double_t > >   resStdv;    // std<Phi> / resampling event
  std::vector< std::vector< Double_t > >   cnvMean;    // <Phi> for bootstrapped events
  std::vector< std::vector< Double_t > >   cnvStdv;    // std<Phi> for bootstrapped events
  std::vector< std::vector< Double_t > >   cnvCosMean; // cos(std_Phi) for bootstrapped events
  std::vector< std::vector< Double_t > >   cnvStdv2;   // std<std<Phi>> for bootstrapped events

  TRandom3 rnd;

};

#endif
