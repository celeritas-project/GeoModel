
#ifndef MyTrackingAction_h
#define MyTrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class G4Track;
class MyEventAction;

class MyTrackingAction : public G4UserTrackingAction {

public:

   MyTrackingAction(MyEventAction*);
  ~MyTrackingAction() override;

   void PreUserTrackingAction(const G4Track*) override;

private:

  MyEventAction *fEventAction;

};

#endif
