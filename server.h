#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include "robots.h"

class QTcpServer;
class QTcpSocket;
class QThread;
class ComPort;

/*
 * PROTOCOL
 *
 * 2 bytes size
 * Array of dots 4 bytes
 *
 */

typedef QVector<Point2D> PointVector;

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();
    bool startServer();

protected:
    void timerEvent(QTimerEvent*);

private:
    QTcpServer *server;
    QTcpSocket *sockets[2];
    Robots *robots;
    QThread *comThread;
    bool started;
    bool connected[2];
    unsigned char wheels1[2];
    unsigned char wheels2[2];
    PointVector points;
    QMap <uint8_t, uint8_t> data;

private slots:
    void newConnection();
    void readyRead1();
    void readyRead2();
    void disconnected1();
    void disconnected2();
    void getPoints(PointVector p) {points = p;}

signals:

public slots:
};

#endif // SERVER_H
