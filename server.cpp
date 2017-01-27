#include "server.h"
#include "robots.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QThread>
#include <stdint-gcc.h>

Server::Server(QObject *parent) : QObject(parent),
    m_started(false)
{
    qRegisterMetaType<PointVector>("PointVector");
    m_server = new QTcpServer(this);
    m_comThread = new QThread;
    m_robots = new Robots;
    m_robots->moveToThread(m_comThread);
    connect(m_server, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(m_robots, &Robots::sendPoints, this, &Server::getPoints);
    connect(m_comThread, &QThread::started, m_robots,
            &Robots::process);
    m_comThread->start();
    startTimer(1000 / 100);
}

Server::~Server()
{
    m_robots->stopProcess();
    m_comThread->quit();
    m_comThread->wait(1000);
}

void Server::newConnection()
{
    if(m_sockets.size() < MAX_CONNECTIONS)
    {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
        connect(socket, &QTcpSocket::disconnected, this,
                &Server::disconnected);
        m_sockets << socket;
    }
    else
    {
        QTcpSocket *unknownSocket = m_server->nextPendingConnection();
        unknownSocket->disconnectFromHost();
        delete unknownSocket;
        qDebug() << "Something else trying to connect";
    }
}

bool Server::startServer()
{
    if(!m_started)
    {
        if(m_server->listen(QHostAddress::Any, 3336))
        {
            m_started = true;
            qDebug() << "Server started";
            return true;
        }
        else
        {
            qDebug() << "Error: can't start server";
            return false;
        }
    }
    return true;
}

void Server::disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    qDebug() << "Socket disconnected" << socket->peerAddress().toString();
    if(socket)
    {
        socket->readAll();
        m_sockets.removeAll(socket);
        socket->deleteLater();
    }
}

void Server::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if((quint64)socket->bytesAvailable() >= sizeof(RobotData))
    {
        RobotData newData;
        socket->read((char*)&newData, sizeof(RobotData));
        m_data[newData.number] = newData.cByte;
        uint16_t size = m_points.size();
        socket->write((char*)(&size), sizeof(uint16_t));
        socket->write((char*)m_points.data(), sizeof(Point2D) *
                      m_points.size());
        socket->flush();
    }
}

void Server::timerEvent(QTimerEvent *)
{
    if(!m_data.isEmpty())
    {
        m_robots->sendWheels(m_data);
        m_data.clear();
    }
}
