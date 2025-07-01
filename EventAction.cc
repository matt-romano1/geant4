#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4RootAnalysisManager.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction* runAction)
 : G4UserEventAction(),
   fRunAction(runAction)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    fRunAction->AddEdep(fEdep);
    
    // CRITICAL: Manually write ROOT data every 50 events to prevent memory overflow
    // This is essential to prevent corruption around event 775
    G4int eventID = event->GetEventID();
    
    // Write more frequently than every 100 events to be safe
    if (eventID > 0 && eventID % 50 == 0) {
        G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
        
        // Force write to disk
        analysisManager->Write();
        
        G4cout << "EventAction: Forced write to ROOT file at event " << eventID 
               << " to prevent memory overflow." << G4endl;
    }
    
    // Additional safety check - if we're approaching the problematic event range
    if (eventID > 700 && eventID < 850) {
        // Write even more frequently in the danger zone
        if (eventID % 10 == 0) {
            G4RootAnalysisManager* analysisManager = G4RootAnalysisManager::Instance();
            analysisManager->Write();
            G4cout << "EventAction: Extra safety write at event " << eventID << G4endl;
        }
    }
}