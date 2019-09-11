// -------------------------------------------------------------------------
// -----                      STFieldMap source file                  -----
// -----         Created 12/01/04  by M. Al/Turany (FairField.cxx)      -----
//  -----          Edited on 25/10/2014  by Prabi for  Samurai Map     -----
// -------------------------------------------------------------------------


#include <iomanip>
#include <iostream>
#include <fstream>
#include "stdlib.h"
#include "FairLogger.h"

#include "TArrayD.h"
#include "TFile.h"
#include "TMath.h"

#include "STFieldMap.hh"
#include "STFieldMapData.hh"
#include "STFieldPar.hh"

using namespace std;

// -------------   Default constructor  ----------------------------------
STFieldMap::STFieldMap() 
  : FairField(),
	fFileName(""),
    fScale(1.0),
    funit(10.0),
    fPosX(0), fPosY(0), fPosZ(0),
    fXmin(0), fXmax(0), fXstep(0),
	fYmin(0), fYmax(0), fYstep(0),
    fZmin(0), fZmax(0), fZstep(0),
    fNx(0),fNy(0),fNz(0),
    fBx(NULL), fBy(NULL), fBz(NULL),
    fLoaded(kFALSE)   
{
  fLogger = FairLogger::GetLogger();
  SetName("");
  fType = 1;
}
// ------------------------------------------------------------------------



// -------------   Standard constructor   ---------------------------------
STFieldMap::STFieldMap(const char* mapName, const char* fileType)
  : FairField(mapName),
    fFileName(TString("")),
    fScale(1.0),
    funit(10.0),
    fPosX(0), fPosY(0), fPosZ(0),
    fXmin(0), fXmax(3000), fXstep(10),
    fYmin(-400), fYmax(400), fYstep(10),
    fZmin(0), fZmax(3000), fZstep(10),
    fNx(301),fNy(81),fNz(301),
    fBx(NULL), fBy(NULL), fBz(NULL),
    fLoaded(kFALSE)
{
  fLogger = FairLogger::GetLogger();
  SetName(mapName);
  TString dir = getenv("VMCWORKDIR");
  fFileName = dir + "/input/" + mapName;
  if ( fileType[0] == 'R' ) fFileName += ".root";
  else                      fFileName += ".dat";
  fType = 1;
  cout << fFileName << endl;
}
// ------------------------------------------------------------------------



// ------------   Constructor from STFieldPar   --------------------------
STFieldMap::STFieldMap(STFieldPar* fieldPar) 
  : FairField(),
	fFileName(TString("")),
    fScale(1.0),
    funit(10.0),
    fPosX(0), fPosY(0), fPosZ(0),
    fXmin(0), fXmax(0), fXstep(0),
    fYmin(0), fYmax(0), fYstep(0),
    fZmin(0), fZmax(0), fZstep(0),
    fNx(0),fNy(0),fNz(0),
    fBx(NULL), fBy(NULL), fBz(NULL), 
    fLoaded(kFALSE)
{
  fLogger = FairLogger::GetLogger();
  fType = 1;
  if ( ! fieldPar ) {
//FairLogger *fLogger;
    fLogger->Error(MESSAGE_ORIGIN, "STConstField::STConstField: empty parameter container!");
    SetName("");
	fType     = -1;
  }
  else {
    TString Name=GetName();
    fieldPar->MapName(Name);
    fPosX  = fieldPar->GetPositionX();
    fPosY  = fieldPar->GetPositionY();
    fPosZ  = fieldPar->GetPositionZ();
    fScale = fieldPar->GetScale();
    TString dir = getenv("VMCWORKDIR");
    fFileName = dir + "/input/" + Name + ".root";
    fType = fieldPar->GetType();
  }
}
// ------------------------------------------------------------------------



// ------------   Destructor   --------------------------------------------
STFieldMap::~STFieldMap() {
	
	//printf("STFieldMap::~STFieldMap() \n");
  if ( fBx ) delete fBx;
  if ( fBy ) delete fBy;
  if ( fBz ) delete fBz;
}
// ------------------------------------------------------------------------



// -----------   Intialisation   ------------------------------------------
void STFieldMap::Init() {
  if(!fLoaded)
  {
    if      (fFileName.EndsWith(".root")) ReadRootFile(fFileName, fName);
    else if (fFileName.EndsWith(".dat"))  ReadAsciiFile(fFileName);
    else {
      fLogger->Error(MESSAGE_ORIGIN,"-E- STFieldMap::Init: No proper file name defined! (%s) ");
      Fatal("Init", "No proper file name");
    }
    fLoaded = kTRUE;
  } 
  else fLogger->Info(MESSAGE_ORIGIN, "-Info- STFieldMap::Init: File map is loaded");
}
// ------------------------------------------------------------------------



// -----------   Get x component of the field   ---------------------------
Double_t STFieldMap::GetBx(Double_t x, Double_t y, Double_t z) {

  Int_t ix    = 0;
  Int_t iy    = 0;
  Int_t iz    = 0;
  Int_t af    = x < fPosX ? -1 : 1; 
  Double_t dx = 0.;
  Double_t dy = 0.;
  Double_t dz = 0.;

  if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

  // Get Bx field values at grid cell corners
  fHa[0][0][0] = fBx->At(ix    *fNy*fNz + iy    *fNz + iz);
  fHa[1][0][0] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + iz);
  fHa[0][1][0] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
  fHa[1][1][0] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
  fHa[0][0][1] = fBx->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
  fHa[1][0][1] = fBx->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
  fHa[0][1][1] = fBx->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
  fHa[1][1][1] = fBx->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

  // Return interpolated field value
  return af*Interpolate(dx, dy, dz);

  }

  return 0.;
}
// ------------------------------------------------------------------------



// -----------   Get y component of the field   ---------------------------
Double_t STFieldMap::GetBy(Double_t x, Double_t y, Double_t z) {

  Int_t ix    = 0;
  Int_t iy    = 0;
  Int_t iz    = 0;
  Double_t dx = 0.;
  Double_t dy = 0.;
  Double_t dz = 0.;

  if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

  // Get By field values at grid cell corners
  fHa[0][0][0] = fBy->At(ix    *fNy*fNz + iy    *fNz + iz);
  fHa[1][0][0] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + iz);
  fHa[0][1][0] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + iz);
  fHa[1][1][0] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + iz);
  fHa[0][0][1] = fBy->At(ix    *fNy*fNz + iy    *fNz + (iz+1));
  fHa[1][0][1] = fBy->At((ix+1)*fNy*fNz + iy    *fNz + (iz+1));
  fHa[0][1][1] = fBy->At(ix    *fNy*fNz + (iy+1)*fNz + (iz+1));
  fHa[1][1][1] = fBy->At((ix+1)*fNy*fNz + (iy+1)*fNz + (iz+1));

  // Return interpolated field value
  return Interpolate(dx, dy, dz);

  }

  return 0.;
}
// ------------------------------------------------------------------------



// -----------   Get z component of the field   ---------------------------
Double_t STFieldMap::GetBz(Double_t x, Double_t y, Double_t z) {

  Int_t ix    = 0;
  Int_t iy    = 0;
  Int_t iz    = 0;
  Int_t af    = z < fPosZ ? -1 : 1; 
  Double_t dx = 0.;
  Double_t dy = 0.;
  Double_t dz = 0.;

  if ( IsInside(x, y, z, ix, iy, iz, dx, dy, dz) ) {

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
  return af*Interpolate(dx, dy, dz);

  }

  return 0.;
}
// ------------------------------------------------------------------------



// -----------   Check whether a point is inside the map   ----------------
Bool_t STFieldMap::IsInside(Double_t x, Double_t y, Double_t z,
			     Int_t& ix, Int_t& iy, Int_t& iz,
			     Double_t& dx, Double_t& dy, Double_t& dz) {

  // --- Transform into local coordinate system
  Double_t xl = (x - fPosX)*10.;
  Double_t yl = (y - fPosY)*10.;
  Double_t zl = (z - fPosZ)*10.;

  xl=abs(xl);
  zl=abs(zl);

  // ---  Check for being outside the map range
  if ( ! ( xl >= fXmin && xl < fXmax && yl >= fYmin && yl < fYmax &&
	   zl >= fZmin && zl < fZmax ) ) {
    ix = iy = iz = 0;
    dx = dy = dz = 0.;
    return kFALSE;
  }

  xl = xl - fXmin;
  yl = yl - fYmin;
  zl = zl - fZmin;
 
  // --- Determine grid cell
  ix = Int_t( xl / fXstep );
  iy = Int_t( yl / fYstep );
  iz = Int_t( zl / fZstep );



  // Relative distance from grid point (in units of cell size)
  dx = xl / fXstep - Double_t(ix);
  dy = yl / fYstep - Double_t(iy);
  dz = zl / fZstep - Double_t(iz);

  return kTRUE;

}
// ------------------------------------------------------------------------



// ----------   Write the map to an ASCII file   --------------------------
void STFieldMap::WriteAsciiFile(const char* fileName) {

  // Open file
  fLogger->Info(MESSAGE_ORIGIN, "STFieldMap: Writing field map to ASCII file %s ",fileName);
  ofstream mapFile(fileName);
  if ( ! mapFile.is_open() ) {
    fLogger->Fatal(MESSAGE_ORIGIN, "STFieldMap:ReadAsciiFile: Could not open file! ");
    return;
  }

  // Write field map grid parameters
  mapFile.precision(4);
  mapFile << showpoint;
  if ( fType == 1 ) mapFile << "nosym" << endl;
  if ( fType == 2 ) mapFile << "Solenoid" << endl;
  if ( fType == 3 ) mapFile << "Dipole" << endl;
  if ( fType == 4 ) mapFile << "Trans" << endl;
  if ( funit == 10.0  ) mapFile << "T" << endl;
  if ( funit == 0.001 ) mapFile << "G" << endl;
  if ( funit == 1.0   ) mapFile << "kG" << endl;

  mapFile << fXmin << " " << fXmax << " " << fNx << endl;
  mapFile << fYmin << " " << fYmax << " " << fNy << endl;
  mapFile << fZmin << " " << fZmax << " " << fNz << endl;

  // Write field values
  Double_t factor = funit * fScale;  // Takes out scaling 
  cout << right;
  Int_t nTot = fNx * fNy * fNz;
  cout << "-I- STFieldMap: " << fNx*fNy*fNz << " entries to write... " 
       << setw(3) << 0 << " % ";
  Int_t index=0;
  div_t modul;
  Int_t iDiv = TMath::Nint(nTot/100.);
  for(Int_t ix=0; ix<fNx; ix++) {
    for(Int_t iy=0; iy<fNy; iy++) {
      for(Int_t iz=0; iz<fNz; iz++) {
	index =ix*fNy*fNz + iy*fNz + iz;
	modul = div(index,iDiv);
	if ( modul.rem == 0 ) {
	  Double_t perc = TMath::Nint(100.*index/nTot);
	  cout << "\b\b\b\b\b\b" << setw(3) << perc << " % " << flush;
	}
	mapFile << fBx->At(index)/factor << " " << fBy->At(index)/factor 
		<< " " << fBz->At(index)/factor << endl;
      } // z-Loop
    }   // y-Loop
  }     // x-Loop
  cout << "   " << index+1 << " written" << endl;
  mapFile.close();		

}	
// ------------------------------------------------------------------------



// -------   Write field map to a ROOT file   -----------------------------
void STFieldMap::WriteRootFile(const char* fileName,
				const char* mapName) {

  STFieldMapData* data = new STFieldMapData(mapName, *this);
  TFile* oldFile = gFile;
  TFile* file = new TFile(fileName, "RECREATE");
  data->Write();
  file->Close();
  if(oldFile) oldFile->cd();

}
// ------------------------------------------------------------------------



// -----  Set the position of the field centre in global coordinates  -----
void STFieldMap::SetPosition(Double_t x, Double_t y, Double_t z) {
  fPosX = x;
  fPosY = y;
  fPosZ = z;
}
// ------------------------------------------------------------------------



// ---------   Screen output   --------------------------------------------
void STFieldMap::Print() {
  TString type = "Map";
  if ( fType == 2 ) type = "Soleniod Map ";
  if ( fType == 3 ) type = "Dipole Map ";
  if ( fType == 4 ) type = "Trans Map ";
  cout << "======================================================" << endl;
  cout.precision(4);
  cout << showpoint;
  cout << "----  " << fTitle << " : " << fName << endl;
  cout << "----" << endl;
  cout << "----  Field type     : " << type << endl;
  cout << "----" << endl;
  cout << "----  Field map grid : " << endl;
  cout << "----  x = " << setw(4) << fXmin << " to " << setw(4) << fXmax 
       << " mm, " << fNx << " grid points, dx = " << fXstep << " mm" << endl;
  cout << "----  y = " << setw(4) << fYmin << " to " << setw(4) << fYmax 
       << " mm, " << fNy << " grid points, dy = " << fYstep << " mm" << endl;
  cout << "----  z = " << setw(4) << fZmin << " to " << setw(4) << fZmax 
       << " mm, " << fNz << " grid points, dz = " << fZstep << " mm" << endl;
  cout << endl;
  cout << "----  Field centre position: ( " << setw(6) << fPosX << ", "
       << setw(6) << fPosY << ", " << setw(6) << fPosZ << ") cm" << endl;
  cout << "----  Field scaling factor: " << fScale << endl;
  Double_t bx = GetBx(0.,0.,0.);
  Double_t by = GetBy(0.,0.,0.);
  Double_t bz = GetBz(0.,0.,0.);
  cout << "----" << endl;
  cout << "----  Field at origin is ( " << setw(6) << bx << ", " << setw(6)
       << by << ", " << setw(6) << bz << ") kG" << endl;
 cout << "======================================================" << endl;
}
// ------------------------------------------------------------------------  



// ---------    Reset parameters and data (private)  ----------------------
void STFieldMap::Reset() {
  fPosX = fPosY = fPosZ = 0.;
  fXmin = fYmin = fZmin = 0.;
  fXmax = fYmax = fZmax = 0.;
  fXstep = fYstep = fZstep = 0.;
  fNx = fNy = fNz = 0;
  fScale = 1.;
  funit = 10.0;
  if ( fBx ) { delete fBx; fBx = NULL; }
  if ( fBy ) { delete fBy; fBy = NULL; }
  if ( fBz ) { delete fBz; fBz = NULL; }
}
// ------------------------------------------------------------------------  



// -----   Read field map from ASCII file (private)   ---------------------
void STFieldMap::ReadAsciiFile(const char* fileName) {

  Double_t bx = 0, by = 0, bz = 0;
  Double_t xx = 0, yy = 0, zz = 0;

  // Open file
  cout << "-I- STFieldMap: Reading field map from ASCII file " << fileName << endl;
  ifstream mapFile(fileName);
  if ( ! mapFile.is_open() ) {
    cerr << "-E- STFieldMap:ReadAsciiFile: Could not open file! " << endl;
    Fatal("ReadAsciiFile","Could not open file");
  }

  for (int i = 0; i < 8; i++)
    mapFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  fNx = 301;
  fNy = 81;
  fNz = 301;
  fXmin = 0; fXmax= 3000;
  fYmin = -400; fYmax= 400;
  fZmin = 0; fZmax= 3000;

  fXstep = ( fXmax - fXmin ) / Double_t( fNx - 1 );
  fYstep = ( fYmax - fYmin ) / Double_t( fNy - 1 );
  fZstep = ( fZmax - fZmin ) / Double_t( fNz - 1 );

  fBx = new TArrayD(fNx * fNy * fNz);
  fBy = new TArrayD(fNx * fNy * fNz);
  fBz = new TArrayD(fNx * fNy * fNz);

  Int_t nTot = fNx * fNy * fNz;

  Int_t index = 0;
  div_t modul;
  Int_t iDiv = TMath::Nint(nTot/100.);
  for (Int_t ix=0; ix<fNx; ix++) {
    for (Int_t iy = 0; iy<fNy; iy++) {
      for (Int_t iz = 0; iz<fNz; iz++) {
	if (! mapFile.good()) cerr << "-E- STFieldMap::ReadAsciiFile: " << "I/O Error at " << ix << " " << iy << " " << iz << endl;

	index = ix*fNy*fNz + iy*fNz + iz;
	modul = div(index,iDiv);
	if ( modul.rem == 0 ) {
	  Double_t perc = TMath::Nint(100.*index/nTot);
	  cout << "\b\b\b\b\b\b" << setw(3) << perc << " % " << flush;
	}
	mapFile >> xx >> yy >> zz >>  bx >> by >> bz ;
	bx = TMath::Abs(bx) > 1e-10 ? bx : 0;
  by = TMath::Abs(by) > 1e-10 ? by : 0;
  bz = TMath::Abs(bz) > 1e-10 ? bz : 0;
	fBx->AddAt(bx*funit, index);
	fBy->AddAt(by*funit, index);
	fBz->AddAt(bz*funit, index);

	if ( mapFile.eof() ) {
	  cerr << endl << "-E- STFieldMap::ReadAsciiFile: EOF reached at " << ix << " " << iy << " " << iz << endl;
	  mapFile.close();
	  break;
	}

      }   // z-Loop
    }     // y-Loop
  }       // x-Loop

  cout << "   " << index+1 << " read" << endl;
  cout << "B field map loaded!" << endl;
  mapFile.close();

}
/*
void STFieldMap::ReadAsciiFile(const char* fileName) {

  Double_t bx=0., by=0., bz=0.;
  Double_t  xx, yy, zz;
  // Open file
  cout << "-I- STFieldMap: Reading field map from ASCII file " 
       << fileName << endl;
  ifstream mapFile(fileName);
  if ( ! mapFile.is_open() ) {
    cerr << "-E- STFieldMap:ReadAsciiFile: Could not open file! " << endl;
    Fatal("ReadAsciiFile","Could not open file");
  }

  // Read map type
  TString type;
  mapFile >> type;

  Int_t iType = 0;
  if ( type == "nosym" ) iType = 1;
  if ( type == "Solenoid") iType = 2;
  if ( type == "Dipole"  ) iType = 3;
  if ( type == "Trans"  ) iType = 4;
  if ( fType != iType ) {
    cout << "-E- STFieldMap::ReadAsciiFile: Incompatible map types!"
	 << endl;
    cout << "    Field map is of type " << fType 
	 << " but map on file is of type " << iType << endl;
    Fatal("ReadAsciiFile","Incompatible map types");
  }
  // Read Units
  TString unit;
  mapFile >> unit;
  if ( unit == "G" ) funit = 0.001;
  else if ( unit == "T"  ) funit = 10.0;
  else if ( unit == "kG"  ) funit=1.0;
  else {
    cout << "-E- FieldMap::ReadAsciiFile: No units!"
	 << endl;
        Fatal("ReadAsciiFile","No units defined");
  }


  // Read grid parameters
 
  mapFile >>fXmin >> fXmax >> fNx;
  mapFile >>fYmin >> fYmax >> fNy;
  mapFile >>fZmin >> fZmax >> fNz;
  fXstep = ( fXmax - fXmin ) / Double_t( fNx - 1 );
  fYstep = ( fYmax - fYmin ) / Double_t( fNy - 1 );
  fZstep = ( fZmax - fZmin ) / Double_t( fNz - 1 );
  
  // Create field arrays
  fBx = new TArrayD(fNx * fNy * fNz);
  fBy = new TArrayD(fNx * fNy * fNz);
  fBz = new TArrayD(fNx * fNy * fNz);

  // Read the field values
  Double_t factor = fScale * funit;   // Factor 1/1000 for G -> kG
  cout << right;
  Int_t nTot = fNx * fNy * fNz;
  cout << "-I- STFieldMap: " << nTot << " entries to read... " 
       << setw(3) << 0 << " % ";
  Int_t index = 0;
  div_t modul;
  Int_t iDiv = TMath::Nint(nTot/100.);
  for (Int_t ix=0; ix<fNx; ix++) {
    for (Int_t iy = 0; iy<fNy; iy++) {
      for (Int_t iz = 0; iz<fNz; iz++) {
	if (! mapFile.good()) cerr << "-E- STFieldMap::ReadAsciiFile: "
				   << "I/O Error at " << ix << " "
				   << iy << " " << iz << endl;
	index = ix*fNy*fNz + iy*fNz + iz;
	modul = div(index,iDiv);
	if ( modul.rem == 0 ) {
	  Double_t perc = TMath::Nint(100.*index/nTot);
	  cout << "\b\b\b\b\b\b" << setw(3) << perc << " % " << flush;
	}
	mapFile >> xx>>yy>>zz>>  bx >> by >> bz ;
	//mapFile >>  bx >> by >> bz ;
	//cout  << " x= " <<xx <<" y= " << yy<<" z= " << zz<<" bx= " <<  bx <<" by= " <<by <<" bz= " << bz<< endl;
	fBx->AddAt(factor*bx, index);
	fBy->AddAt(factor*by, index);
	fBz->AddAt(factor*bz, index);
	if ( mapFile.eof() ) {
	  cerr << endl << "-E- STFieldMap::ReadAsciiFile: EOF"
	       << " reached at " << ix << " " << iy << " " << iz << endl;
	  mapFile.close();
	  break;
	}
      }   // z-Loop
    }     // y-Loop0)
  }       // x-Loop

  cout << "   " << index+1 << " read" << endl;

  mapFile.close();

}
*/
// ------------------------------------------------------------------------



// -------------   Read field map from ROOT file (private)  ---------------
void STFieldMap::ReadRootFile(const char* fileName, 
			       const char* mapName) {

  // Store gFile pointer
  TFile* oldFile = gFile;

  // Open root file
  fLogger->Info(MESSAGE_ORIGIN, "STFieldMap: Reading field map from ROOT file  %s ",fileName); 
  TFile* file = new TFile(fileName, "READ");		
  if (file->IsZombie()) {
    fLogger->Error(MESSAGE_ORIGIN, "-E- STFieldMap::ReadRootfile: Cannot read from file! ");
    Fatal("ReadRootFile","Cannot read from file");
  }

  // Get the field data object
  STFieldMapData* data = NULL;
  file->GetObject(mapName, data);
  if ( ! data ) {
     fLogger->Error(MESSAGE_ORIGIN,"STFieldMap::ReadRootFile: data object %s not found in file! ", fileName);
     exit(-1);
  }

  // Get the field parameters
  SetField(data);

  // Close the root file and delete the data object
  file->Close();
  delete data;
  delete file;	
  if ( oldFile ) oldFile->cd();

}
// ------------------------------------------------------------------------



// ------------   Set field parameters and data (private)  ----------------
void STFieldMap::SetField(const STFieldMapData* data) {

  // Check compatibility
  if ( data->GetType() != fType ) {
    fLogger->Error(MESSAGE_ORIGIN,"STFieldMap::SetField: Incompatible map types Field map is of type %s \n but map on file is of type %s ",fType,data->GetType());
    Fatal("SetField","Incompatible map types");
  }
  
  
  fXmin = data->GetXmin();
  fYmin = data->GetYmin();
  fZmin = data->GetZmin();
  fXmax = data->GetXmax();
  fYmax = data->GetYmax();
  fZmax = data->GetZmax();
  fNx = data->GetNx();
  fNy = data->GetNy();
  fNz = data->GetNz();
  fXstep = ( fXmax - fXmin ) / Double_t( fNx - 1 );
  fYstep = ( fYmax - fYmin ) / Double_t( fNy - 1 );
  fZstep = ( fZmax - fZmin ) / Double_t( fNz - 1 );
  if ( fBx ) delete fBx;
  if ( fBy ) delete fBy;
  if ( fBz ) delete fBz;
  fBx = new TArrayD(*(data->GetBx()));
  fBy = new TArrayD(*(data->GetBy()));
  fBz = new TArrayD(*(data->GetBz()));

  // Scale and convert from G(or T) to kG
  Double_t factor = fScale * funit;
  Int_t index = 0;
  for (Int_t ix=0; ix<fNx; ix++) {
    for (Int_t iy=0; iy<fNy; iy++) {
      for (Int_t iz=0; iz<fNz; iz++) {
	index = ix*fNy*fNz + iy*fNz + iz;
	if ( fBx ) (*fBx)[index] = (*fBx)[index] * factor;
	if ( fBy ) (*fBy)[index] = (*fBy)[index] * factor;
	if ( fBz ) (*fBz)[index] = (*fBz)[index] * factor;
      }
    }
  }

}
// ------------------------------------------------------------------------  



// ------------   Interpolation in a grid cell (private)  -----------------
Double_t STFieldMap::Interpolate(Double_t dx, Double_t dy, Double_t dz) {

  // Interpolate in x coordinate
  fHb[0][0] = fHa[0][0][0] + ( fHa[1][0][0]-fHa[0][0][0] ) * dx;
  fHb[1][0] = fHa[0][1][0] + ( fHa[1][1][0]-fHa[0][1][0] ) * dx;
  fHb[0][1] = fHa[0][0][1] + ( fHa[1][0][1]-fHa[0][0][1] ) * dx;
  fHb[1][1] = fHa[0][1][1] + ( fHa[1][1][1]-fHa[0][1][1] ) * dx;

  // Interpolate in y coordinate
  fHc[0] = fHb[0][0] + ( fHb[1][0] - fHb[0][0] ) * dy;
  fHc[1] = fHb[0][1] + ( fHb[1][1] - fHb[0][1] ) * dy;

  // Interpolate in z coordinate
  return fHc[0] + ( fHc[1] - fHc[0] ) * dz;

}
// ------------------------------------------------------------------------



ClassImp(STFieldMap)
