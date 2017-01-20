#include "comport.h"
#include <QDebug>
#ifdef linux
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#endif

ComPort::ComPort() : portNumber(0),
                     opened(false)
{
#if defined(_WIN32) || defined(WIN32)
    dcb=(DCB*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DCB));
    dcb->DCBlength = sizeof(DCB);
    BuildCommDCB(TEXT("parity=N data=8 stop=1"), dcb);
    ct.ReadIntervalTimeout = 0;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.ReadTotalTimeoutConstant = 100;
    ct.WriteTotalTimeoutMultiplier = ct.WriteTotalTimeoutConstant = 0;
    dcb->BaudRate = 115200;
#endif
}

ComPort::~ComPort()
{
    closePort();
}

int ComPort::openPort(int number)
{
#if defined(_WIN32) || defined(WIN32)
    if(!opened)
    {
        if(number < 1 || number > 4)
        {
            return WRONG_PORT_NUMBER;
        }

        portNumber = number;
        switch(portNumber)
        {
        case 1: port = CreateFile(TEXT("COM1"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL); break;
        case 2: port = CreateFile(TEXT("COM2"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL); break;
        case 3: port = CreateFile(TEXT("COM3"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL); break;
        case 4: port = CreateFile(TEXT("COM4"), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL); break;
        }
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
    }
    else
    {
        return PORT_IS_ALREADY_OPENED;
    }
#endif

#ifdef linux
    if(!opened)
    {
        char portName[] = "/dev/ttyUSB0";
        //portName[9] = (unsigned char)number - 1 + '0';
        port = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);

        if(port == -1)
        {
            return PORT_OPEN_ERROR;
        }

        tcgetattr(port, &portOptions);

        cfmakeraw(&portOptions);

        cfsetispeed(&portOptions, B115200);
        cfsetospeed(&portOptions, B115200);

        portOptions.c_cflag &= ~PARENB;
        portOptions.c_cflag &= ~CSTOPB;
        portOptions.c_cflag &= ~CSIZE;
        portOptions.c_cflag |= CS8;

        portOptions.c_cflag |= (CLOCAL | CREAD);

        //portOptions.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        portOptions.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                            INLCR | PARMRK | INPCK | ISTRIP | IXON);

        //portOptions.c_oflag &= ~OPOST;
        portOptions.c_oflag = 0;

        portOptions.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

        portOptions.c_cc[VMIN] = 0;
        portOptions.c_cc[VTIME] = 1;

        tcsetattr(port, TCSANOW, &portOptions);
        fcntl(port, F_SETFL, 0);
        opened = true;

        return PORT_OPEN_OK;
    }
    else
    {
        return PORT_IS_ALREADY_OPENED;
    }
#endif
}

void ComPort::closePort()
{
    if(opened)
    {
#if defined(_WIN32) || defined(WIN32)
        CloseHandle(port);
#endif

#ifdef linux
        close(port);
#endif
        opened = false;
    }
}

unsigned char ComPort::readByte(int &status)
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

#ifdef linux
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

#ifdef linux
    write(port, &byte, 1);
#endif
}

ComPort &ComPort::operator <<(unsigned char number)
{
#if defined(_WIN32) || defined(WIN32)
    WriteFile(port, &number, 1, &bc, NULL);
    return *this;
#endif

#ifdef linux
    write(port, &number, 1);
    return *this;
#endif
}

ComPort &ComPort::operator <<(unsigned char *string)
{
#if defined(_WIN32) || defined(WIN32)
    while(*string)
    {
        WriteFile(port, string, 1, &bc, NULL);
        ++string;
    }
    return *this;
#endif

#ifdef linux
    while(*string)
    {
        write(port, string, 1);
        ++string;
    }
    return *this;
#endif
}
