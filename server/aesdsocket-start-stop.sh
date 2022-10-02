#!/bin/sh

# Tester script for assignment 5 part 2
# Start-Stop-daemon script
# Author: Tejaswini Lakshminarayan
# Ref : https://gist.github.com/RichardBronosky/b037de3e8763887b034298057200bd02

case "$1" in
  start)
  	echo -n "Starting daemon "
        start-stop-daemon -S -n aesdsocket -a usr/bin/aesdsocket -- -d
        ;;
  stop)
        echo -n "Stopping daemon "
        start-stop-daemon -K -n aesdsocket
        ;;
  *)
        echo "Usage: "$1" {start|stop}"
        exit 1
esac
exit 0
