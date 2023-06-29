# 画像ファイルの削除
rm -r GNSS_position/

# Estimate_positionの実行
g++ cpp/GNSS_position.cpp -o "out/GNSS_position.out"
./out/GNSS_position.out

# gifアニメーションの作成
python3 py/gif_GNSS_position.py
