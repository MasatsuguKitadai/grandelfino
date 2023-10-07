import pandas as pd
import os
import matplotlib.pyplot as plt


def extract_columns_to_arrays_updated(df, convert_time_to_seconds=True):
    if convert_time_to_seconds and "Time" in df.columns:
        df["Time"] = df["Time"] / 1000
    arrays = {}
    for column in df.columns:
        arrays[column] = df[column].values
    return arrays


def compute_moving_averages(data, window_size=5):
    columns_to_average = [col for col in data.columns if col != "Time"]
    moving_averages = {}
    for col in columns_to_average:
        moving_averages[col] = data[col].rolling(window=window_size).mean()
    return moving_averages


def compute_exponential_moving_averages(data, span=5):
    columns_to_average = [col for col in data.columns if col != "Time"]
    ema_data = {}
    for col in columns_to_average:
        ema_data[col] = data[col].ewm(span=span, adjust=False).mean()
    return ema_data


def save_to_csv(data, original_data, filename):
    df_to_save = pd.DataFrame(data)
    df_to_save = df_to_save.dropna()
    df_to_save = df_to_save.drop(columns=["Time"], errors="ignore")
    df_to_save.insert(0, "Time", original_data["Time"].iloc[df_to_save.index])
    df_to_save.to_csv(filename, index=False, encoding="cp932")


def plot_and_save_graph(data, start_time, end_time, save_path, y_lims=None):
    """
    指定された時間範囲のデータでグラフを描画し、画像として保存する関数

    Parameters:
    - data: データフレーム
    - start_time: グラフの開始時間
    - end_time: グラフの終了時間
    - save_path: 保存先のパス
    - y_lims: 各グラフのy軸の範囲を指定する辞書。指定しない場合は自動で設定される
    """

    fig, axes = plt.subplots(6, 1, figsize=(10, 15))
    filtered_data = data[(data["Time"] >= start_time) & (data["Time"] <= end_time)]

    for i, col in enumerate(data.columns[1:]):
        axes[i].plot(filtered_data["Time"], filtered_data[col], label=col)
        axes[i].set_title(col)
        axes[i].set_xlim([start_time, end_time])
        axes[i].yaxis.set_major_formatter("{:.1f}".format)  # y軸のラベルを0.0の形式に設定

        if y_lims and col in y_lims:  # そのグラフのy軸の範囲が指定されている場合
            axes[i].set_ylim(y_lims[col])

        axes[i].legend()

    plt.tight_layout()
    plt.savefig(save_path)
    plt.close()


def generate_graphs_one_second_interval(
    data, step=1, duration=5, graph_path="graph/", y_lims=None
):
    max_time = int(data["Time"].max())
    current_time = 0
    graph_counter = 1

    if not os.path.exists(graph_path):
        os.mkdir(graph_path)

    while current_time + duration <= max_time:
        start_time = current_time
        end_time = current_time + duration
        save_path = os.path.join(graph_path, f"graph_{graph_counter:04d}.png")
        plot_and_save_graph(data, start_time, end_time, save_path, y_lims=y_lims)
        current_time += step
        graph_counter += 1


data = pd.read_csv("data/data_20231007.csv", encoding="cp932")
data["rgx"] = pd.to_numeric(data["rgx"])
data["rgy"] = pd.to_numeric(data["rgy"])
arrays = extract_columns_to_arrays_updated(data)

window_size = 21
moving_averages = compute_moving_averages(data, window_size=window_size)

span = 21
exponential_moving_averages = compute_exponential_moving_averages(data, span=span)

result_path = "result/"
if not os.path.exists(result_path):
    os.mkdir(result_path)

# 移動平均と指数移動平均をCSVファイルに保存
filename_1 = os.path.join(result_path, "data_1.csv")
filename_2 = os.path.join(result_path, "data_2.csv")
save_to_csv(moving_averages, data, filename_1)
save_to_csv(exponential_moving_averages, data, filename_2)

# 1秒ごとにグラフを作成
plot_data = pd.read_csv(filename_2, encoding="cp932")
y_axis_limits = {
    "rax": (-2.0, 2.0),
    "ray": (-2.0, 2.0),
    "raz": (-2.0, 2.0),
    "rgx": (-50.0, 50.0),
    "rgy": (-50.0, 50.0),
    "rgz": (-100.0, 100.0),
}
generate_graphs_one_second_interval(plot_data, y_lims=y_axis_limits)
