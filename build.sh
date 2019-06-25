#!/bin/bash
dirname $0
cd `dirname $0`
make clean

#find out phpize
if $(command -v phpize 2>/dev/null);then
    echo "find phpize ...";
else
    echo "can not find phpize ...";
    echo "begin to search phpize ...";
    findResults=`find / -name  phpize|head -n 1`
    if $(command -v ${findResults} 2>/dev/null);then
        echo "find phpize ...";
    else
	echo "can not find phpize";
        exit 1
    fi
fi

#find out php-config
PHP_CONFIG=`which php-config|head -n 1`


./configure --with-php-config=${PHP_CONFIG} --enable-debug
make && make install && killall php-fpm && service php-fpm restart
