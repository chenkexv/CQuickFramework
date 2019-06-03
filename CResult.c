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
#include "php_CResult.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CResult_functions[] = {
	PHP_ME(CResult,asArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setCache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setCastTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getCastTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,count,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,offsetGet,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getKey,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,current,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setValue,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setIsMaster,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getIsMaster,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setIsCache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,isCache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getAction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,getWhere,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CResult,setWhereValue,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CResult)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CResult",CResult_functions);
	CResultCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CResultCe, "instance",strlen("instance"),ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);

	//定义值
	zend_declare_property_null(CResultCe, ZEND_STRL("value"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CResultCe, ZEND_STRL("sql"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CResultCe, ZEND_STRL("castTime"),0.00,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CResultCe, ZEND_STRL("isMaster"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CResultCe, ZEND_STRL("isFromCache"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CResultCe, ZEND_STRL("action"),"SELECT",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CResultCe, ZEND_STRL("whereValue"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

//获取结果集的单例对象
void CResult_getInstance(zval **returnZval TSRMLS_DC){

	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CResultCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//查询自身类对象
		zend_hash_find(EG(class_table),"cresult",strlen("cresult")+1,(void**)&classCePP);
		classCe = *classCePP;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);

		//执行构造器
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_update_static_property(CResultCe,"instance",strlen("instance"),saveObject TSRMLS_CC);
		zval_ptr_dtor(&saveObject);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;

}

//将结果集转换成数组
PHP_METHOD(CResult,asArray)
{
	zval *val,
		 **thisVal;
	int  i,num;

	val = zend_read_property(CResultCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);


	if(IS_ARRAY != Z_TYPE_P(val)){
		array_init(return_value);
		return;
	}

	ZVAL_ZVAL(return_value,val,1,0);
}

PHP_METHOD(CResult,setCache)
{
	int isCache = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&isCache) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setCache(bool isCache) Method, parameter error", 6001 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_bool(CResultCe,getThis(),ZEND_STRL("isFromCache"),isCache TSRMLS_CC);

	RETVAL_TRUE;
}

//设置执行时间
PHP_METHOD(CResult,setCastTime)
{
	zval *val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setCastTime(float time) Method, parameter error", 6001 TSRMLS_CC);
		return;
	}

	//更新值
	if(Z_TYPE_P(val) == IS_DOUBLE){
		zend_update_property(CResultCe,getThis(),ZEND_STRL("castTime"),val TSRMLS_CC);
	}

	RETVAL_TRUE;
}

//获取执行时间
PHP_METHOD(CResult,getCastTime)
{
	zval *returnZval;

	returnZval = zend_read_property(CResultCe,getThis(),ZEND_STRL("castTime"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CResult,getAction)
{
	RETVAL_STRING("SELECT",1);
}

PHP_METHOD(CResult,count)
{
	//返回结果集数据
	zval *val;
	int num = 0;

	val = zend_read_property(CResultCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(val)){
		RETVAL_LONG(0);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	RETVAL_LONG(num);
}

//获取指定key的值
PHP_METHOD(CResult,offsetGet)
{
	long offset = 0;
	zval *val,
		 **keyZval;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&offset) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->offsetGet(int offset) Method, parameter error", 6012 TSRMLS_CC);
		return;
	}


	val = zend_read_property(CResultCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);

	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(val),offset,(void**)&keyZval)){
		RETVAL_ZVAL(*keyZval,1,0);
	}else{
		php_error_docref(NULL TSRMLS_CC,E_NOTICE,"[CResultException] Call CResult->offsetGet(int offset) the index [%d] not exists",offset);
		array_init(return_value);
	}
}

//以key重新排序
PHP_METHOD(CResult,getKey)
{
	char *key,
		 *thisKeyValChar;
	int keyLen = 0;
	zval *val,
		 **thisVal,
		 *thisSaveVal,
		 *returnZval,
		 **thisKeyVal;
	int i,num;

	array_init(return_value);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->getKey(string key) Method, parameter error", 6010 TSRMLS_CC);
		return;
	}

	val = zend_read_property(CResultCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(val)){
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
	for(i = 0 ; i < num ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(val),(void**)&thisVal);

		if(IS_ARRAY != Z_TYPE_PP(thisVal)){
			return;
		}

		//key的值
		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),key,strlen(key)+1,(void**)&thisKeyVal)){

			/*zval_add_ref(thisVal);

			if(Z_TYPE_PP(thisKeyVal) == IS_STRING){

				add_assoc_zval(return_value,Z_STRVAL_PP(thisKeyVal),*thisVal);

			}else if(Z_TYPE_PP(thisKeyVal) == IS_LONG){

				add_index_zval(return_value,Z_LVAL_PP(thisKeyVal),*thisVal);
			}*/

			zval *saveZval;
			MAKE_STD_ZVAL(saveZval);
			ZVAL_ZVAL(saveZval,*thisVal,1,0);

			if(Z_TYPE_PP(thisKeyVal) == IS_STRING){

				add_assoc_zval(return_value,Z_STRVAL_PP(thisKeyVal),saveZval);

			}else if(Z_TYPE_PP(thisKeyVal) == IS_LONG){

				add_index_zval(return_value,Z_LVAL_PP(thisKeyVal),saveZval);
			}

		}else{

			zend_throw_exception(CDbExceptionCe, "[ResultException] Call [CResult->getKey(string key)] The result set does not exist the field", 6011 TSRMLS_CC);
			return;
		}
	
		zend_hash_move_forward(Z_ARRVAL_P(val));
	}
}

//返回第一个值
PHP_METHOD(CResult,current)
{
	zval *val,
		 *returnZval,
		 **thisVal;
	int i,num;

	val = zend_read_property(CResultCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(val)){
		array_init(return_value);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	if(num == 0){
		array_init(return_value);
		return;
	}

	//获取第一个值
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
	zend_hash_get_current_data(Z_ARRVAL_P(val),(void**)&thisVal);
	RETVAL_ZVAL(*thisVal,1,0);
}

//设置值
PHP_METHOD(CResult,setValue)
{
	zval *val;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&val) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setValue(zval val) Method, parameter error", 6009 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property(CResultCe,getThis(),ZEND_STRL("value"),val TSRMLS_CC);
	RETVAL_TRUE;
}

//设置值
PHP_METHOD(CResult,setIsMaster)
{
	int useMaster = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&useMaster) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setIsMaster(bool use) Method, parameter error", 6002 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_bool(CResultCe,getThis(),ZEND_STRL("isMaster"),useMaster TSRMLS_CC);
	RETVAL_TRUE;
}

//是否来自主库
PHP_METHOD(CResult,getIsMaster)
{
	zval *returnZval;

	returnZval = zend_read_property(CResultCe,getThis(),ZEND_STRL("isMaster"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

//设置值表示是否来自缓存
PHP_METHOD(CResult,setIsCache)
{
	int useMaster = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&useMaster) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setIsCache(bool use) Method, parameter error", 6003 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_bool(CResultCe,getThis(),ZEND_STRL("isFromCache"),useMaster TSRMLS_CC);
	RETVAL_TRUE;
}

//来自缓存
PHP_METHOD(CResult,isCache)
{
	zval *returnZval;

	returnZval = zend_read_property(CResultCe,getThis(),ZEND_STRL("isFromCache"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CResult,setSql)
{
	char *sql;
	int sqlLen;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&sql,&sqlLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setSql(string sql) Method, parameter error", 6004 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CResultCe,getThis(),ZEND_STRL("sql"),sql TSRMLS_CC);
	RETVAL_TRUE;
}

//执行的SQL
PHP_METHOD(CResult,getSql)
{
	zval *returnZval;

	returnZval = zend_read_property(CResultCe,getThis(),ZEND_STRL("sql"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CResult,getWhere)
{
	zval *returnZval;

	returnZval = zend_read_property(CResultCe,getThis(),ZEND_STRL("whereValue"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CResult,setWhereValue)
{
	zval *whereValue;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&whereValue) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CResultException] Call CResult->setWhereValue(array where) Method, parameter error", 6003 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property(CResultCe,getThis(),ZEND_STRL("whereValue"),whereValue TSRMLS_CC);
	RETVAL_TRUE;
}
