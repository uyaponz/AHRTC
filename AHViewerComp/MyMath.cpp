/*
 * MyMath.cpp
 *
 *  Created on: 2012/05/05
 *      Author: wtd
 */

#include "MyMath.hpp"

/* 台上座標 -> CV座標 */
int convH2CV_x(double x, double hkyW, double scaleCV2H, int drawOfsX) {
    int tmp = static_cast<int>((hkyW/2.0) / scaleCV2H + drawOfsX);
    return static_cast<int>(-(x / scaleCV2H) + tmp);
}
int convH2CV_y(double y, double hkyH, double scaleCV2H, int drawOfsY) {
    int tmp = static_cast<int>((hkyH/2.0) / scaleCV2H + drawOfsY);
    return  static_cast<int>((y / scaleCV2H) + tmp);
}
CvPoint convH2CV(const CvPoint2D64f &pos, const CvPoint2D64f &hkyWH,
                 double scaleCV2H, const CvPoint &drawOfsXY)
{
    CvPoint ret
        = cvPoint(convH2CV_x(pos.x, hkyWH.x, scaleCV2H, drawOfsXY.x),
                  convH2CV_y(pos.y, hkyWH.y, scaleCV2H, drawOfsXY.y));
    return ret;
}

/* CV座標 -> 台上座標 */
double convCV2H_x(int x, double hkyW, double scaleCV2H, int drawOfsX) {
    double tmp = static_cast<double>(x) - (hkyW/2.0)/scaleCV2H - drawOfsX;
    return -scaleCV2H * tmp;
}
double convCV2H_y(int y, double hkyH, double scaleCV2H, int drawOfsY) {
    double tmp = static_cast<double>(y) - (hkyH/2.0)/scaleCV2H - drawOfsY;
    return scaleCV2H * tmp;
}
CvPoint2D64f convCV2H(const CvPoint &pos, const CvPoint2D64f &hkyWH,
                      double scaleCV2H, const CvPoint &drawOfsXY)
{
    CvPoint2D64f ret
        = cvPoint2D64f(convCV2H_x(pos.x, hkyWH.x, scaleCV2H, drawOfsXY.x),
                       convCV2H_y(pos.y, hkyWH.y, scaleCV2H, drawOfsXY.y));
    return ret;
}

