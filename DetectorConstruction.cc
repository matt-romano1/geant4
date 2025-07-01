#include "DetectorConstruction.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSTrackLength.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

// Needed for optical surfaces
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"

// Constructor
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   fScintillatorLV(nullptr), // Will be the LV for a SINGLE cell
   // Pointers to physical volumes of plane envelopes are still useful if needed elsewhere
   fUpperDetector1PV(nullptr),
   fUpperDetector2PV(nullptr),
   fLowerDetector1PV(nullptr),
   fLowerDetector2PV(nullptr),
   fWorldMaterial(nullptr),
   fPVTMaterial(nullptr),
   scintillatorSizeXY_FullPlane(50.0*cm), // Initialize here or in DefineVolumes
   scintillatorThickness(2.0*cm),       // Initialize here or in DefineVolumes
   nCellsPerSide(8),                     // Initialize here or in DefineVolumes
   cellWidth(0.), // Will be calculated
   cellDepth(0.)  // Will be calculated
{}


// DefineMaterials method (assuming it's unchanged and correct)
void DetectorConstruction::DefineMaterials()
{
    G4NistManager* nistManager = G4NistManager::Instance();
    
    // Air for world volume
    fWorldMaterial = nistManager->FindOrBuildMaterial("G4_AIR");
    
    // PVT (Polyvinyltoluene) scintillator material
    G4double density = 1.032*g/cm3;
    G4int ncomponents = 2;
    
    fPVTMaterial = new G4Material("PVT", density, ncomponents);
    fPVTMaterial->AddElement(nistManager->FindOrBuildElement("C"), 9);
    fPVTMaterial->AddElement(nistManager->FindOrBuildElement("H"), 10);
    
    const G4int nEntries = 2;
    G4double photonEnergy[nEntries] = {2.0*eV, 4.0*eV};
    G4double scintillationYield[nEntries] = {1000./MeV, 1000./MeV};
    G4double rIndex[nEntries] = {1.58, 1.58};
    G4double absorption[nEntries] = {380.*cm, 380.*cm};
    
    G4MaterialPropertiesTable* mptPVT = new G4MaterialPropertiesTable();
    mptPVT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, scintillationYield, nEntries);
    mptPVT->AddProperty("RINDEX", photonEnergy, rIndex, nEntries);
    mptPVT->AddProperty("ABSLENGTH", photonEnergy, absorption, nEntries);
    mptPVT->AddConstProperty("SCINTILLATIONYIELD", 1000./MeV);
    mptPVT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.1*ns);
    mptPVT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    
    fPVTMaterial->SetMaterialPropertiesTable(mptPVT);
    
    G4double airPhotonEnergy[nEntries] = {2.0*eV, 4.0*eV};
    G4double airRIndex[nEntries] = {1.0, 1.0};
    
    G4MaterialPropertiesTable* mptAir = new G4MaterialPropertiesTable();
    mptAir->AddProperty("RINDEX", airPhotonEnergy, airRIndex, nEntries);
    
    fWorldMaterial->SetMaterialPropertiesTable(mptAir);
}


G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // Define materials
    DefineMaterials();
    
    // Define volumes
    return DefineVolumes();
}

G4VPhysicalVolume* DetectorConstruction::DefineVolumes()
{
    // --- Geometry Parameters ---
    // These might have been initialized in the constructor already
    // If not, ensure they are set here.
    // scintillatorSizeXY_FullPlane = 50.0*cm;
    // scintillatorThickness = 2.0*cm;
    // nCellsPerSide = 8;

    cellWidth = scintillatorSizeXY_FullPlane / nCellsPerSide;
    cellDepth = scintillatorSizeXY_FullPlane / nCellsPerSide;
    // cellThickness is the same as scintillatorThickness

    G4double scanningGap = 60.0*cm;
    G4double detectorSpacing = 5.0*cm;

    // Calculate world size
    G4double worldSizeXY = scintillatorSizeXY_FullPlane + 40.0*cm;
    G4double worldSizeZ = scanningGap + 4*scintillatorThickness + 2*detectorSpacing + 60.0*cm;
    
    // --- World Volume ---
    G4Box* worldS = new G4Box("WorldS", worldSizeXY/2, worldSizeXY/2, worldSizeZ/2);
    G4LogicalVolume* worldLV = new G4LogicalVolume(worldS, fWorldMaterial, "WorldLV");
    G4VPhysicalVolume* worldPV = new G4PVPlacement(0, G4ThreeVector(), worldLV, "WorldPV", 0, false, 0, true); // Changed name for clarity
    worldLV->SetVisAttributes(G4VisAttributes::GetInvisible());

    // --- Define Logical Volume for a SINGLE Scintillator CELL ---
    // This LV will be shared by all cell physical volumes.
    G4Box* scintillatorCellS = new G4Box("ScintillatorCellS",
                                         cellWidth/2,
                                         cellDepth/2,
                                         scintillatorThickness/2);
    fScintillatorLV = new G4LogicalVolume(scintillatorCellS,
                                          fPVTMaterial,
                                          "ScintillatorCellLV");

    static G4VisAttributes cellVisAtt(G4Colour(0.0, 1.0, 1.0, 0.3)); // Cyan, slightly transparent
    cellVisAtt.SetVisibility(true);
    cellVisAtt.SetForceSolid(true);
    fScintillatorLV->SetVisAttributes(&cellVisAtt);

    // --- Optical Surface for Scintillator Cells (applied to the shared fScintillatorLV) ---
    G4OpticalSurface* cellOpticalSurface = new G4OpticalSurface("ScintillatorCellSurface");
    cellOpticalSurface->SetType(dielectric_dielectric);
    cellOpticalSurface->SetModel(glisur);
    cellOpticalSurface->SetFinish(polished);

    G4MaterialPropertiesTable* cellOpticalSurfaceMPT = new G4MaterialPropertiesTable();
    const G4int NUMENTRIES_OPSURF = 2;
    G4double opSurfPhotonEnergy[NUMENTRIES_OPSURF] = {2.0*eV, 4.0*eV};
    G4double opSurfReflectivity[NUMENTRIES_OPSURF] = {0.0, 0.0}; // Absorb all
    cellOpticalSurfaceMPT->AddProperty("REFLECTIVITY", opSurfPhotonEnergy, opSurfReflectivity, NUMENTRIES_OPSURF);
    cellOpticalSurface->SetMaterialPropertiesTable(cellOpticalSurfaceMPT);

    new G4LogicalSkinSurface("ScintillatorCellSkin", fScintillatorLV, cellOpticalSurface);


    // --- Z positions for the centers of the detector plane envelopes ---
    G4double upperDetector1_Zpos = scanningGap/2 + scintillatorThickness/2 + detectorSpacing;
    G4double upperDetector2_Zpos = scanningGap/2 + scintillatorThickness/2;
    G4double lowerDetector1_Zpos = -scanningGap/2 - scintillatorThickness/2;
    G4double lowerDetector2_Zpos = -scanningGap/2 - scintillatorThickness/2 - detectorSpacing;

    G4double planeZPositions[] = {upperDetector1_Zpos, upperDetector2_Zpos, lowerDetector1_Zpos, lowerDetector2_Zpos};
    G4VPhysicalVolume* planePVs[] = {fUpperDetector1PV, fUpperDetector2PV, fLowerDetector1PV, fLowerDetector2PV}; // To store the PVs

    G4int globalCellCopyNo = 0; // Unique ID for each cell across all planes

    // --- Create and Place the 4 Detector Planes and their Cells ---
    for (int planeNum = 0; planeNum < 4; ++planeNum) {
        // Create a NEW logical volume for EACH plane's envelope
        G4String planeEnvelopeS_Name = "PlaneEnvelopeS" + std::to_string(planeNum);
        G4Box* planeEnvelopeS = new G4Box(planeEnvelopeS_Name,
                                          scintillatorSizeXY_FullPlane/2,
                                          scintillatorSizeXY_FullPlane/2,
                                          scintillatorThickness/2);

        G4String planeEnvelopeLV_Name = "PlaneEnvelopeLV" + std::to_string(planeNum);
        G4LogicalVolume* currentPlaneEnvelopeLV = new G4LogicalVolume(planeEnvelopeS,
                                                                    fWorldMaterial, // Filled with Air
                                                                    planeEnvelopeLV_Name);
        currentPlaneEnvelopeLV->SetVisAttributes(G4VisAttributes::GetInvisible());

        // Place this specific plane's envelope
        G4String planeEnvelopePV_Name = "PlaneEnvelopePV" + std::to_string(planeNum);
        // Store the physical volume pointer if needed, e.g., for access later
        planePVs[planeNum] = new G4PVPlacement(nullptr,
                                     G4ThreeVector(0, 0, planeZPositions[planeNum]),
                                     currentPlaneEnvelopeLV, // Use the NEW LV for this plane
                                     planeEnvelopePV_Name,
                                     worldLV,
                                     false,
                                     1000 + planeNum, // Unique copy number for the envelope itself
                                     true); // Check overlaps

        // Populate THIS envelope with its 64 cells
        for (G4int i = 0; i < nCellsPerSide; ++i) { // Index for Y cells
            for (G4int j = 0; j < nCellsPerSide; ++j) { // Index for X cells
                G4double xPos = -scintillatorSizeXY_FullPlane/2 + cellWidth/2 + j*cellWidth;
                G4double yPos = -scintillatorSizeXY_FullPlane/2 + cellDepth/2 + i*cellDepth;
                G4double zPos = 0; // Cells are centered in Z within their thin envelope

                new G4PVPlacement(nullptr,
                                  G4ThreeVector(xPos, yPos, zPos),
                                  fScintillatorLV,    // Shared LV for all cells
                                  "ScintillatorCellPV", // Name can be the same, copy number makes it unique
                                  currentPlaneEnvelopeLV, // Mother is THIS plane's specific envelope LV
                                  false,
                                  globalCellCopyNo++, // Unique copy number for the cell (0-63 for plane 0, 64-127 for plane 1, etc.)
                                  true); // Check overlaps
            }
        }
    }
    
    // Assign to member variables if needed (though planePVs array holds them too)
    fUpperDetector1PV = planePVs[0];
    fUpperDetector2PV = planePVs[1];
    fLowerDetector1PV = planePVs[2];
    fLowerDetector2PV = planePVs[3];

    // fScintillatorLV correctly points to the logical volume of a single cell.
    return worldPV;
}

// ConstructSDandField method (assuming it's largely unchanged)
// Make sure it refers to fScintillatorLV if setting SD for cells.
void DetectorConstruction::ConstructSDandField()
{
    // If you assign a Sensitive Detector to the cells, you do it on fScintillatorLV
    // as it's the shared logical volume for all cells.
    // Example:
    // if (fScintillatorLV) {
    //     MySensitiveDetector* sensDet = new MySensitiveDetector("ScintSD");
    //     fScintillatorLV->SetSensitiveDetector(sensDet);
    //     // Or using SetSensitiveDetector("ScintillatorCellLV", sensDet);
    // }


    G4cout << "\n=== Cosmic Muon Tomography Geometry (Segmented) ===" << G4endl;
    G4cout << "Each detector plane is " << scintillatorSizeXY_FullPlane/cm << " cm x " << scintillatorSizeXY_FullPlane/cm << " cm" << G4endl;
    G4cout << "Segmented into " << nCellsPerSide << "x" << nCellsPerSide << " cells." << G4endl;
    G4cout << "Each cell size: " << cellWidth/cm << " cm x " << cellDepth/cm << " cm x " << scintillatorThickness/cm << " cm" << G4endl;
    G4cout << "Number of detector planes: 4" << G4endl;
    G4cout << "Cell material: PVT (Polyvinyltoluene)" << G4endl;
    G4cout << "=================================================\n" << G4endl;
}

