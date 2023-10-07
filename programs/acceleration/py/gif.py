import os
import imageio.v2 as imageio  # こちらの行を変更


def create_gif(input_path, output_gif_name, duration):
    """
    指定されたパスの画像を読み込み、GIF画像を作成する関数

    Parameters:
    - input_path: 画像が保存されているディレクトリのパス
    - output_gif_name: 出力するGIFファイルの名前
    - duration: 各フレームの表示時間（秒）
    """

    # 指定したディレクトリの画像ファイルを読み込む
    filenames = sorted(
        [
            os.path.join(input_path, fname)
            for fname in os.listdir(input_path)
            if fname.endswith(".png")
        ]
    )

    # 画像を読み込み、GIFとして保存
    images = [imageio.imread(filename) for filename in filenames]
    imageio.mimsave(output_gif_name, images, duration=duration)


# GIF画像を作成
input_path = "graph/"
output_gif_name = "result/animated_graph.gif"
duration = 1.0  # 各フレームの表示時間（秒）
create_gif(input_path, output_gif_name, duration)
