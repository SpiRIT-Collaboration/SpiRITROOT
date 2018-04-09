#include "STEmbedTrack.hh"

#include <iostream>
using namespace std;

ClassImp(STEmbedTrack)

STEmbedTrack::STEmbedTrack()
{
  Clear();
}

STEmbedTrack::STEmbedTrack(STEmbedTrack *track)
{
  Clear();
  iPID = track -> GetInitialPID();
  fPID = track -> GetFinalPID();

  iMomentum = track -> GetInitialMom();
  fMomentum = track -> GetFinalMom();

  iVertex = track -> GetInitialVertex();
  fVertex = track -> GetFinalVertex();
}

STEmbedTrack::~STEmbedTrack()
{
}

void STEmbedTrack::Clear()
{
  //  fIsMatched = false;

  iPID = STPID::kNon;
  fPID = STPID::kNon;
    
  iMomentum.SetXYZ(-9999,-9999,-9999);
  fMomentum.SetXYZ(-9999,-9999,-9999);

  iVertex.SetXYZ(-999,-999,-999);
  fVertex.SetXYZ(-999,-999,-999);

}

void STEmbedTrack::SetFinalTrack(STRecoTrack *track)
{
  fMomentum = track -> GetMomentum();
  fPID      = track -> GetPID();
  fVertex   = track -> GetPOCAVertex();
}

void STEmbedTrack::SetInitialTrack(STMCTrack *track)
{
  track -> GetMomentum(iMomentum);
  track -> GetStartVertex(iVertex);
  int pid_c = track -> GetPdgCode();
  if(pid_c == 0)
    iPID = STPID::kNon;
  else if(pid_c == 211)
    iPID = STPID::kPion;
  else if(pid_c == 2212)
    iPID = STPID::kProton;
  else if(pid_c == 1000010020)
    iPID = STPID::kDeuteron;
  else if(pid_c ==1000010030)
    iPID = STPID::kTriton;
  else if(pid_c == 1000020030)
    iPID = STPID::k3He;
  else if(pid_c == 1000020040)
    iPID = STPID::k4He;

}



