#include <QApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Server server;
    server.start();  // 启动服务器

    return a.exec();
}
