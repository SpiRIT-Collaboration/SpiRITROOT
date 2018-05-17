#ifndef STTRANSPORTMODELEVENTGENERATOR
#define STTRANSPORTMODELEVENTGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TString.h"


class STTransportModelEventGenerator : public FairGenerator
{
   public:
      STTransportModelEventGenerator();
      STTransportModelEventGenerator(TString fileName);
      STTransportModelEventGenerator(TString fileName, TString treeName);
      /* copy constructor */
      STTransportModelEventGenerator(const STTransportModelEventGenerator&);
      STTransportModelEventGenerator& operator=(const STTransportModelEventGenerator&);
      /* destructor */
      virtual ~STTransportModelEventGenerator();

      void SetPrimaryVertex(TVector3 vtx)	{ fVertex = vtx; }
      void SetVertexXYSigma(TVector2 sig)	{ fVertexXYSigma = sig; }
      void SetTargetThickness(Double_t t)	{ fTargetThickness = t; }
      void SetBeamAngle(TVector2 angle)		{ fBeamAngle = angle; }
      void SetBeamAngleSigma(TVector2 sig)	{ fBeamAngleABSigma = sig; }
      void SetRandomRP(Bool_t flag)		{ fIsRandomRP = flag; }

      virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);
      Long64_t GetNEvents() { return fNEvents; }
      void RegisterHeavyIon();

      enum TransportModel
      {
	 UrQMD,
	 AMD,
	 PHITS,
      };

   private:
      TransportModel	fGen;			// model type
      TFile*		fInputFile;		// input root file
      TTree*		fInputTree;		// input TTree
      Double_t		fB;			// impact parameter
      TClonesArray*	fPartArray;		// particle array
      Int_t		fCurrentEvent;		// current event ID
      Int_t		fNEvents;		// # of events
      TVector3		fVertex;		// user set vertex position
      TVector2		fVertexXYSigma;		// vertex position fluctuation in XY (gauss dist.)
      Double_t		fTargetThickness;	// vertex position fluctuation in Z  (uniform dist.)
      TVector2		fBeamAngle;		// beam angle
      TVector2		fBeamAngleABSigma;	// beam angle fluctuation in AB (gaus dist.)
      Bool_t		fIsRandomRP;		// flag for random reaction plane input

      Int_t kfToPDG(Long64_t kfCode);  // for PHITS data.

      ClassDef(STTransportModelEventGenerator,1);

};



#include "TObject.h"
#include "TLorentzVector.h"

class PHITSParticle : public TObject
{
   public:
      PHITSParticle() { kf=0; }
      virtual ~PHITSParticle() {}

      Int_t kf; // kf code, see page 14 of http://phits.jaea.go.jp/manual/manualJ-phits.pdf
      Int_t q; // charge
      Int_t A; // mass number
      Double_t m; // rest mass [MeV]
      Double_t ke; // kinetic energy [MeV/c2]
      Double_t pos[3]; // position [mm]
      Double_t mom[3]; // momentum in MeV for colpart, unit vec for part

      Double_t brho; // brho
      Double_t xang; // atan(px/pz)
      Double_t yang; // atan(py/pz)

      ClassDef(PHITSParticle, 1);
};

class AMDParticle : public TObject
{
   public:
      AMDParticle(): fPdg(0), fZ(-2), fN(-1), fCharge(-2), fEventID(0), fFragmentID(0)
   {
      fMomentum = TLorentzVector(); fPosition = TLorentzVector();
   }
      virtual ~AMDParticle(){}
      AMDParticle(Int_t pdg, Int_t z, Int_t n, Int_t chg, TLorentzVector mom, TLorentzVector pos, Int_t eve, Int_t frg)
	 : fPdg(pdg), fZ(z), fN(n), fCharge(chg), fEventID(eve), fFragmentID(frg)
      {
	 fMomentum = mom; fPosition = pos;
      }

      Int_t fPdg;    // pdg code ( pion 211, proton 2212, neutron 2122, deuteron 1000010020, ... 1000000000+Z*10000+A*10 )
      Int_t fZ;      // # of proton
      Int_t fN;      // # of neutron
      Int_t fCharge; // charge in e unit.

      TLorentzVector fMomentum;  // 4-d momentum at the primary vertex
      TLorentzVector fPosition;  // 4-d position of the primary vertex

      Int_t fEventID; // event # of AMD
      Int_t fFragmentID;  // fragment ID of AMD

      ClassDef(AMDParticle,1);
};

#endif
