#ifndef SDAPPLICATION_H
#define SDAPPLICATION_H

#include <QQmlContext>
#include <QGuiApplication>
#include "clusterdata.h"

class SDApplication : public QGuiApplication
{
    Q_OBJECT
public:
    explicit SDApplication(int &argc, char** argv);
    void initQmlContext(QQmlContext* context);
    void qmlInitDone();
signals:

private slots:
    void init();

private:
    ClusterData *m_ClusterData;
};

#endif // SDAPPLICATION_H
