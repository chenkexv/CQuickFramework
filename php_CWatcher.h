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
zend_class_entry	*CWatcherCe;


//类方法:创建应用
PHP_METHOD(CWatcher,run);
PHP_METHOD(CWatcher,__construct);
PHP_METHOD(CWatcher,setRunNum);
PHP_METHOD(CWatcher,getRunNum);
PHP_METHOD(CWatcher,stopAll);
PHP_METHOD(CWatcher,stop);
PHP_METHOD(CWatcher,getRunList);
PHP_METHOD(CWatcher,setLogFile);

