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
import QtPositioning 5.9
import QtLocation 5.9
import Qt.labs.platform 1.0

import "qrc:/qml/utils" 1.0

QtObject {
    id: root
    property bool naviActive: clusterDataSource.naviActive
    property string naviIcon: clusterDataSource.naviIcon
    property string currentRoad: clusterDataSource.currentRoad
    property string nextRoad: clusterDataSource.nextRoad
    property real nextTurnDistance: clusterDataSource.nextTurnDistance
    property string nextTurnDistanceMeasuredIn: clusterDataSource.nextTurnDistanceMeasuredIn
}
