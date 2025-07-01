#include "RunAction.hh"
#include "G4RunManager.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"

// For ROOT output
#include "G4RootAnalysisManager.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction()
 : G4UserRunAction(),
   fEdep("Edep", 0.),
   fEdep2("Edep2", 0.),
   fSpectrumNtupleId(-1),
   fEdepNtupleId(-1),
   fMuonTrackNtupleId(-1),
   fAnalysisManager(nullptr)
{
    // Register thread-local accumulables
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->RegisterAccumulable(fEdep);
    accumulableManager->RegisterAccumulable(fEdep2);

    // Get the ROOT analysis manager instance
    G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
    fAnalysisManager = analysisManager;
    analysisManager->SetVerboseLevel(1);

    analysisManager->SetNtupleMerging(true);
    
    // Set compression level to reduce file size
    analysisManager->SetCompressionLevel(1);

    // Set the base filename
    if (G4Threading::IsMasterThread() || analysisManager->GetFileName().empty()) {
        analysisManager->SetFileName("tomography_output");
        G4cout << "RunAction (Master or first thread): ROOT Analysis Manager. Base filename set to: "
               << analysisManager->GetFileName() << G4endl;
    }

    // IMPORTANT: Create smaller buffer sizes for ntuples to force more frequent writes
    // This helps prevent memory overflow
    
    // Ntuple for SpectrumData (Photon Data)
    fSpectrumNtupleId = analysisManager->CreateNtuple("SpectrumData", "Particle data (photons, etc.) reaching cell boundary");
    analysisManager->CreateNtupleIColumn("EventID");
    analysisManager->CreateNtupleIColumn("CellID");
    analysisManager->CreateNtupleSColumn("ParticleName");
    analysisManager->CreateNtupleDColumn("EnergyMeV");
    analysisManager->FinishNtuple();

    // Ntuple for EdepData
    fEdepNtupleId = analysisManager->CreateNtuple("EdepData", "Energy depositions in cells");
    analysisManager->CreateNtupleIColumn("EventID");
    analysisManager->CreateNtupleIColumn("CellID");
    analysisManager->CreateNtupleDColumn("EdepMeV");
    analysisManager->FinishNtuple();

    // Ntuple for True Muon Trajectory Data
    fMuonTrackNtupleId = analysisManager->CreateNtuple("MuonTrackData", "Primary Muon Step-by-Step Trajectory");
    analysisManager->CreateNtupleIColumn("EventID");
    analysisManager->CreateNtupleDColumn("PreStepX_cm");
    analysisManager->CreateNtupleDColumn("PreStepY_cm");
    analysisManager->CreateNtupleDColumn("PreStepZ_cm");
    analysisManager->CreateNtupleDColumn("PostStepX_cm");
    analysisManager->CreateNtupleDColumn("PostStepY_cm");
    analysisManager->CreateNtupleDColumn("PostStepZ_cm");
    analysisManager->FinishNtuple();

    // Activate the manager
    analysisManager->SetActivation(true);

    G4cout << "RunAction (Thread " << G4Threading::G4GetThreadId()
           << "): Ntuples defined and manager activated. Filename: "
           << analysisManager->GetFileName() 
           << " | NTuple IDs: Spectrum=" << fSpectrumNtupleId 
           << ", Edep=" << fEdepNtupleId 
           << ", MuonTrack=" << fMuonTrackNtupleId << G4endl;
}

RunAction::~RunAction()
{
}

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
    G4cout << "RunAction (Thread " << G4Threading::G4GetThreadId()
           << "): BeginOfRunAction for run " << aRun->GetRunID() << G4endl;

    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    // Open ROOT file
    G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
    if (!analysisManager->OpenFile()) {
        G4Exception("RunAction::BeginOfRunAction",
                    "AnalysisFileOpenError", FatalException,
                    "Failed to open ROOT analysis file");
    }
    G4cout << "RunAction (Thread " << G4Threading::G4GetThreadId()
           << "): Called OpenFile. Effective filename for manager: "
           << analysisManager->GetFileName() << ".root" << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();
    G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();

    // Merge and print accumulables (only from master thread for global summary)
    if (G4Threading::IsMasterThread()) {
        if (nofEvents > 0) {
            G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
            accumulableManager->Merge();
            G4double edep = fEdep.GetValue();
            G4double edep2 = fEdep2.GetValue();
            G4double rms = (nofEvents > 0 && edep2 - edep*edep/nofEvents >= 0.)
                           ? std::sqrt(edep2 - edep*edep/nofEvents) : 0.;

            G4cout << G4endl
                   << "--------------------End of Global Run (Master Thread)-----------------------" << G4endl
                   << " The run consists of " << nofEvents << " events." << G4endl
                   << " Cumulative Edep (summed): " << G4BestUnit(edep,"Energy")
                   << " +- " << G4BestUnit(rms,"Energy")
                   << G4endl
                   << "---------------------------------------------------------------------------" << G4endl;
        } else {
            G4cout << "RunAction (Master): EndOfRunAction, no events processed." << G4endl;
        }
    }

    // Write any remaining data and close ROOT file
    analysisManager->Write();
    analysisManager->CloseFile();

    G4cout << "RunAction (Thread " << G4Threading::G4GetThreadId()
           << "): ROOT data written and file closed." << G4endl;
}

void RunAction::AddEdep(G4double edep)
{
    fEdep  += edep;
    fEdep2 += edep*edep;
}