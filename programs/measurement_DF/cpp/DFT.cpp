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
const float t = 300; // 計測時刻 [s]
const int hz = 10;   // サンプリング周波数 [Hz]

/** グローバル変数 **/
vector<float> data(t *hz);  // 基本データ
vector<float> err_1(t *hz); // 乱数配列
vector<float> err_2(t *hz); // 乱数配列

/** プロトタイプ宣言 **/
void DFT(const char readfile[], const char writefile[]);
void Gnuplot_DFT(const char filename[], const char graphname[], const char title[]);

/**************************************************************/
// Function name : main
// Description   : メインプログラム
/**************************************************************/
int main()
{
    /** ディレクトリの作成 **/
    const char dir_0[] = "DFT";
    const char dir_1[] = "DFT/data";
    const char dir_2[] = "DFT/graph";
    mkdir(dir_0, dir_mode);
    mkdir(dir_1, dir_mode);
    mkdir(dir_2, dir_mode);

    /** 基本データのDFT **/
    const char readfile_1[] = "data/data.dat";
    const char writefile_1[] = "DFT/data/basic_data.dat";
    const char graphfile_1[] = "DFT/graph/basic_data.svg";
    const char graphtitle_1[] = "DFT : Basic data";
    DFT(readfile_1, writefile_1);
    Gnuplot_DFT(writefile_1, graphfile_1, graphtitle_1);

    return 0;
}

/**************************************************************/
// Function name : DFT
// Description   : 離散フーリエ変換
/**************************************************************/
void DFT(const char readfile[], const char writefile[])
{
    vector<float> t;
    vector<float> f;
    vector<float> re;
    vector<float> im;
    vector<float> spectrum;

    /** ファイルの読み込み **/
    float t_tmp, f_tmp;
    fp = fopen(readfile, "r");
    while ((fscanf(fp, "%f,%f", &t_tmp, &f_tmp)) != EOF)
    {
        t.push_back(t_tmp);
        f.push_back(f_tmp);
    }
    fclose(fp);

    /** 実数部分と虚数部分に分けてフーリエ変換 **/
    const int n = f.size();
    for (int i = 0; i < f.size(); i++)
    {
        float re_tmp = 0;
        float im_tmp = 0;
        for (int j = 0; j < f.size(); j++)
        {
            re_tmp += f[j] * cos(2.0 * pi * j * i / n);
            im_tmp += -f[j] * sin(2.0 * pi * j * i / n);
        }
        float spectrum_tmp = sqrt(re_tmp * re_tmp + im_tmp * im_tmp);
        re.push_back(re_tmp);
        im.push_back(im_tmp);
        spectrum.push_back(spectrum_tmp);
    }

    /** データの書き出し **/
    fp = fopen(writefile, "w");
    for (int i = 0; i < f.size(); i++)
    {
        fprintf(fp, "%f\t%f\t%f\t%f\n", (float)i, spectrum[i], re[i], im[i]);
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
    const float x_max = 500;
    const float x_min = 0;
    const float y_max = 1000;
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