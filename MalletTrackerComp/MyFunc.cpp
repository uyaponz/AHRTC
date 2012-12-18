#include "MyFunc.hpp"

#include <iostream>
#include <cstdio>

const int N = 9*5; //   number  of   reference   points for  calibration algorithm
//const int N = 9; //   number  of   reference   points for  calibration algorithm

double ReferencePoint[N][2]; // ideal position of reference points
double SamplePoint[N][2];    // sampling position of reference points
double KX1, KX2, KX3, KY1, KY2, KY3;   // coefficients for calibration algorithm

bool loadCalibData(std::string filename)
{
    FILE *fp = fopen(filename.c_str(), "r");
    if (NULL == fp) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }

    double refX, refY;
    double samX, samY;
    for (int i=0; i<N; i++) {
        if  (EOF == fscanf(fp, "%lf %lf %lf %lf", &refX, &refY, &samX, &samY)) {
            fclose(fp);
            std::cerr << "Unknown file format." << std::endl;
            return false;
        }

        ReferencePoint[i][0] = refX;
        ReferencePoint[i][1] = refY;
        SamplePoint   [i][0] = samX;
        SamplePoint   [i][1] = samY;
    }

    fclose(fp);

    return true;
}

// do calibration for point (Px, Py) using the calculated coefficients
void Do_Calibration(double &Px, double &Py)
{
    Px = (KX1*Px + KX2*Py + KX3 + 0.5);
    Py = (KY1*Px + KY2*Py + KY3 + 0.5);
}

// calculate the coefficients for calibration algorithm: KX1, KX2, KX3, KY1, KY2, KY3
int Get_Calibration_Coefficient()
{
    int i;
    int Points=N;
    double a[3],b[3],c[3],d[3],k;

    if(Points<3) {
        return 0;
    }

    else {
        if (Points==3) {
            for(i=0; i<Points; i++) {
                a[i] = (SamplePoint[i][0]);
                b[i] = (SamplePoint[i][1]);
                c[i] = (ReferencePoint[i][0]);
                d[i] = (ReferencePoint[i][1]);
            }
        }

        else if(Points>3) {
            for(i=0; i<3; i++) {
                a[i] = 0;
                b[i] = 0;
                c[i] = 0;
                d[i] = 0;
            }

            for(i=0; i<Points; i++) {
                a[2] = a[2] + (SamplePoint[i][0]);
                b[2] = b[2] + (SamplePoint[i][1]);
                c[2] = c[2] + (ReferencePoint[i][0]);
                d[2] = d[2] + (ReferencePoint[i][1]);
                a[0] = a[0] + (SamplePoint[i][0]) * (SamplePoint[i][0]);
                a[1] = a[1] + (SamplePoint[i][0]) * (SamplePoint[i][1]);
                b[0] = a[1];
                b[1] = b[1] + (SamplePoint[i][1]) * (SamplePoint[i][1]);
                c[0] = c[0] + (SamplePoint[i][0]) * (ReferencePoint[i][0]);
                c[1] = c[1] + (SamplePoint[i][1]) * (ReferencePoint[i][0]);
                d[0] = d[0] + (SamplePoint[i][0]) * (ReferencePoint[i][1]);
                d[1] = d[1] + (SamplePoint[i][1]) * (ReferencePoint[i][1]);
            }

            a[0] = a[0] / a[2];
            a[1] = a[1] / b[2];
            b[0] = b[0] / a[2];
            b[1] = b[1] / b[2];
            c[0] = c[0] / a[2];
            c[1] = c[1] / b[2];
            d[0] = d[0] / a[2];
            d[1] = d[1] / b[2];
            a[2] = a[2] / Points;
            b[2] = b[2] / Points;
            c[2] = c[2] / Points;
            d[2] = d[2] / Points;
        }

        k   = (a[0]-a[2]) * (b[1]-b[2])  -  (a[1]-a[2]) * (b[0]-b[2]);
        KX1 = ((c[0]-c[2]) * (b[1]-b[2])  -  (c[1]-c[2]) * (b[0]-b[2]))  /  k;
        KX2 = ((c[1]-c[2]) * (a[0]-a[2])  -  (c[0]-c[2]) * (a[1]-a[2]))  /  k;
        KX3 = (b[0] * (a[2]*c[1] - a[1]*c[2])  +  b[1] * (a[0]*c[2] - a[2]*c[0])  +  b[2] * (a[1]*c[0] - a[0]*c[1]))  /  k;
        KY1 = ((d[0]-d[2]) * (b[1]-b[2])  -  (d[1]-d[2]) * (b[0]-b[2]))  /  k;
        KY2 = ((d[1]-d[2]) * (a[0]-a[2])  -  (d[0]-d[2]) * (a[1]-a[2]))  /  k;
        KY3 = (b[0] * (a[2]*d[1] - a[1]*d[2])  +  b[1] * (a[0]*d[2] - a[2]*d[0])  +  b[2] * (a[1]*d[0] - a[0]*d[1]))  /  k;

        return Points;
    }
}
