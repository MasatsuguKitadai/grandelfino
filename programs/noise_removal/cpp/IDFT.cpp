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
const float t = 1.0;    // 計測時刻 [s]
const int hz = 1000;    // サンプリング周波数 [Hz]
const int hz_sin = 2.0; // 正弦波の周期 [Hz]

/** グローバル変数 **/
vector<float> data(t *hz);  // 基本データ
vector<float> err_1(t *hz); // 乱数配列
vector<float> err_2(t *hz); // 乱数配列

/** プロトタイプ宣言 **/
void IDFT(const char readfile[], const char writefile[]);
void Gnuplot_DFT(const char filename[], const char graphname[], const char title[]);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "IDFT";
    const char dir_1[] = "IDFT/data";
    const char dir_2[] = "IDFT/graph";
    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);

    /** 基本データのDFT **/
    const char readfile_1[] = "DFT/data/basic_data.dat";
    const char writefile_1[] = "IDFT/data/basic_data.dat";
    const char graphfile_1[] = "IDFT/graph/basic_data.svg";
    const char graphtitle_1[] = "IDFT : Basic data";
    DFT(readfile_1, writefile_1);
    Gnuplot_DFT(writefile_1, graphfile_1, graphtitle_1);

    /** ノイズデータのDFT **/
    const char readfile_2[] = "DFT/data/noise_data.dat";
    const char writefile_2[] = "IDFT/data/noise_data.dat";
    const char graphfile_2[] = "IDFT/graph/noise_data.svg";
    const char graphtitle_2[] = "IDFT : Noise data";
    DFT(readfile_2, writefile_2);
    Gnuplot_DFT(writefile_2, graphfile_2, graphtitle_2);

    return 0;
}

/**************************************************************/
// Function name : DFT
// Description   : 離散フーリエ変換
/**************************************************************/
void DFT(const char readfile[], const char writefile[])
{
    vector<float> t;
    vector<float> value;
    vector<float> re;
    vector<float> im;
    vector<float> spectrum;

    /** ファイルの読み込み **/
    float t_tmp, value_tmp;
    fp = fopen(readfile, "r");
    while ((fscanf(fp, "%f\t%f", &t_tmp, &value_tmp)) != EOF)
    {
        t.push_back(t_tmp);
        value.push_back(value_tmp);
    }
    fclose(fp);

    /** 実数部分と虚数部分に分けてフーリエ変換 **/
    int n = value.size();
    for (int i = 0; i < value.size(); i++)
    {
        float re_tmp = 0;
        float im_tmp = 0;
        for (int j = 0; j < value.size(); j++)
        {
            re_tmp += value[j] * cos(2.0 * pi * j * i / n);
            im_tmp += -value[j] * sin(2.0 * pi * j * i / n);
        }
        float spectrum_tmp = sqrt(re_tmp * re_tmp + im_tmp * im_tmp);
        re.push_back(re_tmp);
        re.push_back(im_tmp);
        spectrum.push_back(spectrum_tmp);
    }

    /** データの書き出し **/
    fp = fopen(writefile, "w");
    for (int i = 0; i < value.size(); i++)
    {
        fprintf(fp, "%d\t%f\n", i, spectrum[i]);
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
    // fprintf(gp, "set xtics 0.1 offset 0.0, 0.0\n");                                       // x軸の間隔
    // fprintf(gp, "set ytics 0.5 offset 0.0, 0.0\n");                                       // y軸の間隔

    /** Gnuplot 書き出し **/
    fprintf(gp, "plot '%s' using 1:2 with lines lc 'black' notitle\n", filename);

    /** Gnuplot 終了 **/
    fflush(gp);            // Clean up Data
    fprintf(gp, "exit\n"); // Quit gnuplot
    pclose(gp);
}