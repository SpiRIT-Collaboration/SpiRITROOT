#ifndef GETFRAMEINFO
#define GETFRAMEINFO

#include "TObject.h"

class GETFrameInfo : public TObject {
  public:
    GETFrameInfo();

            void  SetDataID(UInt_t value);
            void  SetEventID(UInt_t value);
            void  SetEventTime(ULong64_t value);
            void  SetDeltaT(UInt_t value);
            void  SetStartByte(ULong64_t value);
            void  SetEndByte(ULong64_t value);
            void  SetNextInfo(GETFrameInfo *pointer);

          UInt_t  GetDataID();
          UInt_t  GetEventID();
       ULong64_t  GetEventTime();
          UInt_t  GetDeltaT();
       ULong64_t  GetStartByte();
       ULong64_t  GetEndByte();
    GETFrameInfo *GetNextInfo();
          UInt_t  GetNumFrames();

          Bool_t  IsFill();
            void  Clear(Option_t * = "");

            void  Print();

            void  Copy(GETFrameInfo *frameInfo);

  private:
          UInt_t  fDataID;
          UInt_t  fEventID;
       ULong64_t  fEventTime;
          UInt_t  fDeltaT;
       ULong64_t  fStartByte;
       ULong64_t  fEndByte;
    GETFrameInfo *fNextInfo;

  ClassDef(GETFrameInfo, 1)
};

#endif
