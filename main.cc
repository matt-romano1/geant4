#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"
#include "FTFP_BERT.hh"
#include "G4OpticalPhysics.hh" 

#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "Randomize.hh"


int main(int argc, char** argv)
{
    // Detect interactive mode (if no arguments) and define UI session
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Optionally: choose a different Random engine...
    G4Random::setTheEngine(new CLHEP::RanecuEngine);

    // Use G4SteppingVerboseWithUnits
    G4int precision = 4;
    G4SteppingVerbose::UseBestUnit(precision);

    // Construct the default run manager
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

    // Set mandatory initialization classes
    runManager->SetUserInitialization(new DetectorConstruction());

    // Physics list with optical physics for scintillation
    G4VModularPhysicsList* physicsList = new FTFP_BERT;
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
    physicsList->RegisterPhysics(opticalPhysics);
    physicsList->SetVerboseLevel(1);
    runManager->SetUserInitialization(physicsList);

    // User action initialization
    runManager->SetUserInitialization(new ActionInitialization());

    // Initialize visualization
    G4VisManager* visManager = new G4VisExecutive;
    // G4VisExecutive can take a verbosity argument - see /vis/verbose guidance.
    // G4VisManager* visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();


    // Get the pointer to the User Interface manager
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // Process macro or start UI session
    if (!ui) {
        // batch mode
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    } else {
        // interactive mode
        G4int result = UImanager->ApplyCommand("/control/execute init_vis.mac");
        if (result != 0) {
            G4cout << "init_vis.mac not found. Starting basic visualization..." << G4endl;
            
            // Initialize kernel
            UImanager->ApplyCommand("/run/initialize");
            
            // Create scene and visualization
            UImanager->ApplyCommand("/vis/scene/create");
            UImanager->ApplyCommand("/vis/open OGL 600x600-0+0");
            UImanager->ApplyCommand("/vis/drawVolume");
            
            // Set up viewing
            UImanager->ApplyCommand("/vis/viewer/set/autoRefresh false");
            UImanager->ApplyCommand("/vis/viewer/set/viewpointVector -1 0 0");
            UImanager->ApplyCommand("/vis/viewer/set/style wireframe");
            
            // Add trajectories
            UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
            UImanager->ApplyCommand("/vis/modeling/trajectories/create/drawByParticleID");
            UImanager->ApplyCommand("/vis/modeling/trajectories/drawByParticleID-0/set mu- green");
            UImanager->ApplyCommand("/vis/modeling/trajectories/drawByParticleID-0/set mu+ red");
            
            // Enable trajectory storage
            UImanager->ApplyCommand("/tracking/storeTrajectory 1");
            
            // Accumulate events
            UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
            
            // Add decorations
            // UImanager->ApplyCommand("/vis/scene/add/axes 0 0 0 10 cm");
            UImanager->ApplyCommand("/vis/scene/add/scale");
            
            // Refresh
            UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
            UImanager->ApplyCommand("/vis/verbose warnings");

            UImanager->ApplyCommand("/run/beamOn 1");
            UImanager->ApplyCommand("/vis/viewer/rebuild");
            
            
            G4cout << "=== Visualization Setup Complete ===" << G4endl;
            G4cout << "Type '/run/beamOn 10' to see muon trajectories" << G4endl;
        }
        ui->SessionStart();
        delete ui;
    }

    // Job termination
    delete visManager;
    delete runManager;

    return 0;
}