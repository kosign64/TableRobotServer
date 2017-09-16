#include "server.h"
#include "robots.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QThread>
#include <stdint-gcc.h>

Server::Server(QObject *parent) : QObject(parent),
    started_(false)
{
    qRegisterMetaType<PointVector>("PointVector");
    server_ = new QTcpServer(this);
    comThread_ = new QThread;
    robots_ = new Robots;
    robots_->moveToThread(comThread_);
    connect(server_, &QTcpServer::newConnection, this,
            &Server::newConnection);
    connect(robots_, &Robots::sendPoints, this, &Server::getPoints);
    connect(comThread_, &QThread::started, robots_,
            &Robots::process);
    comThread_->start();
    startTimer(1000 / 100);
}

Server::~Server()
{
    robots_->stopProcess();
    comThread_->quit();
    comThread_->wait(1000);
}

void Server::newConnection()
{
    if(sockets_.size() < MAX_CONNECTIONS)
    {
        QTcpSocket *socket = server_->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
        connect(socket, &QTcpSocket::disconnected, this,
                &Server::disconnected);
        sockets_ << socket;
    }
    else
    {
        QTcpSocket *unknownSocket = server_->nextPendingConnection();
        unknownSocket->disconnectFromHost();
        delete unknownSocket;
        qDebug() << "Something else trying to connect";
    }
}

bool Server::startServer()
{
    if(!started_)
    {
        if(server_->listen(QHostAddress::Any, 3336))
        {
            started_ = true;
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
        sockets_.removeAll(socket);
        socket->deleteLater();
    }
}

void Server::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    quint64 bytesAvailable = socket->bytesAvailable();
    if(bytesAvailable >= sizeof(RobotData))
    {
        RobotData newData;
        int robotsNumber = bytesAvailable / sizeof(RobotData);
        for(int i = 0; i < robotsNumber; ++i)
        {
            socket->read((char*)&newData, sizeof(RobotData));
            data_[newData.number] = newData.cByte;
        }
        uint16_t size = points_.size();
        socket->write((char*)(&size), sizeof(uint16_t));
        socket->write((char*)points_.data(), sizeof(Point2D) *
                      points_.size());
        socket->flush();
    }
}

void Server::timerEvent(QTimerEvent *)
{
    if(!data_.isEmpty())
    {
        robots_->sendWheels(data_);
        data_.clear();
    }
}
