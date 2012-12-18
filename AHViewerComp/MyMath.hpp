/*
 * MyMath.hpp
 *
 *  Created on: 2012/05/05
 *      Author: wtd
 */

#ifndef MY_MATH_HPP_1F3E660B_EB43_4224_A095_6051705900A8
#define MY_MATH_HPP_1F3E660B_EB43_4224_A095_6051705900A8

#include <vector>
#include <opencv2/opencv.hpp>

/* 台上座標 -> CV座標 */
int convH2CV_x(double x, double hkyW, double scaleCV2H, int drawOfsX);
int convH2CV_y(double y, double hkyH, double scaleCV2H, int drawOfsY);
CvPoint convH2CV(const CvPoint2D64f &pos, const CvPoint2D64f &hkyWH,
                 double scaleCV2H, const CvPoint &drawOfsXY);

/* CV座標 -> 台上座標 */
double convCV2H_x(int x, double hkyW, double scaleCV2H, int drawOfsX);
double convCV2H_y(int y, double hkyH, double scaleCV2H, int drawOfsY);
CvPoint2D64f convCV2H(const CvPoint &pos, const CvPoint2D64f &hkyWH,
                      double scaleCV2H, const CvPoint &drawOfsXY);
#endif // MY_MATH_HPP_1F3E660B_EB43_4224_A095_6051705900A8
