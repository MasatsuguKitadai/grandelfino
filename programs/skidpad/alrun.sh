# 画像ファイルの削除
rm -r graph/

# シミュレーションの実行
g++ simulation.cpp -o "simulation.out"
./simulation.out

# gifアニメーションの作成
python3 gif.py