#ifndef ROBOTS_H
#define ROBOTS_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <comport.h>
#include <stdint-gcc.h>


struct Point2D
{
    uint16_t x;
    uint16_t y;
};

struct RobotData
{
    uint8_t number;
    uint8_t cByte;
};

typedef QVector<Point2D> PointVector;

class Robots : public QObject
{
    Q_OBJECT
public:
    explicit Robots(QObject *parent = 0);
    ~Robots();
    void sendWheels(QVector<RobotData> data);
    void sendWheels(QMap <uint8_t, uint8_t> data);
    void stopProcess() {m_stop = true;}

private:
    ComPort m_port;
    bool m_stop;
    PointVector m_points;

signals:
    void sendPoints(PointVector p);

public slots:
    void process();
};

#endif // ROBOTS_H
