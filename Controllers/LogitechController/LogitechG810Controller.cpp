/*-----------------------------------------*\
|  LogitechG810Controller.cpp               |
|                                           |
|  Driver for Logitech G810 Orion Spectrum  |
|  keyboard light controller                |
|                                           |
|  Adam Honse (CalcProgrammer1) 6/11/2020   |
\*-----------------------------------------*/

#include "LogitechG810Controller.h"
#include <cstring>

LogitechG810Controller::LogitechG810Controller(hid_device* dev_handle_0x11, hid_device* dev_handle_0x12)
{
    dev_pkt_0x11 = dev_handle_0x11;
    dev_pkt_0x12 = dev_handle_0x12;
}

LogitechG810Controller::~LogitechG810Controller()
{

}

std::string LogitechG810Controller::GetSerialString()
{
    wchar_t serial_string[128];
    hid_get_serial_number_string(dev_pkt_0x11, serial_string, 128);

    std::wstring return_wstring = serial_string;
    std::string return_string(return_wstring.begin(), return_wstring.end());

    return(return_string);
}

void LogitechG810Controller::Commit()
{
    SendCommit();
}

void LogitechG810Controller::SetDirect
    (
    unsigned char       zone,
    unsigned char       frame_count,
    unsigned char *     frame_data
    )
{
    SendDirectFrame(zone, frame_count, frame_data);
}

void LogitechG810Controller::SetMode
    (
    unsigned char       mode,
    unsigned short      speed,
    unsigned char       red,
    unsigned char       green,
    unsigned char       blue
    )
{
    SendMode(LOGITECH_G810_ZONE_MODE_KEYBOARD, mode, speed, red, green, blue);
    SendMode(LOGITECH_G810_ZONE_MODE_LOGO,     mode, speed, red, green, blue);

    SendCommit();
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void LogitechG810Controller::SendCommit()
{
    char usb_buf[20];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Commit packet                                  |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x11;
    usb_buf[0x01]           = 0xFF;
    usb_buf[0x02]           = 0x0C;
    usb_buf[0x03]           = 0x5D;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev_pkt_0x11, (unsigned char *)usb_buf, 20);
    hid_read(dev_pkt_0x11, (unsigned char *)usb_buf, 20);
}

void LogitechG810Controller::SendDirectFrame
    (
    unsigned char       zone,
    unsigned char       frame_count,
    unsigned char *     frame_data
    )
{
    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Lighting Control packet                        |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x12;
    usb_buf[0x01]           = 0xFF;
    usb_buf[0x02]           = 0x0C;
    usb_buf[0x03]           = 0x3D;
    usb_buf[0x05]           = zone;
    usb_buf[0x07]           = frame_count;

    /*-----------------------------------------------------*\
    | Copy in frame data                                    |
    \*-----------------------------------------------------*/
    memcpy(&usb_buf[0x08], frame_data, frame_count * 4);

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev_pkt_0x12, (unsigned char *)usb_buf, 64);
    hid_read(dev_pkt_0x11, (unsigned char *)usb_buf, 20);
}

void LogitechG810Controller::SendMode
    (
    unsigned char       zone,
    unsigned char       mode,
    unsigned short      speed,
    unsigned char       red,
    unsigned char       green,
    unsigned char       blue
    )
{
    char usb_buf[20];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Lighting Control packet                        |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x11;
    usb_buf[0x01]           = 0xFF;
    usb_buf[0x02]           = 0x0D;
    usb_buf[0x03]           = 0x3D;
    usb_buf[0x04]           = zone;

    usb_buf[0x05]           = mode;

    usb_buf[0x06]           = red;
    usb_buf[0x07]           = green;
    usb_buf[0x08]           = blue;

    speed = 100 * speed;
    if(mode == LOGITECH_G810_MODE_CYCLE)
    {
        usb_buf[0x0B]       = speed >> 8;
        usb_buf[0x0C]       = speed & 0xFF;
        usb_buf[0x0D]       = 0x64;
    }
    else if(mode == LOGITECH_G810_MODE_BREATHING)
    {
        usb_buf[0x09]       = speed >> 8;
        usb_buf[0x0A]       = speed & 0xFF;
        usb_buf[0x0C]       = 0x64;
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev_pkt_0x11, (unsigned char *)usb_buf, 20);
    hid_read(dev_pkt_0x11, (unsigned char *)usb_buf, 20);
}