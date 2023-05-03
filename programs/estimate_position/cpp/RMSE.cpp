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
const float hz_imu = 100.0; // サンプリング周期 [Hz]
const float hz_gps = 2.0;   // サンプリング周期 [Hz]

/** 変数設定 **/
vector<float> x;     // x方向のシミュレーション結果(真値)
vector<float> y;     // y方向のシミュレーション結果(真値)
vector<float> x_imu; // x方向のIMU単独推定値
vector<float> y_imu; // y方向のIMU単独推定値
vector<float> x_gps; // x方向のIMU＋GPS推定値
vector<float> y_gps; // y方向のIMU＋GPS推定値

/** プロトタイプ宣言 **/
int Get_number();
void Read_data(const char *filename, vector<float> &x, vector<float> &y);
float RMSE(vector<float> &data1, vector<float> &data2);
float RMSE_2(vector<float> &data11, vector<float> &data12, vector<float> &data21, vector<float> &data22);

/**************************************************************/
// Function name : main
// Description   : メイン
/**************************************************************/
int main()
{
    int data_length = Get_number();

    char filename[100];

    /** シミュレーション(真値) **/
    sprintf(filename, "Simulation/route/%d.dat", data_length - 1); // 読み込みファイル
    Read_data(filename, x, y);
    printf("Read: %s\n", filename);

    /** IMU単独 **/
    sprintf(filename, "Estimate_position_IMU/route/%d.dat", data_length - 1); // 読み込みファイル
    Read_data(filename, x_imu, y_imu);
    printf("Read: %s\n", filename);

    /** IMU+GPS **/
    sprintf(filename, "Estimate_position_IMU+GPS/route/%d.dat", data_length - 1); // 読み込みファイル
    Read_data(filename, x_gps, y_gps);
    printf("Read: %s\n", filename);

    float rmse_x_imu = RMSE(x, x_imu);
    float rmse_y_imu = RMSE(y, y_imu);
    float rmse_v_imu = RMSE_2(x, x_imu, y, y_imu);

    float rmse_x_gps = RMSE(x, x_gps);
    float rmse_y_gps = RMSE(y, y_gps);
    float rmse_v_gps = RMSE_2(x, x_gps, y, y_gps);

    printf("IMU    \tx = %.3f [m]\ty = %.3f [m]\td = %.3f [m]\n", rmse_x_imu, rmse_y_imu, rmse_v_imu);
    printf("IMU+GPS\tx = %.3f [m]\ty = %.3f [m]\td = %.3f [m]\n", rmse_x_gps, rmse_y_gps, rmse_v_gps);

    return 0;
}

/**************************************************************/
// Function name : Get_number
// Description   : データの読み込み
/**************************************************************/
int Get_number()
{
    int data_length; // データの長さ [-]

    /** ファイル名の取得 **/
    char buf[200];                                // 文字列用バッファ
    char datafile[] = "Simulation/data/data.dat"; // 読み込みファイル
    fp = fopen(datafile, "r");
    while (fgets(buf, 200, fp) != NULL)
    {
        data_length += 1;
    }
    fclose(fp);

    printf("data_length = %d\n", data_length);

    return data_length;
}

/**************************************************************/
// Function name : Read_data
// Description   : データの読み込み
/**************************************************************/
void Read_data(const char *filename, vector<float> &x, vector<float> &y)
{
    float tmp[5]; // 読み込み時のバッファ

    fp = fopen(filename, "r");
    while (fscanf(fp, "%f\t%f\t%f\t%f\t%f", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4]) != EOF)
    {
        x.push_back(tmp[1]);
        y.push_back(tmp[2]);
    }
    fclose(fp);
}

/**************************************************************/
// Function name : RMSE
// Description   : RMSEの計算
/**************************************************************/
float RMSE(vector<float> &data1, vector<float> &data2)
{
    float sum = 0;
    float n = 0;

    /** 総和計算 **/
    for (int i = 0; i < data1.size(); i++)
    {
        sum += (data2[i] - data1[i]) * (data2[i] - data1[i]);
        n += 1;
    }

    /** RSMEの計算 **/
    float rmse = sqrt(1.0 / n * sum);

    return rmse;
}

/**************************************************************/
// Function name : RMSE_2
// Description   : 2成分のRMSEの計算
/**************************************************************/
float RMSE_2(vector<float> &data11, vector<float> &data12, vector<float> &data21, vector<float> &data22)
{
    float sum = 0;
    float n = 0;

    /** 総和計算 **/
    for (int i = 0; i < data11.size(); i++)
    {
        sum += (data12[i] - data11[i]) * (data12[i] - data11[i]) + (data22[i] - data21[i]) * (data22[i] - data21[i]);
        n += 1;
    }

    /** RSMEの計算 **/
    float rmse = sqrt(1.0 / n * sum);

    return rmse;
}