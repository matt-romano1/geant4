#include "PrimaryGeneratorAction.hh"

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(),
   fParticleGun(nullptr),
   fEnvelopeBox(nullptr)
{
    G4int n_particle = 1;
    fParticleGun = new G4ParticleGun(n_particle);

    // Default particle kinematic
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    G4ParticleDefinition* particle = particleTable->FindParticle(particleName="mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,-1.));
    fParticleGun->SetParticleEnergy(4.*GeV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // This function is called at the beginning of each event

    // Generate cosmic muons with realistic angular distribution
    G4double theta = G4RandGauss::shoot(0., 0.1); // Small angular spread
    if (theta > 0.5) theta = 0.5; // Limit maximum angle
    G4double phi = G4UniformRand() * 2 * CLHEP::pi;
    
    G4double px = sin(theta) * cos(phi);
    G4double py = sin(theta) * sin(phi);
    G4double pz = -cos(theta); // Downward direction
    
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz));
    
    // Energy distribution (simplified - peak around 4 GeV)
    G4double energy = 4.*GeV;
    if (energy < 1.*GeV) energy = 1.*GeV; // Minimum energy
    fParticleGun->SetParticleEnergy(energy);
    
    // Random position above the detector
    G4double x0 = (G4UniformRand() - 0.5) * 15.*cm;
    G4double y0 = (G4UniformRand() - 0.5) * 15.*cm;
    G4double z0 = 50.*cm; // Start above the detector
    
    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
