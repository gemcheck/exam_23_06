#include "tcpserver.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  Server server;
  server.listen(QHostAddress::Any, 33333);

  return a.exec();
}
