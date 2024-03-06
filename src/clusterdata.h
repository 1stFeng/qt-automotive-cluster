#ifndef CLUSTERDATA_H
#define CLUSTERDATA_H

#include <QObject>
#include <QTimer>
#include <QQuickImageProvider>
#include "v1/com/mosi/mosc/InfotainmentBridgeProxy.hpp"

/* 添加CAN信号类型定义，支持读写信号 */
#define INFO_CAN_SIGNAL(name)  void name##Changed();

#define INFO_CAN_PROPERTY(type,name) \
Q_PROPERTY(type name READ name NOTIFY name##Changed)

#define INFO_CAN_DEFINE(type,name,val) \
public: \
    type name() {return m_##name;}\
private:type m_##name = val;

/* 添加SOA数据类型定义，支持读写信号 */
#define INFO_IPC_SIGNAL(name) INFO_CAN_SIGNAL(name)
#define INFO_IPC_PROPERTY(type,name) INFO_CAN_PROPERTY(type,name)
#define INFO_IPC_DEFINE(type,name,val) INFO_CAN_DEFINE(type,name,val)

using namespace v1::com::mosi::mosc;

class ClusterData : public QObject
{
    Q_OBJECT
    /* 车速(Km/H)*/
    INFO_CAN_PROPERTY(float,speed)
    /* 车速限制(Km/H) */
    INFO_CAN_PROPERTY(float,speedLimit)
    /* 巡航车速(Km/H) */
    INFO_CAN_PROPERTY(float,speedCruise)
    /* 最大时速(Km/H) */
    INFO_CAN_PROPERTY(float,maxSpeed)
    /* 转速(rpm) */
    INFO_CAN_PROPERTY(float,rotationSpeed)
    /* 最大转速(rpm) */
    INFO_CAN_PROPERTY(float,maxRotationSpeed)
    /* 电量 Between 0 (inclusive) and 100 (inclusive) */
    INFO_CAN_PROPERTY(float,ePower)
    /* 档位
     * 0 -> Park
     * 1 -> Neutral
     * 2 -> Drive
     * 3 -> Reverse
     */
    INFO_CAN_PROPERTY(int,driveTrainState)
    /* 近光灯 */
    INFO_CAN_PROPERTY(bool,lowBeamHeadlight)
    /* 远光灯 */
    INFO_CAN_PROPERTY(bool,highBeamHeadlight)
    /* 雾灯 */
    INFO_CAN_PROPERTY(bool,fogLight)
    /* 左转向灯 */
    INFO_CAN_PROPERTY(bool,leftTurn)
    /* 右转向灯 */
    INFO_CAN_PROPERTY(bool,rightTurn)
    /* 稳定控制系统 */
    INFO_CAN_PROPERTY(bool,stabilityControl)
    /* 安全带 */
    INFO_CAN_PROPERTY(bool,seatBeltNotFastened)
    /* ABS */
    INFO_CAN_PROPERTY(bool,ABSFailure)
    /* 驻车制动器 */
    INFO_CAN_PROPERTY(bool,parkBrake)
    /* 胎压 */
    INFO_CAN_PROPERTY(bool,tyrePressureLow)
    /* 制动器故障 */
    INFO_CAN_PROPERTY(bool,brakeFailure)
    /* 安全气囊 */
    INFO_CAN_PROPERTY(bool,airbagFailure)
    /* 室外温度 - 摄氏度 */
    INFO_CAN_PROPERTY(float,outsideTemperatureCelsius)
    /* 总里程 - km */
    INFO_CAN_PROPERTY(float,mileageKm)
    /* 驾驶模式 */
    INFO_CAN_PROPERTY(int,drivingMode)
    /* 驾驶模式里程 */
    INFO_CAN_PROPERTY(int,drivingModeRangeKm)
    /* 驾驶模式里程 */
    INFO_CAN_PROPERTY(int,drivingModeECORangeKm)

    /* UI信息 */
    /* 音量 */
    INFO_CAN_PROPERTY(float,volume)
    /* 静音 */
    INFO_CAN_PROPERTY(bool,muted)
    /* -1.0 left and +1.0 right and 0 is centered */
    INFO_CAN_PROPERTY(float,balance)
    /* 仪表主题 */
    INFO_CAN_PROPERTY(int,theme)
    /* 主题颜色 */
    INFO_CAN_PROPERTY(QString,accentColor)
    /* true: 车速在左，电量在右，false则反过来*/
    INFO_CAN_PROPERTY(bool,rtlMode)
    /* 左前门 */
    INFO_CAN_PROPERTY(bool,door1Open)
    /* 右前门 */
    INFO_CAN_PROPERTY(bool,door2Open)
    /* 天窗 */
    INFO_CAN_PROPERTY(float,roofOpenProgress)
    /* 后备箱 */
    INFO_CAN_PROPERTY(bool,trunkOpen)
    /*
     * Whether the instrument cluster should be in navigation mode
     * in this mode its gauges etc will be reshaped to give more room
     * for the maps navigation content being displayed behind it
     */
    INFO_CAN_PROPERTY(bool,navigationMode)
    /** Whether both gauges should be hidden or not */
    INFO_CAN_PROPERTY(bool,hideGauges)
    /* 3D仪表 */
    INFO_CAN_PROPERTY(bool,threeDGauges)
    /* mock展示页*/
    INFO_CAN_PROPERTY(QString,mockPage)

    /* 导航信息 */
    /* 激活导航 */
    INFO_IPC_PROPERTY(bool,naviActive)
    /* 导航图标 */
    INFO_IPC_PROPERTY(QString,naviIcon)
    /* 当前路名 */
    INFO_IPC_PROPERTY(QString,currentRoad)
    /* 下一个路名 */
    INFO_IPC_PROPERTY(QString,nextRoad)
    /* 距离下一个转向的距离 */
    INFO_IPC_PROPERTY(QString,nextTurnDistance)
    /* 距离下一个转向的距离单位 - km/m */
    INFO_IPC_PROPERTY(QString,nextTurnDistanceMeasuredIn)

    /* 媒体信息 */
    /* 播放状态 */
    INFO_IPC_PROPERTY(bool,playbackState)
    /* 随机播放 */
    INFO_IPC_PROPERTY(bool,shuffle)
    /* 循环播放 */
    INFO_IPC_PROPERTY(bool,repeat)
    /* 总时长 ms*/
    INFO_IPC_PROPERTY(int,duration)
    /* 当前播放位置 ms*/
    INFO_IPC_PROPERTY(int,position)
    /* 歌曲名 */
    INFO_IPC_PROPERTY(QString,title)
    /* 歌手名 */
    INFO_IPC_PROPERTY(QString,artist)
    /* 专辑封面 */
    INFO_IPC_PROPERTY(QString,albumArt)

    /* [自定义动画使用]室外温度变化频率 */
    INFO_CAN_PROPERTY(int,temperatureChangeCycle)
    /* [自定义动画使用]车速每次变化动画时长 */
    INFO_CAN_PROPERTY(int,speedDuration)
    /* [自定义动画使用]转速每次变化动画时长 */
    INFO_CAN_PROPERTY(int,rotationSpeedDuration)
    /* [自定义动画使用]电耗每次变化动画时长 */
    INFO_CAN_PROPERTY(int,epowerDuration)

public:
    explicit ClusterData(QObject *parent = nullptr);

    INFO_CAN_DEFINE(float, speed, 0)
    INFO_CAN_DEFINE(float, speedLimit, 100)
    INFO_CAN_DEFINE(float, speedCruise, 120)
    INFO_CAN_DEFINE(float, maxSpeed, 260)
    INFO_CAN_DEFINE(float, rotationSpeed, 0)
    INFO_CAN_DEFINE(float, maxRotationSpeed, 8000)
    INFO_CAN_DEFINE(float, ePower, 100)
    INFO_CAN_DEFINE(int, driveTrainState, 0)
    INFO_CAN_DEFINE(bool, lowBeamHeadlight, true)
    INFO_CAN_DEFINE(bool, highBeamHeadlight, true)
    INFO_CAN_DEFINE(bool, fogLight, true)
    INFO_CAN_DEFINE(bool, leftTurn, true)
    INFO_CAN_DEFINE(bool, rightTurn, true)
    INFO_CAN_DEFINE(bool, stabilityControl, true)
    INFO_CAN_DEFINE(bool, seatBeltNotFastened, true)
    INFO_CAN_DEFINE(bool, ABSFailure, true)
    INFO_CAN_DEFINE(bool, parkBrake, true)
    INFO_CAN_DEFINE(bool, tyrePressureLow, true)
    INFO_CAN_DEFINE(bool, brakeFailure, true)
    INFO_CAN_DEFINE(bool, airbagFailure, true)
    INFO_CAN_DEFINE(float,outsideTemperatureCelsius, 30)
    INFO_CAN_DEFINE(float,mileageKm, 100)
    INFO_CAN_DEFINE(int,drivingMode, 0)
    INFO_CAN_DEFINE(int,drivingModeRangeKm, 0)
    INFO_CAN_DEFINE(int,drivingModeECORangeKm, 0)

    /* 导航信息 */
    INFO_IPC_DEFINE(bool,naviActive, false)
    INFO_IPC_DEFINE(QString,naviIcon, "")
    INFO_IPC_DEFINE(QString,currentRoad, "")
    INFO_IPC_DEFINE(QString,nextRoad, "")
    INFO_IPC_DEFINE(QString,nextTurnDistance, "10")
    INFO_IPC_DEFINE(QString,nextTurnDistanceMeasuredIn, "km")

    /* 媒体信息 */
    INFO_IPC_DEFINE(bool, playbackState, false)
    INFO_IPC_DEFINE(bool, shuffle, false)
    INFO_IPC_DEFINE(bool, repeat, true)
    INFO_IPC_DEFINE(int, duration, 0)
    INFO_IPC_DEFINE(int, position, 0)
    INFO_IPC_DEFINE(QString, title, "")
    INFO_IPC_DEFINE(QString, artist, "")
    INFO_IPC_DEFINE(QString, albumArt, "")

    /* UI信息 */
    INFO_CAN_DEFINE(float,volume, 1)
    INFO_CAN_DEFINE(bool,muted, false)
    INFO_CAN_DEFINE(float,balance, 1)
    INFO_CAN_DEFINE(int,theme, 0)
    INFO_CAN_DEFINE(QString,accentColor, "")
    INFO_CAN_DEFINE(bool,rtlMode, false)
    INFO_CAN_DEFINE(bool,door1Open, true)
    INFO_CAN_DEFINE(bool,door2Open, false)
    INFO_CAN_DEFINE(float,roofOpenProgress, 0)
    INFO_CAN_DEFINE(bool,trunkOpen, true)
    INFO_CAN_DEFINE(bool,navigationMode, false)
    INFO_CAN_DEFINE(bool,hideGauges, false)
    INFO_CAN_DEFINE(bool,threeDGauges, false)
    INFO_CAN_DEFINE(QString,mockPage, "")
    INFO_CAN_DEFINE(int,temperatureChangeCycle, 5)
    INFO_CAN_DEFINE(int,speedDuration, 1000)
    INFO_CAN_DEFINE(int,rotationSpeedDuration, 1000)
    INFO_CAN_DEFINE(int,epowerDuration, 1000)
    INFO_CAN_DEFINE(int,pointerAniInterval, 500)

signals:
    INFO_CAN_SIGNAL(speed)
    INFO_CAN_SIGNAL(speedLimit)
    INFO_CAN_SIGNAL(speedCruise)
    INFO_CAN_SIGNAL(maxSpeed)
    INFO_CAN_SIGNAL(rotationSpeed)
    INFO_CAN_SIGNAL(maxRotationSpeed)
    INFO_CAN_SIGNAL(ePower)
    INFO_CAN_SIGNAL(driveTrainState)
    INFO_CAN_SIGNAL(lowBeamHeadlight)
    INFO_CAN_SIGNAL(highBeamHeadlight)
    INFO_CAN_SIGNAL(fogLight)
    INFO_CAN_SIGNAL(leftTurn)
    INFO_CAN_SIGNAL(rightTurn)
    INFO_CAN_SIGNAL(stabilityControl)
    INFO_CAN_SIGNAL(seatBeltNotFastened)
    INFO_CAN_SIGNAL(ABSFailure)
    INFO_CAN_SIGNAL(parkBrake)
    INFO_CAN_SIGNAL(tyrePressureLow)
    INFO_CAN_SIGNAL(brakeFailure)
    INFO_CAN_SIGNAL(airbagFailure)
    INFO_CAN_SIGNAL(outsideTemperatureCelsius)
    INFO_CAN_SIGNAL(temperatureChangeCycle)
    INFO_CAN_SIGNAL(mileageKm)
    INFO_CAN_SIGNAL(drivingMode)
    INFO_CAN_SIGNAL(drivingModeRangeKm)
    INFO_CAN_SIGNAL(drivingModeECORangeKm)

    /* 导航信息 */
    INFO_IPC_SIGNAL(naviActive)
    INFO_IPC_SIGNAL(naviIcon)
    INFO_IPC_SIGNAL(currentRoad)
    INFO_IPC_SIGNAL(nextRoad)
    INFO_IPC_SIGNAL(nextTurnDistance)
    INFO_IPC_SIGNAL(nextTurnDistanceMeasuredIn)

    /* 媒体信息 */
    INFO_IPC_SIGNAL(playbackState)
    INFO_IPC_SIGNAL(shuffle)
    INFO_IPC_SIGNAL(repeat)
    INFO_IPC_SIGNAL(duration)
    INFO_IPC_SIGNAL(position)
    INFO_IPC_SIGNAL(title)
    INFO_IPC_SIGNAL(artist)
    INFO_IPC_SIGNAL(albumArt)

    /* UI信息 */
    INFO_CAN_SIGNAL(volume)
    INFO_CAN_SIGNAL(muted)
    INFO_CAN_SIGNAL(balance)
    INFO_CAN_SIGNAL(theme)
    INFO_CAN_SIGNAL(accentColor)
    INFO_CAN_SIGNAL(rtlMode)
    INFO_CAN_SIGNAL(door1Open)
    INFO_CAN_SIGNAL(door2Open)
    INFO_CAN_SIGNAL(roofOpenProgress)
    INFO_CAN_SIGNAL(trunkOpen)
    INFO_CAN_SIGNAL(navigationMode)
    INFO_CAN_SIGNAL(hideGauges)
    INFO_CAN_SIGNAL(threeDGauges)
    INFO_CAN_SIGNAL(speedDuration)
    INFO_CAN_SIGNAL(rotationSpeedDuration)
    INFO_CAN_SIGNAL(epowerDuration)
    void mockPageChanged(QString mockPage);

public:
    enum ListType {
        SPEED_LIST,
        ROTATION_LIST,
        EPOWER_LIST,
        REMAINKM_LIST,
        REMAINPOWER_LIST
    };

    /* 专辑封面 */
    class ImageProvider : public QQuickImageProvider
    {
    public:
        explicit ImageProvider(): QQuickImageProvider(QQmlImageProviderBase::Image){}
        QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize) override
        {
            Q_UNUSED(id)
            Q_UNUSED(size)
            Q_UNUSED(requestedSize)
            return mAlbumArtImage;
        }
        QPixmap requestPixmap(const QString &id, QSize *size, const QSize& requestedSize) override
        {
            Q_UNUSED(id)
            Q_UNUSED(size)
            Q_UNUSED(requestedSize)
            return QPixmap::fromImage(mAlbumArtImage);
        }
        QImage mAlbumArtImage;
    } m_ImageProvider;

    void init();
    void start();
    void stop();
    Q_INVOKABLE bool startInfoCAN();
    Q_INVOKABLE bool startInfoIPC();
    void setSpeed(int speed);
    void setEpower(int epower);
    void setTheme(int theme);
    void setMockPage(QString page);
    void setRotationSpeed(int speed);
    void setNaviActive(bool naviActive);
    void setNaviIcon(QString naviIcon);
    void setCurrentRoad(QString currentRoad);
    void setNextRoad(QString nextRoad);
    void setNextTurnDistance(QString nextTurnDistance);
    void setNextTurnDistanceMeasuredIn(QString nextTurnDistanceMeasuredIn);
    void setDefaultAlbumArt();
    void setNextPage();
    void setPreviousPage();
    void setTitle(QString title);
    void setArtist(QString artist);
    void setDuration(int duration);
    void setPosition(int position);
    void setAlbumArt(const std::vector<uint8_t>& buffer);
    void setTemperatureChangeCycle(int temperatureChangeCycle);

public slots:
    void speedUpdate();
    void epowerUpdate();
    void rotationUpdate();
    void mockIndexUpdate();
    void navigationUpdate();
    void temperatureValueUpdate();

private:
    void processCANMsg();
    void splitString(const QString& list, ListType lType);

    QTimer *speed_timer = nullptr;
    QTimer *epower_timer = nullptr;
    QTimer *rotation_timer = nullptr;
    QTimer *temperature_timer = nullptr; // 车外温度
    QTimer *mock_timer = nullptr;        // map - music - vehicle
    QTimer *navi_timer = nullptr;
    QStringList speedList;
    QStringList epowerList;
    QStringList rotationList;
    QStringList remainKmList;
    QStringList remainPowerList;
    QStringList mockIndexList;

    int speedCurrIndex = 0;
    int rotationCurrIndex = 0;
    int epowerCurrIndex = 0;
    int mockCurrIndex = 0;
    int mockPageSpeed = 0;

    QImage mDefaultAlbumArt;
    std::shared_ptr<InfotainmentBridgeProxy<>> myProxy;
};

#endif // CLUSTERDATA_H



