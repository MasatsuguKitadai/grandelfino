# 画像ファイルの削除
rm -r simulation/

# シミュレーションの実行
g++ cpp/Simulation.cpp -o "out/Simulation.out"
./out/Simulation.out

# gifアニメーションの作成
python3 py/gif_Simulation.py