#include "MinCelPrimaryGenAction.hh"

#include <cmath>

#include "G4Event.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"



namespace MinCeleritas {
MinCelPrimaryGenAction::MinCelPrimaryGenAction()
  : G4VUserPrimaryGeneratorAction(),
    fParticleGun{std::make_unique<G4ParticleGun>(1)},
    fRNG{},
    fGen{std::bind(std::generate_canonical<G4double, std::numeric_limits<G4double>::digits, decltype(fRNG)>, std::ref(fRNG))}
{

    // default particle gun parameters (can be changed via UI)
    //
    auto particleDefinition = G4ParticleTable::GetParticleTable()->FindParticle("pi+");
    fParticleGun->SetParticleDefinition(particleDefinition);
    fParticleGun->SetParticleEnergy(10. * GeV);
    fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
}

void MinCelPrimaryGenAction::GeneratePrimaries(G4Event* event)
{
    constexpr G4double maxCosTheta = 0.997;
    constexpr G4double minCosTheta = 0.9;
    fRNG.seed(event->GetEventID());
    G4double z = (maxCosTheta - minCosTheta) * fGen() + minCosTheta;
    G4double rho = std::sqrt((1. + z) * (1. - z));
    G4double phi = 2 * M_PI * fGen();
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(rho * std::cos(phi), rho * std::sin(phi), z));
    fParticleGun->GeneratePrimaryVertex(event);
}
}  // namespace MinCeleritas