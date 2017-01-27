#include "robots.h"
#include "comport.h"
#include <QDebug>

bool dataLessThan(const RobotData &d1, const RobotData &d2)
{
    return d1.number < d2.number;
}

Robots::Robots(QObject *parent) : QObject(parent),
    stop(false)
{
    port = new ComPort;
    if(port->openPort("/dev/ttyUSB0") > 0)
    {
        qDebug() << "Port opened";
    }
    else
    {
        qDebug() << "Error: Can't open port";
    }
}

Robots::~Robots()
{

}

void Robots::process()
{
    int byte, dotNumber;
    ComStatus status;
    points.clear();
    qDebug() << "Thread started";
    while(!stop)
    {
        byte = port->readByte(status);
        if(status > 0)
        {
            if(byte == 255)
            {
                byte = port->readByte(status);
                if(status > 0)
                {
                    dotNumber = byte;
                    points.resize(dotNumber);
                    for(int i = 0; i < points.size(); ++i)
                    {
                        byte = port->readByte(status);
                        if(status > 0)
                        {
                            int x = (int)byte * 100;
                            byte = port->readByte(status);
                            x += byte;
                            byte = port->readByte(status);
                            int y = (int)byte * 100;
                            byte = port->readByte(status);
                            y += byte;
                            Point2D point;
                            point.x = x;
                            point.y = y;
                            points[i] = point;
                        }
                    }
                    emit sendPoints(points);
                }
            }
        }
    }
}

void Robots::sendWheels(unsigned char wheels1[], unsigned char wheels2[])
{
    //*port << 0xFF << wheels1[0] << wheels1[2] << wheels2[0] << wheels2[1];
    unsigned char cByte = 0;
    cByte |= (1 << 6);
    switch(wheels1[0])
    {
    case 1:
        cByte |= (1 << 3);
        break;
    case 255:
        cByte |= (1 << 5) | (1 << 3);
        break;
    }
    switch(wheels1[1])
    {
    case 1:
        cByte |= (1 << 0);
        break;
    case 255:
        cByte |= (1 << 2) | (1 << 0);
        break;
    }
    qDebug() << QString::number(cByte, 2);
    *port << 0xFF << cByte;
}

void Robots::sendWheels(QVector <RobotData> data)
{
    qSort(data.begin(), data.end(), dataLessThan);
    qDebug() << "0xFF" << data.size();
    *port << 0xFF;
    int j = 0;
    qDebug() << data.back().number;
    for(int i = 0; i < (data.back().number); ++i)
    {
        if(data[j].number == (i + 1))
        {
            qDebug() << data[j].cByte;
            *port << data[j].cByte;
            j++;
        }
        else
        {
            qDebug() << "N";
            *port << (unsigned char)0;
        }
    }
}

void Robots::sendWheels(QMap <uint8_t, uint8_t> data)
{
    //qDebug() << "0xFF" << data.size();
    *port << 0xFF;
    for(int i = 0; i < data.lastKey(); ++i)
    {
        if(data.contains(i + 1))
        {
            qDebug() << i + 1 << data[i + 1];
            *port << data[i + 1];
        }
        else
        {
            *port << (unsigned char)0;
            qDebug() << i + 1 << "No";
        }
    }
    *port << 0xFE;
}
