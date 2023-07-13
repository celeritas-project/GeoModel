//**************************************************
// \file ATLTileCalTBTrackingAction.hh
// \brief: definition of ATLTileCalTBTrackingAction
//         class
//**************************************************

#ifndef ATLTileCalTBTrackingAction_h
#  define ATLTileCalTBTrackingAction_h 1

// Includers from Geant4
//
#  include "G4UserTrackingAction.hh"

class ATLTileCalTBTrackingAction : public G4UserTrackingAction
{
 public:
  void PreUserTrackingAction(G4Track const* track);
};

#endif  // ATLTileCalTBTrackingAction_h 1

//**************************************************
