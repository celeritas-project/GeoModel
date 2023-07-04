//---------------------------------------------------------------------------//
//! \file CelerTrackingAction.hh
//! \brief Minimal Geant4 TrackingAction for interfacing to Celeritas SimpleOffload 
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserTrackingAction.hh>

#include "Celeritas.hh"

class CelerTrackingAction : public G4UserTrackingAction
{
  public:
    void PreUserTrackingAction(G4Track const* track)
    {
        CelerSimpleOffload().PreUserTrackingAction(const_cast<G4Track*>(track));
    }
};
