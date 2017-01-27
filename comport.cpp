#include "comport.h"
#include <QDebug>
#ifdef __linux__
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#endif

ComPort::ComPort() : opened(false)
{
#if defined(_WIN32) || defined(WIN32)
    dcb=(DCB*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DCB));
    dcb->DCBlength = sizeof(DCB);
    BuildCommDCB(TEXT("parity=N data=8 stop=1"), dcb);
    ct.ReadIntervalTimeout = 0;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.ReadTotalTimeoutConstant = 100;
    ct.WriteTotalTimeoutMultiplier = ct.WriteTotalTimeoutConstant = 0;
    dcb->BaudRate = CBR_9600;
#endif
}

ComPort::~ComPort()
{
    closePort();
}

ComStatus ComPort::openPort(int number, ComSpeed speed)
{
#if defined(_WIN32) || defined(WIN32)
    string portName = "\\\\.\\COM";
    portName += to_string(number);
    return openPort(portName, speed);
#endif

#ifdef __linux__
    string portName = "/dev/ttyS";
    portName += to_string(number);
    return openPort(portName, speed);
#endif
}

ComStatus ComPort::openPort(string name, ComSpeed speed)
{
    closePort();
#if defined(_WIN32) || defined(WIN32)
    switch(speed)
    {
    case COM4800:
        dcb->BaudRate = CBR_4800;
        break;
    case COM9600:
        dcb->BaudRate = CBR_9600;
        break;
    case COM115200:
        dcb->BaudRate = CBR_115200;
        break;
    default:
        dcb->BaudRate = CBR_115200;
    }

    string portName = "\\\\.\\";
    portName += name;

    port = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                       NULL, OPEN_EXISTING, 0, NULL);

    if(port == INVALID_HANDLE_VALUE)
    {
        return PORT_OPEN_ERROR;
    }

    SetCommState(port, dcb);
    SetCommTimeouts(port, &ct);
    HeapFree(GetProcessHeap(), 0, dcb);
    PurgeComm(port, PURGE_TXCLEAR | PURGE_RXCLEAR);
    opened = true;

    return PORT_OPEN_OK;
#endif

#ifdef __linux__
    port = open(name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if(port == -1)
    {
        return PORT_OPEN_ERROR;
    }

    tcgetattr(port, &portOptions);

    cfmakeraw(&portOptions);

    speed_t s;
    switch(speed)
    {
    case COM4800:
        s = B4800;
        break;
    case COM9600:
        s = B9600;
        break;
    case COM115200:
        s = B115200;
        break;
    default:
        s = B115200;
    }
    cfsetispeed(&portOptions, s);
    cfsetospeed(&portOptions, s);

    portOptions.c_cflag &= ~PARENB;
    portOptions.c_cflag &= ~CSTOPB;
    portOptions.c_cflag &= ~CSIZE;
    portOptions.c_cflag |= CS8;

    portOptions.c_cflag |= (CLOCAL | CREAD);

    portOptions.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                             INLCR | PARMRK | INPCK | ISTRIP | IXON);

    portOptions.c_oflag = 0;

    portOptions.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

    portOptions.c_cc[VMIN] = 0;
    portOptions.c_cc[VTIME] = 1;

    tcsetattr(port, TCSANOW, &portOptions);
    fcntl(port, F_SETFL, 0);
    opened = true;

    return PORT_OPEN_OK;
#endif
}

void ComPort::closePort()
{
    if(opened)
    {
#if defined(_WIN32) || defined(WIN32)
        CloseHandle(port);
#endif

#ifdef __linux__
        close(port);
#endif
        opened = false;
    }
}

unsigned char ComPort::readByte(ComStatus &status)
{
#if defined(_WIN32) || defined(WIN32)
    unsigned char byte;
    if(opened)
    {
        ReadFile(port, &byte, 1, &bc, NULL);
    }
    else
    {
        status = PORT_CLOSED;
        return byte;
    }
    if(!bc)
    {
        status = BYTE_READ_TIMEOUT;
    }
    else
    {
        status = BYTE_READ_OK;
    }

    return byte;
#endif

#ifdef __linux__
    unsigned char byte;
    int count = 0;
    if(opened)
    {
        count = read(port, &byte, 1);
    }
    else
    {
        status = PORT_CLOSED;
        return byte;
    }
    if(count == 0)
    {
        status = BYTE_READ_TIMEOUT;
    }
    else if(count == -1)
    {
        status = PORT_OPEN_ERROR;
    }
    else
    {
        status = BYTE_READ_OK;
    }

    return byte;
#endif
}

void ComPort::sendByte(unsigned char byte)
{
#if defined(_WIN32) || defined(WIN32)
    WriteFile(port, &byte, 1, &bc, NULL);
#endif

#ifdef __linux__
    write(port, &byte, 1);
#endif
}

ComPort &ComPort::operator <<(unsigned char byte)
{
#if defined(_WIN32) || defined(WIN32)
    WriteFile(port, &byte, 1, &bc, NULL);
    return *this;
#endif

#ifdef __linux__
    write(port, &byte, 1);
    return *this;
#endif
}

ComPort &ComPort::operator <<(const char *string)
{
#if defined(_WIN32) || defined(WIN32)
    while(*string)
    {
        WriteFile(port, string, 1, &bc, NULL);
        ++string;
    }
    return *this;
#endif

#ifdef __linux__
    while(*string)
    {
        write(port, string, 1);
        ++string;
    }
    return *this;
#endif
}

ComPort &ComPort::operator >> (unsigned char &byte)
{
    if(opened)
    {
        ComStatus status = BYTE_READ_TIMEOUT;
        while(status != BYTE_READ_OK)
        {
            byte = readByte(status);
        }
    }

    return *this;
}

vector<string> ComPort::getAvailablePorts()
{
    vector<string> ports;

#if defined(_WIN32) || defined(WIN32)

    for(int i = 1; i < 16; ++i)
    {
        string portName = "\\\\.\\COM";
        portName += to_string(i);
        HANDLE p = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, 0, NULL);
        if(p != INVALID_HANDLE_VALUE)
        {
            ports.push_back(portName.substr(4, portName.length() - 4));
            CloseHandle(p);
        }
    }

#endif

#ifdef __linux__

    for(int i = 0; i < 10; ++i)
    {
        int p;
        string portName = "/dev/ttyS";
        string portNameUsb = "/dev/ttyUSB";
        portName += to_string(i);
        portNameUsb += to_string(i);
        p = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if(p != -1)
        {
            ports.push_back(portName);
            close(p);
        }
        p = open(portNameUsb.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if(p != -1)
        {
            ports.push_back(portName);
            close(p);
        }
    }

#endif

    return ports;
}
