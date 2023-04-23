# 画像ファイルの削除
rm -r SLAM/

# SLAMの実行
g++ SLAM.cpp -o "SLAM.out"
./SLAM.out

# gifアニメーションの作成
python3 gif.py