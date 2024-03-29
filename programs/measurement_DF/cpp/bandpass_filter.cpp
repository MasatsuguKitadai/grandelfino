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
const float lower_threshold = 30;  // バンドパスフィルタの下限しきい値 [Hz]
const float upper_threshold = 100; // バンドパスフィルタの上限しきい値 [Hz]
const int hz = 1000;               // サンプリング周波数 [Hz]

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

    /** 基本データのDFT **/
    const char readfile_1[] = "DFT/data/basic_data.dat";
    const char writefile_1[] = "Bandpass/data/basic_data.dat";
    const char graphfile_1[] = "Bandpass/graph/basic_data.svg";
    const char graphtitle_1[] = "Bandpass Filter : Basic data";
    Bandpass_Filter(readfile_1, writefile_1);
    Gnuplot_DFT(writefile_1, graphfile_1, graphtitle_1);

    return 0;
}

/**************************************************************/
// Function name : Bandpass_Filter
// Description   : 離散フーリエ変換
/**************************************************************/
void Bandpass_Filter(const char readfile[], const char writefile[])
{
    vector<float> hz_dft;
    vector<float> spectrum;
    vector<float> re;
    vector<float> im;

    /** ファイルの読み込み **/
    float hz_tmp, spectrum_tmp, re_tmp, im_tmp;
    fp = fopen(readfile, "r");
    while ((fscanf(fp, "%f\t%f\t%f\t%f", &hz_tmp, &spectrum_tmp, &re_tmp, &im_tmp)) != EOF)
    {
        hz_dft.push_back(hz_tmp);
        spectrum.push_back(spectrum_tmp);
        re.push_back(re_tmp);
        im.push_back(im_tmp);
    }
    fclose(fp);

    /** バンドパスフィルタの適用 **/
    for (int i = 0; i < hz_dft.size(); i++)
    {
        if (upper_threshold > hz_dft[i] || hz_dft[i] > lower_threshold)
        {
            spectrum[i] = 0;
            re[i] = 0;
            im[i] = 0;
        }
    }

    /** データの書き出し **/
    fp = fopen(writefile, "w");
    for (int i = 0; i < spectrum.size(); i++)
    {
        fprintf(fp, "%f\t%f\t%f\t%f\n", hz_dft[i], spectrum[i], re[i], im[i]);
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
    const float x_max = 100;
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
