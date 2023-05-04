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
using namespace std;
FILE *fp;
mode_t dir_mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IXOTH;

/** 物理法則 **/
const float pi = 4 * atan(1.0); // 円周率 [rad]
const float g = 9.80665;        // 重力加速度 [m/s2]

/** 各種パラメータ **/
const float threshold = 50.0; // バンドパスフィルタのしきい値 [-]
const int hz = 1000;          // サンプリング周波数 [Hz]

/** プロトタイプ宣言 **/
void Bandpass_Filter(const char readfile[], const char writefile[]);
void Gnuplot_DFT(const char filename[], const char graphname[], const char title[]);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "Bandpass";
    const char dir_1[] = "Bandpass/data";
    const char dir_2[] = "Bandpass/graph";
    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);

    /** ノイズデータのDFT **/
    const char readfile[] = "DFT/data/noise_data.dat";
    const char writefile[] = "Bandpass/data/noise_data.dat";
    const char graphfile[] = "Bandpass/graph/noise_data.svg";
    const char graphtitle[] = "Bandpass Filter : Noise data";
    Bandpass_Filter(readfile, writefile);
    Gnuplot_DFT(writefile, graphfile, graphtitle);

    return 0;
}

/**************************************************************/
// Function name : Bandpass_Filter
// Description   : 離散フーリエ変換
/**************************************************************/
void Bandpass_Filter(const char readfile[], const char writefile[])
{
    vector<float> t;
    vector<float> value;

    /** ファイルの読み込み **/
    float t_tmp, value_tmp;
    fp = fopen(readfile, "r");
    while ((fscanf(fp, "%f\t%f", &t_tmp, &value_tmp)) != EOF)
    {
        t.push_back(t_tmp);
        value.push_back(value_tmp);
    }
    fclose(fp);

    /** バンドパスフィルタの適用 **/
    for (int i = 0; i < value.size(); i++)
    {
        if (threshold > value[i])
        {
            value[i] = 0;
        }
    }

    /** データの書き出し **/
    fp = fopen(writefile, "w");
    for (int i = 0; i < value.size(); i++)
    {
        fprintf(fp, "%d, %f\n", i, value[i]);
    }
    fclose(fp);
}

/**************************************************************/
// Function name : Gnuplot_DFT
// Description  :
/**************************************************************/
void Gnuplot_DFT(const char filename[], const char graphname[], const char title[])
{
    FILE *gp;

    /** Gnuplot 初期設定 **/
    const float x_max = hz / 2.0;
    const float x_min = 0;
    const float y_max = 550;
    const float y_min = 0;

    /** Gnuplot 呼び出し **/
    if ((gp = popen("gnuplot", "w")) == NULL)
    {
        printf("gnuplot is not here!\n");
        exit(0); // gnuplotが無い場合、異常ある場合は終了
    }

    /** Gnuplot 描画設定 **/
    fprintf(gp, "set terminal svg size 800, 500 font 'Times New Roman, 20'\n");
    fprintf(gp, "set size ratio 0.5\n");
    fprintf(gp, "set output '%s'\n", graphname);                                          // 出力ファイル
    fprintf(gp, "unset key\n");                                                           // 凡例非表示
    fprintf(gp, "set xrange [%.3f:%.3f]\n", x_min, x_max);                                // x軸の描画範囲
    fprintf(gp, "set yrange [%.3f:%.3f]\n", y_min, y_max);                                // y軸の描画範囲
    fprintf(gp, "set title '%s'\n", title);                                               // グラフタイトル
    fprintf(gp, "set xlabel '{/Times-Italic Frequency} [Hz]' offset 0.0, 0.0\n");         // x軸のラベル
    fprintf(gp, "set ylabel '{/Times-Italic Amplitude Spectrum} [-]' offset 1.0, 0.0\n"); // y軸のラベル

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 1:2 with lines lc 'black' notitle\n", filename);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}