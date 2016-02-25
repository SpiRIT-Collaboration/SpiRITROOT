/**
 * @brief  STRiemann Track
 *
 * @author Sebastian Neubert (TUM) -- original author for FOPIROOT
 * @author Genie Jhang (Korea University) -- implementation for SpiRITROOT
 * @author JungWoo Lee (Korea University) -- implementation for SpiRITROOT
 *
 * @detail Track on Riemann Sphere
 *         Circle parameters can be calculated from plane parameters
 *         plane(c,nx,ny,nz);
 */

// SpiRITROOT classes
#include "STEvent.hh"
#include "STRiemannTrack.hh"

// STL
#include <iostream>
#include <iterator>

// ROOT classes
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TVector3.h"
#include "TVector2.h"
#include "TGraph.h"
#include "TF1.h"
#include "TMath.h"
#include "TPolyMarker3D.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TPolyLine3D.h"
#include "TSystem.h"

ClassImp(STRiemannTrack);

#define DEBUG 0

// Simple getter and setter methods ----------------------------------------------------------------------------------
          void  STRiemannTrack::SetVerbose(Bool_t value)   { fVerbose = value; }
          void  STRiemannTrack::SetSort(Bool_t value)      { fDoSort = value; }
          void  STRiemannTrack::SetFinished(Bool_t value)  { fIsFinished = value; }
          void  STRiemannTrack::SetGood(Bool_t value)      { fIsGood = value; }

const TVector3 &STRiemannTrack::GetN()               const { return fN; }
      Double_t  STRiemannTrack::GetC()               const { return fC; }
const TVector3 &STRiemannTrack::GetAv()              const { return fAv; }
      Double_t  STRiemannTrack::GetRiemannScale()    const { return fRiemannScale; }

      Double_t  STRiemannTrack::GetR()               const { return fRadius; }
const TVector3 &STRiemannTrack::GetCenter()          const { return fCenter; }
      Double_t  STRiemannTrack::GetDip()             const { return fDip; }
      Double_t  STRiemannTrack::GetSinDip()          const { return fSinDip; }

        Bool_t  STRiemannTrack::IsFitted()           const { return fIsFitted; }
        Bool_t  STRiemannTrack::IsInitialized()      const { return fIsInitialized; }
        Bool_t  STRiemannTrack::IsFinished()               { return fIsFinished; }
        Bool_t  STRiemannTrack::IsGood()                   { return fIsGood; }

      Double_t  STRiemannTrack::GetM()               const { return fM; }
      Double_t  STRiemannTrack::GetT()               const { return fT; }

        UInt_t  STRiemannTrack::GetNumHits()                   const { return fHits.size(); }
  STRiemannHit *STRiemannTrack::GetHit(UInt_t iHit)            const { return fHits[iHit]; }
  STRiemannHit *STRiemannTrack::GetLastHit()                   const { return fHits.back(); }
  STRiemannHit *STRiemannTrack::GetFirstHit()                  const { return fHits.front(); }
  const std::vector<STRiemannHit *> *STRiemannTrack::GetHits() const { return &fHits; }
// -------------------------------------------------------------------------------------------------------------------

STRiemannTrack::STRiemannTrack()
{
  InitVariables();
}

STRiemannTrack::STRiemannTrack(Double_t scale)
{
  InitVariables();

  fRiemannScale = scale;
}

void
STRiemannTrack::InitVariables()
{
  fN = TVector3(0., 0., 0.);
  fC = 0;
  fCenter = TVector3(0., 0., 0.);
  fRadius = 0;
  fAv = TVector3(0., 0., 0.);
  fSumOfWeights = 0;
  fM = 0;
  fT = 0;
  fDip = 0;
  fSinDip = 0;
  fRms = 0;
  fRiemannScale = 86.1;
  fIsFitted = kFALSE;
  fIsInitialized = kFALSE;
  fIsFinished = kFALSE;
  fIsGood = kFALSE;
  fDoSort = kTRUE;
}

void
STRiemannTrack::DeleteHits()
{
  for (UInt_t iHit = 0; iHit < GetNumHits(); iHit++)
    delete fHits[iHit];

  fHits.clear();
}

Bool_t 
STRiemannTrack::CheckScale(STRiemannHit *hit) const
{
  if (!(hit -> GetRiemannScale() == this -> GetRiemannScale())) {
    fLogger -> Warning(MESSAGE_ORIGIN, Form("Riemann scale not matching! Hit: %f, Track: %f", hit -> GetRiemannScale(), this -> GetRiemannScale()));

    return kFALSE;
  }

  return kTRUE;
}

Int_t
STRiemannTrack::GetClosestHit(STRiemannHit *hit, 
                                  Double_t &dist, 
                                  TVector3 &outdir) const
{
  Int_t iHit2 = GetClosestHit(hit, dist);

  if (fHits.size() > 1) {
    // catch the case where we are at boundary
    Int_t iHit1 = iHit2;

    if(iHit1 > 0)
      iHit1--;

    Int_t iHit3 = iHit2;

    if (iHit3 < fHits.size() - 1)
      iHit3++;
    
    TVector3 pos1 = fHits[iHit1] -> GetHit() -> GetPosition(); //next point
    TVector3 pos3 = fHits[iHit3] -> GetHit() -> GetPosition();

    // construct general direction of track from these three
    outdir = pos3 - pos1;
  } else {
    TVector3 pos = hit -> GetHit() -> GetPosition(); //next point
    TVector3 pos2 = fHits[iHit2] -> GetHit() -> GetPosition();
    outdir = (pos2 - pos);
  }

  outdir.SetMag(1);

  return iHit2;
}


Int_t
STRiemannTrack::GetClosestHit(STRiemannHit *hit, Double_t &dist, Int_t from, Int_t to) const
{
  TVector3 pos1 = hit -> GetHit() -> GetPosition();
  TVector3 pos2;

  dist = 9.E99;

  Int_t closestHit;
  Double_t tempDist;

  if (from < 0)
    from = 0;

  if (to < from)
    to = from + 1;

  if (to > fHits.size())
    to = fHits.size();

  for (Int_t iHit = from; iHit < to; iHit++) {
    pos2 = fHits[iHit] -> GetHit() -> GetPosition(); 
    tempDist = (pos2 - pos1).Mag();

    if (tempDist < dist) {
      closestHit = iHit;
      dist = tempDist;
    }
  }

  return closestHit;
}


void // I just leave it only using RMS value as weight. Maybe layer we can add taking into account the charge.
STRiemannTrack::AddHit(STRiemannHit *hit){
  if (!CheckScale(hit))
    throw; // check if Riemannscale of hits matches that of the track!

  Int_t numHits = fHits.size();

  // update average
  Double_t weightFactor = hit -> GetHit() -> GetPosSigma().Perp();

  if (weightFactor > 1.E-3)
    weightFactor = 1./weightFactor;
  else
    weightFactor = 1.E3;
  
  fAv *= fSumOfWeights;
  fAv += hit -> GetX() * weightFactor;
  fSumOfWeights += weightFactor;
  fAv *= 1./fSumOfWeights;

  fHits.push_back(hit);
}


void
STRiemannTrack::RemoveHit(UInt_t iHit){
  delete fHits[iHit];

  fHits.erase(fHits.begin() + iHit);
}


Int_t
STRiemannTrack::GetWinding() const { // returns winding sense along z-axis
  if (!fIsFitted && !fIsInitialized)
    return 1;

  Double_t angle0 = fHits.front() -> GetAngleOnHelix();
  Double_t angle1;

  if (fIsInitialized) {
    TVector3 hit1(0, 0, 0);
    hit1 -= fCenter;
    angle1 = hit1.Phi(); // [-pi, pi]
  } else
    angle1 = fHits.back() -> GetAngleOnHelix();

  if (angle1 > angle0)
    return 1;

  return -1;
}


Double_t
STRiemannTrack::Dist(STRiemannHit *hit, TVector3 n, Double_t c, Bool_t useArguments) const {
  if (!useArguments) {
    if (!fIsFitted)
      return 0.;

    n = fN;
    c = fC; 
  }

  // distance plane - center of sphere
  TVector3 center(0, 0, 0.5); // center of sphere
  Double_t l = c + n*center;  // distance plane to center
  Double_t thetaPlane = TMath::ACos(2.*l); // l/0.5 - angle
      
  // construct vector of hit relative to center of sphere
	TVector3 vh = hit -> GetX() - center;
	vh.SetMag(1);
	Double_t cos1 = (-1.*n)*vh;
	Double_t thetaHit = TMath::ACos(cos1);

	return (thetaHit - thetaPlane); // positive when hit is outside of the cone, negative when hit is inside of the cone
                                  // the cone is made by the center of sphere and the circlue by plane.
}


void
STRiemannTrack::Refit() { // helix fit
  Bool_t hasBeenFitted = fIsFitted;
  fIsFitted = kFALSE;

  UInt_t numHits = fHits.size();

  if (numHits < 3)
    return; // need at least 3 points to make a planefit

  TMatrixT<Double_t> Av(3, 1);
  Av[0][0] = fAv[0];
  Av[1][0] = fAv[1];
  Av[2][0] = fAv[2];

  TMatrixD sampleCov(3, 3);
  
  Double_t nh = 0;
  Double_t weightFactor;
  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    TMatrixD h(3, 1);
    // weigh hits with 1/hit error
    weightFactor = fHits[iHit] -> GetHit() -> GetPosSigma().Perp();

    if (weightFactor > 1.E-3)
      weightFactor = 1./weightFactor;
    else
      weightFactor = 1.E3;

    nh += weightFactor;
    h[0][0] = fHits[iHit] -> GetX().X();
    h[1][0] = fHits[iHit] -> GetX().Y();
    h[2][0] = fHits[iHit] -> GetX().Z();
    TMatrixD d(3, 1);
    d = h - Av;
    TMatrixD dt(TMatrixD::kTransposed, d);
    TMatrixD ddt(d, TMatrixD::kMult, dt);
    ddt *= weightFactor;
    sampleCov += ddt;  
  }

  if (sampleCov == 0) {
    // can happen if a pad fires continuously and the resulting hits have the same xy coords
    //  ->  they are mapped to one single point on the riemann sphere
    //std::cerr<<"STRiemannTrack::refit() - can't fit plane, covariance matrix is zero"<<std::endl;
    return;
  }

  sampleCov *= 1./nh;
  
  TVectorD eigenValues(3);
  TMatrixD eigenVec = sampleCov.EigenVectors(eigenValues);
  
  // eigenvalues are sorted according to their value
  // in descending order  ->  last one is smallest
  
  // check smallest and second smallest eigenvector
  // for this we use the rms distance of hits to section of plane with sphere on the sphere

  fRms = 1.E32;
  UInt_t imin = 2;
  
  Double_t norm, c1, rms;
  TVectorD planeN(3);
  TVectorD planeNmin(3);

  UInt_t iVec(1);

  // no plane switching for long tracks needed! If track long enough (on riemann sphere)  ->  calc rms only for smallest eigenvec!
  if (numHits > 10 && (GetFirstHit() -> GetX() - GetLastHit() -> GetX()).Mag() > 0.2) {
    iVec = 2;
  }

  for (; iVec < 3; ++iVec) {
    planeN = TMatrixDColumn(eigenVec, iVec);
    norm = TMath::Sqrt(planeN.Norm2Sqr());

    if (norm < 1E-10) {
      //std::cerr<<"STRiemannTrack::refit() - eigenvector too small"<<std::endl;
      return;
    }

    planeN *= 1./norm;
    TVector3 plane3(planeN[0], planeN[1], planeN[2]);
    c1 = -1.*plane3*fAv; // distance plane to origin
    rms = CalcRMS(plane3, c1);
    if (rms < fRms) {
	    fRms = rms;
	    planeNmin = planeN;
    }
  }
	
  fN.SetXYZ(planeNmin[0], planeNmin[1], planeNmin[2]);
  fC = -1.*fN*fAv; 

  fIsFitted = kTRUE;
  // finished planefit


  //
  // fit dip
  //

  // calc angles on helix (hits must be sorted at least roughly!)
  CenterR(); // fIsFitted must be kTRUE for this to work!! Calculate center and radius

  TVector3 hit0 = fHits[0] -> GetHit() -> GetPosition() - fCenter;

  Double_t firstangle = hit0.Phi(); // [-pi, pi]
  fHits[0] -> SetAngleOnHelix(firstangle); // set angle of first hit relative to x axis

  Double_t lastangle(firstangle);

  TVector3 hiti;

  // phi goes counterclockwise and can be > 2Pi for curlers
  Double_t meanAngle, twoPi(TMath::TwoPi()), nTurns(1), dZ, dZnull, dZminus, dZplus;
  Bool_t twoPiCheck(hasBeenFitted && fRadius < 800. && numHits > 10 && TMath::Abs(fM*twoPi) > 10. && TMath::Abs(fM*twoPi) < 250.);

  for (Int_t i = 1; i < numHits; i++) {
    hiti = fHits[i] -> GetHit() -> GetPosition() - fCenter;

    Double_t angle = hiti.DeltaPhi(hit0);

    // check if we have to go +-2Pi further
    if (twoPiCheck && i > 4) {
      meanAngle = (lastangle - firstangle)/i;

      if (TMath::Abs(meanAngle) < 1.E-10)
        goto skipCheck;
      
      dZ = hiti.Z() - hit0.Z();
      
      dZnull = TMath::Abs(dZ - angle * fM);
      dZplus = TMath::Abs(dZ - (angle + twoPi) * fM);
      dZminus = TMath::Abs(dZ - (angle - twoPi) * fM);
      
      if (angle/meanAngle < -4) {
        if (dZplus < dZnull) {
          angle += twoPi;
          firstangle += angle;
          nTurns++;
          //std::cout<<"angle += twoPi;\n";
        }
        else if (dZminus < dZnull) {
          angle -= twoPi;
          firstangle += angle;
          ++nTurns;
          //std::cout<<"angle -= twoPi;\n";          
        }
      }
    }

    skipCheck:

    lastangle += angle;

    fHits[i] -> SetAngleOnHelix(lastangle);

    hit0 = hiti;
  }

  // get phis and zs
  TGraph g(numHits);

  for (UInt_t it = 0; it < numHits; it++) 
    g.SetPoint(it, fHits[it] -> GetAngleOnHelix(), fHits[it] -> GetZ());

  Int_t errorcode;
  g.LeastSquareLinearFit(numHits, fT, fM, errorcode, -999, 999);

  if (errorcode != 0) {
    fIsFitted = kFALSE;
    //std::cerr<<"STRiemannTrack::refit() - can't fit dip"<<std::endl;
    return; // phi z fit did not work
  }
  
  // calc dip
  fDip = TMath::ATan(fM/fRadius) + TMath::PiOver2();
  fSinDip = TMath::Sin(fDip);
}

void
STRiemannTrack::FitAndSort() {
  fIsInitialized = kFALSE;

  // sort by z so that angle calculation in refit() is possible
  if (fDoSort) {
    // keep rough sorting!
    /*
    if ((fHits.front()) -> GetZ() < (fHits.back()) -> GetZ())
      std::sort(fHits.begin(), fHits.end(), SortByX());
    else
      std::sort(fHits.begin(), fHits.end(), SortByXInv());
      */
    std::sort(fHits.begin(), fHits.end(), SortByX());
  }

  Refit(); // fit plane and dip

  // if dip [90°+-50°] or R<0.5, resort by angle
  if (fDoSort && fIsFitted &&
      ((fDip > 0.698131701 && fDip < 2.44346095) ||
      fRadius < 5.)) {
    // keep rough sorting!
    if (GetWinding() > 0)
      std::sort(fHits.begin(), fHits.end(), SortByAngle());
    else
      std::sort(fHits.begin(), fHits.end(), SortByAngleInv());
  }

  // update dip
  if (GetWinding() < 0) {
    fDip = TMath::Pi() - fDip; // doensn't affect fSinDip; if not fitted: fDip = 0
  }
}

Double_t
STRiemannTrack::CalcRMS(TVector3 n1, Double_t c1) const
{
  // loop over hits and calculate RMS
  Double_t rms = 0.;
  Double_t norm = 0.;
  
  for (Int_t it = 0; it < fHits.size(); it++) {
    Double_t weightFactor = fHits[it] -> GetHit() -> GetCharge(); // weigh with amplitude
    norm += weightFactor;
    Double_t distance = Dist(fHits[it], n1, c1, kTRUE);
    rms += weightFactor * distance*distance;
  }

  rms /= norm;
  rms = sqrt(rms);

  return rms;
}

Double_t
STRiemannTrack::DistRMS() const
{
  if (!fIsFitted && !fIsInitialized)
    return 0;

  Double_t d2s = 0;
  Double_t dis;

  UInt_t nHits = GetNumHits();

  for (Int_t it = 0; it < nHits; it++) {
    dis = DistHelix(fHits[it], kFALSE);
    dis *= dis;
    d2s += dis;
  }

  d2s /= Double_t(nHits);

  if (d2s < 1E-50)
    return 1E-25;

  return sqrt(d2s);
}

void
STRiemannTrack::CenterR()
{
  if (!fIsFitted && !fIsInitialized)
    return;

  // look at sphere from side, perpendicular to plane, so that plane becomes a line
  // line:   x = -fC*nx + a*nz;  z = -fC*nz - a*nx
  // nx = sqrt(1-nz^2)
  // circle: x^2 + (z-0.5)^2 + 0.5^2
  // then intersect line with circle  ->  solutions a1, a2;
  Double_t nz = fN[2];    // z component 
  Double_t c2 = fC*fC;
  Double_t arg1 = -1.*(nz - 1)*(nz + 1);
  Double_t root1(1E-5);

  if (arg1 > 1E-10)
    root1 = sqrt(arg1);

  Double_t arg2 = 1. - nz*nz - 4.*c2 - 4.*fC*nz;
  Double_t root2(1E-5);

  if (arg2 > 1E-10)
    root2 = sqrt(arg2);

  Double_t a1 = -0.5*root1 + 0.5*root2;
  Double_t a2 = -0.5*root1 - 0.5*root2;

  // now we get two points on the sphere (x1,z1), (x2,z2)
  Double_t argnx = 1. - nz*nz;
  Double_t nx(1E-5);

  if (argnx > 1E-10)
    nx = sqrt(argnx);

  Double_t x1 = -1.*fC*nx + a1*nz;
  Double_t z1 = -1.*fC*nz - a1*nx;
  Double_t x2 = -1.*fC*nx + a2*nz;
  Double_t z2 = -1.*fC*nz - a2*nx; 

  // project them back onto the plane
  // we get two radii of a track
  Double_t r1, r2;

  if (z1 > 0.99999999)
    r1 = 1.E4;
  else if (z1 < 0.0000000001)
    r1 = 1.E-5;
  else
    r1 = sqrt(z1/(1. - z1));

  if (x1 < 0)
    r1 *= -1.;

  r1 *= fRiemannScale;

  if (z2 > 0.99999999)
    r2 = 1.E4;
  else if (z2 < 0.0000000001)
    r2 = 1.E-5;
  else
    r2 = sqrt(z2/(1. - z2));

  if (x2 < 0)
    r2 *= -1.;

  r2 *= fRiemannScale;

  fRadius = 0.5*TMath::Abs(r2 - r1);
  
  //std::cout<< "r1 = " << r1 << "   r2 = " << r2 << "   radius = " << fRadius;

  // limit
  if (fRadius < 20.)
    fRadius = 20.;   // 10 mm
  //if (fRadius>1.E5) fRadius = 1.E5; // 1 km

  // center
  fCenter = fN;
  fCenter.SetZ(0);
  fCenter.SetMag(0.5*(r1 + r2));

  //fCenter.Print();
}

Double_t
STRiemannTrack::DistHelix(STRiemannHit *hit, Bool_t calcPos, Bool_t TwoPiCheck, TVector3 *POCA) const
{
  if (!fIsFitted && !fIsInitialized)
    return 0.;

  Double_t hit_angle = hit -> GetAngleOnHelix();
  TVector3 pos = hit -> GetHit() -> GetPosition();
  Double_t hitZ = hit -> GetZ();

  TVector3 hitX = pos - fCenter;

  Double_t hit_angleR, hit_angleZ;

  if (calcPos) {
    Double_t d;
    Int_t ahit = 0;
    if (!fIsInitialized) { // when track is initialized, only hit0 has a meaningful angle!
      ahit = GetClosestHit(hit, d);

      if (ahit == GetNumHits())
        ahit--;
    }

    TVector3 hit1 = fHits[ahit] -> GetHit() -> GetPosition() - fCenter;
    Double_t phi1 = fHits[ahit] -> GetAngleOnHelix();

    hit_angleR = hitX.DeltaPhi(hit1) + phi1;

    // check if nearest position position lies multiples of 2Pi away
    if (TwoPiCheck && fRadius < 50.) {
      Double_t z = fM * hit_angleR + fT;
      Double_t dZ = hitZ - z; // positive when above helix, negative when below

      Double_t twoPi = 2*TMath::Pi();
      Int_t nn = 0;

      Double_t zCheck;

      const UInt_t maxIt = 4;
      UInt_t it = 0;

      while (it < maxIt) {
        zCheck = fM * (hit_angleR + twoPi) + fT;
        if (TMath::Abs(hitZ - zCheck) < TMath::Abs(dZ)) {
          dZ = hitZ - zCheck;
          hit_angleR += twoPi;
        } else
          break;

        it++;
      }

      nn = 0;
      it = 0;

      while (it < maxIt) {
        zCheck = fM * (hit_angleR - twoPi) + fT;
        if (TMath::Abs(hitZ - zCheck) < TMath::Abs(dZ)) {
          dZ = hitZ - zCheck;
          hit_angleR -= twoPi;
        } else
          break;

        it++;
      }
    }

    hit_angleZ = 0;
    if (TMath::Abs(fM) > 1.E-3)
      hit_angleZ = (hitZ - fT)/fM;

    Double_t zWeigh = 0.5*(TMath::Cos(2.*fDip) + 1.);
    hit_angle = hit_angleR*(1 - zWeigh) + hit_angleZ*zWeigh;

    //hit_angle = 0;
    //if (TMath::Abs(fM) > 1.E-22)
      //hit_angle = (hitZ - fT)/fM;
  } // end recalcPos


  Double_t sinphi, cosphi, xHelix, yHelix, zHelix, dist2, distance, deltadist, delta, mindist = 999999, accuracy = 1E-4;
  UInt_t i = 0, maxIt = 4;

  // newtons method for finding POCA
  while (kTRUE) {
    sinphi = TMath::Sin(hit_angle);
    cosphi = TMath::Cos(hit_angle);

    xHelix = fCenter.X() + cosphi * fRadius - pos.X();
    yHelix = fCenter.Y() + sinphi * fRadius - pos.Y();
    zHelix = fM * hit_angle + fT - hitZ;

    dist2 = (xHelix*xHelix + yHelix*yHelix + zHelix*zHelix);

    if (dist2 > 1E-20)
      distance = sqrt(dist2);
    else
      distance = 1E-10;

    deltadist = mindist - distance;

    if (distance < mindist)
      mindist = distance;

    if (deltadist < accuracy || i > maxIt)
      break;

    // f  = (-1.* xHelix*sinphi*fRadius + yHelix*cosphi*fRadius + zHelix*fM) / distance; // first derivative of  distance  wrt  phi
    // f1 = f/(distance*distance) + ( fRadius*fRadius - xHelix*cosphi*fRadius - yHelix*sinphi*fRadius + 2.*fM*fM )/distance; // second derivative of  distance  wrt  phi
    // hit_angle -= f/f1;

    // simplified:
    delta = (-1.* xHelix*sinphi*fRadius + yHelix*cosphi*fRadius + zHelix*fM);
    delta = delta / (delta/(distance*distance) + fRadius*fRadius - xHelix*cosphi*fRadius - yHelix*sinphi*fRadius + 2.*fM*fM);
    hit_angle -= delta;

    // corrected: (original neuton's method)
    //delta = (-1.* xHelix*sinphi*fRadius + yHelix*cosphi*fRadius + zHelix*fM);
    //delta = delta / (-delta*delta/(distance*distance) + fRadius*fRadius - xHelix*cosphi*fRadius - yHelix*sinphi*fRadius + fM*fM);
    //hit_angle -= delta;

    i++;
  }

  if (POCA != NULL)
    POCA -> SetXYZ(xHelix, yHelix, zHelix);

  return mindist;
}

void
STRiemannTrack::GetPosDirOnHelix(UInt_t i, TVector3& pos, TVector3& dir) const
{
  if (!fIsFitted && !fIsInitialized)
    return;

  Double_t hit_angle;

  if (fIsInitialized) {
    hit_angle = fHits[0] -> GetAngleOnHelix();
  } else {
    Double_t hit_angleR = fHits[i] -> GetAngleOnHelix();
    Double_t hit_angleZ = 0;
    if (TMath::Abs(fM) > 1.E-3)
      hit_angleZ = (fHits[i] -> GetZ() - fT)/fM;

    Double_t zWeigh = 0.5*(TMath::Cos(2.*fDip) + 1.);
    hit_angle = hit_angleR*(1 - zWeigh) + hit_angleZ*zWeigh;
  }

  TVector3 Rn(fRadius, 0., 0.);
  Rn.SetPhi(hit_angle);

  pos = fCenter + Rn;
  pos.SetZ(fM * hit_angle + fT);

  // direction
  TVector3 z(0., 0., -1.);
  dir = z.Cross(pos - fCenter);
  if (GetWinding() > 0)
    dir *= -1;

  dir.SetTheta(TMath::Pi() - fDip);

  dir.SetMag(1.);
}


Double_t
STRiemannTrack::GetMom(Double_t Bz) const
{ // Bz in kGauss!!!!!!
  if (!fIsFitted && !fIsInitialized)
    return 0;

  if (fSinDip < 1E-2)
    return TMath::Abs(fRadius/1.E-2 * 0.00003 * Bz);

  return TMath::Abs(fRadius/fSinDip * 0.00003 * Bz);
}

TVector3 // Need to be studied
STRiemannTrack::PocaToIP(Double_t z) const
{
  TVector3 POCA(0, 0, z);

  if (!fIsFitted)
    return POCA;

//  STHitHit *TestHit = new STHitHit(POCA, 1, 0);
//  STRiemannHit *TestHit = new STRiemannHit(TestHit);
//  Double_t hDist(DistHelix(TestHit, kTRUE, kTRUE, &POCA));

//  delete TestHit;
//  delete TestHit;

  return POCA;
}

Double_t
STRiemannTrack::GetResolution() const
{
  if (fIsInitialized || !fIsFitted)
    return 1.E3;

  static const Double_t minrms = 1E-4;
  Double_t rms = minrms;

  if (fRms > minrms)
    rms = fRms;

  Double_t projLength = TMath::Abs((GetFirstHit() -> GetAngleOnHelix() - GetLastHit() -> GetAngleOnHelix()) * fRadius);

  if (projLength < 1.)projLength = 1.;

  // estimate the resolution
  return rms*fRiemannScale/(projLength*projLength) * sqrt(720/(GetNumHits() + 4)); // from pdg book, rms instead of epsilon (spacial resolution)
}

Double_t
STRiemannTrack::GetQuality() const
{
  if (fIsInitialized || !fIsFitted)
    return 0;

  // todo: this is a test
  Double_t q = GetNumHits();
  q /= 40;

  if (q > 1)
    return 1.;

  return q;

  Double_t res = sqrt(1./GetResolution())/50.; // invert and scale [0..1]

  if (res > 1)
    return 1.;

  return res;
}


void
STRiemannTrack::Plot(Bool_t standalone)
{
  TCanvas *cc = NULL;

  if (standalone)
    cc = new TCanvas("c");

  TPolyMarker3D *maker = new TPolyMarker3D(fHits.size());
  TPolyLine3D *line = new TPolyLine3D(fHits.size());

  for (Int_t it = 0; it < fHits.size(); it++) {
    TVector3 pos = fHits[it] -> GetHit() -> GetPosition();
    //pos.Print();
    maker -> SetPoint(it, pos.X(), pos.Y(), pos.Z());
    line -> SetPoint(it, pos.X(), pos.Y(), pos.Z());
  }

  maker -> SetMarkerStyle(23);
  maker -> Draw();
  line -> Draw();

  if (standalone) {
    gApplication -> SetReturnFromRun(kTRUE);
    gSystem -> Run();

    delete maker;
    delete line;
    delete cc;
  }
}

void
STRiemannTrack::InitTargetTrack(Double_t dip, Double_t curvature)
{
  if (GetNumHits() != 1)
    return;

  fIsInitialized = kTRUE;
  fIsFitted = kTRUE;

  TVector3 hit0 = fHits[0] -> GetHit() -> GetPosition();

  // hit1 is the "origin" or vertex of the track to be initialized
  Double_t z1 = hit0.Z() - hit0.Perp()/TMath::Tan(TMath::PiOver2() - dip);
  TVector3 hit1(0, 0, z1);

  // init plane parameters
  fN.SetXYZ(hit0.X(), hit0.Y(), curvature);
  fN.RotateZ(TMath::PiOver2()); // rotate 90 deg
  fN.SetMag(1.);
  fC = 0.; // track coming from origin

  // calc fCenter and fRadius
  CenterR();

  fRadius = (hit0 - fCenter).Perp();

  // get angle of hit0
  hit0 -= fCenter;
  Double_t angle0 = hit0.Phi(); // [-pi, pi]
  fHits[0] -> SetAngleOnHelix(angle0); // set angle of first hit relative to x axis

  // get angle of hit1
  hit1 -= fCenter;
  Double_t angle1 = hit1.Phi(); // [-pi, pi]

  fM = (z1 - hit0.Z()) / (angle1 - angle0) * -1.;
  fT = hit0.Z() - fM*angle0;

  fDip = TMath::ATan(-1.* fM/fRadius) + TMath::PiOver2();
  fSinDip = TMath::Sin(fDip);

  //std::cout << "dip " << fDip << " angle 0 " << angle0 << "  angle1 " << angle1 << "\n";
}

void
STRiemannTrack::InitCircle(Double_t phi) {
  if (GetNumHits() != 1)
    return;

  fIsInitialized = kTRUE;
  fIsFitted = kTRUE;

  TVector3 hit0 = fHits[0] -> GetHit() -> GetPosition();

  fCenter.SetXYZ(0, 0, 0);
  fRadius = hit0.Perp();

  fM = 0;
  fT = hit0.Z();

  fDip = TMath::PiOver2();
  fSinDip = TMath::Sin(fDip);
}

/*
Int_t
STRiemannTrack::GetClosestRiemannHit(STRiemannHit *hit, Double_t &dist) const {
  TVector3 posX = hit -> x();
  TVector3 pos2;

  Int_t found;
  Double_t mindis = 9.E99;
  Double_t dis;

  for(Int_t it = 0; it < fHits.size(); ++it){
    pos2 = fHits[it] -> x(); 
    dis = (pos2 - posX).Mag();
    if(dis < mindis){
      found = it;
      mindis = dis;
    }
  }
  dist = mindis;
  return found;
}
*/

/*
TVector3
STRiemannTrack::PocaToZ() const
{
  TVector3 POCA(0,0,0);
  if (!fIsFitted) return POCA;

  Double_t closestAngle; // angle on track of hit with smallest distance to z axis
  TVector3 ctrToClHit; // vector from center to hit with smallest distance to z axis
  if (fHits.front() -> GetHit() -> GetPosition().Perp() < fHits.back() -> GetHit() -> GetPosition().Perp()){
    closestAngle = fHits.front() -> getAngleOnHelix();
    ctrToClHit = fHits.front() -> GetHit() -> GetPosition() - fCenter;
  }
  else {
    closestAngle = fHits.back() -> getAngleOnHelix();
    ctrToClHit = fHits.back() -> GetHit() -> GetPosition() - fCenter;
  }

  Double_t angle = closestAngle - ctrToClHit.DeltaPhi(-1.*fCenter); // angle on helix of POCA
  //Double_t angle = (-1.*fCenter).Phi();

  POCA.SetX(1.);
  POCA.SetPhi(angle);
  POCA.SetMag(fRadius);
  POCA += fCenter;
  POCA.SetZ(fM*angle + fT);

  //std::cout<<"POCA "; POCA.Print();

  return POCA;
}
*/
