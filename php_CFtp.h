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
zend_class_entry	*CFtpCe;


//类方法:创建应用
PHP_METHOD(CFtp,getInstance);
PHP_METHOD(CFtp,__construct);
PHP_METHOD(CFtp,__destruct);
PHP_METHOD(CFtp,chdir);
PHP_METHOD(CFtp,mkdir);
PHP_METHOD(CFtp,upload);
PHP_METHOD(CFtp,download);
PHP_METHOD(CFtp,getConnection);
PHP_METHOD(CFtp,deleteFile);
PHP_METHOD(CFtp,deleteDir);
PHP_METHOD(CFtp,chmod);
PHP_METHOD(CFtp,list);
PHP_METHOD(CFtp,close);



