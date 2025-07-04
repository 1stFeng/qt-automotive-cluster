/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
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

import QtQuick 2.12
import QtQuick.Window 2.12
import shared.Style 1.0
import shared.Sizes 1.0
import "qrc:/qml/utils" 1.0

Window {
    id: root
    visible: true
    width: 1920
    height: 1080
    title: qsTr("Cluster")
    color: "black"

    readonly property real mockedWindowsAcpectRatio: 1920 / 720

    Component.onCompleted: {
        root.Style.theme = Style.theme;
        clrst.behaviourInterface.navigationMode = true;
        mockedWindows.runApp("qrc:/qml/apps/map-ic/Main.qml");
    }

    ClusterRootStore {
        id:clrst
        devMode: true
    }

    Connections {
        target: clusterDataSource
        onThemeChanged: {root.Style.theme = clusterDataSource.theme }
        onAccentColorChanged: {root.Style.accentColor = clusterDataSource.accentColor }
        onMockPageChanged: {
            clrst.behaviourInterface.navigationMode = clusterDataSource.mockPage.indexOf("map") >= 0 ? true : false
            mockedWindows.runApp("qrc:/qml/apps/" + clusterDataSource.mockPage + "-ic/Main.qml");
        }
    }

    MockedWindows {
        id: mockedWindows

        clip: true
        height: Math.floor(root.width / mockedWindowsAcpectRatio) <= root.height
                ? root.width / mockedWindowsAcpectRatio
                : root.height;
        width: height * mockedWindowsAcpectRatio

        onWidthChanged: {
            root.Sizes.scale = mockedWindows.width / 1920;
        }

        anchors.centerIn: parent

        ClusterView {
            rtlMode: clusterDataSource.rtlMode
            anchors.fill: parent
            store: clrst;
        }

        Image {
            z: -1
            anchors.fill: parent
            source: Config.localAsset("instrument-cluster-bg", Style.theme)
            fillMode: Image.Stretch
        }
    }

    Launcher {
        id:lnch
        z: 2
        anchors.top: mockedWindows.top
        anchors.horizontalCenter: mockedWindows.horizontalCenter
        onAppClicked: {
            mockedWindows.runApp(appUrl)
            clrst.behaviourInterface.navigationMode = appUrl.indexOf("map") > 0 ? true : false
        }
    }
}
