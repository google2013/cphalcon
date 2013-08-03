#! /bin/sh

DIR=$(readlink -enq $(dirname $0))

if [ "$(php -r 'echo substr(PHP_VERSION, 0, 3);')" = "5.5" ]; then
	( pecl install apcu < /dev/null || ( pecl config-set preferred_state beta; pecl install apcu < /dev/null ) && phpenv config-add "$DIR/apcu.ini" ) &
else
	phpenv config-add "$DIR/apc.ini"
fi

pecl install igbinary < /dev/null &
CFLAGS="-O1 -g" pecl upgrade mongo < /dev/null &

phpenv config-add "$DIR/memcache.ini"
phpenv config-rm xdebug.ini
wait
