/*
 * PositionController.hpp - 位置決め制御
 */

#ifndef POSITIONCONTROLLER_HPP
#define POSITIONCONTROLLER_HPP

#include <boost/thread.hpp>
#include <pthread.h>
#include <memory>

#define MY_MUTEX_RETURN(type,name,mtx) \
    { pthread_mutex_lock(&mtx); type ret=name; pthread_mutex_unlock(&mtx); return ret; }
#define MY_MUTEX_SETVAL(from,to,mtx) \
    { pthread_mutex_lock(&mtx); to=from; pthread_mutex_unlock(&mtx); }

namespace PositionControllerData
{
    /* ServoData - 位置制御時に使用するデータ */
    class ServoData {
    private:
        bool servoOn;    // ドライバ制御するか
        bool doMove; // 移動指令
        signed long moveTo; // 移動先エンコーダカウント値
        int moveRPM; // 回転速度
    private:
        pthread_mutex_t mutex; // 排他制御用

    public: // accessors
        bool getServoOn()       { MY_MUTEX_RETURN(bool, servoOn, mutex); }
        void setServoOn(bool v) { MY_MUTEX_SETVAL(v, servoOn, mutex);    }

        bool getDoMove()       { MY_MUTEX_RETURN(bool, doMove, mutex); }
        void setDoMove(bool v) { MY_MUTEX_SETVAL(v, doMove, mutex);    }

        signed long getMoveTo()       { MY_MUTEX_RETURN(signed long, moveTo, mutex); }
        void setMoveTo(signed long v) { MY_MUTEX_SETVAL(v, moveTo, mutex);           }

        int getMoveRPM()       { MY_MUTEX_RETURN(int, moveRPM, mutex); }
        void setMoveRPM(int v) { MY_MUTEX_SETVAL(v, moveRPM, mutex);   }

    public:
        ServoData()
            :
            servoOn(false), doMove(false), moveTo(0), moveRPM(0)
        {
            pthread_mutex_init(&mutex, NULL);
        }
        ~ServoData() { pthread_mutex_destroy(&mutex); }
    };
}

/* PositionController - 位置決め制御クラス */
class PositionController
{
private:
    bool isInited;   // 初期化が完了したか
    bool isEncoderInited; // エンコーダカウント値の初期化が完了しているか
    bool isRunning;  // 位置決めスレッドが動いているか
    bool isStopping; // 位置決めスレッドの停止要求が出ているか
    std::auto_ptr<boost::thread> thre; // スレッドオブジェクト
private:
    int devNo;       // 操作するデバイス番号
    int evaluateNum; // 逓倍数
    int motorRPM6V;  // 6V印加時のモーター回転数
    int slowLimit;   //
private:
    signed long encCount;
    bool moved;
private:
    PositionControllerData::ServoData servoData; // サーボ動作用データ構造

/* スレッド */
private:
    void run(); // positioning method
    void setEncCount(signed long c);
public:
    int start(bool useZeroClear=false); // 位置決め制御スタート
    int stop();  // 位置決め制御ストップ

/* 初期化、解放 */
public:
    int init(int devNo_, int evNum, int slowLimit, int rpm6V=1000); // 初期化
    int release(); // 解放
    int encoderInit(bool useZeroClear=false); // エンコーダカウント値をZ相で初期化する

public:
    void servoOn();
    void servoOff();
    void startMove();
    void stopMove();
    void moveTo(signed long toCount, int rpm);
    void moveTo(signed long toCount);
    void setMoveRPM(int rpm);
    signed long getEncCount();
    bool isMoved();

public:
    PositionController();
    ~PositionController(); // uninheritable
private: // uncopyable
    PositionController(const PositionController &);
    PositionController& operator=(const PositionController &);
};

#endif

