/*
 * IOBoardFunc.cpp - インタフェースのボードを扱うための関数集
 */

#include "IOBoardFunc.hpp"

namespace IOBoardFunc361316
{
    /* --- 電圧値(実数)をDaOutputData()用の値(unsigned short)に変換する --- */
    unsigned short getVoltageDA_10V(double v)
    {
        static const int boardMaxV = 10; // 入出力ボードの出力限界電圧
        static const unsigned short maxVal = 0xFFFF; // 最大設定値

        if (v >=  boardMaxV) v= boardMaxV;
        if (v <= -boardMaxV) v=-boardMaxV;

        return static_cast<unsigned short>((v+boardMaxV) / ((2.0*boardMaxV)/maxVal));
    }

    /* --- 逓倍数が有効な値であるか --- */
    bool isEvaluateNumValid(int evNum)
    {
        if (evNum!=1 && evNum!=2 && evNum!=4) return false;
        return true;
    }
}
