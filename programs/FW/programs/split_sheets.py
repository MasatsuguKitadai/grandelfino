import os
import pandas as pd

# Read all sheets from the Excel file
excel_file_path = "data/analysis.xlsx"
all_sheets = pd.read_excel(excel_file_path, sheet_name=None)

# Create the directory if it doesn't exist
output_directory = "sheets"
os.makedirs(output_directory, exist_ok=True)

# Save each sheet as a CSV
csv_files = []
for sheet_name, sheet_data in all_sheets.items():
    # If 'ms' column exists, create 's' column as ms/1000
    if "ms" in sheet_data.columns:
        sheet_data.insert(
            sheet_data.columns.get_loc("ms") + 1, "s", sheet_data["ms"] / 1000
        )

    # If 'dv3' column exists, create 'sum' column as sum of dv0, dv1, dv2, dv3
    if "dv3" in sheet_data.columns:
        dv_columns = ["dv0", "dv1", "dv2", "dv3"]
        sheet_data.insert(
            sheet_data.columns.get_loc("dv3") + 1,
            "sum",
            sheet_data[dv_columns].sum(axis=1),
        )

    # Escape sheet name for safe filename
    safe_sheet_name = sheet_name.replace("/", "_")  # Example: replace "/" with "_"
    csv_filename = os.path.join(output_directory, f"{safe_sheet_name}.csv")
    sheet_data.to_csv(csv_filename, index=False)
    csv_files.append(csv_filename)

csv_files
