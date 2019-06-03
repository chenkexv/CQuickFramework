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
#include "php_CShareCache.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#ifndef PHP_WIN32
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <pwd.h>
#endif


//zend类方法
zend_function_entry CShareCache_functions[] = {
	PHP_ME(CShareCache,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CShareCache,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CShareCache,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CShareCache,del,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CShareCache,delId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CShareCache,info,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CShareCache,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CShareCache)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CShareCache",CShareCache_functions);
	CShareCacheCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CShareCacheCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CShareCacheCe, ZEND_STRL("flags"),"c",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CShareCacheCe, ZEND_STRL("mode"),0644,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void CShareCache_getInstance(zval **returnZval,char *groupName TSRMLS_DC){

	zval	*instanceZval,
			**instaceSaveZval;

	int hasExistConfig;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CShareCacheCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CShareCacheCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CShareCacheCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(instanceZval), groupName, strlen(groupName)+1);

	//为空时则实例化自身
	if(0 == hasExistConfig ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CShareCacheCe);

		//执行构造器
		if (CShareCacheCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 params1;
			zval *paramsList[1];
			paramsList[0] = &params1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],groupName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CShareCacheCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
		}

		//判断是否有报告异常
		if(EG(exception)){
			MAKE_STD_ZVAL(*returnZval);
			zval_ptr_dtor(&object);
			return;
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);

		zend_hash_add(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CShareCacheCe, ZEND_STRL("instance"),instanceZval TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	//直接取instace静态变量中的返回值
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,(void**)&instaceSaveZval) ){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,*instaceSaveZval,1,0);
		return;
	}

	zend_throw_exception(CacheExceptionCe, "[CShareCacheException] An internal error occurred while CQuickFramework was acquired by Linux Share Memory ", 1 TSRMLS_CC);
}

PHP_METHOD(CShareCache,getInstance){
	
	zval *instanceZval;
	char *groupName,
			*trueGroupName;
	int groupNameLen = 0;

	//判断是否指定序列
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] call [CShareCache->getInstance] params error", 1 TSRMLS_CC);
		return;
	}

	if(groupNameLen == 0){
		trueGroupName = estrdup("main");
	}else{
		trueGroupName = estrdup(groupName);
	}

	CShareCache_getInstance(&instanceZval,trueGroupName TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	zval_ptr_dtor(&instanceZval);
	efree(trueGroupName);
}

//get a length near x4
int CShareCache_getSaveSizeForStringLen(int len){

	return ((int)ceil((len+1) / 4.0))*4;
}

int CShareCahe_getKeyId(char *name){

	char	*md5String,
			*md510,
			toIntString[11];
	int		i;

	md5(name,&md5String);
	substr(md5String,0,10,&md510);

	for(i = 0 ; i < 10;i++){

		switch(md510[i])
		{
			case 'a':
				toIntString[i] = '1';
				break;
			case 'b':
				toIntString[i] = '2';
				break;
			case 'c':
				toIntString[i] = '3';
				break;
			case 'd':
				toIntString[i] = '4';
				break;
			case 'e':
				toIntString[i] = '5';
				break;
			case 'f':
				toIntString[i] = '6';
				break;
			case 'g':
				toIntString[i] = '7';
				break;
			case 'h':
				toIntString[i] = '8';
				break;
			case 'i':
				toIntString[i] = '9';
				break;
			case 'j':
				toIntString[i] = '0';
				break;
			case 'k':
				toIntString[i] = '1';
				break;
			case 'l':
				toIntString[i] = '2';
				break;
			case 'm':
				toIntString[i] = '3';
				break;
			case 'n':
				toIntString[i] = '4';
				break;
			case 'o':
				toIntString[i] = '5';
				break;
			case 'p':
				toIntString[i] = '6';
				break;
			case 'q':
				toIntString[i] = '7';
				break;
			case 'r':
				toIntString[i] = '8';
				break;
			case 's':
				toIntString[i] = '9';
				break;
			case 't':
				toIntString[i] = '0';
				break;
			case 'u':
				toIntString[i] = '1';
				break;
			case 'v':
				toIntString[i] = '2';
				break;
			case 'w':
				toIntString[i] = '3';
				break;
			case 'x':
				toIntString[i] = '4';
				break;
			case 'y':
				toIntString[i] = '5';
				break;
			case 'z':
				toIntString[i] = '6';
				break;
			case '0':
				toIntString[i] = '0';
				break;
			case '1':
				toIntString[i] = '1';
				break;
			case '2':
				toIntString[i] = '2';
				break;
			case '3':
				toIntString[i] = '3';
				break;
			case '4':
				toIntString[i] = '4';
				break;
			case '5':
				toIntString[i] = '5';
				break;
			case '6':
				toIntString[i] = '6';
				break;
			case '7':
				toIntString[i] = '7';
				break;
			case '8':
				toIntString[i] = '8';
				break;
			case '9':
				toIntString[i] = '9';
				break;
		}
	}

	if(toIntString[0] == '0'){
		toIntString[0] == '1';
	}

	toIntString[10] = '\0';
	efree(md5String);
	efree(md510);
	return toInt(toIntString);
}

int CShareCache_delShmId(long id TSRMLS_DC){
	if (shmctl(id, IPC_RMID, NULL) == -1) {
		return 0;
	}else{
		return 1;
	}
}

int CShareCache_delKey(char *key,int mode TSRMLS_DC){

#ifdef PHP_WIN32
	return 0;
#else

	int		getFlags,
			tryShmid,
			keyId;

	keyId = CShareCahe_getKeyId(key);

	getFlags = mode|IPC_CREAT;
	tryShmid = shmget(keyId, 10, getFlags);
	if(tryShmid != -1){

		char *addr = 	

shmdt(addr);

		if (shmctl(tryShmid, IPC_RMID, NULL) == -1) {
			return 0;
		}else{
			return 1;
		}
	}
	return 0;
#endif

	return 0;
}




int CShareCache_set(char *flags,int mode,char *key,char *val,int pos TSRMLS_DC){

#ifdef PHP_WIN32
	return 0;
#else

	int		shmatflg,
			shmflg,
			size = -1,
			shmid,
			keyId;

	struct	shmid_ds	shm;

	char	*addr;

	shmflg |= mode;

	keyId = CShareCahe_getKeyId(key);

	switch (flags[0]) 
	{
		case 'a':
			shmatflg |= SHM_RDONLY;
			break;
		case 'c':
			shmflg |= IPC_CREAT;
			break;
		case 'n':
			shmflg |= (IPC_CREAT | IPC_EXCL);
			break;
		default:
			shmflg |= IPC_CREAT;
			break;
	}

	size = CShareCache_getSaveSizeForStringLen(strlen(val));


	//create share memory
	shmid = shmget(keyId, size, shmflg);
	//if return -1 ,it may be exist , and try to delete
	if(shmid == -1){
		if(1 == CShareCache_delKey(key,mode TSRMLS_CC)){
			shmid = shmget(keyId, size, shmflg);
		}else{
			zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->set] can not open share memory space ", 12012 TSRMLS_CC);
			return 0;
		}
	}

	//try to get 
	if (shmctl(shmid, IPC_STAT, &shm)) {
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->set] unable to get shared memory segment information ", 12012 TSRMLS_CC);
		return 0;
	}

	//mat to my process mem
	addr = shmat(shmid, 0, 0);
	if(addr == (char*)-1){
		char errorMessage[1024];
		sprintf(errorMessage,"[CShareCacheException] Call [CShareCache->set] unable to attach to shared memory segment : %d",shmid);
		zend_throw_exception(CacheExceptionCe, errorMessage, 12012 TSRMLS_CC);
		return 0;
	}

	//call to write
	memcpy(addr, val, strlen(val)+1);

	return 1;
#endif

	return 0;
}

PHP_METHOD(CShareCache,set){

	char	*key,
			*jsonString;

	long	keyLen = 0,
			valLen = 0,
			pos = 0;

	zval	*val,
			*mode,
			*flags;

	int		status;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|l",&key,&keyLen,&val,&pos) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->set] parameter error", 12012 TSRMLS_CC);
		return;
	}

	if(IS_STRING == Z_TYPE_P(val) || IS_BOOL == Z_TYPE_P(val) || IS_DOUBLE == Z_TYPE_P(val) || IS_LONG == Z_TYPE_P(val)){
	}else{
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->set] the val must can be json in [string,long,double,bool] ", 12012 TSRMLS_CC);
		return;
	}

	flags = zend_read_property(CShareCacheCe,getThis(),ZEND_STRL("flags"),0 TSRMLS_CC);
	mode = zend_read_property(CShareCacheCe,getThis(),ZEND_STRL("mode"),0 TSRMLS_CC);


	//the val change to json string
	json_encode(val,&jsonString);

	//call function
	status = CShareCache_set(Z_STRVAL_P(flags),Z_LVAL_P(mode),key,jsonString,pos TSRMLS_CC);

	efree(jsonString);

	if(status){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CShareCache,del){
	
	char	*key;

	int		keyLen = 0,
			status = 0,
			mode = 0644;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->del] parameter error", 12012 TSRMLS_CC);
		return;
	}

	status = CShareCache_delKey(key,mode TSRMLS_CC);

	if(status){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}

}

PHP_METHOD(CShareCache,delId){
	

	long	shmId;
	int		status;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&shmId) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->del] parameter error", 12012 TSRMLS_CC);
		return;
	}

	status = CShareCache_delShmId(shmId TSRMLS_CC);

	if(status){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}

}

void CShareCache_get(char *key,char **returnString TSRMLS_DC){

#ifdef PHP_WIN32
	*returnString = estrdup("");
#else

	int		keyId,
			shmid;

	char	*addr;

	keyId = CShareCahe_getKeyId(key);

	shmid = shmget(keyId, 0, 0);

	if(shmid == -1){
		*returnString = estrdup("");
		return;
	}

	addr = shmat(shmid, 0, 0);
	*returnString = estrdup(addr);

	shmdt(addr);
#endif


}

PHP_METHOD(CShareCache,get){

	char	*key,
			*returnString;

	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] Call [CShareCache->get] parameter error", 12012 TSRMLS_CC);
		return;
	}

	CShareCache_get(key,&returnString TSRMLS_CC);

	RETVAL_STRING(returnString,0);
}

PHP_METHOD(CShareCache,__construct){
	
	char	*groupName,
			thisConfigKey[60];

	int		groupNameLen = 0;


	zval	*cconfigInstanceZval,
			*shareConfigs,
			**flagsZval,
			**modeZval;

	char	*flags;

	int		mode = 0644;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] CShareCache execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

	//get configs
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	
	//当前配置的名称
	sprintf(thisConfigKey,"%s%s","SHARECACHE.",groupName);

	//获取连接配置
	CConfig_load(thisConfigKey,cconfigInstanceZval,&shareConfigs TSRMLS_CC);
	
	//set configs
	flags = estrdup("n");
	if(IS_ARRAY == Z_TYPE_P(shareConfigs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(shareConfigs),"flags",strlen("flags")+1,(void**)&flagsZval) && IS_STRING == Z_TYPE_PP(flagsZval) ){
		efree(flags);
		flags = estrdup(Z_STRVAL_PP(flagsZval));
	}
	if(IS_ARRAY == Z_TYPE_P(shareConfigs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(shareConfigs),"mode",strlen("mode")+1,(void**)&modeZval) && IS_LONG == Z_TYPE_PP(modeZval) ){
		mode = Z_LVAL_PP(modeZval);
	}


	//save to object
	zend_update_property_string(CShareCacheCe,getThis(),ZEND_STRL("flags"),flags TSRMLS_CC);
	zend_update_property_long(CShareCacheCe,getThis(),ZEND_STRL("mode"),mode TSRMLS_CC);


	//destroy
	zval_ptr_dtor(&shareConfigs);
	zval_ptr_dtor(&cconfigInstanceZval);
	efree(flags);
}

void CSahreCache_getRuningUser(char **user){
#ifdef PHP_WIN32
	*user = estrdup("");
#else
	struct passwd *pwd;
	pwd = getpwuid(getuid());
	*user = estrdup(pwd->pw_name);
#endif
}

void CShareCache_parseRows(char *string,zval **thisArray TSRMLS_DC){

	int		i,n;
	zval	**thisVal,
			*returnArray;
	char	*key;
	ulong	uKey = 0;

	php_explode(" ",string,&returnArray);

	MAKE_STD_ZVAL(*thisArray);
	array_init(*thisArray);

	n = zend_hash_num_elements(Z_ARRVAL_P(returnArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(returnArray));
	for(i = 0 ; i < n ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(returnArray),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(returnArray),&key,&uKey,0);	
		
		convert_to_string(*thisVal);
		if(strlen(Z_STRVAL_PP(thisVal)) > 0){
			add_next_index_string(*thisArray,Z_STRVAL_PP(thisVal),1);
		}

		zend_hash_move_forward(Z_ARRVAL_P(returnArray));
	}
	zval_ptr_dtor(&returnArray);
}

PHP_METHOD(CShareCache,info){
	
	int		showAll = 0,
			i,h;

	char	*user,
			*returnString,
			*trimString;

	zval	*arrayList,
			**thisVal,
			*thisRowArray;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&showAll) == FAILURE){
		zend_throw_exception(CacheExceptionCe, "[CShareCacheException] CShareCache execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

#ifdef PHP_WIN32
	array_init(return_value);
#else

	CSahreCache_getRuningUser(&user);

	//get all list ipcs -m
	exec_shell_return("ipcs -m",&returnString);

	array_init(return_value);

	//cut
	php_explode("\n",returnString,&arrayList);
	h = zend_hash_num_elements(Z_ARRVAL_P(arrayList));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(arrayList),(void**)&thisVal);
		convert_to_string(*thisVal);
		php_trim(Z_STRVAL_PP(thisVal)," ",&trimString);
		if( strlen(trimString) == 0 || (strstr(Z_STRVAL_PP(thisVal),"key") != NULL && strstr(Z_STRVAL_PP(thisVal),"owner") != NULL) || strstr(Z_STRVAL_PP(thisVal),"Shared Memory") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(arrayList));
			efree(trimString);
			continue;
		}
	
		CShareCache_parseRows(Z_STRVAL_PP(thisVal),&thisRowArray TSRMLS_CC);
		add_next_index_zval(return_value,thisRowArray);
		zend_hash_move_forward(Z_ARRVAL_P(arrayList));
		efree(trimString);
	}

	efree(user);
	efree(returnString);
	zval_ptr_dtor(&arrayList);
#endif
}