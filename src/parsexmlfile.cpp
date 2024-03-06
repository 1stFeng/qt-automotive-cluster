#include "parsexmlfile.h"
#include <QFile>

ParseXmlFile::ParseXmlFile(QObject *parent) : QObject(parent)
{
    xmlReader_.clear();
}

bool ParseXmlFile::Parse(const QString& fileName)
{
    QFile xmlFile(fileName);
    if (!xmlFile.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }
    xmlReader_.setDevice(&xmlFile);
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == SETTINGS) {
                return parseSetting();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    xmlFile.close();
    if (xmlReader_.hasError()) {
        return false;
    }
    return true;
}

bool ParseXmlFile::parseSetting()
{
    bool isSucc = false;
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == PROCEDURE_SETTING){
                isSucc = parseProcedureSetting();
            } else if (xmlReader_.name() == ANIMATION_SETTING) {
                isSucc = parseAnimationSetting();
            } else if (xmlReader_.name() == BKG_SETTING) {
                isSucc = parseBkgSetting();
            } else if (xmlReader_.name() == INDICATOR_SETTING) {
                isSucc = parseIndicatorSetting();
            } else if (xmlReader_.name() == INSTRUMENT_SETTING) {
                isSucc = parseInstrumentSetting();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return isSucc;
}

bool ParseXmlFile::parseProcedureSetting()
{
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == INTEGER_ENTRY) {
                if ("playCustomAni" == xmlReader_.attributes().value("name")) {
                    playCustomAni_  = xmlReader_.readElementText().toInt();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return true;
}

bool ParseXmlFile::parseAnimationSetting()
{
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == INTEGER_ENTRY) {
                if ("play" == xmlReader_.attributes().value("name")) {
                    play_  = xmlReader_.readElementText().toInt();
                } else if ("frameStart" == xmlReader_.attributes().value("name")) {
                    frameStart_ = xmlReader_.readElementText().toInt();
                } else if ("frameEnd" == xmlReader_.attributes().value("name")) {
                    frameEnd_ = xmlReader_.readElementText().toInt();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return true;
}

bool ParseXmlFile::parseBkgSetting()
{
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == INTEGER_ENTRY) {
                if ("bkgChange" == xmlReader_.attributes().value("name")) {
                    bkgChange_ = xmlReader_.readElementText().toInt();
                } else if ("cycle" == xmlReader_.attributes().value("name")) {
                    cycle_ = xmlReader_.readElementText().toInt();
                } else if ("duration" == xmlReader_.attributes().value("name")) {
                    duration_ = xmlReader_.readElementText().toInt();
                } else if ("showSwitch" == xmlReader_.attributes().value("name")) {
                    showSwitch_ = xmlReader_.readElementText().toInt();
                }else if ("showDuration" == xmlReader_.attributes().value("name")) {
                    showDuration_ = xmlReader_.readElementText().toInt();
                } else if ("roundSpeed" == xmlReader_.attributes().value("name")) {
                    roundSpeed_ = xmlReader_.readElementText().toInt();
                } else if ("delayTime" == xmlReader_.attributes().value("name")) {
                    delayTime_ = xmlReader_.readElementText().toInt();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return true;
}

bool ParseXmlFile::parseIndicatorSetting()
{
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == INTEGER_ENTRY) {
                if ("temperatureNumber" == xmlReader_.attributes().value("name")) {
                    temperatureNumber_ = xmlReader_.readElementText().toInt();
                } else if ("odoNumber" == xmlReader_.attributes().value("name")) {
                    odoNumber_ = xmlReader_.readElementText().toInt();
                } else if ("dteNumber" == xmlReader_.attributes().value("name")) {
                    dteNumber_ = xmlReader_.readElementText().toInt();
                } else if ("temperatureChangeCycle" == xmlReader_.attributes().value("name")) {
                    temperatureChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("odoChangeCycle" == xmlReader_.attributes().value("name")) {
                    odoChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("dteChangeCycle" == xmlReader_.attributes().value("name")) {
                    dteChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("indicatorActionChangeCycle" == xmlReader_.attributes().value("name")) {
                    indicatorActionChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("gearSwitch" == xmlReader_.attributes().value("name")) {
                    gearSwitch_ = xmlReader_.readElementText().toInt();
                } else if ("indicatorSwitch" == xmlReader_.attributes().value("name")) {
                    indicatorSwitch_= xmlReader_.readElementText().toInt();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            } else if (xmlReader_.name() == STRING_ARRAY_ENTRY) {
                if ("indicatorList" == xmlReader_.attributes().value("name")) {
                    xmlReader_.readNext();
                    while (!xmlReader_.atEnd()) {
                        if (xmlReader_.isEndElement()){
                            xmlReader_.readNext();
                            break;
                        }
                        if (xmlReader_.isStartElement()) {
                            if (xmlReader_.name() == ITEM_ENTRY) {
                                indicatorList_ << xmlReader_.readElementText().trimmed();
                            }
                            xmlReader_.readNext();
                        } else {
                            xmlReader_.readNext();
                        }
                    }
                }
                xmlReader_.readNext();
            } else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return true;
}

bool ParseXmlFile::parseInstrumentSetting()
{
    xmlReader_.readNext();
    while (!xmlReader_.atEnd()) {
        if (xmlReader_.isEndElement()){
            xmlReader_.readNext();
            break;
        }
        if (xmlReader_.isStartElement()) {
            if (xmlReader_.name() == INTEGER_ENTRY) {
                if ("maxCarSpeed" == xmlReader_.attributes().value("name")) {
                    maxCarSpeed_ = xmlReader_.readElementText().toInt();
                } else if ("maxRotationSpeed" == xmlReader_.attributes().value("name")) {
                    maxRotationSpeed_ = xmlReader_.readElementText().toInt();
                } else if ("carSpeedDuration" == xmlReader_.attributes().value("name")) {
                    carSpeedDuration_ = xmlReader_.readElementText().toInt();
                } else if ("rotationSpeedDuration" == xmlReader_.attributes().value("name")) {
                    rotationSpeedDuration_ = xmlReader_.readElementText().toInt();
                } else if ("waterTemperatureChangeCycle" == xmlReader_.attributes().value("name")) {
                    waterTemperatureChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("fuelTankChangeCycle" == xmlReader_.attributes().value("name")) {
                    fuelTankChangeCycle_ = xmlReader_.readElementText().toInt();
                } else if ("pointerAniInterval" == xmlReader_.attributes().value("name")) {
                    pointerAniInterval_ = xmlReader_.readElementText().toInt();
                } else if ("waterTemperatureAniSpeed" == xmlReader_.attributes().value("name")) {
                    waterTemperatureAniSpeed_ = xmlReader_.readElementText().toInt();
                } else if ("fuelTankAniSpeed" == xmlReader_.attributes().value("name")) {
                    fuelTankAniSpeed_ = xmlReader_.readElementText().toInt();
                } else if ("epowerDuration" == xmlReader_.attributes().value("name")) {
                    epowerDuration_ = xmlReader_.readElementText().toInt();
                } else if ("mockPageAniSpeed" == xmlReader_.attributes().value("name")) {
                    mockPageAniSpeed_ = xmlReader_.readElementText().toInt();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            } else if (xmlReader_.name() == STRING_ENTRY) {
                if ("carSpeed" == xmlReader_.attributes().value("name")) {
                    carSpeed_ = xmlReader_.readElementText().trimmed();
                } else if ("rotationSpeed" == xmlReader_.attributes().value("name")) {
                    rotationSpeed_ = xmlReader_.readElementText().trimmed();
                } else if ("waterTemperatureLightNum" == xmlReader_.attributes().value("name")) {
                    waterTemperature_ = xmlReader_.readElementText().trimmed();
                } else if ("fuelTankLightNum" == xmlReader_.attributes().value("name")) {
                    fuelTank_ = xmlReader_.readElementText().trimmed();
                } else if ("epower" == xmlReader_.attributes().value("name")) {
                    epower_ = xmlReader_.readElementText().trimmed();
                } else {
                    return false;
                }
                xmlReader_.readNext();
            }
            else {
                return false;
            }
        } else {
            xmlReader_.readNext();
        }
    }
    return true;
}

int ParseXmlFile::playCustomAni() const
{
    return playCustomAni_;
}

int ParseXmlFile::play() const
{
    return play_;
}

int ParseXmlFile::frameStart() const
{
    return frameStart_;
}

int ParseXmlFile::frameEnd() const
{
    return frameEnd_;
}

int ParseXmlFile::bkgChange() const
{
    return bkgChange_;
}

int ParseXmlFile::cycle() const
{
    return cycle_;
}

int ParseXmlFile::duration() const
{
    return duration_;
}

int ParseXmlFile::showSwitch() const
{
    return showSwitch_;
}

int ParseXmlFile::showDuration() const
{
    return showDuration_;
}

int ParseXmlFile::roundSpeed() const
{
    return roundSpeed_;
}

QVariantList ParseXmlFile::indicatorList() const
{
    return indicatorList_;
}

int ParseXmlFile::temperatureNumber() const
{
    return temperatureNumber_;
}

int ParseXmlFile::odoNumber() const
{
    return odoNumber_;
}

int ParseXmlFile::dteNumber() const
{
    return dteNumber_;
}

int ParseXmlFile::maxCarSpeed() const
{
    return maxCarSpeed_;
}

int ParseXmlFile::maxRotationSpeed() const
{
    return maxRotationSpeed_;
}

QString ParseXmlFile::carSpeed() const
{
    return carSpeed_;
}

QString ParseXmlFile::rotationSpeed() const
{
    return rotationSpeed_;
}

int ParseXmlFile::carSpeedDuration() const
{
    return carSpeedDuration_;
}

int ParseXmlFile::rotationSpeedDuration() const
{
    return rotationSpeedDuration_;
}

QString ParseXmlFile::waterTemperature() const
{
    return waterTemperature_;
}

QString ParseXmlFile::fuelTank() const
{
    return fuelTank_;
}

int ParseXmlFile::temperatureChangeCycle() const
{
    return temperatureChangeCycle_;
}

int ParseXmlFile::odoChangeCycle() const
{
    return odoChangeCycle_;
}

int ParseXmlFile::dteChangeCycle() const
{
    return dteChangeCycle_;
}

int ParseXmlFile::waterTemperatureChangeCycle() const
{
    return waterTemperatureChangeCycle_;
}

int ParseXmlFile::fuelTankChangeCycle() const
{
    return fuelTankChangeCycle_;
}

int ParseXmlFile::indicatorActionChangeCycle() const
{
    return indicatorActionChangeCycle_;
}

int ParseXmlFile::gearSwitch() const
{
    return gearSwitch_;
}

int ParseXmlFile::indicatorSwitch() const
{
    return indicatorSwitch_;
}

int ParseXmlFile::pointerAniInterval() const
{
    return pointerAniInterval_;
}

int ParseXmlFile::waterTemperatureAniSpeed() const
{
    return waterTemperatureAniSpeed_;
}

int ParseXmlFile::fuelTankAniSpeed() const
{
    return fuelTankAniSpeed_;
}

int ParseXmlFile::delayTime() const
{
    return delayTime_;
}
