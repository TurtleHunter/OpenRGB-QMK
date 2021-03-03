/*-------------------------------------------------------------------*\
|  EKController.cpp                                                   |
|                                                                     |
|  Driver for EK Loop Connect                                         |
|                                                                     |
|  Chris M (Dr_No)          16th Jul 2020                             |
|                                                                     |
\*-------------------------------------------------------------------*/

#include "EKController.h"

static unsigned char ek_colour_mode_data[][16] =
{
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x01, 0x00, 0xFF, 0x64},          // Static
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x02, 0x00, 0xFF, 0x64},          // Breathing
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x03, 0xFF, 0xFF, 0x64},          // Fading
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x04, 0x00, 0xFF, 0x64},          // Marquee
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x05, 0x00, 0xFF, 0x64},          // Covering Marquee
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x06, 0x00, 0xFF, 0x64},          // Pulse
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x07, 0x00, 0xFF, 0x64},          // Wave
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x08, 0x00, 0xFF, 0x64},          // Alternating
    { 0x10, 0x12, 0x29, 0xAA, 0x01, 0x10, 0xA2, 0x60,
      0x00, 0x10, 0x20, 0x01, 0x09, 0x00, 0xFF, 0x64},          // Candle
};

static unsigned char ek_speed_mode_data[][9] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // Static
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Breathing
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Fading
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Marquee
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Covering Marquee
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Pulse
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Wave
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 },   // Alternating
    { 0x00, 0x0C, 0x19, 0x25, 0x32, 0x3E, 0x4B, 0x57, 0x64 }    // Candle
};

EKController::EKController(hid_device* dev_handle, char *_path)
{
    const int szTemp = 256;
    wchar_t tmpName[szTemp];

    dev = dev_handle;

    hid_get_manufacturer_string(dev, tmpName, szTemp);
    std::wstring wName = std::wstring(tmpName);
    device_name = std::string(wName.begin(), wName.end());

    hid_get_product_string(dev, tmpName, szTemp);
    wName = std::wstring(tmpName);
    device_name.append(" ").append(std::string(wName.begin(), wName.end()));

    hid_get_serial_number_string(dev, tmpName, szTemp);
    wName = std::wstring(tmpName);
    serial = std::string(wName.begin(), wName.end());

    location = _path;

    current_mode  = EK_MODE_STATIC;
    current_speed = EK_SPEED_NORMAL;
}

EKController::~EKController()
{
    hid_close(dev);
}

std::string EKController::GetDeviceName()
{
    return device_name;
}

std::string EKController::GetSerial()
{
    return serial;
}

std::string EKController::GetLocation()
{
    return("HID: " + location);
}

void EKController::SetMode(unsigned char mode, unsigned char speed)
{
    current_mode  = mode;
    current_speed = speed;

    SendUpdate();
}

void EKController::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    current_red   = red;
    current_green = green;
    current_blue  = blue;

    SendUpdate();
}

void EKController::SendUpdate()
{
    unsigned char buffer[EK_PACKET_LENGTH] = { 0x00 };
    int buffer_size = (sizeof(buffer) / sizeof(buffer[0]));

    for(std::size_t i = 0; i < EK_COLOUR_MODE_DATA_SIZE; i++)
    {
        buffer[i] = ek_colour_mode_data[current_mode][i];
    }

    //Set the relevant colour info
    buffer[EK_RED_BYTE]   = current_red;
    buffer[EK_GREEN_BYTE] = current_green;
    buffer[EK_BLUE_BYTE]  = current_blue;
    buffer[EK_SPEED_BYTE] = ek_speed_mode_data[current_mode][current_speed];

    buffer[10] = 0x10;
    buffer[47] = 0xFF;
    buffer[48] = 0x00;

    hid_write(dev, buffer, buffer_size);
}