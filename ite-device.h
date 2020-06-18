#ifndef __ITE_DEVICE_H_
#define __ITE_DEVICE_H_

#include <array>
#include <libusb-1.0/libusb.h>

#include "colour.h"
#include "speed.h"
#include "brightness.h"

class ITEDevice
{
    public:
        ITEDevice(libusb_context *ctx);
        ~ITEDevice();

        void seBreatheStyle(std::array<Colour, 7> palette,
                            ITESpeed speed,
                            ITEBrightness brightness);

        void setStaticStyle(std::array<Colour, 4> palette,
                            ITEBrightness brightness);

        void setMonoColour(Colour colour, ITEBrightness brightness);

    private:
        void transferColour(Colour c, uint8_t idx);
        void transferMsg(std::array<uint8_t, 8> msg);
        libusb_device_handle *handle;

};


#endif // __ITE_DEVICE_H_
