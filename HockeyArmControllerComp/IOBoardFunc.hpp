/*
 * IOBoardFunc.hpp - インタフェースのボードを扱うための関数集
 */

#ifndef IOBOARDFUNC_HPP
#define IOBOARDFUNC_HPP

namespace IOBoardFunc361316
{
    // 電圧値(実数)をDaOutputData()用の値(unsigned short)に変換する
    unsigned short getVoltageDA_10V(double v);
    // 逓倍数が有効な値であるか
    bool isEvaluateNumValid(int evNum);
}

#endif
