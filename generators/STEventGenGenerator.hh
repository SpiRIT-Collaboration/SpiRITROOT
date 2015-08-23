/**
 * @brief SPiRIT Event Generator
 * @author JungWoo Lee (Korea Univ.)
 *
 * @detail EventGen file must have following format.
 * -# First line (with 1 argument) should read [nEvents].
 * -# Next line (with 2 arguments) comes [eventId] and [nTracks].
 *    [eventId] is just for reading.
 * -# Next #[nTracks] lines follows with [pdg], [px], [py] and [pz].
 * -# Repeat 2. and 3. #[nEvents] times.
 *
 * [nEvents]           \n
 * [eventId] [nTracks] \n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * ...                 \n
 * [eventId] [nTracks] \n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * ...                 \n
 * [eventId] [nTracks] \n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * [pdg] [px] [py] [pz]\n
 * ...                 \n
 * ...                 \n
 *
 *
 * \b example)
 *
 * 10                                      \n
 * 0 181                                   \n
 * 1000020040 0.592732 0.259583 3.45222    \n
 * 1000020040 -0.506001 0.292528 2.11579   \n
 * 1000010030 -0.0118622 -0.105926 2.00932 \n
 * ...                                     \n
 * 1 58                                    \n
 * 1000020040 -0.192129 0.0461225 1.57347  \n
 * 1000020030 -0.00348936 0.0132943 3.30207\n
 * 2212 -0.0527492 -0.214962 0.696101      \n
 * ...                                     \n
 * 2 127                                   \n
 * 1000020040 -0.410034 1.07103 4.34146    \n
 * 1000020040 -0.304002 -0.156396 1.52629  \n
 * 1000020030 -0.0178472 0.464186 1.50824  \n
 * ...                                     \n
 * ...                                     \n
 *
 * @param [nEvents] Total number of events
 * @param [eventId] Event Id.
 * @param [nTracks] Total number of tracks in [eventId](1st argument in same line).
 * @param [pdg] Particle PDG code.
 * @param [px] x-component of momentum [GeV/c].
 * @param [py] y-component of momentum [GeV/c].
 * @param [pz] z-component of momentum [GeV/c].
 *
 */

#ifndef STEVENTGENGENERATOR
#define STEVENTGENGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TVector3.h"

#include <fstream>

class STEventGenGenerator : public FairGenerator
{
  public :

    /** Default constructor without arguments. Do not use this constructor. **/
    STEventGenGenerator();

    /** 
     * Standard constructor.
     * @param fileName The input EventGen file name
     **/
    STEventGenGenerator(TString fileName);

    /** Destructor. **/
    virtual ~STEventGenGenerator();

    /** 
     * Abstract method in base class.
     * Reads event from the input file and pushes the tracks onto the stack. 
     * @param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

    /** Set primary vertex position **/
    void SetPrimaryVertex(Double_t x, Double_t y, Double_t z)
    { fV3Vertex.SetXYZ(x,y,z); };

    /** Get number of events written in EventGen file **/
    Int_t GetNEvents() { return fNEvents; };

  private :

    TString  fGenFileName;   //!< EventGen file name
    std::ifstream fGenFile;  //! < EventGen file

    TVector3 fV3Vertex;      //!< Position of primary vertex
    Int_t    fNEvents;       //!< Total number of events


  STEventGenGenerator(const STEventGenGenerator&);
  STEventGenGenerator& operator=(const STEventGenGenerator&);

  ClassDef(STEventGenGenerator,1);
};

#endif
