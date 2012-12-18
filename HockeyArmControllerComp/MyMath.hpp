#ifndef MY_MATH_HPP_6DDEC90B_13FA_47BD_8D60_4C2682F1426D
#define MY_MATH_HPP_6DDEC90B_13FA_47BD_8D60_4C2682F1426D

#include <opencv2/opencv.hpp>
#include <deque>

//namespace mymath
//{
    typedef struct {
        double x, y;
        double time;
    } MyPos;
    MyPos myPos(double x, double y, double time=0.0);

    template <typename T>
    void addDeque(std::deque<T> &q, T data, int maxSize) {
        int size = q.size();
        if (size >= maxSize) q.pop_back();
        q.push_front(data);
    }

    double gettimeofday_sec();

    // 傾きを求める(inv=true : XとYを逆にする)
    MyPos calcSlope(std::deque<MyPos> &q, int cnt, bool inv=false);

    /* movable? */
    bool isArmMovable(CvPoint2D64f pos, double arm1Length, double arm2Length,
                      double left, double right, double bottom, double top);

    bool isArmMovable(CvPoint2D64f pos, double arm1Len, double arm2Len,
                      double hockeyW, double malletR, double armOffset);

    /* FK(for 2link) */
    CvPoint2D64f getFK(CvPoint2D64f rad, double arm1Length, double arm2Length);

    /* IK(for 2link) */
    CvPoint2D64f getIK(CvPoint2D64f pos,
                       double arm1Length, double arm2Length, bool isRightArm=true);

    /* radian -> pulse (enc1rot: encoder counts per 1 rot) */
    signed long rad2pulse(double rad, signed long enc1rot=10000);

    /* pulse -> radian */
    double pulse2rad(signed long pulse, signed long enc1rot=10000);
//}

#endif
