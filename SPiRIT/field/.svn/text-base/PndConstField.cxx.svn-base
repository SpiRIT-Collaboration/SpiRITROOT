// -------------------------------------------------------------------------
// -----                    PndConstField source file                  -----
// -----                Created 30/01/07  by M. Al/Turany              -----
// -------------------------------------------------------------------------

#include "PndConstField.h"
#include "PndConstPar.h"


#include "FairRun.h"
#include "FairRuntimeDb.h"



#include <iomanip>
#include <iostream>


using namespace std;

// -----   Default constructor   -------------------------------------------
PndConstField::PndConstField() 
: fXmin(0),   
  fXmax(0),
  fYmin(0),
  fYmax(0),
  fZmin(0),
  fZmax(0),
  fBx(0),
  fBy(0),
  fBz(0)
{
  fType = 0;
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
PndConstField::PndConstField(const char* name, Double_t xMin, 
			     Double_t xMax, Double_t yMin, 
			     Double_t yMax, Double_t zMin,
			     Double_t zMax, Double_t bX, 
			     Double_t bY, Double_t bZ) 
  : FairField(name),
    fXmin(xMin),   
    fXmax(xMax),
    fYmin(yMin),
    fYmax(yMax),
    fZmin(zMin),
    fZmax(zMax),
    fBx(bX),
    fBy(bY),
    fBz(bZ)

{
  fType = 0;
}
// -------------------------------------------------------------------------



// --------   Constructor from PndFieldPar   -------------------------------
PndConstField::PndConstField(PndConstPar* fieldPar) 
 : FairField(), 
   fXmin(0),   
   fXmax(0),
   fYmin(0),
   fYmax(0),
   fZmin(0),
   fZmax(0),
   fBx(0),
   fBy(0),
   fBz(0)
 {
  if ( ! fieldPar ) {
    cerr << "-W- PndConstField::PndConstField: empty parameter container!"<< endl;
    fType= -1;
  }
  else {
    fXmin = fieldPar->GetXmin();
    fXmax = fieldPar->GetXmax();
    fYmin = fieldPar->GetYmin();
    fYmax = fieldPar->GetYmax();
    fZmin = fieldPar->GetZmin();
    fZmax = fieldPar->GetZmax();
    fBx   = fieldPar->GetBx();
    fBy   = fieldPar->GetBy();
    fBz   = fieldPar->GetBz();
    fType = fieldPar->GetType();
  }
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
PndConstField::~PndConstField() { }
// -------------------------------------------------------------------------



// -----   Set field region   ----------------------------------------------
void PndConstField::SetFieldRegion(Double_t xMin, Double_t xMax, 
				   Double_t yMin, Double_t yMax, 
				   Double_t zMin, Double_t zMax) {
  fXmin = xMin;
  fXmax = xMax;
  fYmin = yMin;
  fYmax = yMax;
  fZmin = zMin;
  fZmax = zMax;
}
// -------------------------------------------------------------------------



// -----   Set field values   ----------------------------------------------
void PndConstField::SetField(Double_t bX, Double_t bY, Double_t bZ) {
  fBx   = bX;
  fBy   = bY;
  fBz   = bZ;
}
// -------------------------------------------------------------------------



// -----   Get x component of field   --------------------------------------
Double_t PndConstField::GetBx(Double_t x, Double_t y, Double_t z) {
  if ( x < fXmin  ||  x > fXmax  ||
       y < fYmin  ||  y > fYmax  ||
       z < fZmin  ||  z > fZmax ) return 0.;
  return fBx;
}
// -------------------------------------------------------------------------



// -----   Get y component of field   --------------------------------------
Double_t PndConstField::GetBy(Double_t x, Double_t y, Double_t z) {
  if ( x < fXmin  ||  x > fXmax  ||
       y < fYmin  ||  y > fYmax  ||
       z < fZmin  ||  z > fZmax ) return 0.;
  return fBy;
}
// -------------------------------------------------------------------------



// -----   Get z component of field   --------------------------------------
Double_t PndConstField::GetBz(Double_t x, Double_t y, Double_t z) {
  if ( x < fXmin  ||  x > fXmax  ||
       y < fYmin  ||  y > fYmax  ||
       z < fZmin  ||  z > fZmax ) return 0.;
  return fBz;
}
// -------------------------------------------------------------------------



// -----   Screen output   -------------------------------------------------
void PndConstField::Print() {
  cout << "======================================================" << endl;
  cout << "----  " << fTitle << " : " << fName << endl;
  cout << "----" << endl;
  cout << "----  Field type    : constant" << endl;
  cout << "----" << endl;
  cout << "----  Field regions : " << endl;
  cout << "----        x = " << setw(4) << fXmin << " to " << setw(4) 
       << fXmax << " cm" << endl;
  cout << "----        y = " << setw(4) << fYmin << " to " << setw(4) 
       << fYmax << " cm" << endl;
  cout << "----        z = " << setw(4) << fZmin << " to " << setw(4)
       << fZmax << " cm" << endl;
  cout.precision(4);
  cout << "----  B = ( " << fBx << ", " << fBy << ", " << fBz << " ) kG"
       << endl;
  cout << "======================================================" << endl;
}
// -------------------------------------------------------------------------
// ---------   Fill the parameters   --------------------------------------------
void PndConstField::FillParContainer()
{
	TString MapName=GetName();
//	cout << "PndConstField::FillParContainer() " << endl;
	FairRun *fRun=FairRun::Instance();	
	FairRuntimeDb *rtdb=fRun->GetRuntimeDb();
	Bool_t kParameterMerged=kTRUE;
	PndConstPar* Par = (PndConstPar*) rtdb->getContainer("PndConstPar");
	Par->SetParameters(this);
	Par->setInputVersion(fRun->GetRunId(),1);
	Par->setChanged();
	
}


ClassImp(PndConstField)
