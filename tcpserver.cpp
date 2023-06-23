#include "tcpserver.h"

Server::Server(QObject *parent) : QTcpServer(parent) {
  // Setup database
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("clients.db");
  if (!db.open()) {
    qDebug() << "Error: Failed to open the database";
  }
  QSqlQuery query;
  query.exec("CREATE TABLE IF NOT EXISTS clients(login TEXT, games INTEGER, "
             "wins INTEGER)");
}

void Server::incomingConnection(qintptr socketfd) {
  QTcpSocket *client = new QTcpSocket(this);
  client->setSocketDescriptor(socketfd);

  clients << client;

  connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
  connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));

  qDebug() << "New client from:" << client->peerAddress().toString();
}

void Server::readyRead() {
  QTcpSocket *client = (QTcpSocket *)sender();
  while (client->canReadLine()) {
    QStringList tokens = QString(client->readLine()).trimmed().split("&");
    QSqlQuery query;
    if (tokens[0] == "start") {
      if (tokens.size() == 2) {
        QSqlQuery query;
        query.prepare("SELECT login FROM clients WHERE login = :login");
        query.bindValue(":login", tokens[1]);

        if (query.exec()) {
          if (query.next()) {
            // Proceed if client exists
            query.prepare("INSERT INTO clients (login, games, wins) VALUES "
                          "(:login, :games, :wins)");
            query.bindValue(":login", tokens[1]);
            query.bindValue(":games", 0);
            query.bindValue(":wins", 0);
          }
        } else {
          sendToClient(client, "Error: Failed to verify login existence");
        }

        if (clientLogins.count() < maxClients) {
          sendToClient(client, "Started waiting");
          clientLogins[client] = tokens[1];
          if (clientLogins.count() >= maxClients) {
            QMap<QTcpSocket *, QString>::iterator it;
            for (it = clientLogins.begin(); it != clientLogins.end(); ++it) {
              sendToClient(it.key(), "Make move");
            }
          }

        } else {
          sendToClient(client,
                       "The server has reached the maximum number of users");
        }
      } else {
        sendToClient(client,
                     "Error: Insufficient parameters for 'start' command");
      }
    } else if (tokens[0] == "break") {
      if (clientLogins.contains(client)) {
        QString login = clientLogins[client];
        client->write(QString("You have left\n").toUtf8());
        clientLogins.remove(client);
      } else {
        client->write(
            QString("Error: You're not in waiting process\n").toUtf8());
      }
    } else if (tokens[0] == "stats") {
      QStringList waitingClients;
      QMap<QTcpSocket *, QString>::iterator it;
      for (it = clientLogins.begin(); it != clientLogins.end(); ++it) {
        QString login = it.value();
        waitingClients.append(login);
      }
      sendToClient(client, "Waiting clients: " + waitingClients.join(", "));

    } else if (tokens[0] == "choice") {
      if (clientLogins.contains(client)) {
        if (clientLogins.count() == maxClients) {
          if (tokens.size() == 2) {
            bool ok;
            tokens[1].toInt(&ok);
            if (ok) {
              clientChoices[client] = tokens[1].toInt();
              if (clientChoices.count() == maxClients) {
                int maxValue = -1;
                QTcpSocket *clientWinner;
                QMap<QTcpSocket *, int>::iterator it;
                for (it = clientChoices.begin(); it != clientChoices.end();
                     ++it) {
                  maxValue = std::max(it.value(), maxValue);
                  if (it.value() == maxValue) {
                    clientWinner = it.key();
                  }
                }

                QMap<QTcpSocket *, QString>::iterator ite;
                for (ite = clientLogins.begin(); ite != clientLogins.end();
                     ++ite) {
                  query.prepare("UPDATE clients SET games = games + 1 WHERE "
                                "login = :login");
                  query.bindValue(":login", ite.value());
                  query.exec();
                  if (ite.key() != clientWinner) {
                    sendToClient(ite.key(), "Loss");
                  } else {
                    query.prepare("UPDATE clients SET wins = wins + 1 WHERE "
                                  "login = :login");
                    query.bindValue(":login", ite.value());
                    query.exec();
                    sendToClient(ite.key(), "Win");
                  }
                }
                //disconnectAll();
              }
            } else {
              sendToClient(client, "Error: Value must be int");
            }
          } else {
            sendToClient(client,
                         "Error: Insufficient parameters for 'choice' command");
          }
        } else {
          sendToClient(
              client, "The server has not reached the maximum number of users");
        }
      }
    } else {
      client->write(QString("Unknown command\n").toUtf8());
    }
  }
}

void Server::disconnected() {
  QTcpSocket *client = (QTcpSocket *)sender();
  qDebug() << "Client disconnected:" << client->peerAddress().toString();

  // Delete from database
  if (clientLogins.contains(client)) {
    QString login = clientLogins[client];
    clientLogins.remove(client);
  }
  clients.removeOne(client);
  client->deleteLater();
}

void Server::disconnectAll() {
  QMap<QTcpSocket *, QString>::iterator it;
  for (it = clientLogins.begin(); it != clientLogins.end(); ++it) {
    clientLogins.remove(it.key());
    clientChoices.remove(it.key());
  }
}

void Server::sendToClient(QTcpSocket *client, const QString &message) {
  client->write(QString(message + "\n").toUtf8());
}
