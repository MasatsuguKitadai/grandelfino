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

/** グローバル変数 **/
vector<float> xw(t3 *hz_6axis); // 絶対座標系 x軸方向 : World coordinate system
vector<float> yw(t3 *hz_6axis); // 絶対座標系 y軸方向 : World coordinate system
vector<float> xl(t3 *hz_6axis); // 車両座標系 x軸方向 : Local coordinate system
vector<float> yl(t3 *hz_6axis); // 車両座標系 y軸方向 : Local coordinate system

vector<float> acc_xl(t3 *hz_6axis);    // 測定データ：車両に加わる加速度 x軸方向
vector<float> acc_yl(t3 *hz_6axis);    // 測定データ：車両に加わる加速度 y軸方向
vector<float> omegal(t3 *hz_6axis);    // 測定データ：車両に加わる角加速度 z軸方向
vector<float> latitude(t3 *hz_6axis);  // 測定データ：車両位置 緯度
vector<float> longitude(t3 *hz_6axis); // 測定データ：車両位置 経度

vector<float> err_xl(t3 *hz_6axis);     // 乱数配列
vector<float> err_yl(t3 *hz_6axis);     // 乱数配列
vector<float> err_omegal(t3 *hz_6axis); // 乱数配列
vector<float> err_lon(t3 *hz_6axis);    // 乱数配列
vector<float> err_lat(t3 *hz_6axis);    // 乱数配列
vector<float> err_buf(t3 *hz_6axis);    // 乱数配列

/** プロトタイプ宣言 **/
float Start(float t);
float Finish(float t);
float Skidpad_x(float t);
float Skidpad_y(float t);
float Skidpad_y_acc(float t);
float Skidpad_omega_acc(float t);
float GPS_latitude(int n);
float GPS_longitude(int n);
void Random_number(vector<float> &err);
void Box_Mullers_method(vector<float> &x1, vector<float> &x2);
void Write_data(int n);
void Gnuplot(int n);
void Gnuplot_2();

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

    /** 誤差データの生成 **/
    srand((unsigned int)time(NULL)); // シード値の変更

    // 一様乱数の生成
    Random_number(err_xl);
    Random_number(err_yl);
    Random_number(err_omegal);
    Random_number(err_lon);
    Random_number(err_lat);
    Random_number(err_buf);

    // 正規分布乱数への変換
    Box_Mullers_method(err_xl, err_yl);
    Box_Mullers_method(err_omegal, err_lon);
    Box_Mullers_method(err_lat, err_buf);

    /** 助走区間 (t0 <= t < t1) **/
    for (int i = int(t0 * hz_6axis); i < int(t1 * hz_6axis); i++)
    {
        // 時刻の計算
        const float t = i / hz_6axis;

        // 車両位置
        xw[i] = 0;
        yw[i] = Start(t);

        // 加速度
        acc_xl[i] = -1.0 * acc_start;
        acc_yl[i] = 0;
        omegal[i] = 0;

        // 経度・緯度
        longitude[i] = GPS_longitude(i);
        latitude[i] = GPS_latitude(i);
    }

    /** スキッドパッド走行区間 (t1 <= t < t2) **/
    for (int i = int(t1 * hz_6axis); i < int(t2 * hz_6axis); i++)
    {
        // 時刻の計算
        const float t = i / hz_6axis;

        // 車両位置
        xw[i] = Skidpad_x(t);
        yw[i] = Skidpad_y(t);

        // 加速度
        acc_xl[i] = 0;
        acc_yl[i] = Skidpad_y_acc(t);
        omegal[i] = Skidpad_omega_acc(t);

        // 経度・緯度
        longitude[i] = GPS_longitude(i);
        latitude[i] = GPS_latitude(i);
    }

    /** 惰走区間 (t2 <= t < t3) **/
    for (int i = int(t2 * hz_6axis); i < int(t3 * hz_6axis); i++)
    {
        // 時刻の計算
        const float t = i / hz_6axis;

        // 車両位置
        xw[i] = 0;
        yw[i] = Finish(t);

        // 加速度
        acc_xl[i] = 0;
        acc_yl[i] = 0;
        omegal[i] = 0;

        // 経度・緯度
        longitude[i] = GPS_longitude(i);
        latitude[i] = GPS_latitude(i);
    }

    /** 真値の書き出し **/
    for (int i = int(t0 * hz_6axis); i < int(t3 * hz_6axis); i++)
    {
        const float t = i / hz_6axis;
        Write_data(i);
        if (i % 10 == 0)
        {
            Gnuplot(i);
        }
    }

    /** 誤差の足し算 **/
    for (int i = 0; i < acc_xl.size(); i++)
    {
        acc_xl[i] += err_g * err_xl[i];
        acc_yl[i] += err_g * err_yl[i];
        omegal[i] += err_omega * err_omegal[i];
        longitude[i] += err_gps * err_lon[i];
        latitude[i] += err_gps * err_lat[i];
    }

    /** 加速度の書き出し **/
    char filename[] = "simulation/data/data.dat";
    fp = fopen(filename, "w");
    for (int i = 0; i < acc_xl.size(); i++)
    {
        float t_tmp = i / hz_6axis;
        fprintf(fp, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", t_tmp, acc_xl[i], acc_yl[i], 0.0, 0.0, 0.0, omegal[i], longitude[i], latitude[i]);
    }
    fclose(fp);

    /** 模擬計測データの描画 **/
    Gnuplot_2();

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
// Function name : GPS_longitude
// Description   : スキッドパッド区間の角加速度の計算
/**************************************************************/
float GPS_longitude(int n)
{
    /** 緯度情報の取得 **/
    const int interval = hz_6axis / hz_gps;
    float longitude;

    if (n % interval == 0)
    {
        longitude = xw[n];
    }
    else
    {
        longitude = -100;
    }

    return longitude;
}

/**************************************************************/
// Function name : GPS_latitude
// Description   : スキッドパッド区間の角加速度の計算
/**************************************************************/
float GPS_latitude(int n)
{
    /** 経度情報の取得 **/
    const int interval = hz_6axis / hz_gps;
    float latitude;

    if (n % interval == 0)
    {
        latitude = yw[n];
    }
    else
    {
        latitude = -100;
    }

    return latitude;
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
// Description   : 正規分布の乱数を与える (x1,x2は互いに独立な一様乱数)
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
void Write_data(int n)
{
    const float t = n / hz_6axis;

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
        float t_tmp = i / hz_6axis;
        fprintf(fp, "%f\t%f\t%f\t%f\t%f\n", t_tmp, xw[i], yw[i], longitude[i], latitude[i]);
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
    const float t = n / hz_6axis;
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
    fprintf(gp, "set output '%s'\n", graphname);                                        // 出力ファイル
    fprintf(gp, "unset key\n");                                                         // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                              // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                              // y軸の描画範囲
    fprintf(gp, "set title 'Skidpad Simulation : {/Times-Italic t} = %1.3f [s]'\n", t); // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic x} [m]' offset 0.0, 0.0\n");                // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic y} [m]' offset 1.0, 0.0\n");                // y軸のラベル
    fprintf(gp, "set xtics 5.0 offset 0.0, 0.0\n");                                     // x軸の間隔
    fprintf(gp, "set ytics 5.0 offset 0.0, 0.0\n");                                     // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 2:3 with lines lc 'grey50' notitle, '%s' using 2:3 with points lc 'royalblue' ps 3 pt 7 notitle\n", filename_2, filename_1);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}

/**************************************************************/
// Function name : Gnuplot_2
// Description  :
/**************************************************************/
void Gnuplot_2()
{
    FILE *gp;

    /** Gnuplot 初期設定 **/
    const int x_max = t3;
    const int x_min = 0;
    const float y_max = 2.0;
    const float y_min = -2.0;

    /** Gnuplot ファイル名の設定 **/
    const char filename[] = "simulation/data/data.dat";
    const char graphname[] = "simulation/data/data.png";

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
    fprintf(gp, "plot '%s' using 1: with lines lc 'black' notitle\n", filename);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}