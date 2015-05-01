// ------------------------------------------------------------------------
// -----                  STDetectorList header file                  -----
// -----                 Created 12/06/07  by V. Friese               -----
// -----              Redesigned 13/12/19  by G. Jhang                -----
// ------------------------------------------------------------------------


/** Defines unique identifier for all SPiRIT detector systems **/

#ifndef STDETECTORLIST_H
#define STDETECTORLIST_H 1

// kSTOPHERE is needed for iteration over the enum. All detectors have to be put before.
enum DetectorId {kREF, kSPiRIT, kTutDet, kSTOPHERE};

#endif
