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
#include "php_CExec.h"


//zend类方法
zend_function_entry CExec_functions[] = {
	PHP_ME(CExce,isSuccess,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,setRow,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,rows,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,setStatus,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,setSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,getSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,lastInsertId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,isCache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,getIsMaster,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,getCastTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,setLastInsertId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,getWhere,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CExce,setWhereValue,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CExec)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CExec",CExec_functions);
	CExecCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_long(CExecCe, ZEND_STRL("rows"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CExecCe, ZEND_STRL("lastInsertId"),"0",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CExecCe, ZEND_STRL("sql"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CExecCe, ZEND_STRL("castTime"),0.00,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CExecCe, ZEND_STRL("isMaster"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CExecCe, ZEND_STRL("isFromCache"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CExecCe, ZEND_STRL("status"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CExecCe, ZEND_STRL("action"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CExecCe, ZEND_STRL("whereValue"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

//类方法:创建应用对象
PHP_METHOD(CExce,isSuccess)
{
	RETVAL_TRUE;
}

//设置影响行数
PHP_METHOD(CExce,setRow)
{
	//设置rows变量
	long changeRows = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&changeRows) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CExecException] Call CExec->setRow(int rowsNum) method, the parameter type errors", 8001 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_long(CExecCe,getThis(),ZEND_STRL("rows"),changeRows TSRMLS_CC);

	RETVAL_TRUE;
}

//返回影响行数
PHP_METHOD(CExce,rows)
{
	zval *returnZval;

	returnZval = zend_read_property(CExecCe,getThis(),ZEND_STRL("rows"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_LONG){
		RETVAL_LONG(Z_LVAL_P(returnZval));
	}else{
		RETVAL_LONG(0);
	}
}

PHP_METHOD(CExce,setStatus)
{
	int val = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CExecException] Call CExec->setStatus(bool status) method, the parameter type errors", 8002 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_long(CExecCe,getThis(),ZEND_STRL("status"),val TSRMLS_CC);

	RETVAL_TRUE;
}

PHP_METHOD(CExce,setSql)
{
	char *sql;
	int val = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&sql,&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CExecException] Call CExec->setSql(string sql) method, the parameter type errors", 8003 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CExecCe,getThis(),ZEND_STRL("sql"),sql TSRMLS_CC);

	RETVAL_TRUE;
}

PHP_METHOD(CExce,getSql)
{
	zval *returnZval;

	returnZval = zend_read_property(CExecCe,getThis(),ZEND_STRL("sql"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CExce,lastInsertId)
{
	zval *returnZval;

	returnZval = zend_read_property(CExecCe,getThis(),ZEND_STRL("lastInsertId"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CExce,isCache)
{
	RETVAL_FALSE;
}

PHP_METHOD(CExce,getIsMaster)
{
	RETVAL_FALSE;
}

PHP_METHOD(CExce,getCastTime)
{
	zval *returnZval;

	returnZval = zend_read_property(CExecCe,getThis(),ZEND_STRL("castTime"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_DOUBLE){
		RETVAL_DOUBLE(Z_DVAL_P(returnZval));
	}else{
		RETVAL_DOUBLE(0.0);
	}
}

PHP_METHOD(CExce,setLastInsertId)
{
	char *sql;
	int val = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&sql,&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CExecException] Call CExec->setLastInsertId(string lastInsertId) method, the parameter type errors", 8005 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CExecCe,getThis(),ZEND_STRL("lastInsertId"),sql TSRMLS_CC);

	RETVAL_TRUE;
}


PHP_METHOD(CExce,getWhere)
{
	zval *returnZval;

	returnZval = zend_read_property(CExecCe,getThis(),ZEND_STRL("whereValue"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CExce,setWhereValue)
{
	zval *whereValue;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&whereValue) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CExec->setWhereValue(array where) Method, parameter error", 6003 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property(CExecCe,getThis(),ZEND_STRL("whereValue"),whereValue TSRMLS_CC);
	RETVAL_TRUE;
}
