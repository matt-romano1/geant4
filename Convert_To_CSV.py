import pandas as pd
import uproot
import warnings

warnings.filterwarnings("ignore", category=DeprecationWarning)

def process_fractional_energy(root_filename, output_csv="energy_maps_with_labels.csv"):
    """
    Process ROOT file to extract the fractional energy deposited in the
    first scintillator (cells 0-63) for each event.
    The EventID is used for processing but is not saved in the output CSV.
    """
    with uproot.open(root_filename) as file:
        spectrum_df = file['SpectrumData'].arrays(library="pd")
        muon_df = file['MuonTrackData'].arrays(library="pd")

    optical_photons = spectrum_df[spectrum_df['ParticleName'] == 'opticalphoton']
    photon_energy = optical_photons.groupby(['EventID', 'CellID'])['EnergyMeV'].sum().reset_index()
    photon_energy = photon_energy[photon_energy['CellID'] < 64].copy()

    total_event_energy = photon_energy.groupby('EventID')['EnergyMeV'].transform('sum')
    photon_energy['FractionalEnergy'] = photon_energy['EnergyMeV'] / total_event_energy

    cell_energy_df = photon_energy.pivot_table(
        index='EventID',
        columns='CellID',
        values='FractionalEnergy',
        fill_value=0.0
    )
    cell_energy_df.columns = [f'Cell_{int(col)}' for col in cell_energy_df.columns]
    
    muon_positions = muon_df.groupby('EventID')[['PreStepX_cm', 'PreStepY_cm']].mean()
    muon_positions = muon_positions.rename(columns={'PreStepX_cm': 'x_true', 'PreStepY_cm': 'y_true'})

    output_df = cell_energy_df.join(muon_positions, how='left').fillna(0.0)
    
    total_cells = 64
    all_cell_columns = [f'Cell_{i}' for i in range(total_cells)]
    for col in all_cell_columns:
        if col not in output_df:
            output_df[col] = 0.0

    final_columns = all_cell_columns + ['x_true', 'y_true']
    output_df = output_df[final_columns]

    output_df.to_csv(output_csv, float_format='%.6f', index=False)
    
    print(f"Data saved to {output_csv}")
    
    return output_df

# --- Usage Example ---
if __name__ == "__main__":
    root_file = "tomography_output.root"
    df_without_eventid = process_fractional_energy(root_file)

    print("\nDataFrame structure in memory (still has EventID as index):")
    print(df_without_eventid.head())