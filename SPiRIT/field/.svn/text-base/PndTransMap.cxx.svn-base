// -------------------------------------------------------------------------
// -----                      PndTransMap source file                -----
// -----                 Created 29/01/07  by M. Al/Turany             -----
// -------------------------------------------------------------------------

#include "FairRun.h"
#include "FairRuntimeDb.h"



#include "PndTransMap.h"
#include "PndTransPar.h"
#include "FairRunSim.h"
#include "TArrayF.h"


#include <iostream>
#include "stdlib.h"



using namespace std;
// -------------   Default constructor  ----------------------------------
PndTransMap::PndTransMap() 
 :PndFieldMap()
{ 
  fType = 4;
}
// ------------------------------------------------------------------------



// -------------   Standard constructor   ---------------------------------
PndTransMap::PndTransMap(const char* mapName, 
				 const char* fileType)
  : PndFieldMap(mapName, fileType) { 
  fType = 4;
  TString Suffix="";
  FairRunSim *fRun= FairRunSim::Instance();
  if(fRun){
    Double_t BeamMom= fRun->GetBeamMom();
    if(fRun->UseBeamMom() && BeamMom){
      if (BeamMom< 2.0 )Suffix=".0150" ;
      else if (BeamMom< 5.0 )Suffix=".0406";
      else if (BeamMom< 10.0 )Suffix=".0890" ;
      else if (BeamMom< 12.0 )Suffix=".1191";
      else  Suffix=".1500";
    }else{
      Suffix="";
    }
    TString NewName=mapName;
    NewName=mapName+Suffix;
    SetName(NewName.Data());
    TString dir = getenv("VMCWORKDIR");
    fFileName = dir + "/input/" + NewName;
    if ( fileType[0] == 'R' ) fFileName += ".root";
    else                      fFileName += ".dat";

  }

}
// ------------------------------------------------------------------------



// ------------   Constructor from PndFieldPar   --------------------------
PndTransMap::PndTransMap(PndTransPar* fieldPar) 
  : PndFieldMap() {
  fType = 4;
  fPosX  = fPosY  = fPosZ  = 0.;
  fXmin  = fYmin  = fZmin  = 0.;
  fXmax  = fYmax  = fZmax  = 0.;
  fXstep = fYstep = fZstep = 0.;
  fNx    = fNy    = fNz    = 0;
  fScale = 1.;
  funit = 10.0;
  fBx    = fBy    = fBz    = NULL;
  if ( ! fieldPar ) {
    cerr << "-W- PndTransMap::PndTransMap: empty parameter container!"
	 << endl;
    fName     = "";
    fFileName = "";
    fType     = 1;
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
PndTransMap::~PndTransMap() 
{
	//printf("PndTransMap::~PndTransMap() \n ");	
}
// ------------------------------------------------------------------------
void PndTransMap::GetBxyz(const Double_t point[3], Double_t* bField)
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
	
	//Bx is antisymtric in X 
	bField[0] =Interpolate(dx, dy, dz) * fHemiX ;

	// Get By field values at grid cell corners
	fHa[0][0][0] = fBy->At(ix    *fNy*fNz + iy    *fNz + iz);
	fHa[1][0][0] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + iz);
	fHa[0][1][0] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
	fHa[1][1][0] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
	fHa[0][0][1] = fBy->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
	fHa[1][0][1] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
	fHa[0][1][1] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
	fHa[1][1][1] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));
	
	//By is symtric in X
	bField[1] =Interpolate(dx, dy, dz);

        // Get Bz field values at grid cell corners
	fHa[0][0][0] = fBz->At(ix    *fNy*fNz + iy    *fNz + iz);
	fHa[1][0][0] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + iz);
	fHa[0][1][0] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
	fHa[1][1][0] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
	fHa[0][0][1] = fBz->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
	fHa[1][0][1] = fBz->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
	fHa[0][1][1] = fBz->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
	fHa[1][1][1] = fBz->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));
	
	// Return interpolated field value
	//Bz is symtric in X 
	bField[2]=Interpolate(dx, dy, dz);

  }else{
     bField[0]=0;
     bField[1]=0;
     bField[2]=0;
  }

}



// -----------   Check whether a point is inside the map   ----------------
Bool_t PndTransMap::IsInside(Double_t x, Double_t y, Double_t z,
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
 
  // ---  Check for being outside the map range
  if ( ! ( xl >= fXmin && xl <= fXmax && yl >= fYmin && yl <= fYmax &&
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
void PndTransMap::FillParContainer()
{
	TString MapName=GetName();
	//	cout << "PndConstField::FillParContainer() " << endl;
	FairRun *fRun=FairRun::Instance();	
	FairRuntimeDb *rtdb=fRun->GetRuntimeDb();
	Bool_t kParameterMerged=kTRUE;
	PndTransPar* Par = (PndTransPar*) rtdb->getContainer("PndTransPar");
	Par->SetParameters(this);
	Par->setInputVersion(fRun->GetRunId(),1);
	Par->setChanged();
	
}




ClassImp(PndTransMap)
