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
#include "php_CException.h"
#include "php_CDbError.h"


//zend类方法
zend_function_entry CDbError_functions[] = {
	PHP_ME(CDbError,setSQLErrorCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,setDriverErrorCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,setErrorMessage,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,getSqlstatus,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,getCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,getMessage,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,setSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,getSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDbError,getAction,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDbError)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CDbError",CDbError_functions);
	CDbErrorCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_long(CDbErrorCe, ZEND_STRL("_sqlErrorCode"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CDbErrorCe, ZEND_STRL("_driverErrorCode"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CDbErrorCe, ZEND_STRL("_errorMessage"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CDbErrorCe, ZEND_STRL("_sql"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CDbErrorCe, ZEND_STRL("action"),"",ZEND_ACC_PRIVATE TSRMLS_CC);


	return SUCCESS;
}

//类方法:创建应用对象
PHP_METHOD(CDbError,setSQLErrorCode)
{
	long val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CDbError->setSQLErrorCode(int errorCode) method,  method, the parameter type errors", 9001 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_long(CDbErrorCe,getThis(),ZEND_STRL("_sqlErrorCode"),val TSRMLS_CC);


	RETVAL_TRUE;
}

PHP_METHOD(CDbError,setDriverErrorCode)
{
	long val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CDbError->setDriverErrorCode(int errorCode) method, the parameter type errors", 9002 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_long(CDbErrorCe,getThis(),ZEND_STRL("_driverErrorCode"),val TSRMLS_CC);

	RETVAL_TRUE;
}

PHP_METHOD(CDbError,setErrorMessage)
{
	char *error;
	int val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&error,&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CDbError->setErrorMessage(string errorMessage) method, the parameter type errors", 9003 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CDbErrorCe,getThis(),ZEND_STRL("_errorMessage"),error TSRMLS_CC);

	RETVAL_TRUE;
}

PHP_METHOD(CDbError,getSqlstatus)
{
	RETVAL_FALSE;
}

PHP_METHOD(CDbError,getCode)
{
	zval *returnZval;

	returnZval = zend_read_property(CDbErrorCe,getThis(),ZEND_STRL("_sqlErrorCode"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_LONG){
		RETVAL_LONG(Z_LVAL_P(returnZval));
	}else{
		RETVAL_LONG(0);
	}
}

PHP_METHOD(CDbError,getMessage)
{
	zval *returnZval;

	returnZval = zend_read_property(CDbErrorCe,getThis(),ZEND_STRL("_errorMessage"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CDbError,setSql)
{
	char *error;
	int val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&error,&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CDbError->setSql(string sql) method, the parameter type errors", 9004 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CDbErrorCe,getThis(),ZEND_STRL("_sql"),error TSRMLS_CC);

	RETVAL_TRUE;
}

PHP_METHOD(CDbError,getSql)
{
	zval *returnZval;

	returnZval = zend_read_property(CDbErrorCe,getThis(),ZEND_STRL("_sql"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CDbError,getAction)
{
	zval *returnZval;

	returnZval = zend_read_property(CDbErrorCe,getThis(),ZEND_STRL("action"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",1);
	}
}
