#include <gsl/gsl>
#include <map>

#include "ite-device.h"

const static std::map<ITESpeed, uint8_t> speedMap = {
    {ITESpeed::VERY_SLOW, 0x0a},
    {ITESpeed::SLOW,      0x07},
    {ITESpeed::MEDIUM,    0x05},
    {ITESpeed::FAST,      0x03},
    {ITESpeed::VERY_FAST, 0x01}
};

const static std::map<ITEBrightness, uint8_t> brightnessMap {
    {ITEBrightness::OFF,         0x00},
    {ITEBrightness::VERY_DIM,    0x08},
    {ITEBrightness::DIM,         0x16},
    {ITEBrightness::BRIGHT,      0x24},
    {ITEBrightness::VERY_BRIGHT, 0x32}
};

ITEDevice::ITEDevice(libusb_context *ctx)
    : handle(nullptr)
{
    int ret;
    libusb_device **devices;

    auto count = libusb_get_device_list(ctx, &devices);
    auto const freeList = gsl::finally([&devices] {
        libusb_free_device_list(devices, 1);
    });

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devices[i];
        libusb_device_descriptor desc;

        ret = libusb_get_device_descriptor(dev, &desc);

        if (ret != 0)
            throw std::runtime_error(std::string("could not get USB device descriptor: ") +
                                     libusb_strerror((libusb_error)ret));

        if (desc.idVendor == 0x048d && desc.idProduct == 0xce00) {
            ret = libusb_open(dev, &handle);

            if (ret != 0)
                throw std::runtime_error(std::string("could not open USB device: ")
                                         + libusb_strerror((libusb_error)ret));
        }
    }

    if (!handle)
        throw std::runtime_error("could not find backlight controller\n");
}

ITEDevice::~ITEDevice()
{
    libusb_close(handle);
}

void ITEDevice::seBreatheStyle(std::array<Colour, 7> palette, ITESpeed speed,
                               ITEBrightness brightness)
{
    size_t i = 1;

    for (const auto &color : palette)
        transferColour(color, i++);


    transferMsg({0x08, 0x02, 0x02, speedMap.at(speed),
                 brightnessMap.at(brightness), 0x08, 0x00, 0x01});
}

void ITEDevice::setStaticStyle(std::array<Colour, 4> palette, ITEBrightness brightness)
{
    size_t i = 1;

    for (const auto &color : palette)
        transferColour(color, i++);

    transferMsg({0x08, 0x02, 0x01, 0x00, brightnessMap.at(brightness),
                 0x08, 0x00, 0x01});
}

void ITEDevice::setMonoColour(Colour colour, ITEBrightness brightness)
{
    setStaticStyle({colour, colour, colour, colour}, brightness);
}

void ITEDevice::transferColour(Colour c, uint8_t idx)
{
    transferMsg({0x14, 0x00, idx, c.red, c.green, c.blue, 0x00, 0x00});
}

void ITEDevice::transferMsg(std::array<uint8_t, 8> msg)
{
    auto ret = libusb_control_transfer(handle, 0x21, 9, 0x0300, 1,
                                       &msg[0], msg.size(), 1000);

    if (ret < 0)
        throw std::runtime_error(std::string("transfer failed: ") +
                                 libusb_strerror((libusb_error)ret));
}
