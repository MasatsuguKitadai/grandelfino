# 画像ファイルの削除
rm -r SLAM_ver1/
rm -r SLAM_ver2/

# SLAMの実行
g++ cpp/SLAM_ver1.cpp -o "out/SLAM_ver1.out"
./out/SLAM_ver1.out

g++ cpp/SLAM_ver2.cpp -o "out/SLAM_ver2.out"
./out/SLAM_ver2.out

python3 py/gif.py
# gifアニメーションの作成