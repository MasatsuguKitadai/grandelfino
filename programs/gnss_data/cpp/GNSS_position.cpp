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
const float hz_gps = 1.0;       // サンプリング周期 [Hz]
const float pi = 4 * atan(1.0); // 円周率 [rad]

/** 変数宣言 **/
vector<float> x;         // x方向位置 [m]
vector<float> y;         // y方向位置 [m]
vector<float> longitude; // 経度情報 [m]
vector<float> latitude;  // 緯度情報 [m]
const char program_name[] = "GNSS position";

/** プロトタイプ宣言 **/
int Estimate_position();
void Distance(float lat1, float lng1, float lat2, float lng2, int n);
void Write_data(int num);
void Gnuplot(int n);
int Progress_meter(const char program_name[], int i, int max, int progress_count);

/**************************************************************/
// Function name : main
// Description   : メイン
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "GNSS_position";
    const char dir_1[] = "GNSS_position/position";
    const char dir_2[] = "GNSS_position/route";
    const char dir_3[] = "GNSS_position/graph";

    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);
    mkdir(dir_3, dir_mode);

    int data_length = Estimate_position();
    int progress_counter = 0;

    for (int i = 0; i < data_length; i++)
    {
        /* 進捗表示 */
        progress_counter = Progress_meter(program_name, i, data_length - 1, progress_counter);

        Distance(latitude[0], longitude[0], latitude[i], longitude[i], i);
        // printf("(x,y) = (%.3f,\t%.3f)\n", x[i], y[i]);

        Write_data(i);
        if (i % 10 == 0)
        {
            Gnuplot(i);
        }
    }

    return 0;
}

/**************************************************************/
// Function name : Estimate_position
// Description   : 自己位置推定
/**************************************************************/
int Estimate_position()
{
    char str[300];   // 文字列用バッファ [-]
    float tmp[8];    // 一時保存用バッファ [-]
    int data_length; // データの長さ [-]

    /** ファイルの読み込み **/
    char filename[] = "GNSS_data/test.csv";
    fp = fopen(filename, "r");

    // 読み飛ばし
    const int skip = 9; // 読み飛ばす行数
    for (int i = 0; i < skip; i++)
    {
        fgets(str, 256, fp);
    }

    // 読み込み
    while ((fscanf(fp, "%f,%[^,],%f,%f,%f,%f,%f", &tmp[0], str, &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5])) != EOF)
    {
        data_length += 1;
        // printf("%.0f\t%f\t%f\n", tmp[0], tmp[1], tmp[2]);
        latitude.push_back(tmp[1]);
        longitude.push_back(tmp[2]);
    }
    fclose(fp);

    /** 配列のサイズ変更 **/
    x.resize(data_length);
    y.resize(data_length);

    /** 位置の積算 **/

    return data_length;
}

/**************************************************************/
// Function name : Distance
// Description   : 距離計算
/**************************************************************/
void Distance(float lat1, float lng1, float lat2, float lng2, int n)
{
    // 緯度経度をラジアンに変換
    const double rlat1 = lat1 * pi / 180; // 基準値　[rad]
    const double rlng1 = lng1 * pi / 180; // 基準値　[rad]
    const double rlat2 = lat2 * pi / 180; // 移動後の位置 [rad]
    const double rlng2 = lng2 * pi / 180; // 移動後の位置 [rad]

    // 2点の中心角(rad)を求める
    double a_ew = sin(rlat1) * sin(rlat2) + cos(rlat1) * cos(rlat2) * cos(rlng1 - rlng1);
    double a_ns = sin(rlat1) * sin(rlat1) + cos(rlat1) * cos(rlat1) * cos(rlng1 - rlng2);
    double rr_ew = acos(a_ew);
    double rr_ns = acos(a_ns);

    // 地球赤道半径(m)
    const double earth_radius = 6378140;

    // 2点間の距離(km)
    x[n] = earth_radius * rr_ew;
    y[n] = earth_radius * rr_ns;
}

/**************************************************************/
// Function name : Write_data
// Description   : 車両の位置を計算
/**************************************************************/
void Write_data(int n)
{
    const float t = n / hz_gps;

    /** 走行位置の書き出し **/
    char filename[100];
    sprintf(filename, "GNSS_position/position/%d.dat", n);
    fp = fopen(filename, "w");
    fprintf(fp, "%f\t%f\t%f\n", t, x[n], y[n]);
    fclose(fp);

    /** 走行経路の書き出し **/
    sprintf(filename, "GNSS_position/route/%d.dat", n);
    fp = fopen(filename, "w");
    for (int i = 0; i <= n; i++)
    {
        float t_tmp = i / hz_gps;
        fprintf(fp, "%f\t%f\t%f\n", t_tmp, x[i], y[i]);
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
    const float t = n / hz_gps;
    const float x_max = 4000;
    const float x_min = -1000;
    const float y_max = 5000;
    const float y_min = 0;

    /** Gnuplot ファイル名の設定 **/
    char graphname[100], filename_1[100], filename_2[100];
    sprintf(filename_1, "GNSS_position/position/%d.dat", n);
    sprintf(filename_2, "GNSS_position/route/%d.dat", n);
    sprintf(graphname, "GNSS_position/graph/%04d.png", n);

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal png size 800, 600 font 'Times New Roman, 16'\n");
    fprintf(gp, "set size ratio -1\n");
    fprintf(gp, "set output '%s'\n", graphname);                                   // 出力ファイル
    fprintf(gp, "unset key\n");                                                    // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                         // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                         // y軸の描画範囲
    fprintf(gp, "set title 'GNSS Position : {/Times-Italic t} = %1.3f [s]'\n", t); // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic x} [m]' offset 0.0, 0.0\n");           // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic y} [m]' offset 1.0, 0.0\n");           // y軸のラベル
    fprintf(gp, "set xtics 1000.0 offset 0.0, 0.0\n");                             // x軸の間隔
    fprintf(gp, "set ytics 1000.0 offset 0.0, 0.0\n");                             // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 2:3 with lines lc 'grey50' notitle, '%s' using 2:3 with points lc 'red' ps 1.5 pt 7 notitle\n", filename_2, filename_1);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}

/******************************************************************************
FUNCTION : Progress_meter
概要     ：進捗割合を表示する
******************************************************************************/
int Progress_meter(const char program_name[], int i, int max, int progress_count)
{
    /* メーターの表示 */
    if (i == 0)
    {
        printf("\n【%s】\n", program_name);
        printf("0    10   20   30   40   50   60   70   80   90  100 [%%]\n"
               "|----|----|----|----|----|----|----|----|----|----|\n"
               "*");
        fflush(stdout);
        progress_count += 1;
    }

    /* 進捗率の計算 */
    const float progress_ratio = (float)i / (float)max * 50.0; // [%]
    if (progress_ratio > progress_count)
    {
        printf("*");
        fflush(stdout);
        progress_count += 1;
    }

    /* 完了 */
    if (i == max - 1)
    {
        printf("*\n\n");
    }

    return progress_count;
}