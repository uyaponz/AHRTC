#ifndef IGTAG_URG_SCANNER_HPP_F46E8DA6_8EAB_11E1_B50B_002318F915FD
#define IGTAG_URG_SCANNER_HPP_F46E8DA6_8EAB_11E1_B50B_002318F915FD

#include <string>
#include <vector>
#include <scip2awd.h>

namespace URGController
{

    /* ScanPoint: スキャン点格納用の構造体 */
    typedef struct { // 単位 : [mm]
        double dist, x, y;
    } ScanPoint_t;


    /* URGRules: URGのステップ数などの固有データ */
    class URGRules
    {
    public:
        enum ScannerType { // URGの種類
            NONE        = 0,
            TOP_URG     = 1,
            SIMPLE_URG  = 2,
            CLASSIC_URG = 4, // 非対応
            RAPID_URG   = 8  // 非対応
        };
        typedef struct ScannerInfo { // URGの固有データ格納用の構造体
            std::string name_; // スキャナ判別名(S2Ver_t::productに出てくる文字列)
            ScannerType type_; // URGの種類
            int    steps_;     // スキャンのステップ数
            int    beginStep_; // 開始ステップ
            int    endStep_;   // 終了ステップ
            double resol_;     // 1ステップの角度[deg]
            double beginDeg_;  // 開始角度[deg]
        } ScannerInfo_t;

    private:
        ScannerInfo_t info;

    public:
        std::string getName() const;
        ScannerType getType() const;
        int    getSteps()     const;
        int    getBeginStep() const;
        int    getEndStep()   const;
        double getResol()     const;
        double getBeginDeg()  const;

    public:
        void setName(std::string name);
        void setType(ScannerType type);
        void setSteps(int beginStep, int endStep);
        void setResol(double resol);
        void setBeginDeg(double beginDeg);

    public:
        URGRules();
    };

    /* URGScanner: URG制御用クラス */
    class URGScanner
    {
    private:
        URGRules     rule_;
        S2Sdd_t      buffer_;
        S2Port      *port_;
        S2Ver_t      apVer_;

    public:
        // スキャン点を取得してdstに格納する(失敗時: 0以下の戻り値)
        // (本当はconstメンバにしたいけど、バッファを隠蔽したいので。)
        int getScanPoints(std::vector<ScanPoint_t> &dst);
        // 使用中のURGの情報を返す
        URGRules getURGRules() const;
        // URGのバージョン情報を返す
        void getVersionInfo(S2Ver_t &info) const;

    private:
        URGScanner();
        URGScanner(const URGScanner&);
        URGScanner& operator=(const URGScanner&);

    public:
        explicit URGScanner(std::string deviceName = "/dev/ttyACM0");
        ~URGScanner();

        bool isOpened();
        operator int();
    };


    /* URGの種類別設定値 */
    const URGRules::ScannerInfo_t SCANNER_DEFAULTS[2] = {
        {"Top-URG",    URGRules::TOP_URG,    0,  0, 1080, 360.0/1440, -135.0},
        {"Simple-URG", URGRules::SIMPLE_URG, 0, 44,  725, 360.0/1024, -120.0}
    };
    /* URGRules一括設定関数 */
    void setURGRules(URGRules &r, const URGRules::ScannerInfo_t &si);

}

#endif
