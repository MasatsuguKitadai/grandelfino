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
#include <vector>
using namespace std;
FILE *fp;
mode_t dir_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IXOTH;

/** パラメータ **/
const float r = 8.0;  // 旋回半径 [m]
const float v = 40.0; // 走行速度 [km/h]
const float hz = 10;  // サンプリング周期 [Hz]
const float n = 4.0;  // 周回数 [-]

/** パラメータ（自動的に決まる） **/
const float v2 = v * 1000.0 / 3600.0;   // 走行速度 [m/s]
const float omega = v2 / r;             // 角速度 [rad/s]
const float pi = 4 * atan(1.0);         // 円周率 [rad]
const float mileage = 2.0 * pi * r * n; // 走行距離 [m]
const float sec_max = mileage / v2;     // 最大時刻 [s]

/** グローバル変数 **/
vector<float> x(sec_max *hz);
vector<float> y(sec_max *hz);

/** プロトタイプ宣言 **/
float Skidpad_x(float sec);
float Skidpad_y(float sec);
void Write_data(int n);
void Gnuplot(int n);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_1[] = "data_1";
    const char dir_2[] = "data_2";
    const char dir_3[] = "graph";
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);
    mkdir(dir_3, dir_mode);

    /** 車両位置の計算とデータの書き出し **/
    for (int i = 0; i < sec_max * hz; i++)
    {
        const float sec = i / hz;
        x[i] = Skidpad_x(sec);
        y[i] = Skidpad_y(sec);
        Write_data(i);
        Gnuplot(i);
        printf("%.3f\t[s]\tx = %.3f\ty = %.3f\n", sec, x[i], y[i]);
    }

    return 0;
}

/**************************************************************/
// Function name : Skidpad_x
// Description   : 車両の位置を計算
/**************************************************************/
float Skidpad_x(float sec)
{
    /** 角移動量の計算 **/
    float theta = omega * sec;

    /** x方向位置の積算 **/
    float x;
    if (0 <= theta && theta <= 2.0 * pi * 2.0)
    {
        x = -1.0 * r * cos(theta) + r;
    }
    else if (2.0 * pi * 2.0 <= theta)
    {
        x = r * cos(theta) - r;
    }

    return x;
}

/**************************************************************/
// Function name : Skidpad_y
// Description   : 車両の位置を計算
/**************************************************************/
float Skidpad_y(float sec)
{
    /** 角移動量の計算 **/
    float theta = omega * sec;

    /** y方向位置の積算 **/
    float y = r * sin(theta);

    return y;
}

/**************************************************************/
// Function name : Write_data
// Description   : 車両の位置を計算
/**************************************************************/
void Write_data(int n)
{
    const float sec = n / hz;
    char filename_1[100], filename_2[100];
    sprintf(filename_1, "data_1/%d.dat", n);
    sprintf(filename_2, "data_2/%d.dat", n);

    /** 走行位置の書き出し **/
    fp = fopen(filename_1, "w");
    fprintf(fp, "%f\t%f\t%f\n", sec, x[n], y[n]);
    fclose(fp);

    /** 走行経路の書き出し **/
    fp = fopen(filename_2, "w");
    for (int i = 0; i <= n; i++)
    {
        float sec_tmp = i / hz;
        fprintf(fp, "%f\t%f\t%f\n", sec_tmp, x[i], y[i]);
    }
    fclose(fp);
}

/**************************************************************/
// Function name : Gnuplot
// Description  :
/**************************************************************/
void Gnuplot(int n)
{
    FILE *gp;

    /** Gnuplot 初期設定 **/
    const float sec = n / hz;
    const float x_max = 20;
    const float x_min = -20;
    const float y_max = 10;
    const float y_min = -10;

    /** Gnuplot ファイル名の設定 **/
    char graphname[100], filename_1[100], filename_2[100];
    sprintf(filename_1, "data_1/%d.dat", n);
    sprintf(filename_2, "data_2/%d.dat", n);
    sprintf(graphname, "graph/%04d.png", n);

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal png size 800, 500 font 'Times New Roman, 16'\n");
    fprintf(gp, "set size ratio -1\n");
    fprintf(gp, "set output '%s'\n", graphname);                          // 出力ファイル
    fprintf(gp, "unset key\n");                                           // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                // y軸の描画範囲
    fprintf(gp, "set title 'Skidpad Simulation : t = %1.3f [s]'\n", sec); // グラフタイトル
    fprintf(gp, "set xlabel 'x [m]' offset 0.0, 0.0\n");                  // x軸のラベル
    fprintf(gp, "set ylabel 'y [m]' offset 1.0, 0.0\n");                  // y軸のラベル
    fprintf(gp, "set xtics 5.0 offset 0.0, 0.0\n");                       // x軸の間隔
    fprintf(gp, "set ytics 5.0 offset 0.0, 0.0\n");                       // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 2:3 with lines lc 'grey50' notitle, '%s' using 2:3 with points lc 'royalblue' ps 3 pt 7 notitle\n", filename_2, filename_1);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}