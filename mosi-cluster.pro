TEMPLATE = app
TARGET = mosi-cluster
QT += qml quick gui core multimedia
QT += qml-private quick-private quickcontrols2-private

CONFIG += c++11 no_private_qt_headers_warning
macos: CONFIG -= app_bundle

cross_compile: {
    DESTDIR = $$PWD/bin-aarch
        UI_DIR = $$PWD/build/aarch
        MOC_DIR = $$PWD/build/aarch
        RCC_DIR = $$PWD/build/aarch
        OBJECTS_DIR = $$PWD/build/aarch

    QTPLUGIN += qeglfs
} else {
    DESTDIR = $$PWD/bin
        UI_DIR = $$PWD/build/x64
        MOC_DIR = $$PWD/build/x64
        RCC_DIR = $$PWD/build/x64
        OBJECTS_DIR = $$PWD/build/x64
}

SOURCES += \
    src/mirrorvideobuffer.cpp \
    src/mirrorvideoreceiver.cpp \
    plugins/qquickdefaultprogressbar.cpp\
    plugins/Sizes.cpp\
    plugins/Style.cpp\
    plugins/StyleDefaults.cpp\
    src/clusterdata.cpp \
    src/main.cpp \
    src/mirrorview.cpp \
    src/parsexmlfile.cpp \
    src/sdapplication.cpp

HEADERS += \
    src/mirrorvideobuffer.h \
    src/mirrorvideoreceiver.h \
    plugins/qquickdefaultprogressbar_p.h\
    plugins/Sizes.h\
    plugins/Style.h\
    plugins/StyleDefaults.h\
    src/clusterdata.h \
    src/mirrorview.h \
    src/parsexmlfile.h \
    src/sdapplication.h

INCLUDEPATH += $$PWD/plugins
INCLUDEPATH += $$PWD/depends/ipc/include/ \
               $$PWD/depends/ipc/include/CommonAPI \
               $$PWD/depends/ipc/include/CommonAPI/FDBus \
               $$PWD/depends/ipc/include/fdbus \
               $$PWD/depends/ipc/src-gen/core \
               $$PWD/depends/ipc/src-gen/fdbus \
               $$PWD/depends/sdcast/include \
               /usr/include/aarch64-linux-gnu

cross_compile: {
    LIBS += -L$$PWD/depends/ipc/lib/aarch64 -lCommonAPI -lCommonAPI-FDBus -lfdbus
    LIBS += -L$$PWD/depends/sdcast/lib/aarch64 -lSdCast
    LIBS += -L/usr/lib/aarch64-linux-gnu -ldrm

    QMAKE_RPATHDIR += :/usr/lib/aarch64-linux-gnu
} else {
    LIBS += -L$$PWD/depends/ipc/lib -lCommonAPI -lCommonAPI-FDBus -lfdbus
}

RESOURCES += \
    assets.qrc \
    qml.qrc
