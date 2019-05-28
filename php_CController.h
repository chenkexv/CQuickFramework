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
zend_class_entry	*CControllerCe;


//类方法:创建应用
PHP_METHOD(CController,Args);
PHP_METHOD(CController,File);
PHP_METHOD(CController,createUrl);
PHP_METHOD(CController,getView);
PHP_METHOD(CController,display);
PHP_METHOD(CController,fetch);
PHP_METHOD(CController,assign);
PHP_METHOD(CController,setTitle);
PHP_METHOD(CController,setKeyword);
PHP_METHOD(CController,setDescription);
PHP_METHOD(CController,setPageData);
PHP_METHOD(CController,__set);
PHP_METHOD(CController,showMessage);
PHP_METHOD(CController,showMessageData);
PHP_METHOD(CController,showErrorMessage);
PHP_METHOD(CController,setLanguage);


