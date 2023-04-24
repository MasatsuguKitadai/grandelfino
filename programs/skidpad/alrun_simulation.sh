# 画像ファイルの削除
rm -r simulation/

# シミュレーションの実行
g++ cpp/simulation.cpp -o "out/simulation.out"
./out/simulation.out

# gifアニメーションの作成
python3 py/gif.py