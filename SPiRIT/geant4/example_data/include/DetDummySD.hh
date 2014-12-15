#ifndef DETDUMMYSD_h
#define DETDUMMYSD_h 1

#include "G4VSensitiveDetector.hh"

class G4Step;

class DetDummySD : public G4VSensitiveDetector
{
public:
  DetDummySD();
  ~DetDummySD() {};
  
  void Initialize(G4HCofThisEvent* ) {};
  G4bool ProcessHits(G4Step* ,G4TouchableHistory*) {return false;}
  void EndOfEvent(G4HCofThisEvent*) {};
  void clear() {};
  void DrawAll() {};
  void PrintAll() {};
};

DetDummySD::DetDummySD() : G4VSensitiveDetector("dummySD")
{}
#endif
