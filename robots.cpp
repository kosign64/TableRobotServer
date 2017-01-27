#include "robots.h"
#include "comport.h"
#include <QDebug>

Robots::Robots(QObject *parent) : QObject(parent),
    m_stop(false)
{
    if(m_port.openPort("/dev/ttyUSB0") > 0)
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
    unsigned char byte;
    unsigned char dotNumber;
    ComStatus status;
    m_points.clear();
    qDebug() << "Thread started";
    while(!m_stop)
    {
        byte = m_port.readByte(status);
        if(status < 0) continue;
        if(byte == 255)
        {
            byte = m_port.readByte(status);
            if(status < 0) continue;
            dotNumber = byte;
            m_points.resize(dotNumber);
            for(int i = 0; i < m_points.size(); ++i)
            {
                byte = m_port.readByte(status);
                if(status < 0) continue;
                int x = (int)byte * 100;
                byte = m_port.readByte(status);
                x += byte;
                byte = m_port.readByte(status);
                int y = (int)byte * 100;
                byte = m_port.readByte(status);
                y += byte;
                Point2D point;
                point.x = x;
                point.y = y;
                m_points[i] = point;
            }
            emit sendPoints(m_points);
        }
    }
}

void Robots::sendWheels(QVector <RobotData> data)
{
    qSort(data.begin(), data.end(), [](const RobotData &d1,
          const RobotData &d2) {return d1.number < d2.number;});
    qDebug() << "0xFF" << data.size();
    m_port << 0xFF;
    int j = 0;
    qDebug() << data.back().number;
    for(int i = 0; i < (data.back().number); ++i)
    {
        if(data[j].number == (i + 1))
        {
            qDebug() << data[j].cByte;
            m_port << data[j].cByte;
            j++;
        }
        else
        {
            qDebug() << "N";
            m_port << (unsigned char)0;
        }
    }
}

void Robots::sendWheels(QMap <uint8_t, uint8_t> data)
{
    m_port << 0xFF;
    for(int i = 0; i < data.lastKey(); ++i)
    {
        if(data.contains(i + 1))
        {
            qDebug() << i + 1 << data[i + 1];
            m_port << data[i + 1];
        }
        else
        {
            m_port << (unsigned char)0;
            qDebug() << i + 1 << "No";
        }
    }
    m_port << 0xFE;
}
