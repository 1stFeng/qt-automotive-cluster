#ifndef MIRRORVIDEORECEIVER_H
#define MIRRORVIDEORECEIVER_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "FrameReceiver.h"
#include "SurfaceInfo_public.h"


class MirrorVideoReceiver : public QObject
{
    Q_OBJECT
public:
    explicit MirrorVideoReceiver(QObject* parent = nullptr);

public slots:
    void onBufferDisplayed();
    void init();

signals:
    void updateBufferFD(sdm::Surface * info);
    void startMirror();
    void stopMirror();

private slots:
    void onCheckAP1Shutdown();
    void onDumpFps();

private:
    static void FrameUpdate(sdm::Surface *info, update_data_t data) {
        MirrorVideoReceiver *cast = (MirrorVideoReceiver *)data;
        cast->update(info);
    }

    void update(sdm::Surface *info);

    FrameReceiver2 *m_Receiver;
    QTimer *m_Timer;
    QTimer *m_FpsTimer;
    QElapsedTimer mFpsTimer;
    int mFpsTimerID;
    bool haveMirror;
    bool haveTimeout;
    bool firstFrame;
    int mPostFrame;
};

#endif // MIRRORVIDEORECEIVER_H
