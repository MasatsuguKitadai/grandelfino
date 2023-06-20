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
const float hz_gps = 2.0; // サンプリング周期 [Hz]

/** 変数宣言 **/
vector<float> x; // x方向位置 [m]
vector<float> y; // y方向位置 [m]

/** プロトタイプ宣言 **/
int Estimate_position();
void Moving_Average(vector<float> &data);
void Write_data(int num);
void Gnuplot(int n);

/**************************************************************/
// Function name : main
// Description   : メイン
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "Estimate_position_GPS";
    const char dir_1[] = "Estimate_position_GPS/position";
    const char dir_2[] = "Estimate_position_GPS/route";
    const char dir_3[] = "Estimate_position_GPS/graph";

    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);
    mkdir(dir_3, dir_mode);

    int data_length = Estimate_position();

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
// Function name : Estimate_position
// Description   : 自己位置推定
/**************************************************************/
int Estimate_position()
{
    /** 変数設定 **/
    vector<float> t;         // 時刻 [s]
    vector<float> longitude; // 経度情報 [m]
    vector<float> latitude;  // 緯度情報 [m]

    float tmp[8];    // 一時保存用バッファ [-]
    int data_length; // データの長さ [-]

    /** ファイルの読み込み **/
    char filename[] = "GNSS_data/test.csv";
    fp = fopen(filename, "r");

    // 読み飛ばし
    const int skip = 9; // 読み飛ばす行数
    for (int i = 0; i < skip; i++)
    {
        char str[300];
        fgets(str, 256, fp);
    }

    // 読み込み
    while ((fscanf(fp, "%f,%f,%f,%f,%f,%f,%f", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6])) != EOF)
    {
        t.push_back(tmp[0]);
        longitude.push_back(tmp[7]);
        latitude.push_back(tmp[8]);

        data_length += 1;

        printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\n", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5], tmp[6]);
    }
    fclose(fp);

    /** 配列のサイズ変更 **/
    x.resize(data_length);
    y.resize(data_length);

    /** 位置の積算 **/

    return data_length;
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
    sprintf(filename, "Estimate_position_GPS/position/%d.dat", n);
    fp = fopen(filename, "w");
    fprintf(fp, "%f,%f,%f\n", t, x[n], y[n]);
    fclose(fp);

    /** 走行経路の書き出し **/
    sprintf(filename, "Estimate_position_GPS/route/%d.dat", n);
    fp = fopen(filename, "w");
    for (int i = 0; i <= n; i++)
    {
        float t_tmp = i / hz_gps;
        fprintf(fp, "%f,%f,%f\n", t_tmp, x[i], y[i]);
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
    const float x_max = 20.0;
    const float x_min = -20.0;
    const float y_max = 25.0;
    const float y_min = -5.0;

    /** Gnuplot ファイル名の設定 **/
    char graphname[100], filename_1[100], filename_2[100];
    sprintf(filename_1, "Estimate_position_GPS/position/%d.dat", n);
    sprintf(filename_2, "Estimate_position_GPS/route/%d.dat", n);
    sprintf(graphname, "Estimate_position_GPS/graph/%04d.png", n);

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal png size 800, 600 font 'Times New Roman, 16'\n");
    fprintf(gp, "set size ratio -1\n");
    fprintf(gp, "set output '%s'\n", graphname);                                             // 出力ファイル
    fprintf(gp, "unset key\n");                                                              // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                                   // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                                   // y軸の描画範囲
    fprintf(gp, "set title 'Estimated Position + MA : {/Times-Italic t} = %1.3f [s]'\n", t); // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic x} [m]' offset 0.0, 0.0\n");                     // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic y} [m]' offset 1.0, 0.0\n");                     // y軸のラベル
    fprintf(gp, "set xtics 5.0 offset 0.0, 0.0\n");                                          // x軸の間隔
    fprintf(gp, "set ytics 5.0 offset 0.0, 0.0\n");                                          // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 2:3 with lines lc 'grey50' notitle, '%s' using 2:3 with points lc 'red' ps 3 pt 7 notitle\n", filename_2, filename_1);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}