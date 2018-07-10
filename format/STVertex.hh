/* Copyright 2008-2010, Technische Universitaet Muenchen,
   Authors: Christian Hoeppner & Sebastian Neubert & Johannes Rauch

   This file is part of GENFIT.

   GENFIT is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GENFIT is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with GENFIT.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @brief STVertex Class
 *
 * @author Genie Jhang
 *
 * @info
 * The same class as GFRaveVertex but the unit of the position and covariance matrix is in mm.
 */

#ifndef STVERTEX
#define STVERTEX

#include "TObject.h"
#include "GFRaveVertex.h"
#include "GFRaveTrackParameters.h"

class STVertex : public TObject {
  public:
    STVertex();
    STVertex(STVertex *vertex);
    STVertex(const genfit::GFRaveVertex &);

    virtual ~STVertex();

    void SetPos(TVector3 pos)  { fPos = pos; }
    TVector3 GetPos()    const { return fPos; }

    TMatrixDSym GetCov() const { return fCov; }

    Double_t GetNDF()    const { return fNDF; }
    Double_t GetChi2()   const { return fChi2; }

    UInt_t GetNTracks()  const { return fSmoothedTracks.size(); }
    std::vector<genfit::GFRaveTrackParameters *> *GetSmoothedTrackArray() { return &fSmoothedTracks; }
    genfit::GFRaveTrackParameters *GetParameters(UInt_t idx) const { return fSmoothedTracks[idx]; }

    Int_t GetID()        const { return fID; }
    void Print(const Option_t * = "") const;

    void SetIsCollisionVertex(Bool_t val = kTRUE) { fIsCollisionVertex = val; }
    Bool_t IsCollisionVertex() { return fIsCollisionVertex; }

    void SetIsTargetVertex(Bool_t val = kTRUE) { fIsTargetVertex = val; }
    Bool_t IsTargetVertex() { return fIsTargetVertex; }

    void SetIsGoodBDC(Bool_t val = kTRUE) { fIsGoodBDC = val; }
    Bool_t IsGoodBDC() { return fIsGoodBDC; }

  private:
    TVector3 fPos;
    TMatrixDSym fCov;
    Double_t fNDF;
    Double_t fChi2;
    Int_t fID;

    Bool_t fIsCollisionVertex = false;
    Bool_t fIsTargetVertex = false;
    Bool_t fIsGoodBDC = false;

    std::vector<genfit::GFRaveTrackParameters *> fSmoothedTracks; //->

  ClassDef(STVertex, 4)
};

#endif
