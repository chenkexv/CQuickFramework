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
#include "php_CMemcache.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CMemcache_functions[] = {
	PHP_ME(CMemcache,__construct,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMemcache,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMemcache,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMemcache,del,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMemcache,clear,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CMemcache)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CMemcache",CMemcache_functions);
	CMemcacheCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CMemcacheCe, ZEND_STRL("_cacheObject"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void CMemache_addServer(char *thishost,zval *object TSRMLS_DC)
{
	char *host,
		 *port;

	zval *cacheObject,
		 callAction,
		 callReturn,
		 *paramsList[2],
		 param1,
		 param2;

	if(strstr(thishost,":") == NULL){
		host = estrdup(thishost);
		port = "11211";
	}else{

		host = estrdup(strtok(thishost,":"));
		port = strtok(NULL,":");
	}

	cacheObject = zend_read_property(CMemcacheCe,object,ZEND_STRL("_cacheObject"),0 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		zend_throw_exception(CacheExceptionCe, "[CacheException] Internal error CQuickFramework program", 10010 TSRMLS_CC);
		return;
	}

	//调用memcache方法
	INIT_ZVAL(callAction);
	paramsList[0] = &param1;
	paramsList[1] = &param2;

	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);
	ZVAL_STRING(paramsList[0],host,1);
	ZVAL_STRING(paramsList[1],port,1);
	ZVAL_STRING(&callAction,"addServer",0);
	call_user_function(NULL, &cacheObject, &callAction, &callReturn, 2, paramsList TSRMLS_CC);

	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	efree(host);
}

//类方法:创建应用对象
PHP_METHOD(CMemcache,__construct)
{
	zval	*cconfigInstanceZval,
			*cacheList;

	zend_class_entry	**memcacheEntryPP,
							*memcacheEntry;

	zval	*memcacheObj;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//读取cacheList
	CConfig_load("CACHE.MEMORY_LIST",cconfigInstanceZval,&cacheList TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(cacheList) && IS_STRING != Z_TYPE_P(cacheList) ){
		zend_throw_exception(CacheExceptionCe, "[CacheException] No sequence Memcache server available[CONFIG->CACHE->MEMORY_LIST]", 10010 TSRMLS_CC);
		return;
	}

	//查询memcache
	if(1 != extension_loaded("memcache")){
		zend_throw_exception(CacheExceptionCe, "[CacheException] The server is not installed Memcache extension", 10010 TSRMLS_CC);
		return;
	}

	//实例化Memcache对象
	if(zend_hash_find(EG(class_table),"memcache",strlen("memcache")+1,(void**)&memcacheEntryPP ) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CacheException] The server is not installed Memcache extension", 10011 TSRMLS_CC);
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
		ZVAL_STRING(&constructVal, memcacheEntry->constructor->common.function_name, 1);
		call_user_function(NULL, &memcacheObj, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
	}

	//保存变量
	zend_update_property(CMemcacheCe,getThis(),ZEND_STRL("_cacheObject"),memcacheObj TSRMLS_CC);

	//添加服务器
	if(IS_OBJECT == Z_TYPE_P(memcacheObj)){


		//多个服务器组时
		if(IS_ARRAY == Z_TYPE_P(cacheList)){
			//遍历添加
			int i,
				m;
			zval **thisHostZval;
			m = zend_hash_num_elements(Z_ARRVAL_P(cacheList));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(cacheList));

			for(i = 0 ; i < m ; i++){
				zend_hash_get_current_data(Z_ARRVAL_P(cacheList),(void**)&thisHostZval);

				if(IS_STRING != Z_TYPE_PP(thisHostZval)){
					zend_hash_move_forward(Z_ARRVAL_P(cacheList));
					continue;
				}

				//添加服务器
				CMemache_addServer(Z_STRVAL_PP(thisHostZval),getThis() TSRMLS_CC);
				zend_hash_move_forward(Z_ARRVAL_P(cacheList));
			}
		}else if(IS_STRING == Z_TYPE_P(cacheList)){

			CMemache_addServer(Z_STRVAL_P(cacheList),getThis() TSRMLS_CC);
		}
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&cacheList);
	zval_ptr_dtor(&memcacheObj);
}

void CMemcache_get(zval *object,char *key,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CMemcacheCe,object,ZEND_STRL("_cacheObject"),0 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}


	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[1],
				param1,
				*returnStatus;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"get",0);
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],key,1);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn,1, paramsList TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			return;
		}
		returnStatus = &actionReturn;
		ZVAL_ZVAL(*returnZval,returnStatus,1,0);
		zval_ptr_dtor(&paramsList[0]);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(CMemcache,get)
{
	char	*key;
	zval	*returnZval;
	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CMemcache_get(getThis(),key,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

//设置缓存
void CMemcache_set(zval *object,char *key,zval *value,int timeout,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CMemcacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[4],
				param1,
				param2,
				param3,
				param4,
				*returnStatus;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"set",0);
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		paramsList[2] = &param3;
		paramsList[3] = &param4;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);

		ZVAL_STRING(paramsList[0],key,1);
		ZVAL_ZVAL(paramsList[1],value,1,0);
		ZVAL_LONG(paramsList[2],0);
		ZVAL_LONG(paramsList[3],timeout);

		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn, 4, paramsList TSRMLS_CC)){
				ZVAL_BOOL(*returnZval,0);
				return;
		}

		returnStatus = &actionReturn;
		ZVAL_ZVAL(*returnZval,returnStatus,1,0);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);

		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(CMemcache,set)
{
	char	*key;
	zval	*val,
			*returnZval;
	int		keyLen = 0;
	long timeout = 3600;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|l",&key,&keyLen,&val,&timeout) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CMemcache_set(getThis(),key,val,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

void CMemcache_del(zval *object,char *key,int timeout,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CMemcacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

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
				*returnStatus;
		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"delete",0);
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],key,1);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_LONG(paramsList[1],timeout);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn,2, paramsList TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			return;
		}

		MAKE_STD_ZVAL(returnStatus);
		returnStatus = &actionReturn;
		ZVAL_ZVAL(*returnZval,returnStatus,1,0);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(CMemcache,del)
{
	char	*key;
	zval	*returnZval;
	int		keyLen = 0;
	long	timeout = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&key,&keyLen,&timeout) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CMemcache_del(getThis(),key,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

void CMemcache_clear(zval *object,zval **returnZval TSRMLS_DC)
{
	zval	*cacheObject;

	MAKE_STD_ZVAL(*returnZval);

	cacheObject = zend_read_property(CMemcacheCe,object,ZEND_STRL("_cacheObject"),1 TSRMLS_CC);

	if(IS_OBJECT != Z_TYPE_P(cacheObject)){
		ZVAL_BOOL(*returnZval,0);
		return;
	}

	//使用此对象调用memcache方法
	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*returnStatus;
		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"flush",0);
		if(SUCCESS != call_user_function(NULL, &cacheObject, &actionVal, &actionReturn,0, NULL TSRMLS_CC)){
			ZVAL_BOOL(*returnZval,0);
			return;
		}

		MAKE_STD_ZVAL(returnStatus);
		returnStatus = &actionReturn;
		ZVAL_ZVAL(*returnZval,returnStatus,1,0);
		return;
	MODULE_END

	ZVAL_BOOL(*returnZval,0);
	return;
}

PHP_METHOD(CMemcache,clear)
{
	zval	*returnZval;

	CMemcache_clear(getThis(),&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}