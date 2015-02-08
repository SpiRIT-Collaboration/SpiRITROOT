/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TVector3.h"

#include <fstream>

class STSimpleEventGenerator : public FairGenerator
{
  public :

    /** Default constructor without arguments. Do not use this constructor. **/
    STSimpleEventGenerator();

    /** 
     * Standard constructor.
     * @param fileName The input EventGen file name
     **/
    STSimpleEventGenerator(TString fileName);

    /** Destructor. **/
    virtual ~STSimpleEventGenerator();

    /** 
     * Abstract method in base class.
     * Reads event from the input file and pushes the tracks onto the stack. 
     * @param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

    /** Set primary vertex position **/
    void SetPrimaryVertex(Double_t x, Double_t y, Double_t z)
    { fV3Vertex.SetXYZ(x,y,z); };

  private :

    TString  fGenFileName; //!< EventGen file name
    ifstream fGenFile;     //!< EventGen file

    TVector3 fV3Vertex;    //!< Position of primary vertex



  STSimpleEventGenerator(const STSimpleEventGenerator&);
  STSimpleEventGenerator& operator=(const STSimpleEventGenerator&);

  ClassDef(STSimpleEventGenerator,1);
};
