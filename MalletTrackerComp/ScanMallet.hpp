#ifndef IGTAG_SCAN_MALLET_HPP_EEBEF770_2330_48FC_B957_D845B5260D3C
#define IGTAG_SCAN_MALLET_HPP_EEBEF770_2330_48FC_B957_D845B5260D3C

#include <opencv2/opencv.hpp>

/* --- 描画関連 --- */
IplImage     *frame      = NULL; // 描画するフレーム
CvFont        font;
const int     winW       = 640;
const int     winH       = 800;
const char   *winName    = "ScanMallet";
const double  drawScale  = 3.5; // [m] から [pixel] への縮小率
const int     drawOS     = 70;  // 左上の描画オフセット[pixel]
const int     drawOSX    = drawOS;
const int     drawOSY    = drawOS;

/* --- 各種定数 --- */
// マレットの直径
const double malletR    = 100.0;
// ホッケー台
const double hkyW       =  730.0 * 2.0; // 704.0 * 2.0;
const double hkyH       = 2285.0;       //1145.0 * 2.0;
// URGの設置位置(台の中心から見て)
const double urgOSX     = -hkyW/2.0 - 20.0;
const double urgOSY     = 0.0;
// URGで必要ない範囲
const double invalidScanX = 25.0;
const double invalidScanY = 50.0;
// スキャン点の有効範囲(malletR/2.0に加算する値)
const double validSPMalletOffset = 30.0;
// パーティクル(ぽいの)の数
const int particleN = 200;
// 分散(sigma2)
const double ptclSigma2 = 30.0; // 50.0;

/* --- URG固有 --- */
// URGの設置角度(推定)
const double urgRot     = 0.0 * M_PI / 180.0; // [rad], 反時計: 正方向
// URGで検出した点の拡縮
const double urgZX      = 1.055;
const double urgZY      = 1.028;
// 補正板の位置(台の外側に何mm出ているか)
const double revXOSX    = 15.0; // X方向反射板
const double revYOSY    = 0.0;  // Y方向反射板

#endif
