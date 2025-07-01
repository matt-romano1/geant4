#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh" // For G4ThreeVector, etc.

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction();
    virtual ~DetectorConstruction() = default;

public:
    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;


    // Get methods for accessing detector components
    const G4VPhysicalVolume* GetUpperDetector1PV() const { return fUpperDetector1PV; }
    const G4VPhysicalVolume* GetUpperDetector2PV() const { return fUpperDetector2PV; }
    const G4VPhysicalVolume* GetLowerDetector1PV() const { return fLowerDetector1PV; }
    const G4VPhysicalVolume* GetLowerDetector2PV() const { return fLowerDetector2PV; }
    
    const G4LogicalVolume* GetScintillatorLV() const { return fScintillatorLV; }

private:
    // Methods
    void DefineMaterials();
    G4VPhysicalVolume* DefineVolumes();

    // Data members
    G4LogicalVolume* fScintillatorLV;
    
    G4VPhysicalVolume* fUpperDetector1PV;
    G4VPhysicalVolume* fUpperDetector2PV;
    G4VPhysicalVolume* fLowerDetector1PV;
    G4VPhysicalVolume* fLowerDetector2PV;
    
    G4Material* fWorldMaterial;
    G4Material* fPVTMaterial;
    
    G4double scintillatorSizeXY_FullPlane;
    G4double scintillatorThickness;
    G4int    nCellsPerSide;
    G4double cellWidth;
    G4double cellDepth;
};

#endif