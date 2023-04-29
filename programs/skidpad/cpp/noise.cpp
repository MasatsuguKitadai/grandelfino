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
#include "../hpp/functions.hpp"
using namespace std;
FILE *fp;
mode_t dir_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IXOTH;

/** 物理法則 **/
const float pi = 4 * atan(1.0); // 円周率 [rad]
const float g = 9.80665;        // 重力加速度 [m/s2]

/** パラメータ **/
const float v = 40.0;                     // 走行速度 [km/h]
const float r = 7.625;                    // 旋回半径 [m]
const float n = 4.0;                      // スキッドパッドの周回数 [-] (右周り2周 → 左回り2周)
const float hz_6axis = 100.0;             // 6軸センサのサンプリング周期 [Hz]
const float hz_gps = 2.0;                 // GPSのサンプリング周期 [Hz] (ZED-F9P は RTK 時に最大 20 [Hz])
const float err_g = 2.0 * g * 0.010;      // 加速度センサの誤差 [m/s2]
const float err_omega = 2.0 * pi * 0.010; // 加速度センサの誤差 [m/s2]
const float err_gps = 0.01;               // GPSの誤差 [m]

/** パラメータ（自動的に決まる） **/
const float v2 = v * 1000.0 / 3600.0;                                  // 走行速度 [m/s]
const float omega = v2 / r;                                            // 角速度 [rad/s]
const float t_start = 2.0;                                             // 助走の時間 [s]
const float mileage_sp = 2.0 * pi * r * n;                             // スキッドパッドの走行距離 [m]
const float t_sp = mileage_sp / v2;                                    // スキッドパッドの走行時間 [s]
const float t_finish = 1.0;                                            // 惰走時間 [s]
const float acc_start = v2 / t_start;                                  // 助走の加速度 [m/s2]
const float mileage_start = 1.0 / 2.0 * acc_start * t_start * t_start; // 助走距離 [m]

/** 時刻の計算 **/
const float t0 = 0;             // 走行開始時刻 [s]
const float t1 = t_start;       // スキッドパッド開始時刻 [s]
const float t2 = t1 + t_sp;     // スキッドパッド終了時刻 [s]
const float t3 = t2 + t_finish; // 走行終了時刻 [s]

/** プロトタイプ宣言 **/
void Gnuplot_y_acc();

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "images";
    mkdir(dir_0, dir_mode);

    /** グラフの作成 **/
    Gnuplot_y_acc();

    return 0;
}

/**************************************************************/
// Function name : Gnuplot_y_acc
// Description  :
/**************************************************************/
void Gnuplot_y_acc()
{
    FILE *gp;

    /** Gnuplot 初期設定 **/
    const int x_max = t3;
    const int x_min = 0;
    const float y_max = 20.0;
    const float y_min = -20.0;

    /** Gnuplot ファイル名の設定 **/
    const char filename[] = "simulation/data/data.dat";
    const char graphname[] = "images/y_acc.png";

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal png size 800, 600 font 'Times New Roman, 16'\n");
    fprintf(gp, "set size ratio 0.5\n");
    fprintf(gp, "set output '%s'\n", graphname);                                 // 出力ファイル
    fprintf(gp, "unset key\n");                                                  // 凡例非表示
    fprintf(gp, "set xrange [%d:%d]\n", x_min, x_max);                           // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                       // y軸の描画範囲
    fprintf(gp, "set title 'Skidpad Simulation : Acc Y-direction'\n");           // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic t} [s]' offset 0.0, 0.0\n");         // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic Acc y} [m/s^2]' offset 1.0, 0.0\n"); // y軸のラベル
    fprintf(gp, "set xtics 5.0 offset 0.0, 0.0\n");                              // x軸の間隔
    fprintf(gp, "set ytics 5.0 offset 0.0, 0.0\n");                              // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 1:3 with lines lc 'black' notitle\n", filename);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}