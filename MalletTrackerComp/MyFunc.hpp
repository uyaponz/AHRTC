#ifndef IGTAG_MY_FUNC_HPP
#define IGTAG_MY_FUNC_HPP

#include <string>

void Do_Calibration(double &Px, double &Py);
int Get_Calibration_Coefficient();
bool loadCalibData(std::string filename);

#endif // IGTAG_MY_FUNC_HPP
