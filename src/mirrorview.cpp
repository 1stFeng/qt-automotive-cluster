#include "mirrorview.h"
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QDateTime>
#include <QThread>

MirrorView::MirrorView()
{
    m_BufferFD = -1;
    m_haveMirror = false;
    m_surface = nullptr;
    m_Receiver = new MirrorVideoReceiver();
    connect(this, &QQuickItem::windowChanged, this, &MirrorView::setWindow);
    connect(m_Receiver,&MirrorVideoReceiver::updateBufferFD,this,&MirrorView::onBufferFDUpdate);

    connect(m_Receiver,&MirrorVideoReceiver::startMirror,this,&MirrorView::onMirrorStart);
    connect(m_Receiver,&MirrorVideoReceiver::stopMirror,this,&MirrorView::onMirrorStop);
}

void MirrorView::setVideoSurface(QAbstractVideoSurface* surface)
{
    qDebug("-------------setVideoSurface");
    if (m_surface != surface && m_surface && m_surface->isActive()) {
        m_surface->stop();
    }

    if (surface->supportedPixelFormats(QAbstractVideoBuffer::HandleType::EGLImageHandle).size() > 0) {
        qDebug("-------------setVideoSurface surpport EGLImageHandle");
    } else {
        qDebug("-------------setVideoSurface not surpport EGLImageHandle");
    }
    qDebug("-------------setVideoSurface  %p",surface);
    m_surface = surface;
}

void MirrorView::setWindow(QQuickWindow* win)
{
    qDebug()<<("-------------setWindow  thread id is ")<<QThread::currentThreadId();
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this,&MirrorView::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::frameSwapped, this,&MirrorView::onFrameSwapped, Qt::DirectConnection);
        QTimer::singleShot(500,m_Receiver,&MirrorVideoReceiver::init);
    }
}


void MirrorView::sync()
{
    QMutexLocker locker(&mutex);
    if(m_BufferFD <= 0) {
        return;
    }
    QSize size = QSize(IMAGE_WIDTH,IMAGE_HEIGHT);
    QVideoFrame::PixelFormat format = QVideoFrame::PixelFormat::Format_ARGB32;

    if(!m_Buffers.contains(m_BufferFD)) {
        QSharedPointer<QAbstractVideoBuffer> videoBuffer;
        videoBuffer.reset(new MirrorVideoBuffer(m_surfaceInfo));
        m_Buffers.insert(m_BufferFD,videoBuffer);
    }

    if(!m_Frames.contains(m_BufferFD)) {
        QSharedPointer<QVideoFrame> videoFrame;
        videoFrame.reset(new QVideoFrame(static_cast<QAbstractVideoBuffer*> (((QSharedPointer<QAbstractVideoBuffer>)m_Buffers.value(m_BufferFD)).get()),size,format));
        m_Frames.insert(m_BufferFD,videoFrame);
    }
    if (!m_surface->isActive()) {
        m_format = QVideoSurfaceFormat(size, format);
        if(m_surface->start(m_format)) {
            qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")<<("-------------sync m_surface start success");
        }else {
            qDebug()<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")<<("-------------sync m_surface start failed");
            return;
        }
    }
     m_surface->present(*m_Frames.value(m_BufferFD));
}


void MirrorView::onBufferFDUpdate(sdm::Surface *info)
{
    QMutexLocker locker(&mutex);
    if (nullptr == info) {
        m_BufferFD = -1;
    } else {
        m_BufferFD = info->prime_fd;
        m_surfaceInfo = info;
    }

    emit frameReady();
}

void MirrorView::onFrameSwapped()
{
    if(m_BufferFD>0) {
        m_Receiver->onBufferDisplayed();
    }
}

void MirrorView::onMirrorStart()
{
    emit mirrorStart();
    m_haveMirror = true;
}

void MirrorView::onMirrorStop()
{
    qDebug()<<"MirrorView emit mirrorStop"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
    emit mirrorStop();

    qDebug()<<"onMirrorStop clear m_Buffers no reset"<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz");
    QMapIterator<int,QSharedPointer<QAbstractVideoBuffer>> i(m_Buffers);
    while(i.hasNext()) {
        i.next();
        ((QSharedPointer<QAbstractVideoBuffer>)i.value()).reset();
    }
    m_Buffers.clear();

    qDebug("clear m_Frames");
    QMapIterator<int,QSharedPointer<QVideoFrame>> j(m_Frames);
    while(j.hasNext()) {
        j.next();
        ((QSharedPointer<QVideoFrame>)j.value()).reset();
    }
    m_Frames.clear();

    m_BufferFD = -1;
    m_haveMirror = false;
}
