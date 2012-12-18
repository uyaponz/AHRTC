#include "DeviceManager.hpp"

/* --- デバイスオープン --- */
int openDevices(int devNo, unsigned char &opened)
{
    // D/A
    if (DaOpen(devNo) != DA_ERROR_SUCCESS) return -1;
    opened |= BIT_DA;

    // Penc
    if (PencOpen(devNo, PENC_FLAG_SHARE) != PENC_ERROR_SUCCESS) return -1;
    opened |= BIT_PENC;

    return 0;
}

/* --- デバイスクローズ --- */
int closeDevices(int devNo, unsigned char &opened)
{
    int retVal = 0;

    // D/A
    if (opened & BIT_DA) {
        if (DaClose(devNo) != DA_ERROR_SUCCESS) retVal = -1;
        else opened &= ~BIT_DA;
    }

    // Penc
    if (opened & BIT_PENC) {
        if (PencClose(devNo) != PENC_ERROR_SUCCESS) retVal = -1;
        else opened &= ~BIT_PENC;
    }

    return retVal;
}
