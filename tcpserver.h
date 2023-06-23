#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QCoreApplication>
#include <QMap>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QTcpServer {
  Q_OBJECT
  QList<QTcpSocket *> clients;
  QMap<QTcpSocket *, QString> clientLogins;
  QMap<QTcpSocket *, int> clientChoices;
  int maxClients = 5;

public:
  Server(QObject *parent = 0);

private slots:
  void readyRead();
  void disconnected();
  void sendToClient(QTcpSocket *client, const QString &message);
  void disconnectAll();

protected:
  void incomingConnection(qintptr socketfd);
};

#endif // TCPSERVER_H
