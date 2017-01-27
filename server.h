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
    QTcpServer *m_server;
    QList <QTcpSocket *> m_sockets;
    Robots *m_robots;
    QThread *m_comThread;
    bool m_started;
    PointVector m_points;
    QMap <uint8_t, uint8_t> m_data;

    static const int MAX_CONNECTIONS = 10;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();
    void getPoints(PointVector points) {m_points = points;}

signals:

public slots:
};

#endif // SERVER_H
