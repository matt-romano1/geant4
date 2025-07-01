# Cosmic Muon Tomography Simulation

A Geant4 simulation for cosmic muon tomography using segmented scintillator detectors.

## What it does

Simulates cosmic muons passing through 4 scintillator planes (2 upper, 2 lower) with a scanning gap between them. Each plane is divided into 8×8 cells that detect optical photons from muon interactions. Outputs ROOT files with energy deposits and muon trajectories.

## Requirements

- Geant4 (built with visualization and UI support)
- CMake 3.16+
- Python with pandas and uproot (for data conversion)

## Build & Run

```bash
mkdir build && cd build
cmake ..
make
./cosmicMuonTomography
```

For batch mode with macro:
```bash
./cosmicMuonTomography your_macro.mac
```

## Output

- `tomography_output.root` - Contains three trees:
  - SpectrumData: Optical photon data per cell
  - EdepData: Energy deposits per cell  
  - MuonTrackData: True muon positions

Convert to CSV:
```bash
python Convert_To_CSV.py
```

Creates `energy_maps_with_labels.csv` with fractional energy per cell and true muon positions for ML training.
