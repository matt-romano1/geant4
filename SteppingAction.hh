#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;

class G4LogicalVolume;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(EventAction* eventAction);
    virtual ~SteppingAction() = default;

    virtual void UserSteppingAction(const G4Step*) override;

private:
    EventAction* fEventAction;
    const G4LogicalVolume* fScoringVolume;
    G4int fSpectrumNtupleId;
    G4int fEdepNtupleId;
    G4int fMuonTrackNtupleId;
    
};

#endif
