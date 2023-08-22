/**************************************************************/
// Program name : Skidpad_simulation
// Author       : Masatsugu Kitadai
// Date         : 2023/4/22
// Description  :
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
using namespace std;
FILE *fp;
mode_t dir_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IXOTH;

/** 物理法則 **/
const float pi = 4 * atan(1.0); // 円周率 [rad]
const float g = 9.80665;        // 重力加速度 [m/s2]

/** 各種パラメータ **/
const float t = 1.0;          // 計測時刻 [s]
const int hz = 1000;          // サンプリング周波数 [Hz]
const int hz_sin = 10.0;      // 正弦波の周期 [Hz]
const float wave_value = 1.0; // 正弦波の大きさ [-]
const float err_value = 0.5;  // エラーの大きさ [-]

/** グローバル変数 **/
vector<float> data(t *hz);  // 基本データ
vector<float> err_1(t *hz); // 乱数配列
vector<float> err_2(t *hz); // 乱数配列

/** プロトタイプ宣言 **/
void Sin_wave(vector<float> &data);
void Random_number(vector<float> &err);
void Box_Mullers_method(vector<float> &x1, vector<float> &x2);
void Write_data(const char filename[]);
void Gnuplot_noise(const char filename[], const char graphname[], const char title[]);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "Simulation";
    const char dir_1[] = "Simulation/data";
    const char dir_2[] = "Simulation/graph";
    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);

    /** 正弦波の作成 **/
    Sin_wave(data);

    /** データの書き出し(1) **/
    const char basic_data[] = "Simulation/data/basic_data.dat";
    Write_data(basic_data);

    /** 誤差データの作成 **/
    srand((unsigned int)time(NULL)); // シード値の変更

    // 一様乱数の生成
    Random_number(err_1);
    Random_number(err_2);

    // 正規分布乱数への変換
    Box_Mullers_method(err_1, err_2);

    /** 誤差データの足し合わせ **/
    for (int i = 0; i < data.size(); i++)
    {
        data[i] += err_1[i] * err_value;
    }

    /** データの書き出し(2) **/
    const char noise_data[] = "Simulation/data/noise_data.dat";
    Write_data(noise_data);

    /** グラフの作成 **/
    const char basic_graph[] = "Simulation/graph/basic_data.svg";
    const char basic_title[] = "Sin wave : Basic data";
    const char noise_graph[] = "Simulation/graph/noise_data.svg";
    const char noise_title[] = "Sin wave : Noise data";
    Gnuplot_noise(basic_data, basic_graph, basic_title);
    Gnuplot_noise(noise_data, noise_graph, noise_title);

    return 0;
}

/**************************************************************/
// Function name : Sin_wave
// Description   : 正弦波の作成
/**************************************************************/
void Sin_wave(vector<float> &data)
{
    for (int i = 0; i < data.size(); i++)
    {
        float dt = 1.0 / hz;             // 時間間隔 [s]
        float omega = 2.0 * pi * hz_sin; // 角速度 [rad/s]
        float theta = omega * dt * i;    // 角度 [rad]
        data[i] = wave_value * sin(theta);
    }
}

/**************************************************************/
// Function name : Random_number
// Description   : 一様乱数の生成
/**************************************************************/
void Random_number(vector<float> &err)
{
    for (int i = 0; i < err.size(); i++)
    {
        err[i] = (float)rand() / RAND_MAX;
    }
}

/**************************************************************/
// Function name : Box_Mullers_method
// Description   : 正規分布の乱数を与える (x1,x2は互いに独立な一様乱数) | 範囲は -1 <= 1
/**************************************************************/
void Box_Mullers_method(vector<float> &x1, vector<float> &x2)
{
    const float pi = 4 * atan(1.0); // 円周率 [rad]
    for (int i = 0; i < x1.size(); i++)
    {
        // ボックス=ミュラー法による正規分布乱数の計算
        float tmp = sqrt(-2.0 * log(x1[i]));
        float value1 = tmp * cos(2.0 * pi * x2[i]);
        float value2 = tmp * sin(2.0 * pi * x2[i]);

        // 配列への代入
        x1[i] = value1; // -1.0 <= x1 <= 1.0
        x2[i] = value2; // -1.0 <= x2 <= 1.0
    }
}

/**************************************************************/
// Function name : Write_data
// Description   : 車両の位置を計算
/**************************************************************/
void Write_data(const char filename[])
{
    float dt = 1.0 / hz; // 時間間隔 [s]

    /** データの書き出し **/
    fp = fopen(filename, "w");
    for (int i = 0; i < data.size(); i++)
    {
        float t_tmp = dt * i;
        fprintf(fp, "%f\t%f\n", t_tmp, data[i]);
    }
    fclose(fp);
}

/**************************************************************/
// Function name : Gnuplot_y_acc
// Description  :
/**************************************************************/
void Gnuplot_noise(const char filename[], const char graphname[], const char title[])
{
    FILE *gp;

    /** Gnuplot 初期設定 **/
    const float x_max = t;
    const float x_min = 0;
    const float y_max = 2.0;
    const float y_min = -2.0;

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal svg size 800, 500 font 'Times New Roman, 20'\n");
    fprintf(gp, "set size ratio 0.5\n");
    fprintf(gp, "set output '%s'\n", graphname);                            // 出力ファイル
    fprintf(gp, "unset key\n");                                             // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                  // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                  // y軸の描画範囲
    fprintf(gp, "set title '%s'\n", title);                                 // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic t} [s]' offset 0.0, 0.0\n");    // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic F(t)} [-]' offset 1.0, 0.0\n"); // y軸のラベル
    fprintf(gp, "set xtics 0.1 offset 0.0, 0.0\n");                         // x軸の間隔
    fprintf(gp, "set ytics 1.0 offset 0.0, 0.0\n");                         // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 1:2 with lines lc 'black' notitle\n", filename);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}