#!/bin/bash
dirname $0
cd `dirname $0`
make clean
/usr/local/php/bin/phpize
./configure --with-php-config=/usr/local/php/bin/php-config --enable-debug
make && make install && service php-fpm restart
