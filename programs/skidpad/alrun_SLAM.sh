# 画像ファイルの削除
rm -r SLAM/
rm -r SLAM+MA/
rm -r SLAM+GPS/

# SLAMの実行
g++ cpp/SLAM.cpp -o "out/SLAM.out"
./out/SLAM.out

g++ cpp/SLAM+MA.cpp -o "out/SLAM+MA.out"
./out/SLAM+MA.out

g++ cpp/SLAM+GPS.cpp -o "out/SLAM+GPS.out"
./out/SLAM+GPS.out

# gifアニメーションの作成
python3 py/gif_SLAM.py
python3 py/gif_SLAM+MA.py
python3 py/gif_SLAM+GPS.py
