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
#include "php_CSession.h"


//zend类方法
zend_function_entry CSession_functions[] = {
	PHP_ME(CSession,set,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSession,get,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSession,del,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CSession)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CSession",CSession_functions);
	CSessionCe = zend_register_internal_class(&funCe TSRMLS_CC);

	return SUCCESS;
}

//类方法:创建应用对象
PHP_METHOD(CSession,set)
{
	char	*key;
	int		keyLen;
	zval	*val,
			**array,
			*saveVal;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&key,&keyLen,&val) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	MAKE_STD_ZVAL(saveVal);
	ZVAL_ZVAL(saveVal,val,1,0);

	if( zend_hash_find(&EG(symbol_table),"_SESSION",sizeof("_SESSION"),(void**)&array) != SUCCESS ){
		RETVAL_FALSE;
		return;
	}

	if(IS_ARRAY != Z_TYPE_PP(array)){
		array_init(*array);
	}

	zend_hash_update(Z_ARRVAL_PP(array),key,strlen(key)+1,&saveVal,sizeof(zval*),NULL);
	RETVAL_TRUE;
}

PHP_METHOD(CSession,get)
{
	char	*key;
	int		keyLen;
	zval	**array,
			*saveVal;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	if( zend_hash_find(&EG(symbol_table),"_SESSION",strlen("_SESSION")+1,(void**)&array) != SUCCESS){
		RETVAL_FALSE;
		return;
	}

	if(IS_ARRAY != Z_TYPE_PP(array) ){
		RETVAL_FALSE;
		return;
	}


	//如果不是数据返回null
	if(IS_ARRAY != Z_TYPE_PP(array)){
		RETVAL_NULL();
		return;
	}

	//有key则直接返回
	if(zend_hash_exists(Z_ARRVAL_PP(array),key,strlen(key)+1)){
		zval **keyVal;
		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(array),key,strlen(key)+1,(void**)&keyVal)){
			RETVAL_ZVAL(*keyVal,1,0);
			return;
		}
	}

	//判断是否存在分割号.
	if(strstr((key),".") != NULL){

		//将key按照.分割
		zval *cutArr;
		php_explode(".",key,&cutArr);

		if(IS_ARRAY == Z_TYPE_P(cutArr) && zend_hash_num_elements(Z_ARRVAL_P(cutArr)) > 1){
			//对configsData保存的Hashtable进行遍历
			int configTableNum,
				n;

			zval *currentData,
				 **nData,
				 **resetData;

			//当前值
			MAKE_STD_ZVAL(currentData);
			ZVAL_ZVAL(currentData,*array,1,0);

			configTableNum = zend_hash_num_elements(Z_ARRVAL_P(cutArr));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(cutArr));

			for( n = 0 ; n < configTableNum ; n++){

				zend_hash_get_current_data(Z_ARRVAL_P(cutArr),(void**)&nData);
				
				if(IS_ARRAY != Z_TYPE_P(currentData)){
					zval_ptr_dtor(&cutArr);
					zval_ptr_dtor(&currentData);
					RETURN_NULL();
				}

				//不存在该层键则返回null
				if(!zend_hash_exists(Z_ARRVAL_P(currentData),Z_STRVAL_PP(nData),strlen(Z_STRVAL_PP(nData)) + 1)){
					zval_ptr_dtor(&cutArr);
					zval_ptr_dtor(&currentData);
					RETURN_NULL();
				}

					//重置cutArr的值
				if(SUCCESS == zend_hash_find(Z_ARRVAL_P(currentData),Z_STRVAL_PP(nData),Z_STRLEN_PP(nData) + 1,(void**)&resetData)){
					zval *tempZval;

					MAKE_STD_ZVAL(tempZval);
					ZVAL_ZVAL(tempZval,*resetData,1,0);
					zval_ptr_dtor(&currentData);

					MAKE_STD_ZVAL(currentData);
					ZVAL_ZVAL(currentData,tempZval,1,0);
					zval_ptr_dtor(&tempZval);
				}
				zend_hash_move_forward(Z_ARRVAL_P(cutArr));
			}

			RETVAL_ZVAL(currentData,1,0);
			zval_ptr_dtor(&currentData);
			zval_ptr_dtor(&cutArr);
			return;
		}
		zval_ptr_dtor(&cutArr);
	}

	RETVAL_NULL();

}

PHP_METHOD(CSession,del)
{
	char	*key;
	int		keyLen;
	zval	**array;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	if( zend_hash_find(&EG(symbol_table),"_SESSION",sizeof("_SESSION"),(void**)&array) != SUCCESS ){
		RETVAL_FALSE;
		return;
	}

	if(zend_hash_exists(Z_ARRVAL_PP(array),key,strlen(key)+1)){	
		zend_hash_del(Z_ARRVAL_PP(array),key,strlen(key)+1);
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}
