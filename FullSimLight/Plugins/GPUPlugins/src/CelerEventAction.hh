//---------------------------------------------------------------------------//
//! \file CelerEventAction.hh
//! \brief Minimal Geant4 EventAction for interfacing to Celeritas SimpleOffload 
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserEventAction.hh>

#include "Celeritas.hh"

class CelerEventAction : public G4UserEventAction
{
  public:
    void BeginOfEventAction(G4Event const* event)
    {
        CelerSimpleOffload().BeginOfEventAction(event);
    }

    void EndOfEventAction(G4Event const* event)
    {
        CelerSimpleOffload().EndOfEventAction(event);
    }
};
