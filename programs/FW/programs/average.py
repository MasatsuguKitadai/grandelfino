import os
import pandas as pd


def generate_file_paths(range_df_path, base_path="sheets/"):
    # Read the range dataframe
    range_df = pd.read_csv(range_df_path)

    # Generate file paths based on the case column
    file_paths = [base_path + case_name + ".csv" for case_name in range_df["case"]]

    return file_paths


def calculate_averages(file_paths, range_df_path):
    # Read the data files into dataframes
    dataframes = [pd.read_csv(path) for path in file_paths]

    # Read the range dataframe
    range_df = pd.read_csv(range_df_path)

    # ---- Algorithm 1 ----
    cal_averages, zero_averages, dv_averages, v_averages = [], [], [], []
    for _, row in range_df.iterrows():
        df_name = row["case"]
        df_index = file_paths.index(f"sheets/{df_name}.csv")
        target_df = dataframes[df_index]

        # Calculate the average for cal range
        cal_avg = target_df[
            (target_df["s"] >= row["cal_min"]) & (target_df["s"] <= row["cal_max"])
        ]["sum"].mean()
        cal_averages.append(cal_avg)

        # Calculate the average for zero range
        zero_avg = target_df[
            (target_df["s"] >= row["zero_min"]) & (target_df["s"] <= row["zero_max"])
        ]["sum"].mean()
        zero_averages.append(zero_avg)

        # Calculate the average for ave range (added functionality)
        dv_avg = target_df[
            (target_df["s"] >= row["ave_min"]) & (target_df["s"] <= row["ave_max"])
        ]["sum"].mean()
        dv_averages.append(dv_avg)

        # Calculate the v for ave range (added functionality)
        v_avg = target_df[
            (target_df["s"] >= row["ave_min"]) & (target_df["s"] <= row["ave_max"])
        ]["v"].mean()
        v_averages.append(v_avg)

    range_df["cal_average"] = cal_averages
    range_df["zero_average"] = zero_averages
    range_df["dv_average"] = dv_averages
    range_df["v_average"] = v_averages

    # ---- Algorithm 2 ----
    dv0_averages, dv1_averages, dv2_averages, dv3_averages = [], [], [], []
    for _, row in range_df.iterrows():
        df_name = row["case"]
        df_index = file_paths.index(f"sheets/{df_name}.csv")
        target_df = dataframes[df_index]

        # Filter the dataframe for the ave range
        ave_df = target_df[
            (target_df["s"] >= row["ave_min"]) & (target_df["s"] <= row["ave_max"])
        ]

        # Calculate the average for non-zero values for each dv column
        dv_cols = ["dv0", "dv1", "dv2", "dv3"]
        for col in dv_cols:
            avg = ave_df[ave_df[col] != 0][col].mean()
            locals()[f"{col}_averages"].append(avg)

    range_df["dv0_average"] = dv0_averages
    range_df["dv1_average"] = dv1_averages
    range_df["dv2_average"] = dv2_averages
    range_df["dv3_average"] = dv3_averages

    return range_df


def save_to_csv(file_paths, range_df_path, output_path="result.csv"):
    # Use the combined function from earlier to get the calculated dataframe
    result_df = calculate_averages(file_paths, range_df_path)

    # Save the result dataframe to a csv file
    result_df.to_csv(output_path, index=False)


# Create the directory if it doesn't exist
output_directory = "results"
os.makedirs(output_directory, exist_ok=True)

file_paths = generate_file_paths("data/range.csv")
file_paths[:5]  # Displaying first 5 for brevity

output_path = "results/result.csv"
save_to_csv(file_paths, "data/range.csv", output_path)
output_path
