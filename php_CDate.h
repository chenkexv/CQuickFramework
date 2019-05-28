/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

//zend类对象
zend_class_entry	*CDateCe;


//类方法:创建应用
PHP_METHOD(CDate,getTimestampToSay);
PHP_METHOD(CDate,getTimeareaToSay);
PHP_METHOD(CDate,getAllDaysInDates);
PHP_METHOD(CDate,setLanguage);
PHP_METHOD(CDate,getNumToSay);
PHP_METHOD(CDate,getTimestampToMinute);
PHP_METHOD(CDate,getBetweenTimeAreaStamp);
PHP_METHOD(CDate,isWeekend);
PHP_METHOD(CDate,parseNginxDate);
PHP_METHOD(CDate,isLeapYear);
PHP_METHOD(CDate,daysInMonth);
PHP_METHOD(CDate,daysInYear);

