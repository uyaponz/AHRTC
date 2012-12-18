#ifndef IGTAG_MY_MATH_HPP_1F3E660B_EB43_4224_A095_6051705900A8
#define IGTAG_MY_MATH_HPP_1F3E660B_EB43_4224_A095_6051705900A8

#include <vector>
#include <opencv2/opencv.hpp>

#include "URGScanner.hpp"

namespace MyFunc
{

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

    /* URG座標 -> 台上座標 */
    double convU2H_x(double x, double urgX);
    double convU2H_y(double y, double urgY);
    CvPoint2D64f convU2H(const CvPoint2D64f &pos, const CvPoint2D64f &urgXY);

    /* 台上座標 -> URG座標 */
    double convH2U_x(double x, double urgX);
    double convH2U_y(double y, double urgY);
    CvPoint2D64f convH2U(const CvPoint2D64f &pos, const CvPoint2D64f &urgXY);

    /* 外積 */
    double getCross(CvPoint2D64f p1, CvPoint2D64f p2);

    /* スキャン点のY座標符号を反転する */
    void flipScanY(std::vector<URGController::ScanPoint_t> &sp);
    /* URG取り付け角度を加味したスキャン点座標に変換する */
    void applyScanAngle(std::vector<URGController::ScanPoint_t> &sp,
                        double urgAngle);
    /* センサ値のスケールを反映する */
    void applyScanScale(std::vector<URGController::ScanPoint_t> &sp,
                        const CvPoint2D64f &scaleXY);
    /* スキャン点までの距離を再計算する */
    void updateScanDist(std::vector<URGController::ScanPoint_t> &sp);

}

#endif
