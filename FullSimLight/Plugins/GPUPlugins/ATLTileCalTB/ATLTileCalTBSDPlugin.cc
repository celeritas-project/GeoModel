//---------------------------------------------------------------------------//
//! \file ATLTileCalTBSDPlugin.hh
//! \brief FullSimLight SensitiveDetector plugin for ATLAS TileCal Test Beam
//! offload
//---------------------------------------------------------------------------//

#include <FullSimLight/FSLSensitiveDetectorPlugin.h>
#include <G4Threading.hh>

#include "ATLTileCalTB/ATLTileCalTBSensDet.hh"

//---------------------------------------------------------------------------//
/*!
 * Concrete FSLSensitiveDetectorPlugin for constructing TileCal SDs
 */
class ATLTileCalTBSDPlugin final : public FSLSensitiveDetectorPlugin
{
  public:
    // Constructor
    ATLTileCalTBSDPlugin()
    {
        this->addLogicalVolumeName("Tile::Scintillator");
    }

    // Get the senstive detector
    G4VSensitiveDetector* getSensitiveDetector() const override
    {
        return new ATLTileCalTBSensDet("caloSD", this->getHitCollectionName());
    }

    // Get the name of the hit collection into which hits go:
    std::string getHitCollectionName() const override
    {
        return "caloHitsCollection";
    }
};

//---------------------------------------------------------------------------//
/*!
 * Factory function that returns a new instance of `CelerUserActionPlugin`
 *
 * Required by FullSimLight to make and use our plugin. Its name *must* be
 * `createNAMEOFLIBRARY`, where `NAMEOFLIBRARY` is the name of the library
 * in which the plugin is compiled, i.e `libNAMEOFLIBRARY.so`.
 */
extern "C" FSLSensitiveDetectorPlugin* createATLTileCalTBSDPlugin()
{
    return new ATLTileCalTBSDPlugin;
}
