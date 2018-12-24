#!/bin/sh

### BEGIN INIT INFO
# Provides: hellod
# Required-Start: $syslog
# Required-Stop: $syslog
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Short-Description: Example "hello world" service.
# Description: Example "hello world" service.
#              Simple "hello world" testbed service that executes a loop() at regular intervals
### END INIT INFO


NAME=hellod
DAEMON=/usr/local/sbin/hellod
DAEMON_OPTS="-D"


# Die on errors.
set -e

PIDFILE=/var/run/$NAME.pid
export PATH="${PATH:+$PATH:}/usr/local/sbin:/usr/sbin:/sbin"

# What to do.
case "$1" in
  start)
    echo -n "Starting daemon: "$NAME
    start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPTS
    echo "."
    ;;
  stop)
    echo -n "Stopping daemon: "$NAME
    start-stop-daemon --stop --quiet --oknodo --pidfile $PIDFILE
    echo "."
    ;;
  restart)
    echo -n "Restarting daemon: "$NAME
    start-stop-daemon --stop --quiet --oknodo --retry 30 --pidfile $PIDFILE
    start-stop-daemon --start --quiet --pidfile $PIDFILE --exec $DAEMON -- $DAEMON_OPTS
    echo "."
    ;;
  status)
    if start-stop-daemon --status --pidfile $PIDFILE ; then
      echo "$NAME is running." ;
    else
      echo "$NAME is not running." ;
    fi
    ;;
  *)
    echo "Usage: "$1" {start|stop|restart}" >&2
    exit 1
esac

exit 0
