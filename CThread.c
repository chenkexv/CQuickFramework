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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"


#include "php_CMyFrameExtension.h"
#include "php_CThread.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CThread_functions[] = {
	PHP_ME(CThread,run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CThread,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CThread,abort,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CThread,isWatch,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CThread)
{
	//注册CThread类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CThread",CThread_functions);
	CThreadCe = zend_register_internal_class(&funCe TSRMLS_CC);


	return SUCCESS;
}

PHP_METHOD(CThread,run)
{

	
}

PHP_METHOD(CThread,__construct)
{
	
}

PHP_METHOD(CThread,abort)
{
}

PHP_METHOD(CThread,isWatch)
{

}