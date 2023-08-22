/**************************************************************/
// Program name : Estimate_position
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
int Estimate_position();
void Write_data(int num);
void Gnuplot(int n);
void RMSE();

/**************************************************************/
// Function name : main
// Description   : メイン
/**************************************************************/
int main()
{
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
// Function name : Airfoil_size
// Description   : 自己位置推定
/**************************************************************/
int Airfoil_size()
{
    /** データ名の読み取り **/
    string name_str;
    cout << "Data Name:";
    cin >> name_str;
    const char *name = name_str.c_str();

    /** ファイルの読み込み **/
    char filename[100];
    sprintf(filename, "airfoil/");
    fp = fopen(filename, "r");
    while ((fscanf(fp, "%f\t%f", &tmp[0], &tmp[1])) != EOF)
    {
    }
    fclose(fp);
}