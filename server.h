#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include "robots.h"

class QTcpServer;
class QTcpSocket;
class QThread;

/*
 * PROTOCOL
 *
 * Server sends:
 * 2 bytes (uint16_t) size (number of dots)
 * Array of dots, each dot is 4 bytes (2 * uint16_t)
 *
 * Server receives:
 * 1 byte robot number (uint8_t)
 * 1 byte control byte (uint8_t)
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
    QTcpServer *server_;
    QList <QTcpSocket *> sockets_;
    Robots *robots_;
    QThread *comThread_;
    bool started_;
    PointVector points_;
    QMap <uint8_t, uint8_t> data_;

    static const int MAX_CONNECTIONS = 10;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();
    void getPoints(PointVector points) {points_ = points;}

signals:

public slots:
};

#endif // SERVER_H
