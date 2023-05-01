# 画像ファイルの削除
rm -r Estimated_position_IMU/
rm -r Estimated_position_GPS/
rm -r Estimated_position_IMU+GPS/

# Estimated_positionの実行
g++ cpp/Estimated_position_IMU.cpp -o "out/Estimated_position_IMU.out"
./out/Estimated_position_IMU.out

g++ cpp/Estimated_position_IMU+GPS.cpp -o "out/Estimated_position_IMU+GPS.out"
./out/Estimated_position_IMU+GPS.out

# gifアニメーションの作成
python3 py/gif_Estimated_position_IMU.py
# python3 py/gif_Estimated_position_GPS.py
python3 py/gif_Estimated_position_IMU+GPS.py
