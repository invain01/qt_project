QT += core gui widgets network sql multimedia


TARGET = ChatAppClient
TEMPLATE = app

INCLUDEPATH += $$PWD  # 添加当前目录到头文件搜索路径

SOURCES += \
    SocketThread.cpp \
    chatwindow.cpp \
    loginwindow.cpp \
    main.cpp \
    personalinfomanage.cpp

HEADERS += \
    SocketThread.h \
    chatwindow.h \
    loginwindow.h \
    personalinfomanage.h

FORMS += \
    chatwindow.ui \
    loginwindow.ui \
    personalinfomanage.ui

DISTFILES +=

RESOURCES += \
    char.qrc

