#include <QTextCodec>
#include <QDebug>
#include <QGuiApplication>
#include "clusterdata.h"
#include "parsexmlfile.h"
#include "CommonAPI.hpp"
#include "v1/com/mosi/mosc/InfotainmentBridge.hpp"

ClusterData::ClusterData(QObject *parent)
    : QObject{parent}
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    speed_timer = new QTimer(this);
    connect(speed_timer, SIGNAL(timeout()), this, SLOT(speedUpdate()));

    epower_timer = new QTimer(this);
    connect(epower_timer, SIGNAL(timeout()), this, SLOT(epowerUpdate()));

    rotation_timer = new QTimer(this);
    connect(rotation_timer, SIGNAL(timeout()), this, SLOT(rotationUpdate()));

    temperature_timer = new QTimer(this);
    connect(temperature_timer, SIGNAL(timeout()), this, SLOT(temperatureValueUpdate()));

    mock_timer = new QTimer(this);
    connect(mock_timer, SIGNAL(timeout()), this, SLOT(mockIndexUpdate()));

    navi_timer = new QTimer(this);
    connect(navi_timer, SIGNAL(timeout()), this, SLOT(navigationUpdate()));
    init();
}

void ClusterData::init()
{
    ParseXmlFile xmlFile;
    if (xmlFile.Parse(QGuiApplication::applicationDirPath() + "/settings.xml")) {
        m_mileageKm = xmlFile.odoNumber();
        if (m_mileageKm < 0 || m_mileageKm > 99999) {
            m_mileageKm = 16888; // def
        }
        m_maxSpeed = xmlFile.maxCarSpeed();
        if (m_maxSpeed < 240 || m_maxSpeed > 320) {
            m_maxSpeed = 260; // def
        }
        m_maxRotationSpeed = xmlFile.maxRotationSpeed();
        if (m_maxRotationSpeed < 4000 || m_maxRotationSpeed > 9999) {
            m_maxRotationSpeed = 8000; // def
        }
        QString tmpSpeedString = xmlFile.carSpeed();
        splitString(tmpSpeedString, SPEED_LIST);
        QString tmpRotationString = xmlFile.rotationSpeed();
        splitString(tmpRotationString, ROTATION_LIST);
        QString tmpEpowerString = xmlFile.epower();
        splitString(tmpEpowerString, EPOWER_LIST);
        m_speedDuration = xmlFile.carSpeedDuration();
        m_rotationSpeedDuration = xmlFile.rotationSpeedDuration();
        m_epowerDuration = xmlFile.epowerDuration();
        QString tmpTemperatureString = xmlFile.waterTemperature();
        splitString(tmpTemperatureString, REMAINKM_LIST);
        QString tmpFuelTankString = xmlFile.fuelTank();
        splitString(tmpFuelTankString, REMAINPOWER_LIST);
        m_temperatureChangeCycle = xmlFile.temperatureChangeCycle();
        if (m_temperatureChangeCycle <= 0) {
            m_temperatureChangeCycle = 5; // def
        }
        mockPageSpeed = xmlFile.mockPageAniSpeed();
        m_pointerAniInterval = xmlFile.pointerAniInterval();
    }
    mockIndexList << "vehicle" << "music" << "map";
}

void ClusterData::start()
{
    speed_timer->start(m_speedDuration + m_pointerAniInterval);
    rotation_timer->start(m_rotationSpeedDuration + m_pointerAniInterval);
    epower_timer->start(m_epowerDuration + m_pointerAniInterval);
    temperature_timer->start(1000 * m_temperatureChangeCycle);
//    mock_timer->start(mockPageSpeed);
//    navi_timer->start(100);
}

void ClusterData::stop()
{
    speed_timer->stop();
    epower_timer->stop();
    rotation_timer->stop();
    temperature_timer->stop();
//    mock_timer->stop();
//    navi_timer->stop();
}

bool ClusterData::startInfoCAN()
{
    //TODO CAN相关的初始化
    return true;
}

bool ClusterData::startInfoIPC()
{
    CommonAPI::Runtime::setProperty("LogContext", "E01C");
    CommonAPI::Runtime::setProperty("LogApplication", "E01C");
    CommonAPI::Runtime::setProperty("LibraryBase", "InfotainmentBridge");

    std::shared_ptr < CommonAPI::Runtime > runtime = CommonAPI::Runtime::get();
    std::string domain = "android";
    std::string instance = "infotainmentBridge";
    std::string connection = "cluster-client";

    myProxy = runtime->buildProxy<InfotainmentBridgeProxy>(domain, instance, connection);

    std::thread* ipcThread = new std::thread([&](){
        qDebug() << "ipc proxy available test. ";
        while (!myProxy->isAvailable())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        qDebug() << "ipc proxy available. ";
        //播放状态改变
        myProxy->getPlayStatusEvent().subscribe([](const InfotainmentBridge::PlayStatus& status) {
            qDebug() << "media player status changed: " << status.getStatus() << ",current media source:" << status.getSource();
        });

        //播放进度改变,单位: 百分比
        myProxy->getCurrentProgressEvent().subscribe([this](const InfotainmentBridge::Progress& progress) {
            int currentPos = progress.getValue()/10000.f * m_duration;
            setPosition(currentPos);
        });

        //播放曲目改变
        myProxy->getSongDetailEvent().subscribe([this](const InfotainmentBridge::Song& song) {
            setTitle(QString::fromUtf8(song.getSongName().c_str()).toLocal8Bit());
            setArtist(QString::fromUtf8(song.getSinger().c_str()).toLocal8Bit());
            setDuration(song.getDuration());
            setAlbumArt(song.getAlbumArt());
        });

        //导航信息改变
        myProxy->getTbtInfoEvent().subscribe([this](const InfotainmentBridge::Tbt& tbt) {
            int currentPos = tbt.getDistanceToNext();
            if (currentPos < 1000) {
                setNextTurnDistance(QString::number(currentPos));
                setNextTurnDistanceMeasuredIn("m");
            } else {
                setNextTurnDistance(QString("%1").arg(currentPos / 1000.0f, 0, 'f', 1));
                setNextTurnDistanceMeasuredIn("km");
            }
            setNaviActive(tbt.getIconType() != 0);
            setNaviIcon("amap_navi_lbs_sou" + QString::number(tbt.getIconType()));
            setNextRoad(QString::fromUtf8(tbt.getNextRoad().c_str()).toLocal8Bit());
            setCurrentRoad(QString::fromUtf8(tbt.getCurrentRoad().c_str()).toLocal8Bit());
        });

        //mockPage改变
        myProxy->getControlPageEvent().subscribe([this](const InfotainmentBridge::PageOperation& page) {
            qDebug() << "mockpage index changed: " << page;
            switch (page) {
            case InfotainmentBridge::PageOperation::NEXT:
                setNextPage();
                break;
            case InfotainmentBridge::PageOperation::PREVIOUS:
                setPreviousPage();
                break;
            }
        });
    });
    ipcThread->detach();
    return true;
}

void ClusterData::setSpeed(int speed)
{
    m_speed = speed;
    emit speedChanged();
}

void ClusterData::setEpower(int epower)
{
    m_ePower = epower;
    emit ePowerChanged();
}

void ClusterData::setTheme(int theme)
{
    m_theme = theme;
    emit themeChanged();
}

void ClusterData::setMockPage(QString page)
{
    if (m_mockPage != page)
    {
        m_mockPage = page;
        emit mockPageChanged(page);
    }
}

void ClusterData::setRotationSpeed(int speed)
{
    m_rotationSpeed = speed;
    emit rotationSpeedChanged();
}

void ClusterData::setNaviActive(bool naviActive)
{
    m_naviActive = naviActive;
    emit naviActiveChanged();
}

void ClusterData::setNaviIcon(QString naviIcon)
{
    m_naviIcon = naviIcon;
    emit naviIconChanged();
}

void ClusterData::setCurrentRoad(QString currentRoad)
{
    m_currentRoad = currentRoad;
    emit currentRoadChanged();
}

void ClusterData::setNextRoad(QString nextRoad)
{
    m_nextRoad = nextRoad;
    emit nextRoadChanged();
}

void ClusterData::setNextTurnDistance(QString nextTurnDistance)
{
    m_nextTurnDistance = nextTurnDistance;
    emit nextTurnDistanceChanged();
}

void ClusterData::setNextTurnDistanceMeasuredIn(QString nextTurnDistanceMeasuredIn)
{
    m_nextTurnDistanceMeasuredIn = nextTurnDistanceMeasuredIn;
    emit nextTurnDistanceMeasuredInChanged();
}

void ClusterData::setTitle(QString title)
{
    m_title = title;
    emit titleChanged();
}

void ClusterData::setArtist(QString artist)
{
    m_artist = artist;
    emit artistChanged();
}

void ClusterData::setDuration(int duration)
{
    m_duration = duration;
    emit durationChanged();
}

void ClusterData::setPosition(int position)
{
    m_position = position;
    emit positionChanged();
}

void ClusterData::setDefaultAlbumArt()
{
    if (mDefaultAlbumArt.isNull())
    {
        mDefaultAlbumArt.load(":/assets/default_cover_art.png");
    }
    m_ImageProvider.mAlbumArtImage = mDefaultAlbumArt;
    if (m_ImageProvider.mAlbumArtImage.isNull())
    {
        qDebug() << "album art changed, decode failed!";
        return;
    }
    m_albumArt = "image://AlbumArt/default";
    emit albumArtChanged();
}

void ClusterData::setNextPage()
{
    int size = mockIndexList.size();
    if (size <= 0) {
        return;
    }
    mockCurrIndex++;
    mockCurrIndex = mockCurrIndex % size;
    setMockPage(mockIndexList.at(mockCurrIndex));
}

void ClusterData::setPreviousPage()
{
    int size = mockIndexList.size();
    if (size <= 0) {
        return;
    }

    mockCurrIndex--;
    if (mockCurrIndex < 0) {
        mockCurrIndex = size - 1;
    }
    mockCurrIndex = mockCurrIndex % size;
    setMockPage(mockIndexList.at(mockCurrIndex));
}

void ClusterData::setAlbumArt(const std::vector<uint8_t> &buffer)
{
    static int flag = 0;
    qDebug() << "album art changed";
    m_ImageProvider.mAlbumArtImage = QImage::fromData(buffer.data(), buffer.size()).copy();
    if (m_ImageProvider.mAlbumArtImage.isNull())
    {
        qDebug() << "album art changed, decode failed!";
        setDefaultAlbumArt();
        return;
    }

    m_albumArt = "image://AlbumArt/+" + QString::number(flag = !flag);
    emit albumArtChanged();
}

void ClusterData::setTemperatureChangeCycle(int temperatureChangeCycle)
{
    m_temperatureChangeCycle = temperatureChangeCycle;
    emit temperatureChangeCycleChanged();
}

void ClusterData::processCANMsg()
{
    //TODO 处理CAN发送的数据，更新UI
}

void ClusterData::splitString(const QString &list, ListType lType)
{
    QString list_ = list;
    switch (lType) {
    case SPEED_LIST:
        speedList = list_.split(",");
        break;
    case ROTATION_LIST:
        rotationList = list_.split(",");
        break;
    case REMAINKM_LIST:
        remainKmList = list_.split(",");
        break;
    case REMAINPOWER_LIST:
        remainPowerList = list_.split(",");
        break;
    case EPOWER_LIST:
        epowerList = list_.split(",");
        break;
    default:
        // doNothing
        break;
    }
}

void ClusterData::speedUpdate()
{
    int size = speedList.size();
    if (size <= 0) {
        return;
    }
    speedCurrIndex = speedCurrIndex % size;
    setSpeed(speedList.at(speedCurrIndex++).toInt());
}

void ClusterData::epowerUpdate()
{
    int size = epowerList.size();
    if (size <= 0) {
        return;
    }
    epowerCurrIndex = epowerCurrIndex % size;
    setEpower(epowerList.at(epowerCurrIndex++).toInt());
}

void ClusterData::rotationUpdate()
{
    int size = rotationList.size();
    if (size <= 0) {
        return;
    }
    rotationCurrIndex = rotationCurrIndex % size;
    setRotationSpeed(rotationList.at(rotationCurrIndex++).toInt());
}

void ClusterData::mockIndexUpdate()
{
    setNextPage();
}

void ClusterData::navigationUpdate()
{
    static int index = 1;
    static int distance = 1200;
    static bool initial = true;
    QStringList location = QStringList() << "人民南路" << "天府大道" << "南三环" << "锦华路" << "南二环";

    if (initial)
    {
        initial = false;
        setNextTurnDistance("1.2");
        setNextTurnDistanceMeasuredIn("km");
        setNaviActive(true);
        setNaviIcon("amap_navi_lbs_sou8");
        setCurrentRoad(location[0]);
        setNextRoad(location[1]);
    }

    if (distance <= 0)
    {
        distance = 1200;
        index++;
        if (index >= location.size()) {
            index = 0;
        }
        setCurrentRoad(m_nextRoad);
        setNextRoad(location[index]);
        setNaviIcon(QString("amap_navi_lbs_sou%1").arg(index+2));
    }
    if (distance < 1000) {
        setNextTurnDistance(QString::number(distance));
        setNextTurnDistanceMeasuredIn("m");
    } else {
        setNextTurnDistance(QString("%1").arg(distance / 1000.0f, 0, 'f', 1));
        setNextTurnDistanceMeasuredIn("km");
    }
    distance -= 10;
}

void ClusterData::temperatureValueUpdate()
{
    static int minTemperature = qMax(m_outsideTemperatureCelsius - 2, 0.f);
    static int maxTemperature = qMin(m_outsideTemperatureCelsius + 2, 99.f);
    static bool bFlag = true;
    if (bFlag) {
        if (++m_outsideTemperatureCelsius > maxTemperature) {
            bFlag = false;
            m_outsideTemperatureCelsius -= 2;
        }
    } else {
        if (--m_outsideTemperatureCelsius < minTemperature) {
            bFlag = true;
            m_outsideTemperatureCelsius += 2;
        }
    }
    setTheme(!m_theme);
    emit outsideTemperatureCelsiusChanged();
}
