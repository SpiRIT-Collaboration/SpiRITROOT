#ifndef STTRANSPORTMODELEVENTGENERATOR
#define STTRANSPORTMODELEVENTGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TLorentzVector.h"
#include "TString.h"

class pBUUProcessor;

class STTransportModelEventGenerator : public FairGenerator
{
  public:
    STTransportModelEventGenerator();
    STTransportModelEventGenerator(TString fileName);
    STTransportModelEventGenerator(TString filePath, TString fileName);
    /* copy constructor */
    STTransportModelEventGenerator(const STTransportModelEventGenerator&);
    STTransportModelEventGenerator& operator=(const STTransportModelEventGenerator&);
    /* destructor */
    virtual ~STTransportModelEventGenerator();

    void SetPrimaryVertex(TVector3 vtx)       { fVertex = vtx; }
    void SetVertexXYSigma(TVector2 sig)       { fVertexXYSigma = sig; }
    void SetTargetThickness(Double_t t)       { fTargetThickness = t; }
    void SetBeamAngle(TVector2 angle)         { fBeamAngle = angle; }
    void SetBeamAngleSigma(TVector2 sig)      { fBeamAngleABSigma = sig; }
    void SetRandomRP(Bool_t flag)             { fIsRandomRP = flag; }
    void SetStartEvent(int t_start)           { fCurrentEvent = t_start; }
    void SetMaxZAllowed(int t_z);              

    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);
    Long64_t GetNEvents() { return fNEvents; }
    void RegisterHeavyIon();

    enum TransportModel
    {
      UrQMD,
      AMD,
      PHITS,
      pBUU,
      ImQMD
    };

  private:
    TransportModel    fGen;                   // model type
    TString           fInputPath;             // input path
    TString           fInputName;             // input file name
    TFile*            fInputFile;             // input root file
    TTree*            fInputTree;             // input TTree
    Double_t          fB;                     // impact parameter
    TLorentzVector*   fBeamVector;            // beam 4d vector of input
    TLorentzVector*   fTargetVector;          // target 4d vector
    TClonesArray*     fFillBeamVector;        // beam 4d vector of input
    TClonesArray*     fFillTargetVector;      // target 4d vector
    TClonesArray*     fPartArray;             // particle array
    Int_t             fCurrentEvent;          // current event ID
    Int_t             fNEvents;               // # of events
    TVector3          fVertex;                // user set vertex position
    TVector2          fVertexXYSigma;         // vertex position fluctuation in XY (gauss dist.)
    Double_t          fTargetThickness;       // vertex position fluctuation in Z  (uniform dist.)
    TVector2          fBeamAngle;             // beam angle
    TVector2          fBeamAngleABSigma;      // beam angle fluctuation in AB (gaus dist.)
    Bool_t            fIsRandomRP;            // flag for random reaction plane input
    int               fMaxZ = -1;                  // Maximum Z of the allowed particles. Geant4 cannot handle heavy ion well so we may as well discard it....

    pBUUProcessor     *fpBUU;                  // pBUUProcessor

    Int_t kfToPDG(Long64_t kfCode);  // for PHITS data.
    void  RegisterFileIO();

    ClassDef(STTransportModelEventGenerator,1);

};

#include "TObject.h"
#include "TLorentzVector.h"

class ImQMDParticle : public TObject
{
public:
  ImQMDParticle() {};
  virtual ~ImQMDParticle() {};

  int pdg;
  double px, py, pz;
  double x, y, z;
  ClassDef(ImQMDParticle, 1);
};

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

class pBUUProcessor : public TObject
{
  public:
    pBUUProcessor() {}
    virtual ~pBUUProcessor() {}

    void ConnectBranch(TTree *tree) {
      tree -> SetBranchAddress("multi", &multi);
      tree -> SetBranchAddress("pid", &pid);
      tree -> SetBranchAddress("px", &px);
      tree -> SetBranchAddress("py", &py);
      tree -> SetBranchAddress("pz", &pz);
      tree -> SetBranchAddress("t", &t);
      tree -> SetBranchAddress("x", &x);
      tree -> SetBranchAddress("y", &y);
      tree -> SetBranchAddress("z", &z);
      tree -> SetBranchAddress("rho", &rho);

      tree -> SetBranchAddress("totEne", &totEne);
      tree -> SetBranchAddress("kinEne", &kinEne);
      tree -> SetBranchAddress("pt", &pt);
      tree -> SetBranchAddress("theta", &theta);
      tree -> SetBranchAddress("phi", &phi);

      tree -> SetBranchAddress("rap", &rap);
      tree -> SetBranchAddress("pzCMS", &pzCMS);
      tree -> SetBranchAddress("totEneCMS", &totEneCMS);
      tree -> SetBranchAddress("kinEneCMS", &kinEneCMS);
      tree -> SetBranchAddress("thetaCMS", &thetaCMS);
    }

    Int_t GetPDG(Int_t index) {
      switch (pid[index]) {
        case  1: return 2212;
        case  2: return 2112;
        case  3: return 1000010020;
        case  4: return 1000020030;
        case  5: return 1000010030;
        case 13: return 211;
        case 14: return 111;
        case 15: return -211;
        default: break;
      }

      return -9999;
    }

    Short_t multi;
    Short_t pid[300];
    Short_t px[300];
    Short_t py[300];
    Short_t pz[300];
    Short_t t[300];
    Short_t x[300];
    Short_t y[300];
    Short_t z[300];
    Short_t rho[300];

    Float_t totEne[300];
    Float_t kinEne[300];
    Float_t theta[300];
    Float_t phi[300];
    Float_t pt[300];

    Float_t rap[300];
    Float_t pzCMS[300];
    Float_t totEneCMS[300];
    Float_t kinEneCMS[300];
    Float_t thetaCMS[300];

  ClassDef(pBUUProcessor, 2);
};

#endif
