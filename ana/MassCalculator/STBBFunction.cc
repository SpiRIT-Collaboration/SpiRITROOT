#include "STBBFunction.hh"

STBBFunction::STBBFunction()
{
	lnI_eff = (0.9*Z_Ar*TMath::Log(I_Ar*1.e-6) + 0.1*(Z_C+Z_H*4.)*TMath::Log(I_CH4*1.e-6))/Z_eff;
}

STBBFunction::~STBBFunction()
{
}

Double_t STBBFunction::delta_Ar(Double_t x)
{
	Double_t d_Ar=0.;
	if(x<x0_Ar)                d_Ar = 0.;
	else if(x>=x0_Ar&&x<x1_Ar) d_Ar = 2.*TMath::Log(10)*x - C_Ar + a_Ar*TMath::Power(x1_Ar-x,k_Ar);
	else if(x>=x1_Ar)          d_Ar = 2.*TMath::Log(10)*x - C_Ar;
	return d_Ar;
}

Double_t STBBFunction::delta_CH4(Double_t x)
{
	Double_t d_CH4=0.;
	if(x<x0_CH4)                 d_CH4 = 0.;
	else if(x>=x0_CH4&&x<x1_CH4) d_CH4 = 2.*TMath::Log(10)*x - C_CH4 + a_CH4*TMath::Power(x1_CH4-x,k_CH4);
	else if(x>=x1_CH4)           d_CH4 = 2.*TMath::Log(10)*x - C_CH4;
	return d_CH4;
}

Double_t STBBFunction::BarkasCor(Double_t E)
{
	Double_t L_low = 0.001*E;
	Double_t L_high = 1.5/TMath::Power(E,0.4)+45000./(Z_eff*TMath::Power(E,1.6));
	return L_low*L_high/(L_low+L_high);
}

Double_t STBBFunction::BlochCor(Double_t y)
{ // y = z alpha/beta;
	return -y*y*(1.202-y*y*(1.042-0.855*y*y+0.343*y*y*y*y));
}


Double_t STBBFunction::fBBdedx(Double_t z, Double_t m, Double_t *x, Double_t *p)
{
	// variable: x[0] = rigidity magnitude
	// parameters:
	// 0,1 for dEdx, par[0] = normalization [keV/cm]->[ADC/mm], par[1] = offset
	// 2,3 for mom,  par[2] = norm, par[3] = offset
	x[0] = p[2]*x[0]+p[3];
	Double_t mom   = x[0]*z;
	Double_t b2    = mom*mom/(mom*mom+m*m);
	Double_t beta  = TMath::Sqrt(b2);
	Double_t g2    = 1./(1.-b2);
	Double_t gamma = TMath::Sqrt(g2);
	Double_t Wmax  = 2.*kme*b2*g2/(1.+2.*gamma*kme/m+TMath::Power(kme/m,2.));

	Double_t X     = TMath::Log10(TMath::Sqrt(b2*g2));
	Double_t d_Ar  = delta_Ar(X);
	Double_t d_CH4 = delta_CH4(X);
	Double_t delta_eff = (0.9*Z_Ar*d_Ar+0.1*(Z_C+Z_H*4.)*d_CH4)/Z_eff;

	Double_t E  = 1000.*(TMath::Sqrt(mom*mom+m*m)-m)/(m/amu);
	Double_t L1 = BarkasCor(E);
	Double_t y  = z*alpha/beta;
	Double_t L2 = BlochCor(y);

	// Bethe-Bloch dedx [keV/cm]
	Double_t dedx = 1000.*K*rho_P10*z*z*Z_eff/A_eff/b2*(0.5*TMath::Log(2.*kme*b2*g2*Wmax)-lnI_eff-b2-0.5*delta_eff+L1+L2);

	return p[0]*dedx+p[1];
}

Double_t STBBFunction::fBBdedx_pi(Double_t *x, Double_t *p)  { return fBBdedx(1,kmpi,x,p); }
Double_t STBBFunction::fBBdedx_p(Double_t *x, Double_t *p)   { return fBBdedx(1,kmp,x,p); }
Double_t STBBFunction::fBBdedx_d(Double_t *x, Double_t *p)   { return fBBdedx(1,kmd,x,p); }
Double_t STBBFunction::fBBdedx_t(Double_t *x, Double_t *p)   { return fBBdedx(1,kmt,x,p); }
Double_t STBBFunction::fBBdedx_3he(Double_t *x, Double_t *p) { return fBBdedx(2,km3he,x,p); }
Double_t STBBFunction::fBBdedx_4he(Double_t *x, Double_t *p) { return fBBdedx(2,km4he,x,p); }
Double_t STBBFunction::fBBdedx_6he(Double_t *x, Double_t *p) { return fBBdedx(2,km6he,x,p); }
Double_t STBBFunction::fBBdedx_6li(Double_t *x, Double_t *p) { return fBBdedx(3,km6li,x,p); }
Double_t STBBFunction::fBBdedx_7li(Double_t *x, Double_t *p) { return fBBdedx(3,km7li,x,p); }

Double_t STBBFunction::fBBMassFinderEq(Double_t *x, Double_t *p)
{
	// f(x)=0 equation for root-finder
	// in this case, dedx_measured - dedx_calculated = 0 where 1d-variable is mass
	// parameters:
	// 0-3 same as fBBdedx
	// par[4] = z(charge), par[5] = rigidity magnitude, par[6] = measured dedx;
	Double_t m    = x[0];
	Double_t z    = p[4];
	Double_t R    = p[5];
	Double_t dedx_calc = fBBdedx(z,m,&R,p);
	Double_t dedx_meas = p[6];
	return dedx_meas - dedx_calc;
}

Double_t STBBFunction::fBBMassFinderDeriv(Double_t *x, Double_t *p)
{
	Double_t dx = 0.1;
	Double_t x0 = x[0]-dx/2.;
	Double_t x1 = x[0]+dx/2.;
	return ( fBBMassFinderEq(&x1,p) - fBBMassFinderEq(&x0,p) )/dx;
}
