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
  iMomentum = track -> GetInitialMom();
  iVertex = track -> GetInitialVertex();
  recotrack_ary = track -> GetRecoTrackArray();
}

STEmbedTrack::~STEmbedTrack()
{
}

void STEmbedTrack::Clear()
{
  iPID = STPID::kNon;
  iMomentum.SetXYZ(-9999,-9999,-9999);
  iVertex.SetXYZ(-999,-999,-999);

  recotrack_ary -> clear();
  
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

STRecoTrack * STEmbedTrack::GetFinalTrack()
{
  STRecoTrack *finaltrack = nullptr;
  if(recotrack_ary -> size() != 0)
    finaltrack = new STRecoTrack(recotrack_ary -> front()); // first is most probable
  else
    finaltrack = new STRecoTrack(); //initialized to 0. track not found

  return finaltrack;
}

