/**************************************************************/
// Program name : Skidpad_simulation
// Author       : Masatsugu Kitadai
// Date         : 2023/4/22
// Description  :
/**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;

/** パラメータ **/
const float r = 8.0;  // 旋回半径 [m]
const float v = 40.0; // 走行速度 [km/h]
const float hz = 100; // サンプリング周期 [Hz]
const float n = 4.0;  // 周回数 [-]

/** パラメータ（自動的に決まる） **/
const float v2 = v * 1000.0 / 3600.0;   // 走行速度 [m/s]
const float omega = v2 / r;             // 角速度 [rad/s]
const float pi = 4 * atan(1.0);         // 円周率 [rad]
const float mileage = 2.0 * pi * r * n; // 走行距離 [m]
const float sec_max = mileage / v2;     // 最大時刻 [s]

/** プロトタイプ宣言 **/
float Skidpad_x(float sec);
float Skidpad_y(float sec);
void Gnuplot();

/**************************************************************/
// Function name : main
// Description  :
/**************************************************************/
int main()
{
    /** 配列 **/
    vector<float> sec(sec_max * hz);
    vector<float> x(sec_max * hz);
    vector<float> y(sec_max * hz);

    /** 車両位置の計算 **/
    for (int i = 0; i < sec_max * hz; i++)
    {
        sec[i] = 1.0 / hz * i;
        x[i] = Skidpad_x(i);
        y[i] = Skidpad_y(i);
        printf("%.3f\t[s]\tx = %.3f\ty = %.3f\n", sec[i], x[i], y[i]);
    }

    return 0;
}

/**************************************************************/
// Function name : Skidpad_x
// Description  :
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
// Description  :
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
// Function name : Gnuplot
// Description  :
/**************************************************************/
void Gnuplot()
{
}