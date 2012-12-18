/*
 * MotorDriverFunc.cpp - モータードライバを扱うための関数
 */

#include "MotorDriverFunc.hpp"

namespace MyMotorDriverFunc
{
    double rpm2voltage(int rpm, int rpm6V)
    {
        return 6.0 * (static_cast<double>(rpm) / rpm6V);
    }
}
