#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"

#include "G4Step.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

// Changed from CSV to ROOT
#include "G4RootAnalysisManager.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
 : G4UserSteppingAction(),
   fEventAction(eventAction),
   fScoringVolume(nullptr),
   fSpectrumNtupleId(-1),
   fEdepNtupleId(-1),
   fMuonTrackNtupleId(-1)
{
    if (!fEventAction) {
        G4Exception("SteppingAction::SteppingAction()", "NoEventAction",
                    FatalException, "EventAction pointer is null in constructor.");
    }
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Changed from CSV to ROOT
    G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();

    if (!fScoringVolume) {
        const DetectorConstruction* detectorConstruction =
            static_cast<const DetectorConstruction*>(
                G4RunManager::GetRunManager()->GetUserDetectorConstruction());
        if (detectorConstruction) {
            fScoringVolume = detectorConstruction->GetScintillatorLV();
        }
        if (!fScoringVolume) {
            G4Exception("SteppingAction::UserSteppingAction()", "NoScoringVolume",
                        FatalException, "fScoringVolume not set!");
            return;
        }

        const RunAction* runAction = static_cast<const RunAction*>(fEventAction->GetRunAction());
        if (runAction) {
            fSpectrumNtupleId = runAction->GetSpectrumNtupleId();
            fEdepNtupleId = runAction->GetEdepNtupleId();
            fMuonTrackNtupleId = runAction->GetMuonTrackNtupleId();
        } else {
            G4Exception("SteppingAction::UserSteppingAction()", "NoRunActionFromEvent",
                        FatalException, "Could not get RunAction from EventAction to retrieve NTuple IDs.");
            return;
        }

        if (fSpectrumNtupleId < 0 || fEdepNtupleId < 0 || fMuonTrackNtupleId < 0) {
            G4String errMsg = "One or more NTuple IDs were not properly set from RunAction. IDs are: Spectrum=" +
                              std::to_string(fSpectrumNtupleId) + ", Edep=" + std::to_string(fEdepNtupleId) +
                              ", MuonTrack=" + std::to_string(fMuonTrackNtupleId);
            G4Exception("SteppingAction::UserSteppingAction()", "InvalidNtupleIDs",
                        FatalException, errMsg);
            return;
        }
        G4cout << "SteppingAction: NTuple IDs initialized. SpectrumID: " << fSpectrumNtupleId
               << ", EdepID: " << fEdepNtupleId << ", MuonTrackID: " << fMuonTrackNtupleId << G4endl;
    }

    G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    G4StepPoint* preStepPoint = step->GetPreStepPoint();
    G4VPhysicalVolume* preStepPhysicalVolume = preStepPoint->GetTouchableHandle()->GetVolume();
    if (!preStepPhysicalVolume) return;
    G4LogicalVolume* preStepLogicalVolume = preStepPhysicalVolume->GetLogicalVolume();

    if (preStepLogicalVolume == fScoringVolume) {
        G4int cellID = preStepPoint->GetTouchableHandle()->GetCopyNumber(0);
        G4double edepStep = step->GetTotalEnergyDeposit();
        if (edepStep > 0.) {
            analysisManager->FillNtupleIColumn(fEdepNtupleId, 0, eventID);
            analysisManager->FillNtupleIColumn(fEdepNtupleId, 1, cellID);
            analysisManager->FillNtupleDColumn(fEdepNtupleId, 2, edepStep / MeV);
            analysisManager->AddNtupleRow(fEdepNtupleId);

            if (fEventAction) {
                 fEventAction->AddEdep(edepStep);
            }
        }

        G4StepPoint* postStepPoint = step->GetPostStepPoint();
        if (postStepPoint->GetStepStatus() == fGeomBoundary) {
            G4Track* track = step->GetTrack();
            G4ParticleDefinition* particleDef = track->GetDefinition();
            G4String particleName = particleDef->GetParticleName();
            G4double energy = track->GetKineticEnergy();
            if (particleName == "opticalphoton") {
                energy = track->GetTotalEnergy();
            }
            analysisManager->FillNtupleIColumn(fSpectrumNtupleId, 0, eventID);
            analysisManager->FillNtupleIColumn(fSpectrumNtupleId, 1, cellID);
            analysisManager->FillNtupleSColumn(fSpectrumNtupleId, 2, particleName);
            analysisManager->FillNtupleDColumn(fSpectrumNtupleId, 3, energy / MeV);
            analysisManager->AddNtupleRow(fSpectrumNtupleId);
        }
    }

    G4Track* currentTrack = step->GetTrack();
    if (currentTrack->GetTrackID() == 1 && currentTrack->GetDefinition()->GetParticleName() == "mu-") {
        G4StepPoint* postStepPoint = step->GetPostStepPoint();
        G4ThreeVector prePos = preStepPoint->GetPosition();
        G4ThreeVector postPos = postStepPoint->GetPosition();

        analysisManager->FillNtupleIColumn(fMuonTrackNtupleId, 0, eventID);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 1, prePos.x() / cm);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 2, prePos.y() / cm);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 3, prePos.z() / cm);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 4, postPos.x() / cm);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 5, postPos.y() / cm);
        analysisManager->FillNtupleDColumn(fMuonTrackNtupleId, 6, postPos.z() / cm);
        analysisManager->AddNtupleRow(fMuonTrackNtupleId);
    }
}