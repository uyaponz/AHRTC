#include "MyMath.hpp"

namespace MyFunc
{

    /* 台上座標 -> CV座標 */
    int convH2CV_x(double x, double hkyW, double scaleCV2H, int drawOfsX) {
        int tmp = static_cast<int>((hkyW/2.0) / scaleCV2H) + drawOfsX;
        return static_cast<int>(-(x / scaleCV2H)) + tmp;
    }
    int convH2CV_y(double y, double hkyH, double scaleCV2H, int drawOfsY) {
        int tmp = static_cast<int>((hkyH/2.0) / scaleCV2H) + drawOfsY;
        return static_cast<int>( (y / scaleCV2H)) + tmp;
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

    /* URG座標 -> 台上座標 */
    double convU2H_x(double x, double urgX) {
        return x + urgX;
    }
    double convU2H_y(double y, double urgY) {
        return y + urgY;
    }
    CvPoint2D64f convU2H(const CvPoint2D64f &pos, const CvPoint2D64f &urgXY)
    {
        return cvPoint2D64f(convU2H_x(pos.x, urgXY.x),
                            convU2H_y(pos.y, urgXY.y));
    }

    /* 台上座標 -> URG座標 */
    double convH2U_x(double x, double urgX) {
        return x - urgX;
    }
    double convH2U_y(double y, double urgY) {
        return y - urgY;
    }
    CvPoint2D64f convH2U(const CvPoint2D64f &pos, const CvPoint2D64f &urgXY)
    {
        return cvPoint2D64f(convH2U_x(pos.x, urgXY.x),
                            convH2U_y(pos.y, urgXY.y));
    }

    /* 外積 */
    double getCross(CvPoint2D64f p1, CvPoint2D64f p2) {
        return (p1.x)*(p2.y) - (p1.y)*(p2.x);
    }

    /* スキャン点のY座標符号を反転する */
    void flipScanY(std::vector<URGController::ScanPoint_t> &sp) {
        for (std::vector<URGController::ScanPoint_t>::iterator it=sp.begin();
             it != sp.end(); ++it)
        {
            (*it).y = -(*it).y;
        }
    }

    /* URG取り付け角度を加味したスキャン点座標に変換する */
    void applyScanAngle(std::vector<URGController::ScanPoint_t> &sp, double urgAngle) {
        const double C = cos(urgAngle);
        const double S = sin(urgAngle);

        for (std::vector<URGController::ScanPoint_t>::iterator it=sp.begin();
             it != sp.end(); ++it)
        {
            double x = (*it).x;
            double y = (*it).y;
            (*it).x = x*C - y*S;
            (*it).y = x*S + y*C;
        }
    }

    /* センサ値のスケールを反映する */
    void applyScanScale(std::vector<URGController::ScanPoint_t> &sp,
                        const CvPoint2D64f &scaleXY)
    {
        for (std::vector<URGController::ScanPoint_t>::iterator it=sp.begin();
             it != sp.end(); ++it)
        {
            (*it).x *= scaleXY.x;
            (*it).y *= scaleXY.y;
        }
    }

    /* スキャン点までの距離を再計算する */
    void updateScanDist(std::vector<URGController::ScanPoint_t> &sp) {
        for (std::vector<URGController::ScanPoint_t>::iterator it=sp.begin();
             it != sp.end(); ++it)
        {
            double x = (*it).x;
            double y = (*it).y;
            (*it).dist = sqrt( x*x + y*y );
        }
    }

}
