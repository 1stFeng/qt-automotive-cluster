/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
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

import QtQuick 2.10
import QtLocation 5.9
import QtPositioning 5.9
import QtMultimedia 5.9
import QtQuick.Controls 2.12
import MirrorView 1.0

import shared.Style 1.0
import shared.Sizes 1.0

import "qrc:/qml/animations" 1.0
import "../helpers" 1.0
import "qrc:/qml/utils" 1.0

Item {
    id: root

    property string nextRoad
    property string currentRoad
    property string nextTurnDistanceMeasuredIn
    property real nextTurnDistance
    property string naviGuideDirection

    state: "initial"
    states: [
        State {
            // just a map w/o any additional items in it
            name: "initial"
            PropertyChanges { target: instructions; opacity: 0 }
            PropertyChanges { target: mirrorLoader; visible: false }
        },
        State {
            name: "navigation"
            PropertyChanges { target: instructions; opacity: 1 }
            PropertyChanges { target: mirrorLoader; visible: true }
        }
    ]

    Loader {
        id: mirrorLoader
        anchors.fill: parent
        sourceComponent: mirrorPage
    }

    Component {
        id: mirrorPage
        Item {
            anchors.fill: parent
            MirrorView {
                id: mirrorView
                onFrameReady: {
                    videoOutput.update()
                }

                onMirrorStart: {
                    console.log("mirrorView---onMirrorStart")
                }

                onMirrorStop: {
                    console.log("mirrorView---onMirrorStop")
                }
            }
            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                source:mirrorView
            }
        }
    }

    Item {
        id: instructions
        visible: true
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Sizes.dp(100)
        anchors.horizontalCenter: parent.horizontalCenter
        width: Sizes.dp(660)
        height: Sizes.dp(150)

        Rectangle {
            anchors.fill: parent
            opacity: 0.5
            color: Style.theme === Style.Dark ? "black" : "white"
            radius: 10
        }

        Label {
            id: curRoadLabel
            width: Sizes.dp(450)
            height: Sizes.dp(70)
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Sizes.dp(30)
            opacity: 0.9
            font.pixelSize: Sizes.fontSizeXL
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            text: root.currentRoad
        }

        Label {
            id: nextRoadLabel
            width: Sizes.dp(450)
            height: Sizes.dp(80)
            anchors.top: curRoadLabel.bottom
            anchors.left: parent.left
            anchors.leftMargin: Sizes.dp(30)
            opacity: 0.9
            font.pixelSize: Sizes.fontSizeXL
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            text: root.nextTurnDistance
                  + " " + root.nextTurnDistanceMeasuredIn + "后进入 " + root.nextRoad
        }

        Image {
            width: Sizes.dp(80)
            height: Sizes.dp(80)
            anchors.verticalCenter: nextRoadLabel.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: Sizes.dp(30)
            source: root.naviGuideDirection !== ""
                ? Helper.localAsset(root.naviGuideDirection, Style.theme)
                : ""
        }
    }
}
