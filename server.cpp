#include "server.h"
#include "robots.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QThread>
#include <stdint-gcc.h>

Server::Server(QObject *parent) : QObject(parent),
    started(false),
    connected{false, false}
{
    qRegisterMetaType<PointVector>("PointVector");
    server = new QTcpServer(this);
    comThread = new QThread;
    robots = new Robots;
    robots->moveToThread(comThread);
    connect(server, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(robots, &Robots::sendPoints, this, &Server::getPoints);
    connect(comThread, SIGNAL(started()), robots, SLOT(process()));
    comThread->start();
    startTimer(1000 / 100);
}

Server::~Server()
{
    robots->stopProcess();
    comThread->quit();
    comThread->wait(1000);
}

void Server::newConnection()
{
    if(connected[0] == false)
    {
        sockets[0] = server->nextPendingConnection();
        connect(sockets[0], &QTcpSocket::readyRead, this, &Server::readyRead1);
        connect(sockets[0], &QTcpSocket::disconnected, this,
                &Server::disconnected1);
        connect(sockets[0], &QTcpSocket::disconnected, sockets[0],
                &QTcpSocket::deleteLater);
        connected[0] = true;
        qDebug() << "New connection to socket #0 from" << sockets[0]->peerAddress().toString();
    }
    else if(connected[1] == false)
    {
        sockets[1] = server->nextPendingConnection();
        connect(sockets[1], &QTcpSocket::readyRead, this, &Server::readyRead2);
        connect(sockets[1], &QTcpSocket::disconnected, this,
                &Server::disconnected2);
        connect(sockets[1], &QTcpSocket::disconnected, sockets[1],
                &QTcpSocket::deleteLater);
        connected[1] = true;
        qDebug() << "New connection to socket #1 from" << sockets[1]->peerAddress().toString();
    }
    else
    {
        QTcpSocket *unknownSocket = server->nextPendingConnection();
        unknownSocket->disconnectFromHost();
        delete unknownSocket;
        qDebug() << "Something else trying to connect";
    }
}

bool Server::startServer()
{
    if(!started)
    {
        if(server->listen(QHostAddress::Any, 3336))
        {
            started = true;
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

void Server::disconnected1()
{
    qDebug() << "Socket #0 disconnected" << sockets[0]->peerAddress().toString();
    connected[0] = false;
    sockets[0]->readAll();
}

void Server::disconnected2()
{
    qDebug() << "Socket #1 disconnected" << sockets[1]->peerAddress().toString();
    connected[1] = false;
    sockets[1]->readAll();
}

void Server::readyRead1()
{
    if(sockets[0]->bytesAvailable() >= sizeof(RobotData))
    {
        RobotData newData;
        sockets[0]->read((char*)&newData, sizeof(RobotData));
        data[newData.number] = newData.cByte;
        uint16_t size = points.size();
        sockets[0]->write((char*)(&size), sizeof(uint16_t));
        sockets[0]->write((char*)points.data(), sizeof(Point2D) * size);
        sockets[0]->flush();
    }
}

void Server::readyRead2()
{
    if(sockets[1]->bytesAvailable() >= sizeof(RobotData))
    {
        RobotData newData;
        sockets[1]->read((char*)&newData, sizeof(RobotData));
        data[newData.number] = newData.cByte;
        uint16_t size = points.size();
        sockets[1]->write((char*)(&size), sizeof(uint16_t));
        sockets[1]->write((char*)points.data(), sizeof(Point2D) * points.size());
        sockets[1]->flush();
    }
}

void Server::timerEvent(QTimerEvent *)
{
    if(!data.isEmpty())
    {
        robots->sendWheels(data);
        data.clear();
    }
}
