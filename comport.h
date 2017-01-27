#ifndef COMPORT_H
#define COMPORT_H

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif

#ifdef __linux__
#include <termios.h>
#endif

#include <vector>
#include <string>

using namespace std;

enum ComStatus
{
    PORT_OPEN_OK = 1,
    PORT_OPEN_ERROR = -1,
    BYTE_READ_OK = 2,
    BYTE_READ_TIMEOUT = -2,
    PORT_CLOSED = -3
};

enum ComSpeed
{
    COM4800,
    COM9600,
    COM115200
};

class ComPort
{
public:
    ComPort();
    ~ComPort();
    ComStatus openPort(int number, ComSpeed speed = COM115200);
    ComStatus openPort(string name, ComSpeed speed = COM115200);
    void closePort();
    bool isOpened() {return opened;}
    void sendByte(unsigned char byte);
    unsigned char readByte(ComStatus &status);
    ComPort &operator << (unsigned char byte);
    ComPort &operator << (const char *string);
    ComPort &operator >> (unsigned char &byte);
    static vector<string> getAvailablePorts();

private:
    bool opened;
#if defined(_WIN32) || defined(WIN32)
    HANDLE port;
    COMMTIMEOUTS ct;
    DCB *dcb;
    COMMCONFIG *cf;
    DWORD sz;
    DWORD bc;
#endif

#ifdef __linux__
    termios portOptions;
    int port;
#endif
};

#endif // COMPORT_H
