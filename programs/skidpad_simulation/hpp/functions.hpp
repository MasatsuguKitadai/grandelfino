/******************************************************************************
PROGRAM : functions.h
AUTHER  : Masatsugu Kitadai
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/******************************************************************************
FUNCTION : Gaussian
概要：1次元の正規分布を与える
 IN ：x：調べる値，mu：平均値，sigma：分散の二乗根
OUT ：X：確率変数
******************************************************************************/
float Gaussian(float x, float mu, float sigma)
{
    // 円周率 pi
    const float pi = 4 * atan(1.0);

    // ガウス分布に従った値を返す
    float X = exp(-1 * (x - mu) * (x - mu) / (2 * sigma * sigma));

    return X;
}

/******************************************************************************
FUNCTION : Gaussian_2D
概要：2次元の正規分布を与える
 IN ：x：x方向の調べる値，mu_x：x方向の平均値，sigma_x：x方向の分散の二乗根
      y：y方向の調べる値，mu_y：y方向の平均値，sigma_y：y方向の分散の二乗根
OUT ：X：確率変数
******************************************************************************/
float Gaussian_2D(float x, float y, float mu_x, float mu_y, float sigma)
{
    // 円周率 pi
    const float pi = 4 * atan(1.0);

    // ガウス分布に従った値を返す
    float X = exp(-1 * ((x - mu_x) * (x - mu_x) + (y - mu_y) * (y - mu_y)) / (2 * sigma * sigma));

    return X;
}