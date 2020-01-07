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
#include <zend_exceptions.h>


#include "php_CQuickFramework.h"
#include "php_CWebApp.h"
#include <string.h>

//拼接字符串
char *contact(char *s1,char *s2)  
{  
	char *result = emalloc(strlen(s1)+strlen(s2)+1);
    if (result == NULL) exit (1);  
  
    strcpy(result, s1);  
    strcat(result, s2);  
  
    return result; 
} 

size_t strcat2(char **dst_out, ...)
{
    size_t len = 0, len_sub;
    va_list argp;
    char *src;
    char *dst = NULL, *dst_p;

    *dst_out = NULL;

    va_start(argp, dst_out);
    for (;;)
    {
        if ((src = va_arg(argp, char *)) == NULL) break;
        len += strlen(src);
    }
    va_end(argp);

    if (len == 0) return 0;


    dst = (char *)emalloc(sizeof(char) * (len + 1));

	if(sizeof(char) * (len + 1) > 65535){
		TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CQuickFrameworkFatalError] Can not malloc more memory by %d",sizeof(char) * (len + 1));
		return 0;
	}

    if (dst == NULL){
		TSRMLS_FETCH();
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CQuickFrameworkFatalError] Can not malloc more memory");
		return 0;
	}
    dst_p = dst;

    va_start(argp, dst_out);
    for (;;)
    {
        if ((src = va_arg(argp, char *)) == NULL) break;
        len_sub = strlen(src);
        memcpy(dst_p, src, len_sub);
        dst_p += len_sub;
    }
    va_end(argp);
    *dst_p = '\0';

    *dst_out = dst;

    return len;
}


//返回zval类对象
void getClassObject(char *thisClassName,zval **returnZval TSRMLS_DC)
{
	zval *object;
	char *className;
	zend_class_entry	**thisClassEpp,
						*thisClassEp;

	className = estrdup(thisClassName);

	//转成小写
	php_strtolower(className,strlen(className)+1);

	//查询类
	if(zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&thisClassEpp) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CQuickFrameworkFatalError] Class CApplication not Found");
	}

	//实例化至Zval结构体
	thisClassEp = *thisClassEpp;
	MAKE_STD_ZVAL(object);
	object_init_ex(object,thisClassEp);

	//执行其构造器 并传入参数
	if (thisClassEp->constructor) {
		zval	constructReturn,
				constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, thisClassEp->constructor->common.function_name, 0);
		call_user_function(NULL, &object, &constructVal, &constructReturn, 0,NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,object,1,0);
	zval_ptr_dtor(&object);
}

void register_cmyframe_begin(int module_number TSRMLS_DC){

	MODULE_BEGIN
		zval	**startTimeZval,
				*startTime,
				*systemInitZval;


		MAKE_STD_ZVAL(systemInitZval);
		array_init(systemInitZval);
		microtimeTrue(&startTime);
		add_assoc_zval_ex(systemInitZval,"frameBegin",strlen("frameBegin")+1,startTime);
		zend_hash_add(&EG(symbol_table),"SYSTEM_INIT",strlen("SYSTEM_INIT")+1, &systemInitZval,sizeof(zval*),NULL);
	MODULE_END
}


//定义宏
void register_cmyframe_macro(int module_number TSRMLS_DC)
{
	
	//Hooks宏
	REGISTER_STRING_CONSTANT("HOOKS_ROUTE_START",HOOK_ROUTE_START,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_ROUTE_END",HOOKS_ROUTE_END,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_ROUTE_ERROR",HOOKS_ROUTE_ERROR,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_CONTROLLER_INIT",HOOKS_CONTROLLER_INIT,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_ACTION_INIT",HOOKS_ACTION_INIT,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_EXECUTE_BEFORE",HOOKS_EXECUTE_BEFORE,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_EXECUTE_END",HOOKS_EXECUTE_END,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_EXECUTE_ERROR",HOOKS_EXECUTE_ERROR,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_ERROR_HAPPEN",HOOKS_ERROR_HAPPEN,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_EXCEPTION_HAPPEN",HOOKS_EXCEPTION_HAPPEN,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_SYSTEM_SHUTDOWN",HOOKS_SYSTEM_SHUTDOWN,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_CACHE_SET",HOOKS_CACHE_SET,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_CACHE_GET",HOOKS_CACHE_GET,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_LOADER_START",HOOKS_LOADER_START,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_VIEW_GET",HOOKS_VIEW_GET,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_VIEW_SHOW",HOOKS_VIEW_SHOW,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_URL_CREATE",HOOKS_URL_CREATE,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_AUTO_LOAD",HOOKS_AUTO_LOAD,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_MAIL_BEFORE",HOOKS_MAIL_BEFORE,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_MONITOR_END",HOOKS_MONITOR_END,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_SAFE_STOP",HOOKS_SAFE_STOP,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_BASH_BEFORE",HOOKS_BASH_BEFORE,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_BASH_END",HOOKS_BASH_END,CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POOL_REJECT_WAIT",1,CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POOL_REJECT_THROWEXCEPTION",2,CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POOL_REJECT_DISCARD",3,CONST_CS|CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("HOOKS_CCONSUMER_MESSAGE_BEFORE",HOOKS_CCONSUMER_MESSAGE_BEFORE,CONST_CS|CONST_PERSISTENT);
	
}


//获取SERVER中的变量
void getServerParam(char *key,char **getStr TSRMLS_DC)
{

	zval	**SERVER,
			**ret,
			**otherData;


	//判断是否已注册全局变量
#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
	zend_bool jit_init = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
#else
	zend_bool jit_init = (PG(auto_globals_jit));
#endif

    if (jit_init) 
	{ 
		zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
    }   

	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);

	//检索zend内部hash表
	if (zend_hash_find(Z_ARRVAL_PP(SERVER),key,strlen(key) + 1,(void **)&ret) == FAILURE )
	{
		*getStr = NULL;
		return;
	}else if(Z_TYPE_PP(ret) == IS_STRING){

		*getStr = estrdup(Z_STRVAL_PP(ret));
		return;
	}

	*getStr = NULL;
	return;
}

void printfBase64(char *tips){
	char *decode;
	base64Decode(tips,&decode);
	php_printf("%s",decode);
	efree(decode);
}

void setServerParam(char *key,zval *params TSRMLS_DC)
{

	zval	**SERVER,
			*saveParams;


	//判断是否已注册全局变量
#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
	zend_bool jit_init = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
#else
	zend_bool jit_init = (PG(auto_globals_jit));
#endif

    if (jit_init){ 
		zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
    }   

	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);

	if(IS_ARRAY != Z_TYPE_PP(SERVER)){
		return;
	}

	MAKE_STD_ZVAL(saveParams);
	ZVAL_ZVAL(saveParams,params,1,0);
	add_assoc_zval(*SERVER,key,saveParams);

}

//获取GET中的变量
void getGetParam(char *key,char **getStr TSRMLS_DC)
{
	zval	**SERVER,
			**ret,
			**otherData;

	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_GET"), (void **)&SERVER);

	//检索zend内部hash表
	if (zend_hash_find(Z_ARRVAL_PP(SERVER),key,strlen(key) + 1,(void **)&ret) == FAILURE )
	{
		*getStr = NULL;
		return;
	}
	else if(Z_TYPE_PP(ret) == IS_STRING)
	{
		*getStr = estrdup(Z_STRVAL_P(*ret));
		return;
	}

	*getStr = NULL;
	return;
}

//获取GET中的变量
void delGetParam(char *key TSRMLS_DC)
{
	zval	**SERVER,
			**ret,
			**otherData;

	if(zend_hash_find(&EG(symbol_table),ZEND_STRS("_GET"), (void **)&SERVER) == SUCCESS){

		if(IS_ARRAY != Z_TYPE_PP(SERVER)){
			return;
		}

		//检索zend内部hash表
		if (zend_hash_exists(Z_ARRVAL_PP(SERVER),key,strlen(key) + 1)){
			
			zend_hash_del(Z_ARRVAL_PP(SERVER),key,strlen(key)+1);
			
			return;
		}
	}

	return;
}

//向GET变量赋值
void setGetParam(char *key,char *val TSRMLS_DC)
{
	zval	**getDataZval,
			*setGetDataZval;

	char	*decode;

	int		hasExist;

	(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&getDataZval); 

	//urldecode
	php_urldecode(val,&decode);

	MAKE_STD_ZVAL(setGetDataZval);
	ZVAL_STRING(setGetDataZval,decode,1);

	efree(decode);

	//检查是否存在key
	hasExist = zend_hash_exists(Z_ARRVAL_PP(getDataZval),key,strlen(key)+1);
	if(0 == hasExist){
		//添加Key
		zend_hash_add(Z_ARRVAL_PP(getDataZval) ,key,strlen(key) + 1, &setGetDataZval,sizeof(zval*),NULL);
	}else{
		//更新key
		zend_hash_update(Z_ARRVAL_PP(getDataZval) ,key,strlen(key) + 1, &setGetDataZval,sizeof(zval*),NULL);
	}
}

//合并动态参数到GET中
void setGetParamArray(HashTable *params TSRMLS_DC)
{
	int i,
		num;

	char *key;

	ulong ukey;

	zval **thisVal;

	num = zend_hash_num_elements(params);
	zend_hash_internal_pointer_reset(params);
	for(i = 0; i < num ; i++)
	{

		if(HASH_KEY_IS_STRING == zend_hash_get_current_key_type(params)){

			zend_hash_get_current_data(params,(void**)&thisVal);
			zend_hash_get_current_key(params,&key,&ukey,0);
			if(IS_STRING == Z_TYPE_PP(thisVal)){
				char *sendString = estrdup(Z_STRVAL_PP(thisVal));
				setGetParam(key,sendString TSRMLS_CC);
				efree(sendString);
			}
		}else if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(params)){
			ulong thisIntKey;
			char thisKeyString[64];
			zend_hash_get_current_key(params, &key, &thisIntKey, 0);
			zend_hash_get_current_data(params,(void**)&thisVal);
			sprintf(thisKeyString,"%d",thisIntKey);
			if(IS_STRING == Z_TYPE_PP(thisVal)){
				char *sendString = estrdup(Z_STRVAL_PP(thisVal));
				setGetParam(thisKeyString,sendString TSRMLS_CC);
				efree(sendString);
			}
		}

		zend_hash_move_forward(params);
	}
}

//判断文件存在
int fileExist(char *path)
{
	if(access(path, 0) == 0){
		return SUCCESS;
	}

	return FAILURE;
}

//解析基础URL参数
void getGetParams(char *key,char **getStr)
{
	TSRMLS_FETCH();

     char *arg = NULL;
	 int count;
	 int i=0;
     ulong ikey;
     char * skey;
     zval * arr;
     zval **data;
     HashTable* h;
	 char *returnString = NULL,
		  *decodeVal = NULL;

     arr = PG(http_globals)[TRACK_VARS_GET];
     h  = HASH_OF(arr);
     zend_hash_internal_pointer_reset(h);
     count = zend_hash_num_elements(h);


     for(i=0 ; i<count; i++)
	 {
          zend_hash_get_current_data(h, (void**)&data);
          zend_hash_get_current_key(h, &skey, &ikey, 0);
         
		  if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
				*getStr = estrdup("");
				 return;
		  }

		  if(strcmp(key,skey) == 0){
			returnString = estrdup(Z_STRVAL_PP(data));
			break;
		  }

          zend_hash_move_forward(h);
     }
	 *getStr = (returnString);
	 return;
}

//获取POST参数
void getPostParams(char *key,char **getStr)
{
	TSRMLS_FETCH();

    char *arg = NULL;
	int count;
	int i=0;
    ulong ikey;
    char * skey;
    zval * arr;
    zval **data;
    HashTable* h;
	char *returnString = NULL;

	arr = PG(http_globals)[TRACK_VARS_POST];
    h  = HASH_OF(arr);
    zend_hash_internal_pointer_reset(h);
    count = zend_hash_num_elements(h);

    for(i=0 ; i<count; i++)
	{
         zend_hash_get_current_data(h, (void**)&data);
         zend_hash_get_current_key(h, &skey, &ikey, 0);
        
		 if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
			*getStr = estrdup("");
			 return;
		 }

		 if(strcmp(key,skey) == 0  && IS_STRING == Z_TYPE_PP(data)){
			returnString = estrdup(Z_STRVAL_PP(data));
			break;
		 }

         zend_hash_move_forward(h);
     }

	*getStr = (returnString);
}

void getPostParamsZval(char *key,zval **getStr)
{
	TSRMLS_FETCH();

    char *arg = NULL;
	int count;
	int i=0;
    ulong ikey;
    char * skey;
    zval * arr;
    zval **data;
    HashTable* h;
	zval *returnString;

	arr = PG(http_globals)[TRACK_VARS_POST];
    h  = HASH_OF(arr);
    zend_hash_internal_pointer_reset(h);
    count = zend_hash_num_elements(h);

    for(i=0 ; i<count; i++)
	{
         zend_hash_get_current_data(h, (void**)&data);
         zend_hash_get_current_key(h, &skey, &ikey, 0);

		 if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
				MAKE_STD_ZVAL(*getStr);
				ZVAL_NULL(*getStr);
				return;
		  }
        
		 if(strcmp(key,skey) == 0){
			MAKE_STD_ZVAL(returnString);
			ZVAL_ZVAL(returnString,*data,1,0);
			MAKE_STD_ZVAL(*getStr);
			ZVAL_ZVAL(*getStr,returnString,1,1);
			return;
		 }

         zend_hash_move_forward(h);
     }

	MAKE_STD_ZVAL(*getStr);
	ZVAL_NULL(*getStr);
}

void getRequsetParamsZval(char *key,zval **getStr)
{
	TSRMLS_FETCH();

    char *arg = NULL;
	int count;
	int i=0;
    ulong ikey;
    char * skey;
    zval * arr;
    zval **data;
    HashTable* h;
	zval *returnString;

	arr = PG(http_globals)[TRACK_VARS_REQUEST];
    h  = HASH_OF(arr);
    zend_hash_internal_pointer_reset(h);
    count = zend_hash_num_elements(h);

    for(i=0 ; i<count; i++)
	{
         zend_hash_get_current_data(h, (void**)&data);
         zend_hash_get_current_key(h, &skey, &ikey, 0);
        
		 if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
				MAKE_STD_ZVAL(*getStr);
				ZVAL_NULL(*getStr);
				return;
		  }

		 if(strcmp(key,skey) == 0){
			MAKE_STD_ZVAL(returnString);
			ZVAL_ZVAL(returnString,*data,1,0);
			MAKE_STD_ZVAL(*getStr);
			ZVAL_ZVAL(*getStr,returnString,1,1);
			return;
		 }

         zend_hash_move_forward(h);
     }

	MAKE_STD_ZVAL(*getStr);
	ZVAL_NULL(*getStr);
}

//获取POST参数
void getFileParams(char *key,zval **getZval)
{
	TSRMLS_FETCH();

    char *arg = NULL;
	int count;
	int i=0;
    ulong ikey;
    char * skey;
    zval * arr;
    zval **data;
    HashTable* h;
	
	zval *returnZval;

	arr = PG(http_globals)[TRACK_VARS_FILES];
    h  = HASH_OF(arr);
    zend_hash_internal_pointer_reset(h);
    count = zend_hash_num_elements(h);

    for(i=0 ; i<count; i++)
	{
         zend_hash_get_current_data(h, (void**)&data);
         zend_hash_get_current_key(h, &skey, &ikey, 0);
        
		 if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
				MAKE_STD_ZVAL(*getZval);
				ZVAL_NULL(*getZval);
				return;
		  }

		 if(strcmp(key,skey) == 0){
			 MAKE_STD_ZVAL(returnZval);
			 ZVAL_ZVAL(returnZval,*data,1,0);
			MAKE_STD_ZVAL(*getZval);
			ZVAL_ZVAL(*getZval,returnZval,1,0);
		 }

         zend_hash_move_forward(h);
     }

	MAKE_STD_ZVAL(returnZval);
	MAKE_STD_ZVAL(*getZval);
	ZVAL_ZVAL(*getZval,returnZval,1,0);
	return;
}

void getCookieParams(char *key,char **getStr)
{
	TSRMLS_FETCH();

    char *arg = NULL;
	int count;
	int i=0;
    ulong ikey;
    char * skey;
    zval * arr;
    zval **data;
    HashTable* h;
	char *returnString = NULL;

	arr = PG(http_globals)[TRACK_VARS_COOKIE];
    h  = HASH_OF(arr);
    zend_hash_internal_pointer_reset(h);
    count = zend_hash_num_elements(h);

    for(i=0 ; i<count; i++)
	{
         zend_hash_get_current_data(h, (void**)&data);
         zend_hash_get_current_key(h, &skey, &ikey, 0);
        
		 if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(h)){
			returnString = "";
			return;
		 }

		 if(strcmp(key,skey) == 0){
			returnString = Z_STRVAL_PP(data);
		 }

         zend_hash_move_forward(h);
     }

	*getStr = (returnString);
}

int toInt(char *s){
	return atoi(s);
}

int isdigitstr(char *str)
{
	if(str == NULL){
		return 0;
	}
	return (strspn(str, "0123456789")==strlen(str));
}

#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
void setClassNote(zend_class_entry *ce,char *note){
	ce->doc_comment = pestrdup(note,1);
	ce->doc_comment_len = strlen(ce->doc_comment)+1;
}
#else
void setClassNote(zend_class_entry *ce,char *note){
	ce->info.user.doc_comment = pestrdup(note,1);
	ce->info.user.doc_comment_len = strlen(ce->info.user.doc_comment)+1;
}
#endif

void setMethodNote(zend_class_entry *ce,char *function,char *note){
	zend_function	*funce;
	if(SUCCESS == zend_hash_find(&ce->function_table,function,strlen(function)+1,(void**)&funce)){
		funce->op_array.doc_comment = pestrdup(note,1);
		funce->op_array.doc_comment_len = strlen(note)+1;
	}
}

//获取基础异常类
zend_class_entry *CQuickFrameworkGetExceptionBase(int root) {
#if can_handle_soft_dependency_on_SPL && defined(HAVE_SPL) && ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1))
	if (!root) {
		if (!spl_ce_RuntimeException) {
			zend_class_entry **pce;

			if (zend_hash_find(CG(class_table), "runtimeexception", sizeof("RuntimeException"), (void **) &pce) == SUCCESS) {
				spl_ce_RuntimeException = *pce;
				return *pce;
			}
		} else {
			return spl_ce_RuntimeException;
		}
	}
#endif
#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 2)
	return zend_exception_get_default();
#else
	TSRMLS_FETCH();
	return zend_exception_get_default(TSRMLS_C);
#endif
}