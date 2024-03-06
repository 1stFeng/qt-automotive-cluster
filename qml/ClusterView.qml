/****************************************************************************
**
** Copyright (C) 2019 Luxoft Sweden AB
** Copyright (C) 2018 Pelagicore AG
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune 3 Cluster UI.
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

import QtQuick 2.8
import QtQuick.Window 2.2
//import application.windows 1.0

import shared.Sizes 1.0
import shared.Style 1.0

import "qrc:/qml/animations" 1.0
import "qrc:/qml/panels" 1.0
import "qrc:/qml/stores" 1.0
import "qrc:/qml/utils" 1.0

Item {
    id: root

    property RootStoreInterface store
    property bool rtlMode
    readonly property alias blinker: blinker

    Image {
        // Overlay between the ivi content and tellatales, cluster content
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Sizes.dp(390)
        source: Config.localAsset("cluster-fullscreen-overlay", Style.theme)
    }

    Loader {
        id: gaugesPanelLoader
        active: true
        source: "./panels/GaugesPanel.qml"
        opacity: 1.0
        anchors.fill: parent
        Behavior on opacity {
            DefaultNumberAnimation {}
        }
        onLoaded: {
            item.rtlMode = Qt.binding( function() {return root.rtlMode} )
            item.navigating = Qt.binding( function() {return store.behaviourInterface.navigationMode})
            item.speedTarget = Qt.binding( function() {return store.vehicleInterface.speed})
            item.speedLimit = Qt.binding( function() {return store.vehicleInterface.speedLimit})
            item.cruiseSpeed = Qt.binding( function() {return store.vehicleInterface.speedCruise})
            item.ePowerTarget = Qt.binding( function() {return store.vehicleInterface.ePower})
            item.drivetrain = Qt.binding( function() {return store.vehicleInterface.driveTrainState})
            item.clusterUIMode = Qt.binding( function() {return store.behaviourInterface.clusterUIMode})
        }
    }

    TemperatureLabel {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: Sizes.dp(40)
        anchors.topMargin: Sizes.dp(20)
        pixelSize: Sizes.dp(30)
        temperatureObject: store.externalDataInterface.outsideTemperature
    }


    TelltalesLeftPanel {
        LayoutMirroring.enabled: false
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: Sizes.dp(405)
        y: Sizes.dp(23)
        width: Sizes.dp(444)
        height: Sizes.dp(58)

        lowBeamHeadLightOn: store.vehicleInterface.lowBeamHeadlight
        highBeamHeadLightOn: store.vehicleInterface.highBeamHeadlight
        fogLightOn: store.vehicleInterface.fogLight
        stabilityControlOn: store.vehicleInterface.stabilityControl
        seatBeltFastenOn: store.vehicleInterface.seatBeltFasten
        leftTurnOn: store.vehicleInterface.leftTurn
        blinker: blinker.lit
    }

    TelltalesRightPanel {
        LayoutMirroring.enabled: false
        anchors.left: parent.horizontalCenter
        anchors.leftMargin: Sizes.dp(405)
        y: Sizes.dp(23)
        width: Sizes.dp(444)
        height: Sizes.dp(58)

        rightTurnOn: store.vehicleInterface.rightTurn
        absFailureOn: store.vehicleInterface.absFailure;
        parkingBrakeOn: store.vehicleInterface.parkBrake;
        lowTyrePressureOn: store.vehicleInterface.tyrePressureLow;
        brakeFailureOn: store.vehicleInterface.brakeFailure;
        airbagFailureOn: store.vehicleInterface.airbagFailure;
        blinker: blinker.lit
    }

    //common switch for left and right turn and safe part
    Blinker {
        id: blinker
        running: store.vehicleInterface.rightTurn || store.vehicleInterface.leftTurn
    }
}
