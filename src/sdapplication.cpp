#include "sdapplication.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QFont>

SDApplication::SDApplication(int &argc, char** argv)
    : QGuiApplication{argc,argv}
{
    m_ClusterData = new ClusterData;
    setFont(QFont("Arial"));
}

void SDApplication::initQmlContext(QQmlContext* context) {
    context->setContextProperty("clusterDataSource", m_ClusterData);
    context->engine()->addImageProvider("AlbumArt", &m_ClusterData->m_ImageProvider);
    QTimer::singleShot(10000,this,&SDApplication::init);
}

void SDApplication::init() {
    m_ClusterData->startInfoCAN();
    m_ClusterData->startInfoIPC();
    qmlInitDone();
}

void SDApplication::qmlInitDone()
{
    m_ClusterData->start();
}
