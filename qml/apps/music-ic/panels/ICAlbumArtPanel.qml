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

import QtQuick 2.8

import shared.Style 1.0
import shared.Sizes 1.0

import "qrc:/qml/utils" 1.0
import "qrc:/qml/apps/music-ic/controls" 1.0

Item {
    id: root

    property bool musicPlaying: false
    property real musicPosition: 0.0
    property alias currentSongTitle: titleColumn.currentSongTitle
    property alias currentArtisName: titleColumn.currentArtisName
    property alias currentProgressLabel: musicProgress.progressText
    property alias albumArtSource: albumArt.source

    Item {
        id: itemDelegated
        height: Sizes.dp(240)
        width: height
        layer.enabled: true
        anchors.left: parent.left
        anchors.top: parent.top

        Image {
            id: albumArtShadow
            anchors.centerIn: parent
            width: Sizes.dp(sourceSize.width)
            height: Sizes.dp(sourceSize.height)
            source: Config.localAsset("album-art-shadow", Style.theme)
        }

        Image {
            id: albumArtUndefined
            anchors.centerIn: parent
            width: Sizes.dp(180)
            height: width
            source: Config.localAsset("album-art-placeholder", Style.theme)
            fillMode: Image.PreserveAspectCrop
        }

        Image {
            id: albumArt
            cache:false
            anchors.centerIn: parent
            width: Sizes.dp(180)
            height: width
            source: model.item !== undefined ? model.item.coverArtUrl : ""
            fillMode: Image.PreserveAspectCrop
        }
    }

    TitleColumn {
        id: titleColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: itemDelegated.bottom
        anchors.topMargin: Sizes.dp(10)
    }

    MusicProgress {
        id: musicProgress
        width: root.width
        height: Sizes.dp(100)
        anchors.top: titleColumn.bottom
        anchors.leftMargin: Sizes.dp(40)
        anchors.rightMargin: Sizes.dp(40)
        value: root.musicPosition
        progressBarLabelLeftMargin: 3
        clusterView: true
    }
}
