/*
 * PositionController.cpp - 位置決め制御
 */

#include "PositionController.hpp"

#include "IOBoardFunc.hpp"
#include "MotorDriverFunc.hpp"

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <unistd.h>

#include <fbiad.h>   // GPG-3100 (A/D in, Digital in/out)
#include <fbida.h>   // GPG-3300 (D/A out)
#include <fbipenc.h> // GPG-6204 (Penc)

using namespace IOBoardFunc361316;

/* --- コンストラクタ --- */
PositionController::PositionController()
    :
    isInited(false),
    isRunning(false),
    isStopping(false),
    thre(),
    devNo(0),
    evaluateNum(0),
    motorRPM6V(1000),
    slowLimit(10000*4),
    encCount(0),
    moved(false),
    servoData()
{
}

/* --- デストラクタ --- */
PositionController::~PositionController()
{
    release();
}

/* --- 位置決めの初期化 --- */
int PositionController::init(int devNo_, int evNum, int sLimit, int rpm6V)
{
    stop();

    if (devNo_ < 1) {
        fprintf(stderr, "devNo error\n");
        return -1;
    }
    if (!isEvaluateNumValid(evNum)) {
        fprintf(stderr, "evaluateNum error\n");
        return -1;
    }
    if (rpm6V<200 || rpm6V>5000) {
        fprintf(stderr, "Motor RPM error\n");
        return -1;
    }
    if (slowLimit < 0) {
        fprintf(stderr, "Slow Limit error\n");
        return -1;
    }

    // メンバ変数の初期化
    devNo = devNo_;
    evaluateNum = evNum;
    motorRPM6V = rpm6V;
    slowLimit = sLimit;

    // D/Aの設定
    DASMPLCHREQ daSmplChReq;
    daSmplChReq.ulChNo  = 1;
    daSmplChReq.ulRange = DA_10V;
    if (DaSetOutputDAEx(devNo, 1, &daSmplChReq) != DA_ERROR_SUCCESS) {
        fprintf(stderr, "D/A - DaSetOutputDAEx() error\n");
        return -1;
    }
    unsigned short vZero = getVoltageDA_10V(0.0);
    DaOutputDAEx(devNo, &vZero);

    // Pencの設定
    int evFlag = static_cast<int>(log2(evaluateNum));
    if (PencSetMode(devNo, 1, 0x04+evFlag, 0, 0, 0) != PENC_ERROR_SUCCESS) {
        fprintf(stderr, "Penc - PencSetMode() error\n");
        return -1;
    }
    if (PencSetZMode(devNo, 1, 0x00) != PENC_ERROR_SUCCESS) {
        fprintf(stderr, "Penc - PencSetZMode(disable Zclear) error\n");
        return -1;
    }

    isInited = true; // 初期化完了
    fprintf(stderr, "init(devNo:%d,evNum:%d) OK.\n", devNo, evaluateNum);
    return 0;
}

/* --- 解放処理 --- */
int PositionController::release()
{
    stop();
    return 0;
}

void PositionController::setEncCount(signed long c) { encCount = c;    }
signed long PositionController::getEncCount()       { return encCount; }

/* --- エンコーダカウント値をZ相で初期化する --- */
int PositionController::encoderInit(bool useZeroClear)
{
    if (!isInited) {
        fprintf(stderr, "encoderInit() - init error\n");
        return -1;
    }

    if (useZeroClear) {
        PencSetCounter(devNo, 1, 0);
    }
    else {
        // Z相クリア有効
        if (PencSetZMode(devNo, 1, 0x01) != PENC_ERROR_SUCCESS) {
            fprintf(stderr, "Penc - PencSetZMode(enable Zclear) error\n");
            return -1;
        }

        // カウント値がクリアされるまでゆっくり回す
        unsigned long prevCount = 3000;
        PencSetCounter(devNo, 1, prevCount);
        unsigned short vZero = getVoltageDA_10V(0.0);
        unsigned short vOut  = getVoltageDA_10V(0.15);
        DaOutputDAEx(devNo, &vOut);
        usleep(static_cast<int>(100e3));

        // カウントが0に戻るまで待つ
        for (;;) {
            unsigned long count;
            PencGetCounter(devNo, 1, &count); setEncCount(count);
            if (count <= 0) break;
            //if (count < prevCount) break;
            //prevCount = count;
            usleep(100);
        }
        DaOutputDAEx(devNo, &vZero); // 停止
    }

    // Z相クリアを無効にする
    if (PencSetZMode(devNo, 1, 0x00) != PENC_ERROR_SUCCESS) {
        fprintf(stderr, "Penc - PencSetZMode(disable Zclear) error\n");
        return -1;
    }

    isEncoderInited = true; // エンコーダカウント値の初期化完了
    return 0;
}

/* --- 位置決め制御スレッドを起動する --- */
int PositionController::start(bool useZeroClear)
{
    // 初期化していない、スレッドがすでに動いているときは終了
    if (!isInited || isRunning || thre.get()!=NULL) return -1;

    if (!isEncoderInited) {
        if (encoderInit(useZeroClear) != 0) return -1;
    }

    thre = std::auto_ptr<boost::thread>(
        new boost::thread(boost::bind(&PositionController::run, this)));

    return 0;
}

/* --- 位置決め制御スレッドを停止する --- */
int PositionController::stop()
{
    // スレッドが動いていないときは終了
    if (!isRunning || thre.get()==NULL) return -1;

    isStopping = true;
    thre->join();
    isStopping = false;
    delete thre.release();

    return 0;
}

void PositionController::servoOn()  { servoData.setServoOn(true);  }
void PositionController::servoOff() { servoData.setServoOn(false); }
void PositionController::startMove() { servoData.setDoMove(true);   }
void PositionController::stopMove()  { servoData.setDoMove(false);  }
void PositionController::moveTo(signed long toCount) { moveTo(toCount, servoData.getMoveRPM()); }
void PositionController::setMoveRPM(int rpm) { moveTo(servoData.getMoveTo(), rpm); }
void PositionController::moveTo(signed long toCount, int rpm) {
    moved = false;
    servoData.setMoveTo(toCount); servoData.setMoveRPM(rpm);
}

/* --- 位置決め制御スレッド --- */
void PositionController::run()
{
    isRunning = true;
    fprintf(stderr, "PositionController start!!!\n");

    double vMul = 0.0;
    signed long prevCount = 0;
    while (!isStopping) {
        if (servoData.getServoOn()) {
            // 指定された角度まで回転する
            if (servoData.getDoMove()) {
                signed long moveTo  = servoData.getMoveTo();
                signed long moveRPM = servoData.getMoveRPM();

                if (moveTo != prevCount) {
                    prevCount = moveTo;
                    // 現在のエンコーダカウント値を取得する
                    signed long nowCount;
                    PencGetCounter(devNo, 1, reinterpret_cast<unsigned long*>(&nowCount));
                    setEncCount(nowCount);
                    // 回転の方向判定
                    signed long compVal = static_cast<unsigned long>(-1) >> 1;
                    int dir = (nowCount <= moveTo)*2 - 1;
                    if (dir*(moveTo/2 + nowCount/2) > compVal) dir*=-1;
                    // 指定位置まで回す
                    double v = MyMotorDriverFunc::rpm2voltage(moveRPM, motorRPM6V);
                    while (prevCount == servoData.getMoveTo()) {
                        vMul += 0.002;
                        PencGetCounter(devNo, 1, reinterpret_cast<unsigned long*>(&nowCount));
                        setEncCount(nowCount);
                        if (dir== 1 && nowCount>=moveTo) { vMul = 0.0; break; }
                        if (dir==-1 && nowCount<=moveTo) { vMul = 0.0; break; }
                        // 回転速度の調整
                        double vMin = MyMotorDriverFunc::rpm2voltage(5, motorRPM6V);
                        unsigned short vOut = getVoltageDA_10V(
                            dir * std::min(std::min(v, v*vMul),
                                           (v-vMin)*fabs(moveTo-nowCount)*2/slowLimit+vMin));
                        DaOutputDAEx(devNo, &vOut);
                        usleep(10);
                    }
                    if (prevCount == servoData.getMoveTo()) moved = true;
                    continue;
                }
            }

            { // 指定角度(prevCount)を維持する
                moved = true;
                signed long nowCount;
                PencGetCounter(devNo, 1, reinterpret_cast<unsigned long*>(&nowCount));
                setEncCount(nowCount);
                if (nowCount != prevCount) {
                    signed long compVal = static_cast<unsigned long>(-1) >> 1;
                    int dir = (nowCount <= prevCount)*2 - 1;
                    if (dir*(prevCount/2 + nowCount/2) > compVal) dir*=-1;
                    // 指定位置に到達するまで回す
                    double v = MyMotorDriverFunc::rpm2voltage(100, motorRPM6V);
                    while (servoData.getMoveTo()==prevCount && !isStopping) {
                        PencGetCounter(devNo, 1, reinterpret_cast<unsigned long*>(&nowCount));
                        setEncCount(nowCount);
                        if (dir== 1 && nowCount>=prevCount) break;
                        if (dir==-1 && nowCount<=prevCount) break;
                        // 回転速度の調整
                        double vMin = MyMotorDriverFunc::rpm2voltage(5, motorRPM6V);
                        unsigned short vOut = getVoltageDA_10V(
                            dir * std::min(v, (v-vMin)*fabs(prevCount-nowCount)*2/(2500*evaluateNum)+vMin));
                        DaOutputDAEx(devNo, &vOut);
                        usleep(10);
                    }
//                    DaOutputDAEx(devNo, &vZero); // 停止
                    continue;
                }
            }
        }
        usleep(10);
    }

    fprintf(stderr, "PositionController end!!!\n!");
    isRunning = false;
}

bool PositionController::isMoved() { return moved; }
