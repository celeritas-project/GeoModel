#pragma once

// Includers from Geant4
//
#  include "G4Types.hh"
#  include "G4VUserPrimaryGeneratorAction.hh"

// Forward declaration from Geant4
//
class G4ParticleGun;
class G4Event;

namespace MinCeleritas {

class MinCelPrimaryGenAction : public G4VUserPrimaryGeneratorAction
{
  public:
    MinCelPrimaryGenAction();
    virtual ~MinCelPrimaryGenAction();

    virtual void GeneratePrimaries(G4Event* event);
  private:
    G4ParticleGun* fParticleGun;
};
} // namespace MinCeleritas