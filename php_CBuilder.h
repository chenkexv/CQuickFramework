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
zend_class_entry	*CBuilderCe;


//类方法:创建应用
PHP_METHOD(CBuilder,insert);
PHP_METHOD(CBuilder,prepare);
PHP_METHOD(CBuilder,query);
PHP_METHOD(CBuilder,update);
PHP_METHOD(CBuilder,delete);
PHP_METHOD(CBuilder,select);
PHP_METHOD(CBuilder,value);
PHP_METHOD(CBuilder,from);
PHP_METHOD(CBuilder,join);
PHP_METHOD(CBuilder,on);
PHP_METHOD(CBuilder,where);
PHP_METHOD(CBuilder,groupBy);
PHP_METHOD(CBuilder,orderBy);
PHP_METHOD(CBuilder,execute);
PHP_METHOD(CBuilder,getSql);
PHP_METHOD(CBuilder,cache);
PHP_METHOD(CBuilder,beginTransaction);
PHP_METHOD(CBuilder,commit);
PHP_METHOD(CBuilder,rollback);
PHP_METHOD(CBuilder,limit);
PHP_METHOD(CBuilder,setConnCheckInterval);
PHP_METHOD(CBuilder,getLastActiveTime);
PHP_METHOD(CBuilder,destory);