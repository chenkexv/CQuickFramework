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
#include "php_CHashTable.h"


//zend类方法
zend_function_entry CHashTable_functions[] = {

	PHP_ME(CHashTable,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CHashTable,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHashTable,setData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,remove,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,clear,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,toString,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,toJson,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,toSerialize,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,toArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,contains,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,count,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,isEmpty,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,keys,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,values,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,sortByField,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,sort,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,ksort,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHashTable,exist,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CHashTable)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHashTable",CHashTable_functions);
	CHashTableCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CHashTableCe, ZEND_STRL("data"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CHashTableCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	return SUCCESS;
}


PHP_METHOD(CHashTable,__construct)
{
	zval	*data;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|a",&data) == FAILURE){
		return;
	}

	if(data != NULL && IS_ARRAY == Z_TYPE_P(data)){
		zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),data TSRMLS_CC);
		RETURN_TRUE;
	}else{
		zval *saveData;
		MAKE_STD_ZVAL(saveData);
		array_init(saveData);
		zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),saveData TSRMLS_CC);
		zval_ptr_dtor(&saveData);
	}
}

PHP_METHOD(CHashTable,setData)
{
	zval	*data = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|a",&data) == FAILURE){
		RETURN_FALSE;
	}

	if(data != NULL && IS_ARRAY == Z_TYPE_P(data)){
		zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),data TSRMLS_CC);
		RETURN_TRUE;
	}else{
		zval *saveData;
		MAKE_STD_ZVAL(saveData);
		array_init(saveData);
		zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),saveData TSRMLS_CC);
		zval_ptr_dtor(&saveData);
	}

	RETURN_FALSE;
}

PHP_METHOD(CHashTable,set)
{
	char	*key;
	int		keyLen = 0;
	zval	*data,
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&key,&keyLen,&data) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	zval_add_ref(&data);
	add_assoc_zval(nowData,key,data);
}

PHP_METHOD(CHashTable,get)
{
	char	*key;
	int		keyLen = 0;
	zval	*returnZval,
			*nowData,
			*defaultVal = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&key,&keyLen,&defaultVal) == FAILURE){
		RETURN_FALSE;
	}
	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	CArrayHelper_load(key,nowData,&returnZval TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(returnZval) && defaultVal != NULL){
		RETVAL_ZVAL(defaultVal,1,0);
		zval_ptr_dtor(&returnZval);
		return;
	}

	RETVAL_ZVAL(returnZval,1,1);
}

PHP_METHOD(CHashTable,remove)
{
	zval	*key,
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&key) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	
	if(IS_STRING == Z_TYPE_P(key)){
		if(zend_hash_exists(Z_ARRVAL_P(nowData),Z_STRVAL_P(key),strlen(Z_STRVAL_P(key))+1)){
			zend_hash_del(Z_ARRVAL_P(nowData),Z_STRVAL_P(key),strlen(Z_STRVAL_P(key))+1);
			RETURN_TRUE;
		}
	}else if(IS_LONG == Z_TYPE_P(key)){
		if(zend_hash_index_exists(Z_ARRVAL_P(nowData),Z_LVAL_P(key))){
			zend_hash_index_del(Z_ARRVAL_P(nowData),Z_LVAL_P(key));
			RETURN_TRUE;
		}
	}
	RETURN_FALSE;
}

PHP_METHOD(CHashTable,clear)
{
	zval *saveData;
	MAKE_STD_ZVAL(saveData);
	array_init(saveData);
	zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),saveData TSRMLS_CC);
	zval_ptr_dtor(&saveData);
}

PHP_METHOD(CHashTable,toString)
{
	zval	*nowData,
			callFunction,
			callReturn,
			*params[1];

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	//build to url query
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"http_build_query",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(callReturn)){
		RETVAL_STRING(Z_STRVAL(callReturn),1);
	}else{
		RETVAL_STRING("",1);
	}
	zval_dtor(&callReturn);
}

PHP_METHOD(CHashTable,toJson)
{
	zval	*nowData;

	char	*jsonString;

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	json_encode(nowData,&jsonString);

	RETVAL_STRING(jsonString,0);
}

PHP_METHOD(CHashTable,toSerialize)
{
	zval	*nowData,
			callFunction,
			callReturn,
			*params[1];

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	//build to url query serialize
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"serialize",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(callReturn)){
		RETVAL_STRING(Z_STRVAL(callReturn),1);
	}else{
		RETVAL_STRING("",1);
	}
	zval_dtor(&callReturn);
}

PHP_METHOD(CHashTable,toArray)
{
	zval	*nowData;
	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	RETVAL_ZVAL(nowData,1,0);
}

PHP_METHOD(CHashTable,contains)
{

	zval	*key,
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&key) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	convert_to_string(key);
	if(in_array(Z_STRVAL_P(key),nowData)){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CHashTable,count)
{
	zval	*nowData;
	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	RETVAL_LONG(zend_hash_num_elements(Z_ARRVAL_P(nowData)));
}

PHP_METHOD(CHashTable,isEmpty)
{
	zval	*nowData;
	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	if(0 == zend_hash_num_elements(Z_ARRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}

PHP_METHOD(CHashTable,keys){
	zval	*nowData,
			callFunction,
			callReturn,
			*params[1],
			*saveReturn;

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	//build to url query serialize
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"array_keys",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	saveReturn = &callReturn;
	RETVAL_ZVAL(saveReturn,1,0);
	zval_dtor(&callReturn);
}

PHP_METHOD(CHashTable,values)
{
	zval	*nowData,
			callFunction,
			callReturn,
			*params[1],
			*saveReturn;

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	//build to url query serialize
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"array_values",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	saveReturn = &callReturn;
	RETVAL_ZVAL(saveReturn,1,0);
	zval_dtor(&callReturn);
}

PHP_METHOD(CHashTable,sortByField){

	char	*key,
			*sortType;

	int		keyLen = 0,
			sortTypeLen = 0,
			sortTypeInt = 4;

	zval	*nowData,
			*returnData;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&key,&keyLen,&sortType,&sortTypeLen) == FAILURE){
		RETURN_FALSE;
	}

	if(sortTypeLen > 0){
		php_strtolower(sortType,strlen(sortType)+1);
		if(strcmp(sortType,"desc") == 0){
			sortTypeInt = 3;
		}
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	CArrayHelper_sortArrByOneField(nowData,key,sortTypeInt,&returnData TSRMLS_CC);
	zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),returnData TSRMLS_CC);
	RETVAL_ZVAL(returnData,1,1);
}

PHP_METHOD(CHashTable,sort){

	char	*sortType;

	int		sortTypeLen = 0;

	zval	functionName,
			functionReturn,
			*params[1],
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&sortType,&sortTypeLen) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"sort",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
	zval_dtor(&functionReturn);
	zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),params[0] TSRMLS_CC);
	RETVAL_ZVAL(params[0],1,1);
}

PHP_METHOD(CHashTable,ksort){
	char	*sortType;

	int		sortTypeLen = 0;

	zval	functionName,
			functionReturn,
			*params[1],
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&sortType,&sortTypeLen) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"ksort",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
	zval_dtor(&functionReturn);
	zend_update_property(CHashTableCe,getThis(),ZEND_STRL("data"),params[0] TSRMLS_CC);
	RETVAL_ZVAL(params[0],1,1);
}

PHP_METHOD(CHashTable,exist){
	zval	*key,
			*nowData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&key) == FAILURE){
		RETURN_FALSE;
	}

	nowData = zend_read_property(CHashTableCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(IS_STRING == Z_TYPE_P(key)){
		if(zend_hash_exists(Z_ARRVAL_P(nowData),Z_STRVAL_P(key),strlen(Z_STRVAL_P(key))+1)){
			RETURN_TRUE;
		}
	}else if(IS_LONG == Z_TYPE_P(key)){
		if(zend_hash_index_exists(Z_ARRVAL_P(nowData),Z_LVAL_P(key))){
			RETURN_TRUE;
		}
	}else{
		RETVAL_FALSE;
	}

	RETVAL_FALSE;
}

PHP_METHOD(CHashTable,getInstance){

	char	*key;
	int		keyLen = 0;

	zval	*selfInstace,
			**instaceSaveZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	if(keyLen == 0){
		key = "main";
	}

	selfInstace = zend_read_static_property(CHashTableCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CHashTableCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CHashTableCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CHashTableCe);

		//执行其构造器 并传入参数
		if (CHashTableCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CHashTableCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CHashTableCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}