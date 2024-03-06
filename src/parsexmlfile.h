#ifndef PARSEXMLFILE_H
#define PARSEXMLFILE_H

#include <QObject>
#include <QVariantList>
#include <QXmlStreamReader>

#define SETTINGS "settings"
#define PROCEDURE_SETTING "procedureSetting"
#define ANIMATION_SETTING "animationSetting"
#define BKG_SETTING "bkgSetting"
#define INDICATOR_SETTING "indicatorSetting"
#define INSTRUMENT_SETTING "instrumentSetting"
#define INTEGER_ENTRY "integer"
#define STRING_ENTRY "string"
#define STRING_ARRAY_ENTRY "string-array"
#define ITEM_ENTRY "item"

class ParseXmlFile : public QObject
{
    Q_OBJECT

public:
    ParseXmlFile(QObject *parent = nullptr);
    bool Parse(const QString& fileName);

public:
    int playCustomAni() const;
    int play() const;
    int frameStart() const;
    int frameEnd() const;
    int bkgChange() const;
    int cycle() const;
    int duration() const;
    int showSwitch() const;
    int showDuration() const;
    int roundSpeed() const;
    int delayTime() const;
    QVariantList indicatorList() const;
    int temperatureNumber() const;
    int odoNumber() const;
    int dteNumber() const;
    int maxCarSpeed() const;
    int maxRotationSpeed() const;
    QString carSpeed() const;
    QString rotationSpeed() const;
    int carSpeedDuration() const;
    int rotationSpeedDuration() const;
    QString waterTemperature() const;
    QString fuelTank() const;
    int temperatureChangeCycle() const;
    int odoChangeCycle() const;
    int dteChangeCycle() const;
    int waterTemperatureChangeCycle() const;
    int fuelTankChangeCycle() const;
    int indicatorActionChangeCycle() const;
    int gearSwitch() const;
    int indicatorSwitch() const;
    int pointerAniInterval() const;
    int waterTemperatureAniSpeed() const;
    int fuelTankAniSpeed() const;
    int mockPageAniSpeed() const {return mockPageAniSpeed_;}
    QString epower() const {return epower_;}
    int epowerDuration() const {return epowerDuration_;}

private:
    bool parseSetting();
    bool parseProcedureSetting();
    bool parseAnimationSetting();
    bool parseBkgSetting();
    bool parseIndicatorSetting();
    bool parseInstrumentSetting();

private:
    QXmlStreamReader xmlReader_;

    int playCustomAni_ = 0;
    int play_ = 0;
    int frameStart_ = 0;
    int frameEnd_ = 0;
    int bkgChange_ = 0;
    int cycle_ = 0;
    int duration_ = 0;
    int showSwitch_ = 0;
    int showDuration_ = 0;
    int roundSpeed_ = 0;
    int delayTime_ = 0;
    QVariantList indicatorList_;
    int temperatureNumber_ = 0;
    int timeNumber_ = 0;
    int odoNumber_ = 0;
    int dteNumber_ = 0;
    int maxCarSpeed_ = 0;
    int maxRotationSpeed_ = 0;
    QString carSpeed_ = "";
    QString rotationSpeed_ = "";
    QString waterTemperature_ = "";
    QString fuelTank_ = "";
    QString epower_ = "";
    int epowerDuration_ = 0;
    int carSpeedDuration_ = 0;
    int rotationSpeedDuration_ = 0;
    int temperatureChangeCycle_ = 0;
    int odoChangeCycle_ = 0;
    int dteChangeCycle_ = 0;
    int waterTemperatureChangeCycle_ = 0;
    int fuelTankChangeCycle_ = 0;
    int indicatorActionChangeCycle_ = 0;
    int gearSwitch_ = 0;
    int indicatorSwitch_ = 0;
    int pointerAniInterval_ = 0;
    int waterTemperatureAniSpeed_ = 0;
    int fuelTankAniSpeed_ = 0;
    int mockPageAniSpeed_ = 0;
};

#endif // PARSEXMLFILE_H
