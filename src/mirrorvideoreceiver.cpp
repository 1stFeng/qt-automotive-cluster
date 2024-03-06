#include "mirrorvideoreceiver.h"
#include <QDebug>
#include <QDateTime>
#include <QTimerEvent>

MirrorVideoReceiver::MirrorVideoReceiver(QObject *parent) : QObject(parent)
{
    haveMirror = false;
    haveTimeout = false;
    firstFrame = false;
}

void MirrorVideoReceiver::init()
{
    qDebug("-----------MirrorVideoReceiver---init");
    m_Receiver = new FrameReceiver2();
    m_Receiver->Init(nullptr);
    m_Receiver->setUpdateCall(MirrorVideoReceiver::FrameUpdate, (void *)this);

    m_Timer = new QTimer();
    m_Timer->setInterval(10000);
    connect(m_Timer,SIGNAL(timeout()),this,SLOT(onCheckAP1Shutdown()));
    m_Timer->start();
    m_FpsTimer = new QTimer();
    m_FpsTimer->setInterval(1000);
    connect(m_FpsTimer,SIGNAL(timeout()),this, SLOT(onDumpFps()));
    m_FpsTimer->start();
}

void MirrorVideoReceiver::onDumpFps() {
    if (haveMirror) {
        qint64 cost = mFpsTimer.elapsed();
        double fps = mPostFrame / (cost / 1000.f);
        qDebug() << "---------------- Mirror fps = " << fps;
        mPostFrame = 0;
        mFpsTimer.start();
    }
}

void MirrorVideoReceiver::update(sdm::Surface *info)
{
    haveTimeout = false;
    switch (info->cmd) {
    case sdm::MSG_NOTIFY_HAS_CONTENT:
        qDebug()<<"MirrorVideoReceiver--------------startMirror"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
        haveMirror = true;
        firstFrame = true;
        mPostFrame = 0;
        mFpsTimer.start();
        // emit startMirror();
        break;
    case sdm::MSG_NOTIFY_NO_CONTENT:
        if(haveMirror) {
            qDebug()<<"MirrorVideoReceiver--------------stopMirror"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
            haveMirror = false;
            emit stopMirror();
        }
        haveMirror = false;
        mFpsTimer.elapsed();
        emit updateBufferFD(nullptr);
        emit stopMirror();
        break;
    case sdm::MSG_NOTIFY_HEART_BEAT:
        qDebug("MirrorVideoReceiver--------------HEART_BEAT");
        break;
    case sdm::MSG_NOTIFY_SURFACE_INFO:
        qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")<<"MSG_NOTIFY_SURFACE_INFO";
        if(info->prime_fd<=0) {
            qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")<<"prime_fd <= 0";
            return;
        }
        qDebug("-----MirrorVideoReceiver    info->with_modifiers = %d, info->offsets[0] = %d, info->pitches[0] = %d, info->modifiers[0] = 0x%llx",
        info->with_modifiers,
        info->offsets[0],
        info->pitches[0],
        info->modifiers[0]);
        qDebug()<<"----info->prime_fd-----"<<info->prime_fd;
        emit updateBufferFD(info);
        mPostFrame++;
        if(firstFrame) {  // start mirror on first frame available
            firstFrame = false;
            emit startMirror();
        }
        break;
    default:
        break;
    }
}

void MirrorVideoReceiver::onBufferDisplayed()
{
//    m_Receiver->onShowDone();
}

void MirrorVideoReceiver::onCheckAP1Shutdown()
{
    if(!haveMirror) {
        // qDebug("--------------onCheckAP2Shutdown  return");
        return;
    }

    if(haveTimeout) {
        emit stopMirror();
        haveMirror = false;
        return;
    }

    haveTimeout = true;
}
