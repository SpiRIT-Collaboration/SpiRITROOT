/**     
 * @brief STTrack Class         
 *                                   
 * @author Mizuki         
 */

#ifndef STPARTICLE
#define STPARTICLE
#include "TCutG.h"
#include "TFile.h"

#include "TVector3.h"
#include "TRotation.h"

#include "STRecoTrack.hh"

#include <vector>
#include <utility>

class STParticle : public TObject {
public:
  STParticle();
  STParticle(const STParticle &cp);
  STParticle &operator=(const STParticle &cp);

  virtual ~STParticle(){};

  virtual void Clear(Option_t *option = "");

  //private:
  void     SetProperty();
  void     SetLinearPID();

  void     CheckTrackonTarget();
  void     CheckKATANAHit(){};
  void     CheckKYOTOHit(){};


public:
  void     SetRecoTrack(STRecoTrack *atrack);

  void     SetTrackAtTarget(TVector3 value);
  TVector3 GetTrackAtTarget()            {return ftrackatTarget;}

  void     RotateAlongBeamDirection(Double_t valuex, Double_t valuey);
  void     SetP(Double_t value)          {fP = value;}
  void     SetPiPID();
  Int_t    GetPiPID()                    {return fpipid;}

  void     SetPID();
  void     SetPID(Int_t value);           
  Int_t    GetPID()                      {return fPID;}
  Double_t GetPIDProbability()           {return fPIDProbability;}

  void     SetMass();



  void     SetRapidity();
  Double_t GetRapidity()                 {return fRapidity;}

  void     SetpsudoRapidity();
  Double_t GetpsudoRapidity()            {return fpsudoRapidity;}


  Double_t GetEtotal()                   {return fEtotal;}

  Double_t GetP()                        {return fP;}
  void     SetdEdx(Double_t value)       {fdEdx = value;}
  Double_t GetdEdx()                     {return fdEdx;}
  Int_t    GetLinearPID()                {return flnPID;}

  void     SetMass(Double_t value)       {fMass = value;}
  Double_t GetMass()                     {return fMass;}


  void     SetRotatedMomentum(TVector3 value)   
  { fRotatedP3 = value; fcorrPt = TVector2(fRotatedP3.X(),fRotatedP3.Y()); }

  TVector3 GetRotatedMomentum()                 {return fRotatedP3;}

  TVector2 GetRotatedPt()                       {return fRotatedPt;}

  void  SetBeamonTargetFlag(Int_t value)        {fBeamonTargetf = value;}
  Int_t GetBeamonTargetFlag()                   {return fBeamonTargetf;}

  void  SetVertexAtTargetFlag(Int_t value)      {fVatTargetf = value;}
  Int_t GetVertexAtTargetFlag()                 {return fVatTargetf;}

  void  SetVertexZAtTargetFlag(Int_t value)      {fVZatTargetf = value;}
  Int_t GetVertexZAtTargetFlag()                 {return fVZatTargetf;}

  void  SetVertexBDCCorrelationFlag(Int_t value){fVBDCCorf   = value;}
  Int_t GetVertexBDCCorrelationFlag()           {return fVBDCCorf;}

  void  SetBDCCorrelationFlag(Int_t value)      {fBDCCorf     = value;}
  Int_t GetBDCCorrelationFlag()                 {return fBDCCorf;}

  void  SetFromTargetFlag(Int_t value)          {fTargetXYf   = value;}
  Int_t GetFromTargetFlag()                     {return fTargetXYf;}

  void  SetgotoKATANAFlag(Int_t value)          {fgotoKatanaf   = value;}
  Int_t GetgotoKATANAFlag()                     {return fgotoKatanaf;}

  void  SetgotoKYOTOFlag(Int_t value)          {fgotoKyotof   = value;}
  Int_t GetgotoKYOTOFlag()                     {return fgotoKyotof;}

  void  SetBestTrackFlag(Int_t value)           {fgoodtrackf = value;}
  Int_t GetBestTrackFlag()                      {return fgoodtrackf;}


  void     SetPhi(Double_t value)        {fphi  = value;}
  Double_t GetPhi()                      {return fphi;}

  void     SetRotatedPhi(Double_t value) {frphi = value;}
  Double_t GetRotatedPhi()               {return frphi;}

  Int_t    GetTrackID()                 {return   ftrackID;}
  void     SetTrackID(Int_t ival)       {ftrackID = ival;}

  // for flow analysis
  void     SetMixedEventID(Int_t value) {fmxevt = value;}
  void     SetMixedNtrack(Int_t value)  {fmxntrk= value;}
  void     SetMixTrackID(Int_t ival)    {fmxtrackid = ival;}

  void     SetRPWeight(Double_t value)  {fwgt = value;}
  Double_t GetRPWeight()                {return fwgt;}

  void     SetAzmAngle_wrt_RP(Double_t val) {fdeltphi = val;}
  Double_t GetAzmAngle_wrt_RP()         {return fdeltphi;}

  void     SetIndividualRPAngle(Double_t val) {frpphi = val;}
  Double_t GetIndividualRPAngle()       {return frpphi;}


  void     SetFlattenBinID(UInt_t value1, UInt_t value2) 
  {fcorrBin[0]=value1; fcorrBin[1] = value2;}

  Int_t    GetFlattenBinID(UInt_t value)       {if(value<2) return fcorrBin[value]; else return -1;}


  void     SetReactionPlaneFlag(Int_t value)    {fReactionPlanef = value;}
  void     AddReactionPlaneFlag(Int_t value)    {fReactionPlanef += value;}
  Int_t    GetReactionPlaneFlag()               {return fReactionPlanef;}
  
  void     Flattening(Double_t value); 
  TVector3 GetFlattenMomentum()                 {return ffltnP3;}
  TVector2 GetFlattenPt()                       {return ffltnPt;}

  TVector2 GetCorrectedPt()                     {return fcorrPt;}

  void     SetMomentumAtTarget(TVector3 value)  {forigP3 = value;}
  TVector3 GetMomentumAtTarget()                {return forigP3;}

  Int_t    GetCharge()                          {return fChar;}

  TVector3 GetMomentum()                        {return fRotatedP3;}

  STRecoTrack* GetRecoTrack()                   {return fRTrack;}

  void         SetVertexID(Int_t val)           {rVertexID = val;}
  Int_t        GetVertexID()                    {return rVertexID;}

  Int_t        GetdEdxPointSize()               {return rdEdxPointSize;}
  void         SetdEdxPointSizeCut(Int_t value) {rdEdxPointSize_thr = value;}

  Int_t        GetdEdxPointSizeFlag()            
  {
    if(rdEdxPointSize < rdEdxPointSize_thr)       frdEdxPointSizef = 0;
    return frdEdxPointSizef;
  }

  void         SetNDF(Int_t val)                  {rNDF = val;} 
  Int_t        GetNDF()                           {return rNDF;}
  void         SetDistanceAtVergtex(Double_t val) {rDist = val;}
  Double_t     GetDistanceAtVergtex()             {return rDist;}


  void         SetVertex(TVector3 value)        { fvertex = value;}
  TVector3     GetVertex()                      { return fvertex;}


private:
  Bool_t    bRotated = kFALSE;
  Bool_t    bFlatten = kFALSE;



private:
  Int_t    ftrackID;

  TVector3 ftrackatTarget;

  TVector3 forigP3;;
  TVector3 fRotatedP3;
  TVector2 fRotatedPt;
  Double_t fRotatedP;

  Double_t fP;
  Double_t fdEdx;
  Double_t ftheta;
  Double_t frtheta;
  Double_t fphi;
  Double_t frphi;
  Int_t    fPID;
  Double_t fPIDProbability;



  Int_t    flnPID;                 //!
  Double_t flnPIDval;              //!
  Double_t fRapidity;
  Double_t fpsudoRapidity;
  Double_t fEtotal;
  Int_t    fChar;
  Double_t fMass;

  Int_t    fpipid;
  TVector3 fvertex;

  // flow parameters
  TVector2 fcorrPt;

  TVector3 ffltnP3; 
  TVector2 ffltnPt; 

  Double_t frpphi;
  Double_t fdeltphi; 
  Double_t fwgt;

  Int_t   fcorrBin[2];

  
  // --- mixed partiels
  Int_t    fmxevt  = -1;
  Int_t    fmxntrk = -1;
  Int_t    fmxtrackid = -1;


  //flags
  UInt_t   fBeamonTargetf   = 1; //flag for beam tracked by BDC goes on the target
  UInt_t   fVatTargetf      = 1; //flag for reconstructed vertex XY within the target
  UInt_t   fVZatTargetf     = 1; //flag for reconstructed veretx Z comes from the target
  UInt_t   fVBDCCorf        = 1; //flag for reconstructed vertex is correated with BDC at the target
  UInt_t   fBDCCorf         = 1; //
  UInt_t   fTargetXYf       = 1;
  UInt_t   fgotoKatanaf     = 1;
  UInt_t   fgotoKyotof      = 1;
  UInt_t   frdEdxPointSizef = 1;  
  UInt_t   fgoodtrackf      = 0;

  UInt_t   fReactionPlanef  = 0;  

  //STRecoTrack parameters
  STRecoTrack *fRTrack; //!
  Int_t     rVertexID;  
  Int_t     rdEdxPointSize;
  Int_t     rdEdxPointSize_thr = 1;
  Int_t     rNDF;
  Double_t  rDist;
  TVector3  rpocaVertex;  
  ClassDef(STParticle, 6)

};


#endif
