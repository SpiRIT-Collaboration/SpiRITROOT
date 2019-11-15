#ifndef STBBFunction_h
#define STBBFunction_h 1

#include "Rtypes.h"
#include "TMath.h"

class STBBFunction
{
	public:
		STBBFunction();
		~STBBFunction();

		Double_t fBBdedx     (Double_t z, Double_t m, Double_t *x, Double_t *p);
		Double_t fBBdedx_pi  (Double_t *x, Double_t *p);
		Double_t fBBdedx_p   (Double_t *x, Double_t *p);
		Double_t fBBdedx_d   (Double_t *x, Double_t *p);
		Double_t fBBdedx_t   (Double_t *x, Double_t *p);
		Double_t fBBdedx_3he (Double_t *x, Double_t *p);
		Double_t fBBdedx_4he (Double_t *x, Double_t *p);
		Double_t fBBdedx_6he (Double_t *x, Double_t *p);
		Double_t fBBdedx_6li (Double_t *x, Double_t *p);
		Double_t fBBdedx_7li (Double_t *x, Double_t *p);

		Double_t fBBMassFinderEq(Double_t *x, Double_t *p);
		Double_t fBBMassFinderDeriv(Double_t *x, Double_t *p);

	private:
		// particle masses in MeV
		static constexpr Double_t amu   = 931.478;
		static constexpr Double_t kmpi  = 139.57018;
		static constexpr Double_t kmp   = 938.2720813;
		static constexpr Double_t kmd   = 1875.612762;
		static constexpr Double_t kmt   = 2808.921112;
		static constexpr Double_t km3he = 2808.39132;
		static constexpr Double_t km4he = 3727.379378;
		static constexpr Double_t km6he = 6.0188*amu;
		static constexpr Double_t km6li = 6.0151*amu;
		static constexpr Double_t km7li = 7.016*amu;
		static constexpr Double_t kme   = 0.5109989461;
         
		// Bethe-Bloch and Landau-Vavilov dE/dx is described in the PDG chapter 33. Passage of particles through matter

		static constexpr Double_t K = 0.307075;  // = 4*pi*Na*re^2*me*c^2  [MeV*cm2/mol]

		// Absorber material: P10 (Ar+CH4)
		static constexpr Double_t rho_P10 = 1.534*1.e-3;  // [g/cm3]

		// Z: atomic number
		static constexpr Double_t Z_Ar  = 18.;
		static constexpr Double_t Z_C   = 6.;
		static constexpr Double_t Z_H   = 1.;
		static constexpr Double_t Z_eff = 0.9*Z_Ar + 0.1*(Z_C+4.*Z_H);     // effective atomic number of P10
		// A: atomic mass [g/mol]
		static constexpr Double_t A_Ar  = 39.95;
		static constexpr Double_t A_C   = 12.01;
		static constexpr Double_t A_H   = 1.00794;
		static constexpr Double_t A_eff = 0.9*A_Ar + 0.1*(A_C+4.*A_H);     // effective atomic mass of P10
		// I: mean excitation energy [eV], values are obtained from Atomic Data and Nuclear Dta Tables 30, 261 (1984)
		static constexpr Double_t I_Ar    = 188.0;
		static constexpr Double_t I_CH4   = 41.7;
		Double_t lnI_eff;
         
		// density correction
		Double_t delta_Ar(Double_t x);
		static constexpr Double_t C_Ar  = 11.948;
		static constexpr Double_t a_Ar  = 0.19714;
		static constexpr Double_t x0_Ar = 1.7635;
		static constexpr Double_t x1_Ar = 4.4855;
		static constexpr Double_t k_Ar  = 2.9618;

		Double_t delta_CH4(Double_t x);
		static constexpr Double_t C_CH4  = 9.5243;
		static constexpr Double_t a_CH4  = 0.09253;
		static constexpr Double_t x0_CH4 = 1.6263;
		static constexpr Double_t x1_CH4 = 3.9716;
		static constexpr Double_t k_CH4  = 3.6257;


		// Barkas correction
		Double_t BarkasCor(Double_t E);
		// Bloch correction
		Double_t BlochCor(Double_t y);
		static constexpr Double_t alpha = 1./137.;

};

#endif
