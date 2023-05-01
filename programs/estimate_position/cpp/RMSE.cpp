/**************************************************************/
// Program name : RMSE
// Author       : Masatsugu Kitadai
// Date         : 2023/4/23
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
const float hz_imu = 100; // サンプリング周期 [Hz]

/** 変数宣言 **/
vector<float> x; // x方向位置 [m]
vector<float> y; // y方向位置 [m]

/** プロトタイプ宣言 **/
void RMSE();

/**************************************************************/
// Function name : main
// Description   : メイン
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "Estimate_position_IMU";
    const char dir_1[] = "Estimate_position_IMU/position";
    const char dir_2[] = "Estimate_position_IMU/route";
    const char dir_3[] = "Estimate_position_IMU/graph";

    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);
    mkdir(dir_3, dir_mode);

    int data_length = Estimate_position();
    printf("data = %d\n", data_length);

    for (int i = 0; i < data_length; i++)
    {
        Write_data(i);

        if (i % 10 == 0)
        {
            Gnuplot(i);
        }
    }

    return 0;
}

/**************************************************************/
// Function name : RMSE
// Description   : RMSEの計算
/**************************************************************/
void RMSE()
{
    /** 変数設定 **/
    vector<float> t;         // 時刻 [s]
    vector<float> acc_x;     // x方向加速度 [m/s2]
    vector<float> acc_y;     // y方向加速度 [m/s2]
    vector<float> acc_z;     // z方向加速度 [m/s2]
    vector<float> omega_x;   // roll方向角加速度 [rad/s2]
    vector<float> omega_y;   // pitch方向角加速度 [rad/s2]
    vector<float> omega_z;   // yaw方向角加速度 [rad/s2]
    vector<float> longitude; // 経度情報 [m]
    vector<float> latitude;  // 緯度情報 [m]

    float tmp[8];    // 一時保存用バッファ [-]
    int data_length; // データの長さ [-]

    /** ファイルの読み込み **/
    char filename[] = "Simulation/data/data.dat";
    fp = fopen(filename, "r");
    while ((fscanf(fp, "%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7], &tmp[8])) != EOF)
    {
        t.push_back(tmp[0]);
        acc_x.push_back(tmp[1]);
        acc_y.push_back(tmp[2]);
        acc_z.push_back(tmp[3]);
        omega_x.push_back(tmp[4]);
        omega_y.push_back(tmp[5]);
        omega_z.push_back(tmp[6]);
        longitude.push_back(tmp[7]);
        latitude.push_back(tmp[8]);
        data_length += 1;
    }
    fclose(fp);
}