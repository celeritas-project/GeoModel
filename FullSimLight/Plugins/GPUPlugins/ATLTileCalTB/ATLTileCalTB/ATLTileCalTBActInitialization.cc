//**************************************************
// \file ATLTileCalTBActInitialization.cc
// \brief: implementation of
//         ATLTileCalTBActInitialization class
// \author: Lorenzo Pezzotti (CERN EP-SFT-sim)
//          @lopezzot
// \start date: 23 May 2022
//**************************************************

// Includers from project files
//
#include "ATLTileCalTBActInitialization.hh"

#include "ATLTileCalTBEventAction.hh"
#include "ATLTileCalTBPrimaryGenAction.hh"
#include "ATLTileCalTBRunAction.hh"
#include "ATLTileCalTBStepAction.hh"
#include "ATLTileCalTBTrackingAction.hh"
#include "Celeritas.hh"

// Constructor and de-constructor
//
ATLTileCalTBActInitialization::ATLTileCalTBActInitialization() : G4VUserActionInitialization() {}

ATLTileCalTBActInitialization::~ATLTileCalTBActInitialization() {}

// Define Build() and BuildForMaster() methods
//
void ATLTileCalTBActInitialization::BuildForMaster() const
{
  G4cout << "BUILDFORMASTER" << G4endl;
  auto EventAction = new ATLTileCalTBEventAction(nullptr);
  SetUserAction(new ATLTileCalTBRunAction(EventAction));

  CelerSimpleOffload().BuildForMaster(&CelerSetupOptions(), &CelerSharedParams());
}

void ATLTileCalTBActInitialization::Build() const
{
  G4cout << "buildforworker" << G4endl;
  auto PrimaryGenAction = new ATLTileCalTBPrimaryGenAction();
  auto EventAction = new ATLTileCalTBEventAction(PrimaryGenAction);

  SetUserAction(PrimaryGenAction);
  SetUserAction(new ATLTileCalTBRunAction(EventAction));
  SetUserAction(EventAction);
  SetUserAction(new ATLTileCalTBStepAction(EventAction));

  CelerSimpleOffload().Build(&CelerSetupOptions(), &CelerSharedParams(), &CelerLocalTransporter());
  SetUserAction(new ATLTileCalTBTrackingAction());
}

//**************************************************
