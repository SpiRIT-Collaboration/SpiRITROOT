#ifndef DETHIT_h
#define DETHIT_h 1

#include "G4ThreeVector.hh"
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

class DetHit: public G4VHit
{
	private:
		G4double fX, fY, fZ, fT, fE;
    G4int fParentID;

	public:
		DetHit();
		DetHit(G4double, G4double, G4double, G4double, G4double, G4int);
		virtual ~DetHit();

		DetHit(const DetHit &right);
		const DetHit &operator=(const DetHit &right);

		// new/delete operators
		void *operator new(size_t);
		void operator delete(void *aHit);

		// set/get functions
		G4double GetX() const { return fX; };
		G4double GetY() const { return fY; };
		G4double GetZ() const { return fZ; };
		G4double GetT() const { return fT; };
		G4double GetE() const { return fE; };
		G4int    GetParentID() const { return fParentID; };
};

inline DetHit::DetHit(const DetHit &right)
:G4VHit()
{
	fX = right.fX;
	fY = right.fY;
	fZ = right.fZ;
	fT = right.fT;
	fE = right.fE;
	fParentID = right.fParentID;
}

inline const DetHit &DetHit::operator=(const DetHit &right)
{
	fX = right.fX;
	fY = right.fY;
	fZ = right.fZ;
	fT = right.fT;
	fE = right.fE;
	fParentID = right.fParentID;
	return *this;
}

extern G4Allocator<DetHit> DetHitAllocator;

inline void *DetHit::operator new(size_t)
{
	void *aHit = (void *)DetHitAllocator.MallocSingle();
	return aHit;
}

inline void DetHit::operator delete(void *aHit)
{
	DetHitAllocator.FreeSingle((DetHit *) aHit);
}
#endif
