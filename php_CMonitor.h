/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c); 1997-2010 The PHP Group                                |
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
zend_class_entry	*CMonitorCe;


//类方法:创建应用
PHP_METHOD(CMonitor,getCPU);
PHP_METHOD(CMonitor,getMemory);
PHP_METHOD(CMonitor,getSwap);
PHP_METHOD(CMonitor,getNetworkStat);
PHP_METHOD(CMonitor,getDisk);
PHP_METHOD(CMonitor,getProcess);
PHP_METHOD(CMonitor,getLoadAverage);
PHP_METHOD(CMonitor,getNetConnection);

