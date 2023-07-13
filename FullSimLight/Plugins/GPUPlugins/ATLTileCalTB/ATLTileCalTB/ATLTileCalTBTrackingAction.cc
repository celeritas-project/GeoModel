//**************************************************
// \file ATLTileCalTBTrackingAction.cc
// \brief: implementation of ATLTileCalTBTrackingAction
//         class
//**************************************************

// Includes from project files
//
#include "ATLTileCalTBTrackingAction.hh"

// Includes from Celeritas
//
#include "Celeritas.hh"

void ATLTileCalTBTrackingAction::PreUserTrackingAction(G4Track const* track)
{
  CelerSimpleOffload().PreUserTrackingAction(const_cast<G4Track*>(track));
}
