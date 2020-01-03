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


#include "php_CQuickFramework.h"
#include "php_CServiceController.h"
#include "php_CController.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CServiceController_functions[] = {
	PHP_ME(CServiceController,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CServiceController,Action_run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,run,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(CServiceController,Action_install,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CServiceController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CServiceController",CServiceController_functions);
	CServiceControllerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CServiceController,__construct){

	zval	*sapiZval;

	zend_class_entry *nowClass;

	//check is cli
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] 403 Forbidden ");
		return;
	}

	//set memlimit 
	ini_set("memory_limit","4048M");


	//检查子类包含Action_run方法,不允许子类覆盖此方法
	nowClass = Z_OBJCE_P(getThis());
	if(strcmp(nowClass->name,"CServiceController") != 0){

		//获取此类的函数列表
		int		i,h;
		char	*fname;
		ulong	ufunname;
		h = zend_hash_num_elements(&nowClass->function_table);
		zend_hash_internal_pointer_reset(&nowClass->function_table);
		for(i = 0 ; i < h ; i++){
		
			zend_hash_get_current_key(&nowClass->function_table,&fname,&ufunname,0);

			if(strcmp(fname,"action_run") == 0){
				zend_throw_exception(CServiceExceptionCe, "[CServiceException] Call [CActiveRecord->find] Parameter error", 9001 TSRMLS_CC);
				return;
			}


			zend_hash_move_forward(&nowClass->function_table);
		}
	}


	return;



	//设置守护进程模式 
#ifdef PHP_WIN32
	php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] unsupport window server ");
	return;
#else
	int s = daemon(1, 0);
	RETVAL_BOOL(s);
#endif

}


PHP_METHOD(CServiceController,Action_run)
{

}

PHP_METHOD(CServiceController,run)
{
}

PHP_METHOD(CServiceController,Action_install)
{
}