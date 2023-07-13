//---------------------------------------------------------------------------//
//! \file MinCeleritasUserActionPlugin.cc
//! \brief FullSimLight UserActionPlugin to enable minimal Celeritas offload
//---------------------------------------------------------------------------//

#include <FullSimLight/FSLUserActionPlugin.h>
#include <G4Threading.hh>

#include "MinCeleritas/EventAction.hh"
#include "MinCeleritas/RunAction.hh"
#include "MinCeleritas/TrackingAction.hh"
#include "MinCeleritas/Celeritas.hh"

namespace MinCeleritas {
//---------------------------------------------------------------------------//
/*!
 * Concrete FSLUserActionPlugin for constructing user actions for Celeritas offload
 */
class UserActionPlugin final : public FSLUserActionPlugin
{
  public:
    UserActionPlugin();

    G4UserEventAction* getEventAction() const override
    {
        return new EventAction;
    }

    G4UserRunAction* getRunAction() const override
    {
        return new RunAction;
    }

    G4UserTrackingAction* getTrackingAction() const override
    {
        return new TrackingAction;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Constructor to assist in initializing Celeritas on master/worker threads
 *
 * `celeritas::SimpleOffload`s `BuildForMaster/Master` member functions are intended
 * to be called in the equivalent `G4VUserActionInitialization` member functions, but 
 * we don't have access to this in FullSimLight's plugin mechanism. A plugin instance 
 * is however constructed in each of these functions, so the constructor is used to
 * call these functions at the appropriate time/thread (but it is a slight hack)
 */
UserActionPlugin::UserActionPlugin()
{
    if (G4Threading::IsMasterThread())
    {
        CelerSimpleOffload().BuildForMaster(&CelerSetupOptions(),
                                            &CelerSharedParams());
    }
    else
    {
        CelerSimpleOffload().Build(&CelerSetupOptions(),
                                   &CelerSharedParams(),
                                   &CelerLocalTransporter());
    }
}
}

//---------------------------------------------------------------------------//
/*!
 * Factory function that returns a new instance of `MinCeleritas::UserActionPlugin`
 *
 * Required by FullSimLight to make and use our plugin. Its name *must* be
 * `createNAMEOFLIBRARY`, where `NAMEOFLIBRARY` is the name of the library
 * in which the plugin is compiled, i.e `libNAMEOFLIBRARY.so`.
 */
extern "C" FSLUserActionPlugin* createMinCeleritasUserActionPlugin()
{
    return new MinCeleritas::UserActionPlugin;
}
