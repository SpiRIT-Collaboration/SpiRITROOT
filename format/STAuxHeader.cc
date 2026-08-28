#include "STAuxHeader.hh"

#include <iostream>
using namespace std;

ClassImp(STAuxHeader)

STAuxHeader::STAuxHeader()
{
  Clear();
}

void STAuxHeader::Clear(Option_t *option)
{
  fTpcEventNum = -1;
  fBdcID = -1;
  fTpcTime = -1;
}

/*void STEventHeader::Print(Option_t *option) const
{
  TString status = "Not Set Event";

       if (fStatus == 0) status = "Empty Event";
  else if (fStatus == 1) status = "Collision Event";
  else if (fStatus == 2) status = "Active Target Event";
  else if (fStatus == 3) status = "Off Target Event";
  else if (fStatus == 4) status = "Beam Event";
  else if (fStatus == 5) status = "Cosmic Event";
  else if (fStatus == 6) status = "Bad Event";

  cout << " STEventHeader:  Event " << fEventID << "  " << status << endl;
}*/

