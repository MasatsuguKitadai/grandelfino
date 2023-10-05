import os
import pandas as pd
import matplotlib.pyplot as plt


# Create the directory if it doesn't exist
output_directory = "figures"
os.makedirs(output_directory, exist_ok=True)


def get_all_csv_files(directory_path):
    """Return all CSV files from the specified directory."""
    return [f for f in os.listdir(directory_path) if f.endswith('.csv')]


def plot_graph_from_csv(file_path):
    """Read the CSV file and plot a graph using 's' as the x-axis and 'sum' as the y-axis."""
    df = pd.read_csv(file_path)
    plt.figure(figsize=(10, 6))
    plt.plot(df['s'], df['sum'], label=os.path.basename(file_path))
    plt.xlabel('s')
    plt.ylabel('sum')
    plt.title(f"Graph for {os.path.basename(file_path)}")
    plt.legend()
    plt.grid(True)
    plt.show()


def save_plot_to_png(file_path):
    """Read the CSV file and save a plot as PNG using 's' as the x-axis and 'sum' as the y-axis."""
    df = pd.read_csv(file_path)
    plt.figure(figsize=(10, 6))
    plt.plot(df['s'], df['sum'], label=os.path.basename(file_path))
    plt.xlabel('Time [s]')
    plt.ylabel('Sum of output voltage [V]')
    plt.title(f"Graph for {os.path.basename(file_path)}")
    plt.legend()
    plt.grid(True)
    output_path = os.path.join(figures_directory, os.path.basename(
        file_path).replace('.csv', '.png'))
    plt.savefig(output_path)
    plt.close()


# Get all CSV files from the specified directory
csv_files = get_all_csv_files('sheets/')

# Create a directory named "figures" if it doesn't exist
figures_directory = 'figures/'
if not os.path.exists(figures_directory):
    os.makedirs(figures_directory)

# Save plots as PNG for each CSV file
for file in csv_files:
    save_plot_to_png(os.path.join('sheets/', file))

figures_directory
