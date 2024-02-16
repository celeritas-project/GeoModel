//---------------------------------------------------------------------------//
//! \file ATLTileCalTBUserActionPlugin.hh
//! \brief FullSimLight UserActionPlugin for ATLAS TileCal Test Beam offload
//---------------------------------------------------------------------------//

#include <FullSimLight/FSLUserActionPlugin.h>
#include <G4Threading.hh>

#include "ATLTileCalTB/ATLTileCalTBEventAction.hh"
#include "ATLTileCalTB/ATLTileCalTBPrimaryGenAction.hh"
#include "ATLTileCalTB/ATLTileCalTBRunAction.hh"
#include "ATLTileCalTB/ATLTileCalTBStepAction.hh"
#include "ATLTileCalTB/ATLTileCalTBTrackingAction.hh"
#include "ATLTileCalTB/Celeritas.hh"

//---------------------------------------------------------------------------//
/*!
 * Concrete FSLUserActionPlugin for constructing user actions for Celeritas
 * offload
 */
class ATLTileCalTBUserActionPlugin final : public FSLUserActionPlugin
{
  public:
    ATLTileCalTBUserActionPlugin();

    G4UserEventAction* getEventAction() const override
    {
        // Event action needs primary generator action, so ensure only one instance per thread
        static thread_local auto* evtAct_ = new ATLTileCalTBEventAction{dynamic_cast<ATLTileCalTBPrimaryGenAction*>(this->getPrimaryGeneratorAction())};
        return evtAct_;
    }

    G4UserRunAction* getRunAction() const override
    {
        // Unique instance on master due to null primary generator action here
        if (G4Threading::IsMasterThread())
        {
            return new ATLTileCalTBRunAction{
                new ATLTileCalTBEventAction{nullptr}};
        }

        // Otherwise connect up to current thread's event action
        // Not thread_local because not other action needs worker Run Action.
        return new ATLTileCalTBRunAction{dynamic_cast<ATLTileCalTBEventAction*>(this->getEventAction())};
    }

    G4UserSteppingAction* getSteppingAction() const override
    {
        return nullptr;
    }

    G4UserTrackingAction* getTrackingAction() const override
    {
        // Tracking Action is not used by any other action, so we can just return a fresh instance
        return new ATLTileCalTBTrackingAction;
    }

    G4VUserPrimaryGeneratorAction* getPrimaryGeneratorAction() const override
    {
        // Primary Generator Action has no dependencies, but must be thread_local so
        // Event Action can connect up to this thread's instance
        static thread_local auto* primGenAct_ = new ATLTileCalTBPrimaryGenAction;
        return primGenAct_;
    }
};

//---------------------------------------------------------------------------//
/*!
 * Constructor to assist in initializing Celeritas on master/worker threads
 *
 * `celeritas::SimpleOffload`s `BuildForMaster/Master` member functions are
 * intended to be called in the equivalent `G4VUserActionInitialization` member
 * functions, but we don't have access to this in FullSimLight's plugin
 * mechanism. A plugin instance is however constructed in each of these
 * functions, so the constructor is used to call these functions at the
 * appropriate time/thread (but it is a slight hack)
 *
 */
ATLTileCalTBUserActionPlugin::ATLTileCalTBUserActionPlugin()
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

//---------------------------------------------------------------------------//
/*!
 * Factory function that returns a new instance of `CelerUserActionPlugin`
 *
 * Required by FullSimLight to make and use our plugin. Its name *must* be
 * `createNAMEOFLIBRARY`, where `NAMEOFLIBRARY` is the name of the library
 * in which the plugin is compiled, i.e `libNAMEOFLIBRARY.so`.
 */
extern "C" FSLUserActionPlugin* createATLTileCalTBUserActionPlugin()
{
    return new ATLTileCalTBUserActionPlugin;
}
