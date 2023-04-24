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
const float r = 7.625; // 旋回半径 [m]
const float v = 40.0;  // 走行速度 [km/h]
const float hz = 100;  // サンプリング周期 [Hz]
const float n = 4.0;   // 周回数 [-]

/** パラメータ（自動的に決まる） **/
const float v2 = v * 1000.0 / 3600.0;                                  // 走行速度 [m/s]
const float omega = v2 / r;                                            // 角速度 [rad/s]
const float pi = 4 * atan(1.0);                                        // 円周率 [rad]
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

/** グローバル変数 **/
vector<float> xw(t3 *hz); // 絶対座標系 x軸方向 : World coordinate system
vector<float> yw(t3 *hz); // 絶対座標系 y軸方向 : World coordinate system
vector<float> xl(t3 *hz); // 車両座標系 x軸方向 : Local coordinate system
vector<float> yl(t3 *hz); // 車両座標系 y軸方向 : Local coordinate system

vector<float> acc_xl(t3 *hz); // 車両に加わる加速度 x軸方向
vector<float> acc_yl(t3 *hz); // 車両に加わる加速度 y軸方向
vector<float> omegal(t3 *hz); // 車両に加わる角加速度 z軸方向

/** プロトタイプ宣言 **/
float Start(float t);
float Finish(float t);
float Skidpad_x(float t);
float Skidpad_y(float t);
float Skidpad_y_acc(float t);
float Skidpad_omega_acc(float t);
void Write_data(int n);
void Gnuplot(int n);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "simulation";
    const char dir_1[] = "simulation/position";
    const char dir_2[] = "simulation/route";
    const char dir_3[] = "simulation/data";
    const char dir_4[] = "simulation/graph";

    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);
    mkdir(dir_3, dir_mode);
    mkdir(dir_4, dir_mode);

    /** 助走区間 (t0 <= t < t1) **/
    for (int i = int(t0 * hz); i < int(t1 * hz); i++)
    {
        // 時刻の計算
        const float t = i / hz;

        // 車両位置
        xw[i] = 0;
        yw[i] = Start(t);

        // 加速度
        acc_xl[i] = -1.0 * acc_start;
        acc_yl[i] = 0;
        omegal[i] = 0;
    }

    /** スキッドパッド走行区間 (t1 <= t < t2) **/
    for (int i = int(t1 * hz); i < int(t2 * hz); i++)
    {
        // 時刻の計算
        const float t = i / hz;

        // 車両位置
        xw[i] = Skidpad_x(t);
        yw[i] = Skidpad_y(t);

        // 加速度
        acc_xl[i] = 0;
        acc_yl[i] = Skidpad_y_acc(t);
        omegal[i] = Skidpad_omega_acc(t);
        // printf("time = %.3f\tx = %.3f\ty = %.3f\n", t, xw[i], yw[i]);
    }

    /** 惰走区間 (t2 <= t < t3) **/
    for (int i = int(t2 * hz); i < int(t3 * hz); i++)
    {
        // 時刻の計算
        const float t = i / hz;

        // 車両位置
        xw[i] = 0;
        yw[i] = Finish(t);

        // 加速度
        acc_xl[i] = 0;
        acc_yl[i] = 0;
        omegal[i] = 0;
        // printf("%.3f\t[s]\tx = %.3f\ty = %.3f\n", t, xw[i], yw[i]);
    }

    /** データの書き出し **/
    for (int i = int(t0 * hz); i < int(t3 * hz); i++)
    {
        const float t = i / hz;
        Write_data(i);
        if (i % 10 == 0)
        {
            Gnuplot(i);
        }
    }

    /** 加速度の書き出し **/
    char filename[] = "simulation/data/data.dat";
    fp = fopen(filename, "w");
    for (int i = int(t0 * hz); i < int(t3 * hz); i++)
    {
        float t_tmp = i / hz;
        fprintf(fp, "%f\t%f\t%f\t%f\t%f\t%f\t%f\n", t_tmp, acc_xl[i], acc_yl[i], 0.0, 0.0, 0.0, omegal[i]);
    }
    fclose(fp);

    return 0;
}

/**************************************************************/
// Function name : Start
// Description   : 車両の位置を計算
/**************************************************************/
float Start(float t)
{
    /** 走行位置の計算 **/
    float y = 1.0 / 2.0 * acc_start * t * t;
    return y;
}

/**************************************************************/
// Function name : Skidpad_x
// Description   : 走行開始区間の車両位置を計算
/**************************************************************/
float Skidpad_x(float t)
{
    /** 角移動量の計算 **/
    float theta = omega * (t - t1);

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
// Description   : スキッドパッド走行区間の車両位置を計算
/**************************************************************/
float Skidpad_y(float t)
{
    /** 角移動量の計算 **/
    float theta = omega * (t - t1);

    /** y方向位置の積算 **/
    float y = r * sin(theta) + mileage_start;

    return y;
}

/**************************************************************/
// Function name : Finish
// Description   : 惰走区間の車両位置を計算
/**************************************************************/
float Finish(float t)
{
    /** 走行位置の計算 **/
    float y = v2 * (t - t2) + mileage_start;
    return y;
}

/**************************************************************/
// Function name : Skidpad_y_acc
// Description   : スキッドパッド区間のy方向加速度の計算
/**************************************************************/
float Skidpad_y_acc(float t)
{
    /** 角移動量の計算 **/
    float theta = omega * (t - t1); // 角移動量 [rad]
    float acc_y;                    // 角速度 [rad/s]

    if (0 <= theta && theta <= 2.0 * pi * 2.0)
    {
        acc_y = -1.0 * r * omega * omega;
    }
    else if (2.0 * pi * 2.0 <= theta)
    {
        acc_y = r * omega * omega;
    }
    else
    {
        acc_y = 0;
    }

    return acc_y;
}

/**************************************************************/
// Function name : Skidpad_omega_acc
// Description   : スキッドパッド区間の角加速度の計算
/**************************************************************/
float Skidpad_omega_acc(float t)
{
    /** 角移動量の計算 **/
    float theta = omega * (t - t1); // 角移動量 [rad]
    float omegal;                   // 角速度 [rad/s]

    if (0 <= theta && theta <= 2.0 * pi * 2.0)
    {
        omegal = -1.0 * omega;
    }
    else if (2.0 * pi * 2.0 <= theta)
    {
        omegal = omega;
    }
    else
    {
        omegal = 0;
    }

    return omegal;
}

/**************************************************************/
// Function name : Write_data
// Description   : 車両の位置を計算
/**************************************************************/
void Write_data(int n)
{
    const float t = n / hz;

    /** 走行位置の書き出し **/
    char filename[100];
    sprintf(filename, "simulation/position/%d.dat", n);
    fp = fopen(filename, "w");
    fprintf(fp, "%f\t%f\t%f\n", t, xw[n], yw[n]);
    fclose(fp);

    /** 走行経路の書き出し **/
    sprintf(filename, "simulation/route/%d.dat", n);
    fp = fopen(filename, "w");
    for (int i = 0; i <= n; i++)
    {
        float t_tmp = i / hz;
        fprintf(fp, "%f\t%f\t%f\n", t_tmp, xw[i], yw[i]);
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
    const float t = n / hz;
    const float x_max = 20.0;
    const float x_min = -20.0;
    const float y_max = 25.0;
    const float y_min = -5.0;

    /** Gnuplot ファイル名の設定 **/
    char graphname[100], filename_1[100], filename_2[100];
    sprintf(filename_1, "simulation/position/%d.dat", n);
    sprintf(filename_2, "simulation/route/%d.dat", n);
    sprintf(graphname, "simulation/graph/%04d.png", n);

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal png size 800, 600 font 'Times New Roman, 16'\n");
    fprintf(gp, "set size ratio -1\n");
    fprintf(gp, "set output '%s'\n", graphname);                        // 出力ファイル
    fprintf(gp, "unset key\n");                                         // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);              // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);              // y軸の描画範囲
    fprintf(gp, "set title 'Skidpad Simulation : t = %1.3f [s]'\n", t); // グラフタイトル
    fprintf(gp, "set xlabel 'x [m]' offset 0.0, 0.0\n");                // x軸のラベル
    fprintf(gp, "set ylabel 'y [m]' offset 1.0, 0.0\n");                // y軸のラベル
    fprintf(gp, "set xtics 5.0 offset 0.0, 0.0\n");                     // x軸の間隔
    fprintf(gp, "set ytics 5.0 offset 0.0, 0.0\n");                     // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 2:3 with lines lc 'grey50' notitle, '%s' using 2:3 with points lc 'royalblue' ps 3 pt 7 notitle\n", filename_2, filename_1);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}