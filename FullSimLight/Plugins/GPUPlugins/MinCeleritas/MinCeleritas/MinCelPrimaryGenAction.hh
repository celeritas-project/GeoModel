#pragma once

#include <functional>
#include <memory>
#include <random>
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

    virtual void GeneratePrimaries(G4Event* event);
  private:
    std::unique_ptr<G4ParticleGun> fParticleGun;
    std::mt19937 fRNG;
    std::function<G4double()> fGen;
};
} // namespace MinCeleritas