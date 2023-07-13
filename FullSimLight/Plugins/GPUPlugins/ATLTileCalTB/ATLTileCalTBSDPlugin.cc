//---------------------------------------------------------------------------//
//! \file ATLTileCalTBSDPlugin.hh
//! \brief FullSimLight SensitiveDetector plugin for ATLAS TileCal::Scintillator detector
//! offload
//---------------------------------------------------------------------------//

#include <FullSimLight/FSLSensitiveDetectorPlugin.h>
#include <G4Threading.hh>

#include "ATLTileCalTB/ATLTileCalTBSensDet.hh"

//---------------------------------------------------------------------------//
/*!
 * Concrete FSLSensitiveDetectorPlugin for constructing TileCal SD/HitsCollection
 */
class ATLTileCalTBSDPlugin final : public FSLSensitiveDetectorPlugin
{
  public:
    // Constructor
    ATLTileCalTBSDPlugin()
    {
        // Add LV name(s) that should attached this SD
        this->addLogicalVolumeName("Tile::Scintillator");
    }

    // Return new instance of the concrete SD
    G4VSensitiveDetector* getSensitiveDetector() const override
    {
        return new ATLTileCalTBSensDet("caloSD", this->getHitCollectionName());
    }

    // Return the name of the hit collection into which hits go:
    std::string getHitCollectionName() const override
    {
        return "caloHitsCollection";
    }
};

//---------------------------------------------------------------------------//
/*!
 * Factory function that returns a new instance of `ATLTileCalSDPlugin`
 *
 * Required by FullSimLight to make and use our plugin. Its name *must* be
 * `createNAMEOFPLUGINTYPE`, where `NAMEOFPLUGIN` is the class name of the 
 * plugin. It must be compiled into a module librayr named `libNAMEOFPLUGINTYPE.so`
 */
extern "C" FSLSensitiveDetectorPlugin* createATLTileCalTBSDPlugin()
{
    return new ATLTileCalTBSDPlugin;
}
