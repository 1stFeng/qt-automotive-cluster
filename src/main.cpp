/****************************************************************************
**
** Copyright (C) 2019-2020 Luxoft Sweden AB
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

#include <QFontDatabase>
#include <QtGlobal>
#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtGui/QOpenGLContext>
#include <QtQml/QQmlApplicationEngine>
#include <QtCore/QLoggingCategory>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QCommandLineParser>
#include <QtPlugin>
#include "Sizes.h"
#include "Style.h"
#include "mirrorview.h"
#include "qquickdefaultprogressbar_p.h"
#include "sdapplication.h"

#ifdef STUDIO3D_RUNTIME_INSTALLED
    #include <qstudio3dglobal.h>
#endif

int main(int argc, char *argv[])
{
//    FastShow fastShow;
//    fastShow.showFirstFrame();

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName(QStringLiteral("Mosi Cluster"));
    QCoreApplication::setOrganizationName(QStringLiteral("MOS Intelligent Connectivity Technology Co., Ltd."));
    QCoreApplication::setOrganizationDomain(QStringLiteral("www.mosi-tech.cn"));
    QCoreApplication::setApplicationVersion("0.1");
    qputenv("QT_QPA_EGLFS_INTEGRATION", "none");
    qputenv("QT_QPA_PLATFORM", "eglfs");
    setenv("QT_QPA_EGLFS_PHYSICAL_HEIGHT", "720", 1);
    setenv("QT_QPA_EGLFS_PHYSICAL_WIDTH", "1920", 1);
    setenv("QT_QPA_EGLFS_WIDTH", "1920", 1);
    setenv("QT_QPA_EGLFS_HEIGHT", "720", 1);
    setenv("QT_QPA_FONTDIR", "/usr/bin/Fonts", 1);
    setenv("QT_QPA_EGLFS_INTEGRATION", "none", 1);
    SDApplication app(argc, argv);
    // force application path, it'll update @load_path and others
    QDir::setCurrent(qApp->applicationDirPath());

    // @todo: add -c config file option and config file for it (json, xml, etc)
    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription(
                "Mosi Cluster\n\n"
                "Logging is turned off by default, to control log output please check command line "
                "options or Qt Help for QT_LOGGING_RULES environment variable.\n");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    const QCommandLineOption enableDefaultLoggingOption("verbose",
            "Enables default Qt logging filter.");
    cmdParser.addOption(enableDefaultLoggingOption);
    cmdParser.process(app);
    if (!cmdParser.isSet(enableDefaultLoggingOption)) {
//        QLoggingCategory::setFilterRules("*=false");
    }

    if (!qEnvironmentVariableIsSet("QT_QUICK_CONTROLS_STYLE_PATH")) {
        qputenv("QT_QUICK_CONTROLS_STYLE_PATH", QStringLiteral("qrc:/qml/utils/").toLocal8Bit());
    }

#ifdef STUDIO3D_RUNTIME_INSTALLED
    QSurfaceFormat::setDefaultFormat(Q3DS::surfaceFormat());
#endif

    qmlRegisterType<MirrorView>("MirrorView", 1, 0, "MirrorView");
    qmlRegisterType<QQuickDefaultProgressBar>("shared.controls", 1, 0, "NeptuneProgressBar");
    qmlRegisterUncreatableType<Sizes>("shared.Sizes", 1, 0, "Sizes", "Sizes is an attached property");
    qmlRegisterUncreatableType<Style>("shared.Style", 1, 0, "Style", "Style is an attached property");

    QQmlApplicationEngine engine;
    app.initQmlContext(engine.rootContext());
    QString importPath = QDir::currentPath()+QStringLiteral("/shared/");
    engine.addImportPath(importPath);
    engine.load(QStringLiteral("qrc:/qml/main.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
