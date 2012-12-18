#include "MyMath.hpp"

#include <cmath>
#include <sys/time.h>

double gettimeofday_sec() {
    struct timeval t;
    gettimeofday(&t, NULL);
    double retVal
        = static_cast<double>(t.tv_sec)
        + static_cast<double>(t.tv_usec) * 1e-6;
    return retVal;
}

MyPos myPos(double x, double y, double time) {
    MyPos r = {x, y, time};
    return r;
}

// 傾きを求める(inv=true : XとYを逆にする)
MyPos calcSlope(std::deque<MyPos> &q, int cnt, bool inv) {
    MyPos  retVal = myPos(0.0, 0.0);

    double sumX   = 0.0;
    double sumY   = 0.0;
    double sumX2  = 0.0;
    double sumXY  = 0.0;

    int    size   = q.size();
    if (size < cnt || size <= 1) return retVal;

    int i = 0;
    typedef std::deque<MyPos>::iterator DeqIt;
    for (DeqIt it=q.begin(); it!=q.end(); ++it) {
        double x = (inv) ? (*it).y : (*it).x;
        double y = (inv) ? (*it).x : (*it).y;

        sumX  += x;
        sumY  += y;
        sumX2 += x * x;
        sumXY += x * y;

        if (++i >= cnt) break;
    }

    // A
    retVal.x  = sumXY * cnt - sumX * sumY;
    retVal.x /= sumX2 * cnt - sumX * sumX;
    // B
    retVal.y  = sumX2 * sumY - sumXY * sumX;
    retVal.y /= sumX2 * cnt  - sumX  * sumX;

    return retVal;
}

bool isArmMovable(CvPoint2D64f pos,
                  double arm1Length, double arm2Length,
                  double left, double right, double bottom, double top)
{
    double armLen = sqrt(pow(pos.x,2) + pow(pos.y,2));
    double maxLen = arm1Length + arm2Length;
    double minLen = fabs(arm1Length - arm2Length);

    if (minLen < armLen && armLen < maxLen
        && (left   < pos.x && pos.x < right)
        && (bottom < pos.y && pos.y < top))
    {
        return true;
    }

    return false;
}

bool isArmMovable(CvPoint2D64f pos, double arm1Len, double arm2Len,
                  double hockeyW, double malletR, double armOffset)
{
    return isArmMovable(pos, arm1Len, arm2Len,
                        -((hockeyW/2) + (malletR/2)),
                          (hockeyW/2) - (malletR/2),
                        armOffset + (malletR/2),
                        arm1Len + arm2Len);
}

CvPoint2D64f getFK(CvPoint2D64f dir, double arm1Length, double arm2Length)
{
    double th1 = dir.x;
    double th2 = dir.y;
    double l1  = arm1Length;
    double l2  = arm2Length;

    CvPoint2D64f retval;
    retval.x = l1 * cos(th1) + l2 * cos(th1+th2);
    retval.y = l1 * sin(th1) + l2 * sin(th1+th2);

    return retval;
}

CvPoint2D64f getIK(CvPoint2D64f pos,
                   double arm1Length, double arm2Length,
                   bool isRightArm)
{
    double x = pos.x;
    double y = pos.y;
    double l1 = arm1Length;
    double l2 = arm2Length;

    double alpha = -(x*x + y*y) + l1*l1 + l2*l2;
    alpha       /= 2 * l1 * l2;
    alpha = acos(alpha);
    double beta = (x*x + y*y) + l1*l1 - l2*l2;
    beta       /= 2 * l1 * sqrt(x*x + y*y);
    beta = acos(beta);

    CvPoint2D64f retval;
    retval.x = atan2(y,x) + ((isRightArm) ? -beta : beta);
    retval.y = (M_PI - alpha); if (!isRightArm) retval.y = -retval.y;

    return retval;
}

signed long rad2pulse(double rad, signed long enc1rot)
{
    return static_cast<signed long>(enc1rot * (rad / (2.0*M_PI)));
}

double pulse2rad(signed long pulse, signed long enc1rot)
{
    double retVal = 2.0 * M_PI;
    retVal *= static_cast<double>(pulse) / static_cast<double>(enc1rot);
    return retVal;
}
