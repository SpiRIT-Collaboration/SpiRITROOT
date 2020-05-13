#ifndef STMassCalSimpleBB_h
#define STMassCalSimpleBB_h 1

#include "STBBFunction.hh"
#include "TObjArray.h"
#include "TGraph2D.h"
#include "TH2D.h"
#include "TVector3.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"
#include "functional"

class STMassCalSimpleBB
{
	public:
		STMassCalSimpleBB();
		STMassCalSimpleBB(TString funcName);
		~STMassCalSimpleBB();

		void UseBetheBloch();
		void UseSimpleBB();
		void UseEmpiricalBB();

		void AddParameter(TGraph2D*);
		void AddParameters(TGraph2D**);
		void AddParameter(TH2D*);
		void AddParameters(TH2D**);

		Double_t CalcMass(Double_t z,TVector3 mom,Double_t dEdx,Double_t *par);
		Double_t CalcMass(Double_t z,TVector3 mom,Double_t dEdx,Bool_t isInterpolate=kTRUE);
		Double_t CalcZLog(Int_t pid,TVector3 mom,Double_t dEdx,Bool_t isInterpolation=kTRUE);

	private:
		ROOT::Math::RootFinder finder;
		STBBFunction bbfunc;
		std::function<double(double)> funcBB;
		std::function<double(double)> dfuncBB;
		ROOT::Math::GradFunctor1D *gradfunc1dBB;
		Int_t     nPar;
		Double_t  *bbPar;
		TObjArray *bbParArray;
		TObjArray *bbFitArray;

};
#endif
