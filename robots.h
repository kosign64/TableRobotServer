#ifndef ROBOTS_H
#define ROBOTS_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <stdint-gcc.h>

class ComPort;

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
    void sendWheels(unsigned char wheels1[], unsigned char wheels2[]);
    void sendWheels(QVector<RobotData> data);
    void sendWheels(QMap <uint8_t, uint8_t> data);
    void stopProcess() {stop = true;}

private:
    ComPort *port;
    bool stop;
    PointVector points;

signals:
    void sendPoints(PointVector p);

public slots:
    void process();
};

#endif // ROBOTS_H
