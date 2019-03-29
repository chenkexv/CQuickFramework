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
#include "php_CException.h"
#include "php_Cache.h"


//zend类方法
zend_function_entry Cache_functions[] = {
	PHP_ME(Cache,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Cache,__construct,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,_getCacheObject,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,del,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,clear,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(Cache,showStatus,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

zend_function_entry CacheItem_functions[] = {
	PHP_ME(CacheItem,setKey,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CacheItem,getKey,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CacheItem,setValue,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CacheItem,getValue,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CacheItem,setTimeout,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CacheItem,getTimeout,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(Cache)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"Cache",Cache_functions);
	CacheCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CacheCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CacheCe, ZEND_STRL("_cacheObject"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CacheItem)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CacheItem",CacheItem_functions);
	CacheItemCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CacheItemCe, "_key",strlen("_key"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CacheItemCe, "_value",strlen("_value"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CacheItemCe, "_timeout",strlen("_timeout"),ZEND_ACC_PRIVATE TSRMLS_CC);


	return SUCCESS;
}

//获取CRequest单例对象
void Cache_getInstance(char *cacheType,zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CacheCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CacheCe);

		//执行构造器
		if (CacheCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 *paramsList[1],
				 param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],cacheType,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CacheCe->constructor->common.function_name, 1);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_update_static_property(CacheCe,ZEND_STRL("instance"),saveObject TSRMLS_CC);
		zval_ptr_dtor(&saveObject);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;
}

//类方法:创建应用对象
PHP_METHOD(Cache,getInstance)
{
	zval *instanceZval;

	char	*cacheType;
	int		cacheTypeLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&cacheType,&cacheTypeLen) == FAILURE){
		return;
	}

	if(cacheTypeLen <= 0){
		zval *cconfigInstanceZval,
			 *cacheTypeZval;

		//Load配置的cache方式
		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("CACHE.DEFAULT_CACHE",cconfigInstanceZval,&cacheTypeZval TSRMLS_CC);

		if(IS_STRING == Z_TYPE_P(cacheTypeZval) && strlen(Z_STRVAL_P(cacheTypeZval)) > 0 ){
			cacheType = estrdup(Z_STRVAL_P(cacheTypeZval));
		}else{
			cacheType = estrdup("memcache");
		}

		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&cacheTypeZval);
	}else{
		cacheType = estrdup("memcache");
	}

	Cache_getInstance(cacheType,&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	efree(cacheType);
	zval_ptr_dtor(&instanceZval);
}

PHP_METHOD(Cache,__construct)
{
	char	*cacheType;
	int		cacheTypeLen = 0;

	zval	*cconfigInstanceZval;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&cacheType,&cacheTypeLen) == FAILURE){
		return;
	}

	if(cacheTypeLen == 0){
		cacheType = "memcache";
	}

	php_strtolower(cacheType,strlen(cacheType)+1);

	if(strcmp(cacheType,"filecache") == 0){

		
	}else if(strcmp(cacheType,"memcache") == 0){

		zend_class_entry	**memcacheEntryPP,
							*memcacheEntry;

		zval	*memcacheObj,
				*serverList,
				*cacheObject;

		cacheObject = zend_read_property(CacheCe,getThis(),ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

		//查询memcache
		if(1 != extension_loaded("memcache")){
			zend_throw_exception(CacheExceptionCe, "[CacheException]The server is not currently installed Memcache extension", 10010 TSRMLS_CC);
			return;
		}

		//实例化Memcache对象
		if(zend_hash_find(EG(class_table),"cmemcache",strlen("cmemcache")+1,(void**)&memcacheEntryPP ) == FAILURE){
			zend_throw_exception(CacheExceptionCe, "[CacheException]The server is not currently installed Memcache extension", 10011 TSRMLS_CC);
			return;
		}

		memcacheEntry = *memcacheEntryPP;
		MAKE_STD_ZVAL(memcacheObj);
		object_init_ex(memcacheObj,memcacheEntry);

		//尝试调用构造器
		if (memcacheEntry->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, memcacheEntry->constructor->common.function_name, 0);
			call_user_function(NULL, &memcacheObj, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		}

		//保存变量
		zend_update_property(CacheCe,getThis(),ZEND_STRL("_cacheObject"),memcacheObj TSRMLS_CC);

		zval_ptr_dtor(&memcacheObj);

	}else{
		char tempError[256];
		sprintf(tempError,"%s%s%s","[CacheException]CMyFrame now temporarily does not support the way [",cacheType,"] cache");
		zend_throw_exception(CacheExceptionCe, tempError, 5010 TSRMLS_CC);
		return;
	}

	zval_ptr_dtor(&cconfigInstanceZval);

}

PHP_METHOD(Cache,_getCacheObject)
{

}

//设置缓存
void Cache_set(zval *object,char *key,zval *value,int timeout,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject,
			*cacheItemObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}

	//组装一个CacheItem对象
	MAKE_STD_ZVAL(cacheItemObject);
    object_init_ex(cacheItemObject,CacheItemCe);
	zend_update_property_string(CacheItemCe,cacheItemObject,"_key",strlen("_key"),key TSRMLS_CC);
	zend_update_property(CacheItemCe,cacheItemObject,"_value",strlen("_value"),value TSRMLS_CC);
	zend_update_property_long(CacheItemCe,cacheItemObject,"_timeout",strlen("_timeout"),timeout TSRMLS_CC);

	//触发setcache时HOOKS_CACHE_SET函数
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],cacheItemObject,1,0);
		CHooks_callHooks("HOOKS_CACHE_SET",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	//从对象中重新读取值
	MODULE_BEGIN
		zval	*getValue,
				*saveSer;

		getValue = zend_read_property(CacheItemCe,cacheItemObject,"_key",strlen("_key"),1 TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(getValue) && strlen(Z_STRVAL_P(getValue)) > 0 ){
			key = Z_STRVAL_P(getValue);
		}

		getValue = zend_read_property(CacheItemCe,cacheItemObject,"_value",strlen("_value"),1 TSRMLS_CC);
		//对getValue序列化
		serialize(getValue,&value);

		getValue = zend_read_property(CacheItemCe,cacheItemObject,"_timeout",strlen("_timeout"),1 TSRMLS_CC);
		if(IS_LONG == Z_TYPE_P(getValue) ){
			timeout = Z_LVAL_P(getValue);
		}
	MODULE_END
	zval_ptr_dtor(&cacheItemObject);

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[3],
				param1,
				param2,
				param3,
				*returnStatus,
				*returnBool;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"set",0);
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		paramsList[2] = &param3;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);

		ZVAL_STRING(paramsList[0],key,1);
		ZVAL_ZVAL(paramsList[1],value,1,1);
		ZVAL_LONG(paramsList[2],timeout);

		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn, 3, paramsList TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			zval_ptr_dtor(&paramsList[0]);
			zval_ptr_dtor(&paramsList[1]);
			zval_ptr_dtor(&paramsList[2]);
			return;
		}

		returnStatus = &actionReturn;
		ZVAL_ZVAL(*returnZval,returnStatus,1,0);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

//设置缓存
PHP_METHOD(Cache,set)
{
	char	*key;
	zval	*thisval,
			*val,
			*returnZval;
	int		keyLen = 0;
	long	timeout = 3600;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|l",&key,&keyLen,&thisval,&timeout) == FAILURE){
		RETVAL_FALSE;
		return;
	}


	Cache_set(getThis(),key,thisval,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

//读取缓存
void Cache_get(zval *object,char *key,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject,
			*cacheItemObject,
			*saveReturnObject,
			*unserZval;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return ;
	}

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[1],
				param1,
				*returnStatus,
				*hooksReturn;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"get",0);
		paramsList[0] = &param1;

		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],key,1);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn, 1, paramsList TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			return ;
		}
		zval_ptr_dtor(&paramsList[0]);

		returnStatus = &actionReturn;

		if(IS_BOOL == Z_TYPE_P(returnStatus) && Z_LVAL_P(returnStatus) == 0 ){
			ZVAL_BOOL(*returnZval,0);
			return ;
		}

		//组装一个CacheItem对象
		MAKE_STD_ZVAL(cacheItemObject);
		object_init_ex(cacheItemObject,CacheItemCe);
		zend_update_property_string(CacheItemCe,cacheItemObject,"_key",strlen("_key"),key TSRMLS_CC);
		zend_update_property(CacheItemCe,cacheItemObject,"_value",strlen("_value"),returnStatus TSRMLS_CC);
		zend_update_property_long(CacheItemCe,cacheItemObject,"_timeout",strlen("_timeout"),0 TSRMLS_CC);

		//触发setcache时HOOKS_CACHE_SET函数
		MODULE_BEGIN
			zval	*paramsList[1],
					param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],cacheItemObject,1,0);
			CHooks_callHooks("HOOKS_CACHE_GET",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		MODULE_END

		//从返回的对象中获取值
		hooksReturn = zend_read_property(CacheItemCe,cacheItemObject,"_value",strlen("_value"),0 TSRMLS_CC);

		zval_ptr_dtor(&cacheItemObject);

		//反序列化
		if(IS_STRING == Z_TYPE_P(hooksReturn)){
			unserialize(hooksReturn,&unserZval);
			ZVAL_ZVAL(*returnZval,unserZval,1,0);
		}else{
			ZVAL_ZVAL(*returnZval,hooksReturn,1,0);
		}
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(Cache,get)
{
	char	*key;
	zval	*returnZval;
	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	Cache_get(getThis(),key,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
	zval_ptr_dtor(&returnZval);
}

//删除缓存
void Cache_del(zval *object,char *key,int timeout,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[2],
				param1,
				param2,
				*returnStatus,
				*returnBool;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"del",0);
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],key,1);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_LONG(paramsList[1],timeout);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn, 2, paramsList TSRMLS_CC)){
				ZVAL_BOOL(*returnZval,0);
				return;
		}

		MAKE_STD_ZVAL(returnStatus);
		returnStatus = &actionReturn;
		MAKE_STD_ZVAL(returnBool);
		ZVAL_ZVAL(returnBool,returnStatus,1,0);
		ZVAL_ZVAL(*returnZval,returnBool,1,0);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(Cache,del)
{
	char	*key;
	zval	*returnZval;
	int		keyLen = 0;
	long	timeout = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&key,&keyLen,&timeout) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	Cache_del(getThis(),key,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

//清空缓存
void Cache_clear(zval *object,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*returnStatus,
				*returnBool;
		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"clear",0);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn, 0, NULL TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			return;
		}

		MAKE_STD_ZVAL(returnStatus);
		returnStatus = &actionReturn;
		MAKE_STD_ZVAL(returnBool);
		ZVAL_ZVAL(returnBool,returnStatus,1,0);
		ZVAL_ZVAL(*returnZval,returnBool,1,0);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(Cache,clear)
{
	zval	*returnZval;

	Cache_clear(getThis(),&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

PHP_METHOD(Cache,showStatus)
{

}


PHP_METHOD(CacheItem,setKey)
{
	char	*key;
	int		keyLen;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CacheException] Call [CacheItem->setKey(string key)] Parameter error", 12019 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CacheItemCe,getThis(),"_key",strlen("_key"),key TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CacheItem,getKey)
{
	zval *returnZval;

	returnZval = zend_read_property(CacheItemCe,getThis(),"_key",strlen("_key"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CacheItem,setValue)
{
	zval *val;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&val) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CacheException] Call [CacheItem->setValue(zval value)] Parameter error", 12020 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property(CacheItemCe,getThis(),"_value",strlen("_value"),val TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CacheItem,getValue)
{
	zval *returnZval;

	returnZval = zend_read_property(CacheItemCe,getThis(),"_key",strlen("_key"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CacheItem,setTimeout)
{
	long timeout;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&timeout) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CacheException] Call [CacheItem->setTimeout(int time)] Parameter error", 12020 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_long(CacheItemCe,getThis(),"_timeout",strlen("_timeout"),timeout TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CacheItem,getTimeout)
{
	zval *returnZval;

	returnZval = zend_read_property(CacheItemCe,getThis(),"_timeout",strlen("_timeout"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}
