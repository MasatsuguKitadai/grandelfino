# 画像ファイルの削除
rm -r Estimate_position_IMU/
# rm -r Estimate_position_GPS/
rm -r Estimate_position_IMU+GPS/

# Estimate_positionの実行
g++ cpp/Estimate_position_IMU.cpp -o "out/Estimate_position_IMU.out"
./out/Estimate_position_IMU.out

g++ cpp/Estimate_position_IMU+GPS.cpp -o "out/Estimate_position_IMU+GPS.out"
./out/Estimate_position_IMU+GPS.out

# gifアニメーションの作成
python3 py/gif_Estimate_position_IMU.py
# python3 py/gif_Estimate_position_GPS.py
python3 py/gif_Estimate_position_IMU+GPS.py
