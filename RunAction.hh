// RunAction.hh
#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;
class G4RootAnalysisManager;

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;

    void AddEdep(G4double edep); // For overall Edep summary if still used

    G4int GetSpectrumNtupleId() const { return fSpectrumNtupleId; }
    G4int GetEdepNtupleId() const { return fEdepNtupleId; }
    G4int GetMuonTrackNtupleId() const { return fMuonTrackNtupleId; }

private:
    // For overall Edep summary (optional)
    G4Accumulable<G4double> fEdep;
    G4Accumulable<G4double> fEdep2;

    // Ntuple IDs - properly initialized in constructor
    G4int fSpectrumNtupleId;
    G4int fEdepNtupleId;
    G4int fMuonTrackNtupleId;

    G4RootAnalysisManager* fAnalysisManager;
};

#endif