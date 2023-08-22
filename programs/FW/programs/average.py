import os
import pandas as pd


def compute_column_average_within_range(case_name, min_val, max_val, column_name):
    """Compute the average of the specified column where 's' column values lie between the given min and max values."""
    csv_file_path = os.path.join('sheets/', f"{case_name}.csv")
    df_case = pd.read_csv(csv_file_path)
    filtered_df = df_case[(df_case['s'] >= min_val) &
                          (df_case['s'] <= max_val)]
    average_value = filtered_df[filtered_df[column_name] != 0][column_name].mean(
    ) if column_name.startswith('dv') else filtered_df[column_name].mean()
    return average_value


# Load the range file
range_df = pd.read_excel('data/range.xlsx')

# List of columns to compute the average
columns_to_compute = ['v', 'sum', 'dv0', 'dv1', 'dv2', 'dv3']

# Compute the average for the specified columns
averages = {}
for column in columns_to_compute:
    column_averages = {}
    for index, row in range_df.iterrows():
        case_name = row['case']
        min_val = row['min']
        max_val = row['max']
        column_averages[case_name] = compute_column_average_within_range(
            case_name, min_val, max_val, column)
    averages[column] = column_averages

# Convert the computed averages into a DataFrame and save to 'results.csv'
df_results = pd.DataFrame(averages)
df_results.to_csv('data/results.csv')
