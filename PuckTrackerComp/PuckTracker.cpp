// -*- C++ -*-
/*!
 * @file  PuckTracker.cpp
 * @brief ${rtcParam.description}
 * @date $Date$
 *
 * $Id$
 */

#include "PuckTracker.h"

//#define NO_IMAGE

int             ret;
int             DeviceNum;
unsigned long   MemHandle;
void*           SrcPtr;
void*           DataPtr;
IFCMLCAPFMT     CapFmt;
IplImage* src_img = 0;
IplImage* rgb_img = 0;

IplImage* srcrgb_img = NULL;

// just for test
IplImage* undist_img = 0;
IplImage* homo_img = 0;
IplImage* mapx = 0;
IplImage* mapy = 0;


float bbb =54.0;
float ggg =84.0;
float rrr =201.0;

//float hhh=12.244898;
//float sss =0.731343;
//float vvv =0.788235;
float hhh=0.0;
float sss=90.0;
float vvv=0.0;
float likehood_limit;

int ch;
int count;
int n_stat = 4;
int n_particle = N_PARTICLE;
float max_likehood;
CvConDensation *cond = 0;
CvMat *lowerBound = 0;
CvMat *upperBound = 0;

CvFileStorage *fs;
CvFileNode *param;
CvMat *intrinsic, *distortion;
CvMat *rotation;
CvMat *translation;
CvMat *homo, *inv_homo;


double gettimeofday_sec() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec+tv.tv_usec*1e-6;
}

void calc_hsv(float r, float g, float b, float* h, float* s, float* v) {
    float max, min;
    float tmp;
    int   type = 0;

    max = r;
    if (g > max) { type = 1; max = g; }
    if (b > max) { type = 2; max = b; }

    if (max <= 0.0) return;
    *v = max; /* V */

    min = r;
    if(g < min) min = g;
    if(b < min) min = b;

    tmp = max - min;
    if (tmp <= 0.0) return;
    *s  = 255.0 * (tmp / max); /* S */

    *h = 60.0;
    if (type == 0) { /* max:R */
        (*h) *= (b - g) / tmp;
    } else if (type == 1) { /* max:G */
        (*h) *= 2.0 + ((r-b) / tmp);
    } else { /* max:B */
        (*h) *= 4.0 + ((g-r) / tmp);
    }

    if (*h < 0) *h += 360.0;
}

// (1)尤度の計算を行なう関数
float
calc_likehood_s(float s) {
    if(s < S_MIN) return 0.0;
    return 1.0;
}
float
calc_likehood_v(float v) {
    if(v < V_MIN) return 0.0;
    return 1.0;
}

float
calc_likelihood (IplImage * img, int x, int y, float bb, float gg, float rr)
{
    float b, g, r;
    float h, s, v;
    float dist = 0.0;
    float rslt;

    b = (unsigned char)img->imageData[img->widthStep * y + x * 3];       //B
    g = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 1];   //G
    r = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 2];   //R
    calc_hsv(r,g,b,&h,&s,&v);
    b -=bb;
    g -=gg;
    r -=rr;
    dist = sqrt( b * b + g * g + r * r);
    rslt= 1.0 / (sqrt (2.0 * CV_PI) * SIGMA) * expf (-dist * dist / (2.0 * SIGMA * SIGMA));
    rslt = rslt * calc_likehood_s(s) * calc_likehood_v(v);
    return rslt;
}

float
calc_likelihood_hsv (IplImage * img, int x, int y, float hh, float ss, float vv)
{
    float b, g, r;
    float h, s, v;
//  float dist = 0.0;
//  float rslt;

    b = (unsigned char)img->imageData[img->widthStep * y + x * 3];       //B
    g = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 1];   //G
    r = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 2];   //R
    calc_hsv(r,g,b,&h,&s,&v);

    return (float)( (340.0<=h && h<=360.0) * (float)(s>=ss) * (float)(v>=vv));

//  h -= hh;
//  if(h > 180.0) {
//    h -= 360.0;
//  } else if(h < -180.0) {
//    h += 360.0;
//  }
//
//  dist = sqrt( h * h);
//  rslt = 1.0 / (sqrt (2.0 * CV_PI) * SIGMA) * expf (-dist * dist / (2.0 * SIGMA * SIGMA));
//  if (s < ss || v < vv) return 0.0;
//  return rslt;
}

void my_mouse_callback(int event, int x, int y, int flags, void* param);
void my_mouse_callback_hsv(int event, int x, int y, int flags, void* param);

//void RETPRINT(long ret){
void RETPRINT(unsigned int ret){

    switch(ret){
//    case IFCML_ERROR_SUCCESS:
//        printf("--ret : IFCML_ERROR_SUCCESS [%#x]\n",ret);
//        break;
    case IFCML_ERROR_NOT_DEVICE:
        printf("--ret : IFCML_ERROR_NOT_DEVICE [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOT_OPEN:
        printf("--ret : IFCML_ERROR_NOT_OPEN [%#x]\n",ret);
        break;
    case IFCML_ERROR_INVALID_DEVICE_NUMBER:
        printf("--ret : IFCML_ERROR_INVALID_DEVICE_NUMBER [%#x]\n",ret);
        break;
    case IFCML_ERROR_ALREADY_OPEN:
        printf("--ret : IFCML_ERROR_ALREADY_OPEN [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOT_SUPPORTED:
        printf("--ret : IFCML_ERROR_NOT_SUPPORTED [%#x]\n",ret);
        break;
    case IFCML_ERROR_INVALID_PARAMETER:
        printf("--ret : IFCML_ERROR_INVALID_PARAMETER [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOT_ALLOCATE_MEMORY:
        printf("--ret : IFCML_ERROR_NOT_ALLOCATE_MEMORY [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOW_CAPTURING:
        printf("--ret : IFCML_ERROR_NOW_CAPTURING [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOW_STOP:
        printf("--ret : IFCML_ERROR_NOW_STOP [%#x]\n",ret);
        break;
    case IFCML_ERROR_MEM_NOW_CAPTURING:
        printf("--ret : IFCML_ERROR_MEM_NOW_CAPTURING [%#x]\n",ret);
        break;
    case IFCML_ERROR_MEM_NOW_STOP:
        printf("--ret : IFCML_ERROR_MEM_NOW_STOP [%#x]\n",ret);
        break;
    case IFCML_ERROR_NULL_POINTER:
        printf("--ret : IFCML_ERROR_NULL_POINTER [%#x]\n",ret);
        break;
    case IFCML_ERROR_FIFO_FULL:
        printf("--ret : IFCML_ERROR_FIFO_FULL [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOBUFFER:
        printf("--ret : IFCML_ERROR_NOBUFFER [%#x]\n",ret);
        break;
    case IFCML_ERROR_INVALID_BUF_HANDLE:
        printf("--ret : IFCML_ERROR_INVALID_BUF_HANDLE [%#x]\n",ret);
        break;
    case IFCML_ERROR_SERIAL_TIMEOUT:
        printf("--ret : IFCML_ERROR_SERIAL_TIMEOUT [%#x]\n",ret);
        break;
    case IFCML_ERROR_INVALID_FILE_PRAM:
        printf("--ret : IFCML_ERROR_INVALID_FILE_PRAM [%#x]\n",ret);
        break;
    case IFCML_ERROR_NOTSET_CAMFILE:
        printf("--ret : IFCML_ERROR_NOTSET_CAMFILE [%#x]\n",ret);
        break;
    default:
        printf("--ret : ANOTHER_ERROR [%#x]\n",ret);
        break;
    }
}

void Init()
{
    DeviceNum = 1;
    MemHandle = -1;
    SrcPtr = NULL;
    DataPtr = NULL;

    cvNamedWindow( "Display", CV_WINDOW_AUTOSIZE);
    src_img = cvCreateImage(cvSize(ImgWidth, ImgHeight), IPL_DEPTH_8U, 1);
    rgb_img = cvCreateImage(cvSize(ImgWidth, ImgHeight), IPL_DEPTH_8U, 3);
    srcrgb_img = cvCreateImage(cvSize(ImgWidth, ImgHeight), IPL_DEPTH_8U, 3);

    DataPtr = src_img->imageData;

    //cvSetMouseCallback("Display", my_mouse_callback, (void*) rgb_img);
    // cvSetMouseCallback("Display", my_mouse_callback_hsv, (void*) rgb_img);
    cvSetMouseCallback("Display", my_mouse_callback_hsv, (void*) srcrgb_img);
    likehood_limit = 1.0 / (sqrt (2.0 * CV_PI) * SIGMA) * expf (-2.0);
    //likehood_limit = 1.0 / (sqrt (2.0 * CV_PI) * SIGMA) * expf (-0.5);
    //likehood_limit = 1.0 / (sqrt (2.0 * CV_PI) * SIGMA) * expf (-(15.0*15.0) / (2.0*SIGMA*SIGMA));

    printf("likehood_limit=%f\n",likehood_limit);
}

void Exit()
{
    if(DeviceNum != -1){
        CmlClose(DeviceNum);
    }
    if(SrcPtr != NULL) free(SrcPtr);
    if(DataPtr != NULL) free(DataPtr);
}

void my_mouse_callback(int event, int x, int y, int flags, void* param){
    return;
    IplImage* img = (IplImage*)param;
    switch(event) {
    case CV_EVENT_LBUTTONDOWN: {
        bbb = (unsigned char)img->imageData[img->widthStep * y + x * 3];       //B
        ggg = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 1];   //G
        rrr = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 2];   //R
        printf("uc_bbb=%d, uc_ggg=%d, uc_rrr=%d\n",
               (unsigned char) img->imageData[img->widthStep * y + x * 3],
               (unsigned char) img->imageData[img->widthStep * y + x * 3 + 1],
               (unsigned char) img->imageData[img->widthStep * y + x * 3 + 2]);
        printf("b=%f, g=%f, r=%f\n", bbb,ggg,rrr);
    }
        break;
    }
}

void my_mouse_callback_hsv(int event, int x, int y, int flags, void* param){
    return;
    IplImage* img = (IplImage*)param;
    switch(event) {
    case CV_EVENT_LBUTTONDOWN: {
        bbb = (unsigned char)img->imageData[img->widthStep * y + x * 3];       //B
        ggg = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 1];   //G
        rrr = (unsigned char)img->imageData[img->widthStep * y + x * 3 + 2];   //R
        printf("uc_bbb=%d, uc_ggg=%d, uc_rrr=%d\n",
               (unsigned char) img->imageData[img->widthStep * y + x * 3],
               (unsigned char) img->imageData[img->widthStep * y + x * 3 + 1],
               (unsigned char) img->imageData[img->widthStep * y + x * 3 + 2]);
        printf("b=%f, g=%f, r=%f\n", bbb,ggg,rrr);
        calc_hsv(rrr,ggg,bbb,&hhh,&sss,&vvv);
        printf("h=%f, s=%f, v=%f\n", hhh,sss,vvv);
    }
        break;
    }
}

void init_cond(int p_noise, int v_noise) {
    cvConDensInitSampleSet (cond, lowerBound, upperBound);
    cvRandInit (&(cond->RandS[0]), -p_noise, p_noise, (int) cvGetTickCount());
    cvRandInit (&(cond->RandS[1]), -p_noise, p_noise, (int) cvGetTickCount());
    cvRandInit (&(cond->RandS[2]), -v_noise, v_noise, (int) cvGetTickCount());
    cvRandInit (&(cond->RandS[3]), -v_noise, v_noise, (int) cvGetTickCount());
}

void print_fmat3x3(CvMat* mat) {
    std::cout <<  *(float*)mat->data.ptr << " ";
    std::cout <<  *((float*)(mat->data.ptr)+1) << " ";
    std::cout <<  *((float*)(mat->data.ptr)+2) << std::endl;
    std::cout <<  *((float*)(mat->data.ptr)+3) << " ";
    std::cout <<  *((float*)(mat->data.ptr)+4) << " ";
    std::cout <<  *((float*)(mat->data.ptr)+5) << std::endl;
    std::cout <<  *((float*)(mat->data.ptr)+6) << " ";
    std::cout <<  *((float*)(mat->data.ptr)+7) << " ";
    std::cout <<  *((float*)(mat->data.ptr)+8) << std::endl;
}

void trans_homo(const CvMat* homo, const float* src, float* dst) {
    float h00,h01,h02,h10,h11,h12,h20,h21,h22;
    float w;

    h00=*(((float*)homo->data.ptr)+0);
    h01=*(((float*)homo->data.ptr)+1);
    h02=*(((float*)homo->data.ptr)+2);
    h10=*(((float*)homo->data.ptr)+3);
    h11=*(((float*)homo->data.ptr)+4);
    h12=*(((float*)homo->data.ptr)+5);
    h20=*(((float*)homo->data.ptr)+6);
    h21=*(((float*)homo->data.ptr)+7);
    h22=*(((float*)homo->data.ptr)+8);
    dst[0]=h00*src[0]+h01*src[1]+h02;
    dst[1]=h10*src[0]+h11*src[1]+h12;
    w=h20*src[0]+h21*src[1]+h22;
    dst[0] /= w;
    dst[1] /= w;
}


void undist(const float* uv, float* u_uv, float *u_xy) {
    float dist_uv[2],dist_xy[2],duv[2];
    float intr_ax,intr_ay,intr_cx,intr_cy,dd;
    float x,y,rr;
    float k1, k2, p1, p2;

    // 歪みパラメータ
    k1 = *((float *)(distortion->data.ptr) + 0);
    k2 = *((float *)(distortion->data.ptr) + 1);
    p1 = *((float *)(distortion->data.ptr) + 2);
    p2 = *((float *)(distortion->data.ptr) + 3);

    // カメラ行列
    intr_ax=*(((float*)intrinsic->data.ptr)+0);
    intr_cx=*(((float*)intrinsic->data.ptr)+2);
    intr_ay=*(((float*)intrinsic->data.ptr)+3+1);
    intr_cy=*(((float*)intrinsic->data.ptr)+3+2);

    u_uv[0]=uv[0];
    u_uv[1]=uv[1];
    while(1) {
        //    undist1(uv, u_uv, duv);
        u_xy[0]=(u_uv[0]-intr_cx)/intr_ax;
        u_xy[1]=(u_uv[1]-intr_cy)/intr_ay;
        //    dist(u_xy,dist_xy);
        x=u_xy[0];
        y=u_xy[1];
        rr=x*x+y*y;
        dist_xy[0]=x*(1+k1*rr+k2*rr*rr)+2*p1*x*y+p2*(rr+2*x*x);
        dist_xy[1]=y*(1+k1*rr+k2*rr*rr)+p1*(rr+2*y*y)+2*p2*x*y;
        //
        dist_uv[0]=intr_ax*dist_xy[0]+intr_cx;
        dist_uv[1]=intr_ay*dist_xy[1]+intr_cy;
        duv[0]=dist_uv[0]-uv[0];
        duv[1]=dist_uv[1]-uv[1];
        dd=sqrt(duv[0]*duv[0]+duv[1]*duv[1]);
        if (dd < 0.01) break;
        u_uv[0] -= duv[0];
        u_uv[1] -= duv[1];
    }
}

float uv[2],u_uv[2],u_xy[2],t_xy[2];
coil::TimeValue t_tm(coil::gettimeofday());
int n_match;
void CallBackDisplay(unsigned long IntFlg, unsigned long User)
{
    int i;

    float match;
    float a_x,a_y,a_vx,a_vy;
    int xx, yy;
    float uv[2],u_uv[2],u_xy[2],t_xy[2];

    t_tm = coil::gettimeofday();

    /*
    // Get CaptureData and OptionData;
    ret = CmlGetMemData(DeviceNum,MemHandle,DataPtr,FRAMECNT,&CapFmt.Rect,DataKind);
    if(ret != IFCML_ERROR_SUCCESS){
    RETPRINT(ret);
    Exit();
    return;
    }
    */
    count++;
    cvCvtColor(src_img,rgb_img,CV_BayerBG2BGR);
    cvCvtColor(src_img,srcrgb_img,CV_BayerBG2BGR);

    // (9)各パーティクルについて尤度を計算する．
    max_likehood=0.0;
    a_x=0.0;
    a_y=0.0;
    a_vx=0.0;
    a_vy=0.0;
    n_match=0;
    match=0.0;
    cvRectangle(rgb_img, cvPoint(0,0), cvPoint(30,30),
                CV_RGB(rrr,ggg,bbb), CV_FILLED);
    for (i = 0; i < n_particle; i++) {
        xx = (int) (cond->flSamples[i][0]);
        yy = (int) (cond->flSamples[i][1]);
        if (xx < 0 || xx >= ImgWidth || yy < 0 || yy >= ImgHeight) {
            cond->flConfidence[i] = 0.0;
        }
        else {
            //cond->flConfidence[i] = calc_likelihood (srcrgb_img, xx, yy, bbb,ggg,rrr);
            cond->flConfidence[i] = calc_likelihood_hsv (srcrgb_img, xx, yy, hhh,sss,vvv);
            if(cond->flConfidence[i]>likehood_limit) {
                n_match++;
                match += cond->flConfidence[i];
                a_x += cond->flSamples[i][0]*cond->flConfidence[i];
                a_y += cond->flSamples[i][1]*cond->flConfidence[i];
                a_vx += cond->flSamples[i][2]*cond->flConfidence[i];
                a_vy += cond->flSamples[i][3]*cond->flConfidence[i];
                cvCircle (rgb_img, cvPoint(xx, yy), 2, CV_RGB (0,255, 255), -1,8);
            } else {
                cvCircle (rgb_img, cvPoint(xx, yy), 2, CV_RGB (0, 0, 255), -1,8);
            }
            if(cond->flConfidence[i] > max_likehood) max_likehood = cond->flConfidence[i];
        }
    }
    if(n_match > n_particle/40) {
        a_x /= match;
        a_y /= match;
        a_vx /= match;
        a_vy /= match;
        cvCircle (rgb_img, cvPoint ((int)a_x, (int)a_y), 5, CV_RGB (255, 255, 255), -1,8);
        //cvCircle (rgb_img, cvPoint (a_x+a_vx*5, a_y+a_vy*5), 5, CV_RGB (255, 255, 255), -1,8);
    } else if(max_likehood < likehood_limit) {
        n_match = -1;
        a_x = 0.0;
        a_y = 0.0;
        a_vx = 0.0;
        a_vy = 0.0;
        init_cond(P_NOISE, V_NOISE);
        for (i = 0; i < n_particle; i++) {
            xx = (int) (cond->flSamples[i][0]);
            yy = (int) (cond->flSamples[i][1]);
            if (xx < 0 || xx >= ImgWidth || yy < 0 || yy >= ImgHeight) {
                cond->flConfidence[i] = 0.0;
            } else {
                cond->flConfidence[i] = 1.0;
            }
        }
    } else {
        a_x /= match;
        a_y /= match;
        a_vx /= match;
        a_vy /= match;
    }


    // (10)次のモデルの状態を推定する
    cvConDensUpdateByTime (cond);
    uv[0]=a_x;
    uv[1]=a_y;
    undist(uv,u_uv,u_xy);
    trans_homo(inv_homo,u_uv,t_xy);

//  std::cout << "n: " << n_match << std::endl;
//  std::cout << "uv: " << uv[0] << " " << uv[1] << std::endl;
//  std::cout << "u_xy: " << u_xy[0] << " " << u_xy[1] << std::endl;
//    std::cout << "t_xy: " << t_xy[0] << " " << t_xy[1] << std::endl;

    ::t_xy[0] = t_xy[0];
    ::t_xy[1] = t_xy[1];

    //    printf("0\n");

    static int framecnt = 0;
#ifndef NO_IMAGE
    if (framecnt++ >= 3) {
        framecnt = 0;
        cvShowImage( "Display", rgb_img );
        ch = cvWaitKey(2);
    }
#else
    if (!framecnt) {
        framecnt = 1;
        cvDestroyWindow("Display");
        cvNamedWindow("SubDisplay", CV_WINDOW_AUTOSIZE);
        cvResizeWindow("SubDisplay", 300,300);
        cvShowImage("SubDisplay", NULL);
        ch = cvWaitKey(2);
    }
#endif
}



int i,j;
int type;
//    int yn;
float tmp_x, tmp_y;

unsigned long  DataKind;
unsigned long  AllocBufSize;
unsigned long  StartMode;

double start,end;
CvMat *tmp_mat = cvCreateMat(3,3, CV_32FC1);


// Module specification
// <rtc-template block="module_spec">
static const char* pucktracker_spec[] =
{
    "implementation_id", "PuckTracker",
    "type_name",         "PuckTracker",
    "description",       "${rtcParam.description}",
    "version",           "1.0.0",
    "vendor",            "ysatoh",
    "category",          "Category",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.calibration_data", "./camera.xml",
    "conf.default.n_particle", "400",
    "conf.default.p_noise", "30",
    "conf.default.v_noise", "5",
    "conf.default.sigma", "50.0",
    "conf.default.minH", "340.0",
    "conf.default.maxH", "360.0",
    "conf.default.minS", "90.0",
    "conf.default.maxS", "255.0",
    "conf.default.minV", "0.0",
    "conf.default.maxV", "255.0",
    "conf.default.minR", "0.0",
    "conf.default.maxR", "255.0",
    "conf.default.minG", "0.0",
    "conf.default.maxG", "255.0",
    "conf.default.minB", "0.0",
    "conf.default.maxB", "255.0",
    "conf.default.HSVorRGB", "HSV",
    // Widget
    "conf.__widget__.calibration_data", "text",
    "conf.__widget__.n_particle", "text",
    "conf.__widget__.p_noise", "text",
    "conf.__widget__.v_noise", "text",
    "conf.__widget__.sigma", "text",
    "conf.__widget__.minH", "text",
    "conf.__widget__.maxH", "text",
    "conf.__widget__.minS", "text",
    "conf.__widget__.maxS", "text",
    "conf.__widget__.minV", "text",
    "conf.__widget__.maxV", "text",
    "conf.__widget__.minR", "text",
    "conf.__widget__.maxR", "text",
    "conf.__widget__.minG", "text",
    "conf.__widget__.maxG", "text",
    "conf.__widget__.minB", "text",
    "conf.__widget__.maxB", "text",
    "conf.__widget__.HSVorRGB", "text",
    // Constraints
    ""
};
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
PuckTracker::PuckTracker(RTC::Manager* manager)
    // <rtc-template block="initializer">
    : RTC::DataFlowComponentBase(manager),
      m_puckXY_outOut("PuckXY_out", m_puckXY_out),
      m_AHServicePort("AHService")

      // </rtc-template>
{
}

/*!
 * @brief destructor
 */
PuckTracker::~PuckTracker()
{
}



RTC::ReturnCode_t PuckTracker::onInitialize()
{
    // Registration: InPort/OutPort/Service
    // <rtc-template block="registration">
    // Set InPort buffers

    // Set OutPort buffer
    addOutPort("PuckXY_out", m_puckXY_outOut);

    // Set service provider to Ports

    // Set service consumers to Ports
    m_AHServicePort.registerConsumer("AHCommonDataService", "AHCommon::AHCommonDataService", m_ahCommonDataService);

    // Set CORBA Service Ports
    addPort(m_AHServicePort);

    // </rtc-template>

    // <rtc-template block="bind_config">
    // Bind variables and configuration variable
    bindParameter("calibration_data", m_calibration_data, "./camera.xml");
    bindParameter("n_particle", m_n_particle, "400");
    bindParameter("p_noise", m_p_noise, "30");
    bindParameter("v_noise", m_v_noise, "5");
    bindParameter("sigma", m_sigma, "50.0");
    bindParameter("minH", m_minH, "340.0");
    bindParameter("maxH", m_maxH, "360.0");
    bindParameter("minS", m_minS, "90.0");
    bindParameter("maxS", m_maxS, "255.0");
    bindParameter("minV", m_minV, "0.0");
    bindParameter("maxV", m_maxV, "255.0");
    bindParameter("minR", m_minR, "0.0");
    bindParameter("maxR", m_maxR, "255.0");
    bindParameter("minG", m_minG, "0.0");
    bindParameter("maxG", m_maxG, "255.0");
    bindParameter("minB", m_minB, "0.0");
    bindParameter("maxB", m_maxB, "255.0");
    bindParameter("HSVorRGB", m_HSVorRGB, "HSV");

//    m_puckXY_out.data.length(3); //n, x, y
    m_puckXY_out.data.length(2); //x, y

    // </rtc-template>

    Init();

    // load camera params
    homo=cvCreateMat(3,3, CV_32FC1);
    inv_homo=cvCreateMat(3,3, CV_32FC1);

    fs = cvOpenFileStorage ("camera.xml", 0, CV_STORAGE_READ);
    param = cvGetFileNodeByName (fs, NULL, "intrinsic");
    intrinsic = (CvMat *) cvRead (fs, param);
    param = cvGetFileNodeByName (fs, NULL, "distortion");
    distortion = (CvMat *) cvRead (fs, param);
    param = cvGetFileNodeByName (fs, NULL, "translation");
    translation = (CvMat *) cvRead (fs, param);
    param = cvGetFileNodeByName (fs, NULL, "rotation");
    rotation = (CvMat *) cvRead (fs, param);
    cvReleaseFileStorage (&fs);

    cvRodrigues2(rotation,tmp_mat);

    print_fmat3x3(tmp_mat);

    *((float *)(tmp_mat->data.ptr) + 2)=*((float*)(translation->data.ptr) + 0);
    *((float *)(tmp_mat->data.ptr) + 5)=*((float*)(translation->data.ptr) + 1);
    *((float *)(tmp_mat->data.ptr) + 8)=*((float*)(translation->data.ptr) + 2);

    print_fmat3x3(tmp_mat);

    cvGEMM(intrinsic, tmp_mat, 1.0, 0, 0.0, homo, 0);

    print_fmat3x3(tmp_mat);

    cvInvert(homo, inv_homo);
    print_fmat3x3(inv_homo);

    // check inv
    // cvGEMM(homo, inv_homo, 1.0, 0, 0.0, tmp_mat, 0);
    // print_fmat3x3(tmp_mat);

    // (4)Condensation構造体を作成する．
    cond = cvCreateConDensation (n_stat, 0, n_particle);

    // (5)状態ベクトル各次元の取りうる最小値・最大値を指定する．
    lowerBound = cvCreateMat (4, 1, CV_32FC1);
    upperBound = cvCreateMat (4, 1, CV_32FC1);

    cvmSet (lowerBound, 0, 0, 0.0);
    cvmSet (lowerBound, 1, 0, 0.0);
    cvmSet (lowerBound, 2, 0, -20.0);
    cvmSet (lowerBound, 3, 0, -20.0);
    cvmSet (upperBound, 0, 0, ImgWidth);
    cvmSet (upperBound, 1, 0, ImgHeight);
    cvmSet (upperBound, 2, 0, 20.0);
    cvmSet (upperBound, 3, 0, 20.0);

    init_cond(P_NOISE, V_NOISE);

    // (7)ConDensationアルゴリズムにおける状態ベクトルのダイナミクスを指定する
    cond->DynamMatr[0] = 1.0;
    cond->DynamMatr[1] = 0.0;
    cond->DynamMatr[2] = 1.0;
    cond->DynamMatr[3] = 0.0;
    cond->DynamMatr[4] = 0.0;
    cond->DynamMatr[5] = 1.0;
    cond->DynamMatr[6] = 0.0;
    cond->DynamMatr[7] = 1.0;
    cond->DynamMatr[8] = 0.0;
    cond->DynamMatr[9] = 0.0;
    cond->DynamMatr[10] = 1.0;
    cond->DynamMatr[11] = 0.0;
    cond->DynamMatr[12] = 0.0;
    cond->DynamMatr[13] = 0.0;
    cond->DynamMatr[14] = 0.0;
    cond->DynamMatr[15] = 1.0;

    //void CALLBACK CallBackDisplay(unsigned long IntFlg, unsigned long User) {

    ret = CmlOpen(DeviceNum);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        return RTC::RTC_ERROR;
    }

    // Open Camera ConfigFile
    // Teli  CS6910CL
    // RGB24 640×480
    ret = CmlReadCamConfFile(DeviceNum,"./mv_d640.cfg");
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        printf("aho1\n");
        Exit();
        return RTC::RTC_ERROR;
    }

    CapFmt.Rect.XStart = 0;
    CapFmt.Rect.YStart = 0;
    CapFmt.Rect.XLength = ImgWidth;
    CapFmt.Rect.YLength = ImgHeight;
    CapFmt.Scale.PixelCnt = 0;
    CapFmt.Scale.LineCnt = 0;

    CapFmt.CapFormat = IFCML_CAPFMT_CAM;

    //    if(CapFmt.CapFormat) DataKind |= 0x01;
    if(CapFmt.CapFormat) DataKind = 0x01;

    CapFmt.OptionFormat = IFCML_OPTFMT_NON;
    ret = CmlSetCaptureFormatInfo(DeviceNum,&CapFmt);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        printf("aho2\n");
        Exit();
        return RTC::RTC_ERROR;
    }

    // Allocate the buffer for storing the image data.
    // Size of 1 farame
//    printf("MemType: \n");
//    printf(" 0 - main memory \n");
//    printf(" 1 - board memory \n");
//    scanf("%d", &type);
//    printf("mem size %ld,  no. of line %ld\n", CapFmt.FrameSize_Mem, CapFmt.FrameSize_Mem/ImgWidth);
    type = 0;
    if(type){
        AllocBufSize = FRAMECNT * CapFmt.FrameSize_Mem;
        SrcPtr = NULL;
        StartMode = IFCML_CAM_MEM;
    }else{
        AllocBufSize = FRAMECNT * CapFmt.FrameSize_Buf;
        SrcPtr = _aligned_malloc(AllocBufSize, 2048*2);
        //SrcPtr = malloc(AllocBufSize * 2);
        //printf("SrcPtr0=%x\n",SrcPtr);
        //SrcPtr = ((int)SrcPtr/2048 + 1) * 2048;
        printf("SrcPtr=%p\n",SrcPtr);
        StartMode = IFCML_CAM_DMA;
        src_img->imageData = (char *) SrcPtr;
    }

    ret = CmlRegistMemInfo(DeviceNum,SrcPtr,AllocBufSize,&MemHandle);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        Exit();
        return RTC::RTC_ERROR;
    }

    // Set Capture Configration
    ret = CmlSetCapConfig(DeviceNum,MemHandle,&CapFmt);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        Exit();
        return RTC::RTC_ERROR;
    }

    ret = CmlStartCapture(DeviceNum, 0 , StartMode | IFCML_CAP_ASYNC);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        Exit();
        return RTC::RTC_ERROR;
    }
    printf("Start Capture !!\n");

    CmlSetEventMask(DeviceNum, 0x11);
    CmlSetEvent(DeviceNum, (PIFCMLCALLBACK)CallBackDisplay, 1234);
    if(ret != IFCML_ERROR_SUCCESS){
        RETPRINT(ret);
        Exit();
        return RTC::RTC_ERROR;
    }

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PuckTracker::onFinalize()
{
    CmlSetEventMask(DeviceNum, 0x00);
    CmlStopCapture(DeviceNum, IFCML_MEM_STOP);
    cvDestroyAllWindows();
    cvReleaseImage(&src_img);
    cvReleaseImage(&rgb_img);
    cvReleaseImage(&srcrgb_img);
    CmlFreeMemInfo(DeviceNum, MemHandle);

    std::cout << "Finalized." << std::endl;

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t PuckTracker::onStartup(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t PuckTracker::onShutdown(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/


RTC::ReturnCode_t PuckTracker::onActivated(RTC::UniqueId ec_id)
{
    ch = 0;
    count=0;
    start=gettimeofday_sec();

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PuckTracker::onDeactivated(RTC::UniqueId ec_id)
{
    end=gettimeofday_sec();
    printf("time= %f, count= %d, average=%f\n",end-start,count,(end-start)/count);

    return RTC::RTC_OK;
}


RTC::ReturnCode_t PuckTracker::onExecute(RTC::UniqueId ec_id)
{
    static long int prevSec  = 0;
    static long int prevUSec = 0;

    if (t_tm.sec() - prevSec  ||  t_tm.usec() - prevUSec) {
        m_puckXY_out.tm.sec  = t_tm.sec();
        m_puckXY_out.tm.nsec = t_tm.usec() * 1000;

        if (n_match >= 0) {
            m_puckXY_out.data[0]=t_xy[0];
            m_puckXY_out.data[1]=t_xy[1];
            m_puckXY_outOut.write();
        }

        prevSec  = t_tm.sec();
        prevUSec = t_tm.usec();
    }
//    else {
//        std::cout << "Skipped." << std::endl;
//    }

    return RTC::RTC_OK;
}

/*
  RTC::ReturnCode_t PuckTracker::onAborting(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t PuckTracker::onError(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t PuckTracker::onReset(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t PuckTracker::onStateUpdate(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/

/*
  RTC::ReturnCode_t PuckTracker::onRateChanged(RTC::UniqueId ec_id)
  {
  return RTC::RTC_OK;
  }
*/



extern "C"
{

    void PuckTrackerInit(RTC::Manager* manager)
    {
        coil::Properties profile(pucktracker_spec);
        manager->registerFactory(profile,
                                 RTC::Create<PuckTracker>,
                                 RTC::Delete<PuckTracker>);
    }

};
