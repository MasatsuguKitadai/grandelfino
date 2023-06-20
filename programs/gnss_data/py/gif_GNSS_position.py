from PIL import Image
import glob
import os

# ディレクトリの作成 #
dir_path = "gif"
if not os.path.isdir(dir_path):
    os.makedirs(dir_path)

# GIF動画の作成 #
files = sorted(glob.glob("GNSS_position/graph/*.png"))
images = list(map(lambda file: Image.open(file), files))
images[0].save(
    "gif/GNSS_position.gif",
    save_all=True,
    append_images=images[1:],
    duration=100.00,
    loop=0,
)
