#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include <fbiad.h>   // GPG-3100
#include <fbida.h>   // GPG-3300
#include <fbipenc.h> // GPG-6204

enum DeviceBit {
    BIT_AD   = 0x01,
    BIT_DA   = 0x02,
    BIT_PENC = 0x04,
};

/* --- デバイスオープン --- */
int openDevices(int devNo, unsigned char &opened);

/* --- デバイスクローズ --- */
int closeDevices(int devNo, unsigned char &opened);

#endif // DEVICE_MANAGER_HPP
