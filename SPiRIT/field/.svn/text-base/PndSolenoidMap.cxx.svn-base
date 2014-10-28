#include <iostream>
#include "TArrayF.h"
#include "stdlib.h"
#include "PndSolenoidMap.h"
#include "PndMapPar.h"

Int_t PndSolenoidMap::fNumberOfRegions=0;

using namespace std;
// -------------   Default constructor  ----------------------------------
PndSolenoidMap::PndSolenoidMap()
  :PndFieldMap(),
   fRegionNo(0),
   fHemiX(0),
   fHemiY(0)
{ 
  fType = 2;
}
// ------------------------------------------------------------------------



// -------------   Standard constructor   ---------------------------------
PndSolenoidMap::PndSolenoidMap(const char* mapName, 
				 const char* fileType)
  : PndFieldMap(mapName, fileType), 
    fRegionNo(0),
    fHemiX(0),
    fHemiY(0)
{ 
  fType = 2;
  fNumberOfRegions++; 
  fRegionNo=fNumberOfRegions;
}
// ------------------------------------------------------------------------

// ------------   Constructor from PndFieldPar   --------------------------
PndSolenoidMap::PndSolenoidMap(PndMapPar* fieldPar) 
  : PndFieldMap(),
    fRegionNo(fNumberOfRegions++),
    fHemiX(0),
    fHemiY(0)
{
   fType = 2;
   if ( ! fieldPar ) {
    cerr << "-W- PndSolenoidMap::PndSolenoidMap: empty parameter container!"
	 << endl;
    fName     = "";
    fFileName = "";
    fType     = -1;
  }
  else {
    fieldPar->MapName(fName);
    fPosX  = fieldPar->GetPositionX();
    fPosY  = fieldPar->GetPositionY();
    fPosZ  = fieldPar->GetPositionZ();
    fScale = fieldPar->GetScale();
    TString dir = getenv("VMCWORKDIR");
    fFileName = dir + "/input/" + fName + ".root";
    fType = fieldPar->GetType();
  }
}
// ------------------------------------------------------------------------



// ------------   Destructor   --------------------------------------------
PndSolenoidMap::~PndSolenoidMap() 
{ 
}
// ------------------------------------------------------------------------
void PndSolenoidMap::GetBxyz(const Double_t point[3], Double_t* bField)
{ 
  Double_t x  =point[0];
  Double_t y  =point[1];
  Double_t z  =point[2];
  Int_t ix    = 0;
  Int_t iy    = 0;
  Int_t iz    = 0;
  Double_t dx = 0.;
  Double_t dy = 0.;
  Double_t dz = 0.;

  if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ){

      // Get Bx field values at grid cell corners
	fHa[0][0][0] = fBx->At(ix    *fNy*fNz + iy    *fNz + iz);
	fHa[1][0][0] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + iz);
	fHa[0][1][0] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
	fHa[1][1][0] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
	fHa[0][0][1] = fBx->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
	fHa[1][0][1] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
	fHa[0][1][1] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
	fHa[1][1][1] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));
	
	//Bx is antisymtric in X and symetric in Y
	bField[0] =Interpolate(dx, dy, dz) * fHemiX;
	// Get By field values at grid cell corners
	fHa[0][0][0] = fBy->At(ix    *fNy*fNz + iy    *fNz + iz);
	fHa[1][0][0] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + iz);
	fHa[0][1][0] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
	fHa[1][1][0] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
	fHa[0][0][1] = fBy->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
	fHa[1][0][1] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
	fHa[0][1][1] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
	fHa[1][1][1] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));
	
	//By is symtric in X and antisymetric in Y
	bField[1] = Interpolate(dx, dy, dz) * fHemiY;
	
         
        // Get Bz field values at grid cell corners
	fHa[0][0][0] = fBz->At(ix    *fNy*fNz + iy    *fNz + iz);
	fHa[1][0][0] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + iz);
	fHa[0][1][0] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
	fHa[1][1][0] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
	fHa[0][0][1] = fBz->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
	fHa[1][0][1] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
	fHa[0][1][1] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
	fHa[1][1][1] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));
	
	
	//Bz is symtric in X and Y
	bField[2] =Interpolate(dx, dy, dz) ;

  }else{
     bField[0]=0;
     bField[1]=0;
     bField[2]=0;
  }

}

// -----------   Check whether a point is inside the map   ----------------
Bool_t PndSolenoidMap::IsInside(Double_t x, Double_t y, Double_t z,
				 Int_t& ix, Int_t& iy, Int_t& iz,
				 Double_t& dx, Double_t& dy, 
				 Double_t& dz) {

  // --- Transform into local coordinate system
  Double_t xl = x - fPosX;
  Double_t yl = y - fPosY;
  Double_t zl = z - fPosZ;

  // ---  Reflect w.r.t. symmetry axes
  fHemiX = fHemiY = 1.;
  if ( xl < 0. ) {
    fHemiX = -1.;
    xl     = -1. * xl;
  }
  if ( yl < 0. ) {
    fHemiY = -1.;
    yl     = -1. * yl;
  }

  // ---  Check for being outside the map range
  if ( ! ( xl >= 0 && xl <= fXmax && yl >= 0 && yl <= fYmax &&
	   zl >= fZmin && zl <= fZmax ) ) {
    ix = iy = iz = 0;
    dx = dy = dz = 0.;
    return kFALSE;
  }
 
  // --- Determine grid cell
  ix = Int_t( (xl-fXmin) / fXstep );
  iy = Int_t( (yl-fYmin) / fYstep );
  iz = Int_t( (zl-fZmin) / fZstep );


  // Relative distance from grid point (in units of cell size)
  dx = (xl-fXmin) / fXstep - Double_t(ix);
  dy = (yl-fYmin) / fYstep - Double_t(iy);
  dz = (zl-fZmin) / fZstep - Double_t(iz);

  return kTRUE;

}
// ------------------------------------------------------------------------
void PndSolenoidMap::FillParContainer()
{
	TString MapName=GetName();
	cout << "PndSolenoidMap::FillParContainer() " << endl;
	
}
ClassImp(PndSolenoidMap)
