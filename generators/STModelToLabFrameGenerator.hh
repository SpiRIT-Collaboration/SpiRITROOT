#ifndef STMODELTOLABFRAMEGENERATOR
#define STMODELTOLABFRAMEGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairIon.h"

#include "STBeamInfo.hh"

#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TLorentzVector.h"
#include "TString.h"

#include "TDatabasePDG.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"

struct STTransportParticle;
class STTransportReader;
class STImQMDReader;

namespace Elements
{
  const TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
                              "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
                              "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
                              "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
                              "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};
  TParticlePDG* PDGToParticleData(int pdg); // convert pdg code to TParticlePDG. It works even for heavy ions. 
  // if A = 0 and Z = +/- 1, pdg == +/- 211 (pion is assumed in such case)
  const double kProtonMass = TDatabasePDG::Instance()->GetParticle(2212)->Mass(); // in GeV
};

class STModelToLabFrameGenerator : public FairGenerator
{
  public:
    STModelToLabFrameGenerator();
    STModelToLabFrameGenerator(TString fileName);
    STModelToLabFrameGenerator(TString filePath, TString fileName);
    STModelToLabFrameGenerator(STTransportReader *reader);
    /* copy constructor */
    STModelToLabFrameGenerator(const STModelToLabFrameGenerator&);
    STModelToLabFrameGenerator& operator=(const STModelToLabFrameGenerator&);
    /* destructor */
    virtual ~STModelToLabFrameGenerator();
 
    virtual Bool_t Init();   
    // energy in GeV
    void SetBeamAndTarget(double energyPerN, 
                          int beamMass, 
                          int beamCharge,
                          int targetMass)     { fBeamEnergyPerN = energyPerN; 
                                                fBeamMass = beamMass; 
                                                fBeamCharge = beamCharge; 
                                                fTargetMass = targetMass; }
    void SetPrimaryVertex(TVector3 vtx)       { fVertex = vtx; }
    void SetVertexXYSigma(TVector2 sig)       { fVertexXYSigma = sig; }
    void SetTargetThickness(Double_t t)       { fTargetThickness = t; }
    // all angle information in radian
    void SetBeamAngle(TVector2 angle)         { fBeamAngle = angle; }
    void SetBeamAngleSigma(TVector2 sig)      { fBeamAngleABSigma = sig; }
    void SetBeamDetectorAngleSigma(TVector2 sig)    { fBeamDetectorABSigma = sig; }
    void SetBeamDetectorVertexSigma(TVector2 sig)   { fBeamDetectorVertexSigma = sig; }
    void SetRandomRP(Bool_t flag)             { fIsRandomRP = flag; }
    void SetStartEvent(int t_start)           { fCurrentEvent = t_start; }
    void SetMaxAllowedZ(int t_z);             
    void SetMaxMult(int mult)                 { fMaxMult = mult; }

    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);
    void RegisterHeavyIon(std::set<int> pdgList = {}); // if pdgList is supplied, it will only simulate particles inside the list
    Long64_t GetNEvents() { return fNEvents; }
    void Print();

  private:
    STTransportReader *fReader;               //<! Reader
    TClonesArray      *fBeamInfo;             //<! BeamInfo
    TTree             *fInputTree;
    TString           fInputPath;             //<! input path
    TString           fInputName;             //<! input file name
    Double_t          fB;                     //<! impact parameter
    Double_t          fBeamEnergyPerN;
    Int_t             fBeamMass;              //<! Atomic mass of the beam particle
    Int_t             fBeamCharge;
    Int_t             fTargetMass;
    Int_t             fCurrentEvent;          //<! current event ID
    Int_t             fNEvents;               //<! # of events
    TVector3          fVertex;                //<! user set vertex position
    TVector2          fVertexXYSigma;         //<! vertex position fluctuation in XY (gauss dist.)
    Double_t          fTargetThickness;       //<! vertex position fluctuation in Z  (uniform dist.)
    TVector2          fBeamAngle;             //<! beam angle
    TVector2          fBeamAngleABSigma;      //<! beam angle fluctuation in AB (gaus dist.)
    TVector2          fBeamDetectorABSigma;   //<! the uncertainty in beam angle detection
    TVector2          fBeamDetectorVertexSigma;   //<! the uncertainty in vertex position detection
    TVector3          fBoostVector;
    Bool_t            fIsRandomRP;            //<! flag for random reaction plane input
    Bool_t            fHeavyIonsRegistered;
    int               fMaxZ = -1;             //<! Maximum Z of the allowed particles. Geant4 cannot handle heavy ion well so we may as well discard it....
    int               fMaxMult = -1;
    const double      fNucleonMass = 0.9315;
    std::set<int>     fAllowedPDG;

    void RegisterReader();
    ClassDef(STModelToLabFrameGenerator,1);

};

struct STTransportParticle
{
  int pdg;
  double px, py, pz;
  double x, y, z;
};

class STTransportReader
{
public:
  STTransportReader();
  virtual ~STTransportReader();
  virtual int GetEntries() = 0;
  virtual int GetEntry() = 0;
  virtual void SetEntry(int t_entry) = 0;
  virtual bool GetNext(std::vector<STTransportParticle>& particleList) = 0;  
  virtual TString Print() = 0;
  virtual std::vector<FairIon*> GetParticleList();
};

class STImQMDReader : public STTransportReader
{
public: 
  STImQMDReader(TString fileName);
  virtual ~STImQMDReader();
  virtual void SetEntry(int t_entry);
  virtual int GetEntry() { return fEventID; }
  virtual int GetEntries() { return fEntries; }
  virtual bool GetNext(std::vector<STTransportParticle>& particleList);
  virtual TString Print();
protected:
  TFile fFile;
  TTree *fTree = nullptr;
  int fEventID = 0;
  int fLocalID;
  int fTreeEventID;
  int fEntries;
  int fPartA, fPartZ;
  double fPx, fPy, fPz;
  double fX, fY, fZ;
};

class STpBUUReader : public STTransportReader
{
public: 
  STpBUUReader(TString fileName);
  virtual ~STpBUUReader(){};
  virtual void SetEntry(int t_entry) { fEventID = t_entry; }
  virtual int GetEntry() { return fEventID; }
  virtual int GetEntries() { return fEntries; }
  virtual bool GetNext(std::vector<STTransportParticle>& particleList);
  virtual TString Print();
  virtual std::vector<FairIon*> GetParticleList();
protected:
  TFile fFile;
  TTree *fTree = nullptr;
  int fEntries;
  int fEventID = 0;
  const static int maxMulti = 1024;
  short fMulti;
  short fPID[maxMulti];
  short fPx[maxMulti], fPy[maxMulti];
  float fPz[maxMulti];
  short fX[maxMulti], fY[maxMulti], fZ[maxMulti];
};





#endif
