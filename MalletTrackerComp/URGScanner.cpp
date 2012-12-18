#include "URGScanner.hpp"

#include <cmath>
#include <iostream>
#include <cstring>
#include <limits>

using namespace std;

namespace URGController
{

    /* Scip2の開放処理 */
    static inline void releaseScip2(S2Port* &port, S2Sdd_t &buffer)
    {
        Scip2CMD_StopMS(port, &buffer);
        S2Sdd_Dest(&buffer);
        Scip2_Close(port); port=NULL;
        cerr << "URGScanner: Device closed." << endl;
    }

    /* バージョン情報からURGの種類を判別する */
    static inline size_t getURGType(const URGRules::ScannerInfo_t* const &def, const S2Ver_t &ver)
    {
        string product = ver.product;
        for (size_t i=0; i<sizeof(def); i++)
            if (string::npos != product.find(def[i].name_, 0))
                return i;
        return numeric_limits<size_t>::max();
    }

    /* コンストラクタ */
    URGScanner::URGScanner(string deviceName)
        :
        rule_(),
        buffer_(),
        port_(NULL),
        apVer_()
    {
        port_ = Scip2_Open(deviceName.c_str(), B0); // B115200 -> B0 に変えた

        int r = 0;
        if (NULL != port_) {
            Scip2CMD_RS(port_); // デバイスをリセット
            int rVV = Scip2CMD_VV(port_, &apVer_); // URGのバージョン情報を取得
            if (0 < rVV) {
                /* URGのバージョン情報を表示するする */
                cerr << "---------- Device Info ----------" << endl;
                cerr << "vender  : " << (apVer_.vender)     << endl;
                cerr << "product : " << (apVer_.product)    << endl;
                cerr << "firmware: " << (apVer_.firmware)   << endl;
                cerr << "protocol: " << (apVer_.protocol)   << endl;
                cerr << "serialno: " << (apVer_.serialno)   << endl;
                cerr << "---------------------------------" << endl;

                // URGの種類を判別し、必要であればビットレートを変更する
                size_t urgType = getURGType(SCANNER_DEFAULTS, apVer_);
                if (urgType != numeric_limits<size_t>::max())
                {
                    setURGRules(rule_, SCANNER_DEFAULTS[urgType]);

                    // Simple-URGの場合はビットレートを設定する
                    if (URGRules::SIMPLE_URG == rule_.getType()) {
                        Scip2CMD_SS(port_, B115200);
                    }

                    // スキャン開始
                    S2Sdd_Init(&buffer_);
                    r = Scip2CMD_StartMS(port_,
                                         rule_.getBeginStep(), rule_.getEndStep(),
                                         1, 0, 0, // Num of group, Culling clearance, Num of scan
                                         &buffer_,
                                         SCIP2_ENC_2BYTE);
                }
                else cerr << "URGScanner: Failed to solve URG type." << endl;
            }
            else cerr << "URGScanner: Failed to get version info." << endl;
        }

        /* ポートオープン失敗、またはスキャン開始に失敗した場合は終了する */
        if (NULL==port_ || !r) {
            cerr << "URGScanner: Failed to open device." << endl;

            // Scip2_Open() には成功している場合、ポートの開放をする
            if (NULL != port_) {
                Scip2CMD_RS(port_); // デバイスをリセット
                releaseScip2(port_, buffer_);
            }
            return;
        }

        cerr << "URGScanner: Device opened." << endl;
    }

    /* デストラクタ */
    URGScanner::~URGScanner()
    {
        releaseScip2(port_, buffer_);
    }

    /* 初期化完了判定 */
    bool URGScanner::isOpened() { return (NULL==port_) ? false : true; }
    URGScanner::operator int()  { return static_cast<int>(isOpened()); }


    /* スキャンした点を返す */
    int URGScanner::getScanPoints(vector<ScanPoint_t> &dst)
    {
        S2Scan_t *data = NULL;
        int sddRet = S2Sdd_Begin(&buffer_, &data);

        if (sddRet > 0) {
            size_t dstSize = rule_.getSteps(); // dstSize == data->size ・・・のはず。
            if (dst.size() < dstSize) dst.resize(dstSize);

            for (int i=0; i<(data->size); ++i) {
                double d = dst[i].dist = static_cast<double>(data->data[i]);
                double resol    = rule_.getResol();
                double beginDeg = rule_.getBeginDeg();
                double rad = (beginDeg + resol*i) * (M_PI/180.0);
                dst[i].x = d * cos(rad);
                dst[i].y = d * sin(rad);
            }

            S2Sdd_End(&buffer_);
        }

        return sddRet;
    }

    /* 使用中のURGの情報を返す */
    URGRules URGScanner::getURGRules() const
    {
        return rule_;
    }

    /* URGのバージョン情報を返す */
    void URGScanner::getVersionInfo(S2Ver_t &info) const
    {
        memcpy(&info, &apVer_, sizeof(S2Ver_t));
    }

}
