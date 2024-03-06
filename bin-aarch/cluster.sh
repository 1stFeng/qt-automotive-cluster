#! /bin/sh
### BEGIN INIT INFO
# Provides:          clusterd
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:      2 3 4 5
# Short-Description: One of the first scripts to be executed. Starts or stops
#
### END INIT INFO

# disable input
export QT_QPA_EGLFS_DISABLE_INPUT=1
export QT_QPA_EGLFS_INTEGRATION=none
export QT_QPA_FONTDIR=/usr/bin/Fonts
export LD_LIBRARY_PATH=/usr/lib

/usr/bin/name_server -u tcp://172.20.2.35:60000 &
/usr/bin/mosi-cluster -platform eglfs &

