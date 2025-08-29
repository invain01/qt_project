QT += core gui widgets network sql multimedia

TARGET = ChatAppServer
TEMPLATE = app

INCLUDEPATH += $$PWD  # 添加当前目录到头文件搜索路径

SOURCES += \
    ClientHandlerThread.cpp \
    main.cpp \
    server.cpp

HEADERS += \
    ClientHandlerThread.h \
    server.h \

FORMS += \

DISTFILES +=

RESOURCES += \

