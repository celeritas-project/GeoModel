#include "MinCelPrimaryGenAction.hh"

#include <cmath>
#include <random>

#include "G4Event.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"



namespace MinCeleritas {
MinCelPrimaryGenAction::MinCelPrimaryGenAction()
  : G4VUserPrimaryGeneratorAction(), fParticleGun{new G4ParticleGun(1)}
{

    // default particle gun parameters (can be changed via UI)
    //
    auto particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("pi+");
    fParticleGun->SetParticleDefinition(particleDefinition);
    fParticleGun->SetParticleEnergy(10. * GeV);
    fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
    constexpr G4double maxCosTheta = 0.997;
    constexpr G4double minCosTheta = 0.9;
    std::mt19937 gen{1};
    std::uniform_real_distribution<G4double> dist;
    G4double z = (maxCosTheta - minCosTheta) * dist(gen) + minCosTheta;
    G4double rho = std::sqrt((1. + z) * (1. - z));
    G4double phi = 2 * M_PI * dist(gen);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(rho * std::cos(phi), rho * std::sin(phi), z));
}

MinCelPrimaryGenAction::~MinCelPrimaryGenAction()
{
    delete fParticleGun;
}

void MinCelPrimaryGenAction::GeneratePrimaries(G4Event* event)
{
    fParticleGun->GeneratePrimaryVertex(event);
}
}  // namespace MinCeleritas