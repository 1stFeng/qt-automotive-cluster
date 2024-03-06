#ifndef MIRRORVIEW_H
#define MIRRORVIEW_H

#include <QQuickItem>
#include <QAbstractVideoSurface>
#include <QMutex>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoBuffer>
#include <QVideoFrame>
#include <QMap>
#include "mirrorvideoreceiver.h"
#include "mirrorvideobuffer.h"

class MirrorView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QAbstractVideoSurface* videoSurface READ videoSurface WRITE setVideoSurface)
public:
    MirrorView();
    void setVideoSurface(QAbstractVideoSurface* surface);

signals:
    void frameReady();
    void mirrorStart();
    void mirrorStop();

protected:
    QAbstractVideoSurface* videoSurface() const
    {
        return m_surface;
    }

private slots:
    void setWindow(QQuickWindow* win);
    void sync();
    void onBufferFDUpdate(sdm::Surface *info);

    void onFrameSwapped();
    void onMirrorStart();
    void onMirrorStop();

private:
    QAbstractVideoSurface* m_surface;
    QVideoSurfaceFormat m_format;
    sdm::Surface *m_surfaceInfo;
    QMutex mutex;

    MirrorVideoReceiver *m_Receiver;

    int m_BufferFD;
    bool m_haveMirror;
    QMap<int,QSharedPointer<QAbstractVideoBuffer>> m_Buffers;
    QMap<int,QSharedPointer<QVideoFrame>> m_Frames;
};

#endif // MIRRORVIEW_H
