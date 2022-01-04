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
#include "php_CRedis.h"
#include "php_CException.h"

ZEND_BEGIN_ARG_INFO_EX(CRedis_call_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

//zend类方法
zend_function_entry CRedis_functions[] = {
	PHP_ME(CRedis,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRedis,getObject,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRedis,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CRedis,__call,CRedis_call_arginfo,ZEND_ACC_PUBLIC)
	PHP_ME(CRedis,set,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRedis)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CRedis",CRedis_functions);
	CRedisCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CRedisCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CRedisCe, ZEND_STRL("_redisConn"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

ZEND_BEGIN_ARG_INFO_EX(CRedisMessage_call_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
zend_function_entry CRedisMessage_functions[] = {
	PHP_ME(CRedisMessage,getBody,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRedisMessage,ack,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRedisMessage,__call,CRedisMessage_call_arginfo,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRedisMessage)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CRedisMessage",CRedisMessage_functions);
	CRedisMessageCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CRedisMessageCe, ZEND_STRL("bodyContent"),ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}

PHP_METHOD(CRedisMessage,getBody)
{
	zval	*returnZval;
	returnZval = zend_read_property(CRedisMessageCe,getThis(),ZEND_STRL("bodyContent"), 0 TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CRedisMessage,ack){
	RETVAL_TRUE;
}

PHP_METHOD(CRedisMessage,__call){
	RETVAL_FALSE;
}

//获取CRedis单例对象
void CRedis_getInstance(zval **returnZval,char *groupName TSRMLS_DC)
{
	zval	*instanceZval,
			**instaceSaveZval;

	int hasExistConfig;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CRedisCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CRedisCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CRedisCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(instanceZval), groupName, strlen(groupName)+1);

	//为空时则实例化自身
	if(0 == hasExistConfig ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CRedisCe);

		//执行构造器
		if (CRedisCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 params1;
			zval *paramsList[1];
			paramsList[0] = &params1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],groupName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CRedisCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);

		zend_hash_add(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CRedisCe, ZEND_STRL("instance"),instanceZval TSRMLS_CC);

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

	zend_throw_exception(CRedisExceptionCe, "[CRedisException] An internal error occurred while CQuickFramework was acquired by Redis ", 12011 TSRMLS_CC);
}


//fast function
void CRedis_callFunction(char *config,char *val,zval *args,zval **returnData TSRMLS_DC){

	zval			*redisZval,
					*redisObject,
					*paramsList[64],
					param,
					**thisVal,
					*returnZval;

	zend_function	*requsetAction;

	int				i,num;

	CRedis_getInstance(&redisZval,config TSRMLS_CC);

	redisObject = zend_read_property(CRedisCe,redisZval,ZEND_STRL("_redisConn"),0 TSRMLS_CC);
	MAKE_STD_ZVAL(*returnData);
	if(zend_hash_find(&(Z_OBJCE_P(redisObject)->function_table),val,strlen(val)+1,(void**)&requsetAction) != SUCCESS){
		array_init(*returnData);
		zval_ptr_dtor(&redisZval);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(args));

	if(num >= 64){
		array_init(*returnData);
		zval_ptr_dtor(&redisZval);
		return;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));
	for(i = 0 ; i < num ; i++){
		paramsList[i] = &param;
		MAKE_STD_ZVAL(paramsList[i]);
		zend_hash_get_current_data(Z_ARRVAL_P(args),(void**)&thisVal);
		ZVAL_ZVAL(paramsList[i],*thisVal,1,0);
		zend_hash_move_forward(Z_ARRVAL_P(args));
	}

	MODULE_BEGIN
		zval	constructReturn,
				constructVal,
				*saveZval;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,val, 0);
		call_user_function(NULL, &redisObject, &constructVal, &constructReturn, num, paramsList TSRMLS_CC);
		returnZval = &constructReturn;
		
		for(i = 0 ; i < num ; i++){
			zval_ptr_dtor(&paramsList[i]);
		}

		if(EG(exception)){
			char errMessage[1024];
			sprintf(errMessage,"%s%s%s%s","[CRedisException] Unable to connect to the redis server to CallFunction: ",val,"()",config);
			Z_OBJ_HANDLE_P(EG(exception)) = 0;
			zend_clear_exception(TSRMLS_C);
			ZVAL_NULL(*returnData);
			zend_throw_exception(CRedisExceptionCe, errMessage, 1001 TSRMLS_CC);
			return;
		}

		ZVAL_ZVAL(*returnData,returnZval,1,0);
		zval_dtor(&constructReturn);
	MODULE_END
	zval_ptr_dtor(&redisZval);
}

int CRedis_checkWriteRead(char *config TSRMLS_DC){

	MODULE_BEGIN
		zval	*params,
				*returnData;

		//test write
		MAKE_STD_ZVAL(params);
		array_init(params);
		add_next_index_string(params,"CQuickFrameTestKey",1);
		add_next_index_string(params,"testVal",1);
		add_next_index_long(params,120);
		CRedis_callFunction(config,"set",params,&returnData TSRMLS_CC);
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&returnData);

		if(EG(exception)){
			Z_OBJ_HANDLE_P(EG(exception)) = 0;
			zend_clear_exception(TSRMLS_C);
			return 0;
		}
	MODULE_END

	MODULE_BEGIN
		zval	*params,
				*returnData;

		//test read
		MAKE_STD_ZVAL(params);
		array_init(params);
		add_next_index_string(params,"CQuickFrameTestKey",1);
		add_next_index_string(params,"testVal",1);
		CRedis_callFunction(config,"del",params,&returnData TSRMLS_CC);
		zval_ptr_dtor(&params);
		zval_ptr_dtor(&returnData);

		if(EG(exception)){
			Z_OBJ_HANDLE_P(EG(exception)) = 0;
			zend_clear_exception(TSRMLS_C);
			return 0;
		}
	MODULE_END


	return 1;
}

PHP_METHOD(CRedis,getInstance)
{
	zval *instanceZval;
	char *groupName = NULL,
		 *trueGroupName;
	int groupNameLen = 0;

	//判断是否制定redis序列
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException]The server is not currently installed Redis extension", 12011 TSRMLS_CC);
		return;
	}

	if(groupNameLen == 0){
		trueGroupName = estrdup("main");
	}else{
		trueGroupName = estrdup(groupName);
	}


	CRedis_getInstance(&instanceZval,trueGroupName TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	zval_ptr_dtor(&instanceZval);
	efree(trueGroupName);
}

PHP_METHOD(CRedis,getObject)
{
	zval *redisZval;

	//redis对象
	redisZval = zend_read_property(CRedisCe,getThis(),ZEND_STRL("_redisConn"),0 TSRMLS_CC);

	RETVAL_ZVAL(redisZval,1,0);
}

//获取redis连接对象
void CRedis_getRedisConnection(zval *object,char *groupName TSRMLS_DC)
{

	char	*defaultHost,
		    *redisGroup,
			*password = NULL;
	int		defaultPort = 6379,
			defaultTimeout = 3;

	zval	*cconfigInstanceZval,
			*configHost,
			*configPort,
			*configPassword,
			*configTimeout,
			*redisObject,
			*saveRedis;

	zend_class_entry	**redisPP,
						*redisCe;
		

	defaultHost = "127.0.0.1";
	redisGroup = estrdup(groupName);


	//获取配置参数
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//连接地址
	CConfig_load("REDIS_HOST",cconfigInstanceZval,&configHost TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configHost) && strlen(Z_STRVAL_P(configHost)) >0 ){
		defaultHost = (Z_STRVAL_P(configHost));
	}

	//端口
	CConfig_load("REDIS_PORT",cconfigInstanceZval,&configPort TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(configPort)){
		defaultPort = Z_LVAL_P(configPort);
	}
	zval_ptr_dtor(&configPort);

	//超时
	CConfig_load("REDIS_TIMEOUT",cconfigInstanceZval,&configTimeout TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(configTimeout)){
		defaultTimeout = Z_LVAL_P(configTimeout);
	}
	zval_ptr_dtor(&configTimeout);

	//passowrd
	CConfig_load("REDIS_PASSWORD",cconfigInstanceZval,&configPassword TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configPassword)){
		password = (Z_STRVAL_P(configPassword));
	}
	zval_ptr_dtor(&configPassword);

	//使用指定RedisGroup时
	if(strcmp(redisGroup,"main") != 0){


		//读取配置中REDIS_SERVER键
		zval *redisServer,
			 **thisGroupZval;

		CConfig_load("REDIS_SERVER",cconfigInstanceZval,&redisServer TSRMLS_CC);

		if(IS_ARRAY == Z_TYPE_P(redisServer) && zend_hash_find(Z_ARRVAL_P(redisServer),groupName,strlen(groupName)+1,(void**)&thisGroupZval ) == SUCCESS && IS_ARRAY == Z_TYPE_PP(thisGroupZval) ){

			//读取
			zval **groupHost;
			zval **groupPort;
			zval **groupTimeout;
			zval **groupPassword;
			if(zend_hash_find(Z_ARRVAL_PP(thisGroupZval),"host",strlen("host")+1,(void**)&groupHost) == SUCCESS && IS_STRING == Z_TYPE_PP(groupHost) ){
				defaultHost = (Z_STRVAL_PP(groupHost));
			}

			if(zend_hash_find(Z_ARRVAL_PP(thisGroupZval),"port",strlen("port")+1,(void**)&groupPort) == SUCCESS && IS_LONG == Z_TYPE_PP(groupPort) ){
				defaultPort = Z_LVAL_PP(groupPort);
			}

			if(zend_hash_find(Z_ARRVAL_PP(thisGroupZval),"timeout",strlen("timeout")+1,(void**)&groupTimeout) == SUCCESS && IS_LONG == Z_TYPE_PP(groupTimeout) ){
				defaultTimeout = Z_LVAL_PP(groupTimeout);
			}

			if(zend_hash_find(Z_ARRVAL_PP(thisGroupZval),"password",strlen("password")+1,(void**)&groupPassword) == SUCCESS && IS_STRING == Z_TYPE_PP(groupPassword) ){
				password = Z_STRVAL_PP(groupPassword);
			}

		}
		zval_ptr_dtor(&redisServer);
	}

	efree(redisGroup);

	//查询redis
	if(zend_hash_find(EG(class_table),"redis",strlen("redis")+1,(void**)&redisPP ) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException]The server is not currently installed Redis extension", 12011 TSRMLS_CC);
		return;
	}

	//实例化
	redisCe = *redisPP;
	MAKE_STD_ZVAL(redisObject);
	object_init_ex(redisObject,redisCe);

	//尝试调用构造器
	if (redisCe->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, redisCe->constructor->common.function_name, 0);
		call_user_function(NULL, &redisObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//调用其connect方法
	MODULE_BEGIN
		zval	constructReturn,
				constructVal,
				*paramsList[3],
				param1,
				param2,
				param3;
		INIT_ZVAL(constructVal);
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		paramsList[2] = &param3;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_STRING(paramsList[0],defaultHost,1);
		ZVAL_LONG(paramsList[1],defaultPort);
		ZVAL_LONG(paramsList[2],defaultTimeout);
		ZVAL_STRING(&constructVal,"pconnect", 0);

		call_user_function(NULL, &redisObject, &constructVal, &constructReturn, 3, paramsList TSRMLS_CC);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_dtor(&constructReturn);

		if(EG(exception)){
			char errMessage[1024];
			sprintf(errMessage,"%s%s%s%d","[CRedisException] Unable to connect to the redis server : ",defaultHost,":",defaultPort);
			Z_OBJ_HANDLE_P(EG(exception)) = 0;
			zend_clear_exception(TSRMLS_C);
			zval_ptr_dtor(&configHost);
			zend_throw_exception(CRedisExceptionCe, errMessage, 1001 TSRMLS_CC);
			return;
		}

	MODULE_END


	//判断是否有密码
	if(password != NULL){

		//连接Redis进行密码验证
		MODULE_BEGIN
			zval	constructReturn,
					constructVal,
					*paramsList[1],
					param1;
			INIT_ZVAL(constructVal);
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],password,1);
			ZVAL_STRING(&constructVal,"auth", 0);
			call_user_function(NULL, &redisObject, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
			if(EG(exception)){
				char errMessage[1024];
				sprintf(errMessage,"%s%s%s%d","[CRedisException] auth fail to redis server : ",defaultHost,":",defaultPort);
				Z_OBJ_HANDLE_P(EG(exception)) = 0;
				zend_clear_exception(TSRMLS_C);
				zval_ptr_dtor(&configHost);
				zend_throw_exception(CRedisExceptionCe, errMessage, 1001 TSRMLS_CC);
				return;
			}

		MODULE_END

	}

	//保存redis连接
	zend_update_property(CRedisCe,object,ZEND_STRL("_redisConn"),redisObject TSRMLS_CC);
	zval_ptr_dtor(&redisObject);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&configHost);
}

PHP_METHOD(CRedis,__construct)
{

	char *groupName;
	int groupNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException] Redis execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

	CRedis_getRedisConnection(getThis(),groupName TSRMLS_CC);
}

PHP_METHOD(CRedis,__call)
{
	char	*val;
	int		valLen,
			i,
			num;
	zval	*args,
			*paramsList[64],
			param,
			**thisVal,
			*redisZval,
			*returnZval;

	zend_function *requsetAction;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&val,&valLen,&args) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException]Redis execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

	//redis对象
	redisZval = zend_read_property(CRedisCe,getThis(),ZEND_STRL("_redisConn"),0 TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(redisZval)){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException] CQuickFramework internal error, CRedis connection object is lost", 12013 TSRMLS_CC);
		return;
	}

	//判断redis是否存在该方法
	if(zend_hash_find( &(Z_OBJCE_P(redisZval)->function_table),val,strlen(val)+1,(void**)&requsetAction) != SUCCESS){
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CRedisException]Redis does not exist the method : ",val);
		zend_throw_exception(CRedisExceptionCe, errorMessage, 12014 TSRMLS_CC);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(args));

	if(num >= 64){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException]Redis execution method parameter number too much", 12012 TSRMLS_CC);
		return;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));
	for(i = 0 ; i < num ; i++){
		paramsList[i] = &param;
		MAKE_STD_ZVAL(paramsList[i]);
		zend_hash_get_current_data(Z_ARRVAL_P(args),(void**)&thisVal);
		ZVAL_ZVAL(paramsList[i],*thisVal,1,0);
		zend_hash_move_forward(Z_ARRVAL_P(args));
	}

	MODULE_BEGIN
		zval	constructReturn,
				constructVal,
				*saveZval;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,val, 0);
		call_user_function(NULL, &redisZval, &constructVal, &constructReturn, num, paramsList TSRMLS_CC);
		returnZval = &constructReturn;
		
		for(i = 0 ; i < num ; i++){
			zval_ptr_dtor(&paramsList[i]);
		}

		if(EG(exception)){
			char errMessage[1024];
			char *sendPrams;
			json_encode(args,&sendPrams);
			sprintf(errMessage,"%s%s%s%s","[CRedisException] Unable to connect to the redis server to CallFunction: ",val,"()",sendPrams);
			efree(sendPrams);
			Z_OBJ_HANDLE_P(EG(exception)) = 0;
			zend_clear_exception(TSRMLS_C);
			zend_throw_exception(CRedisExceptionCe, errMessage, 1001 TSRMLS_CC);
			return;
		}

		ZVAL_ZVAL(return_value,returnZval,1,0);
		zval_dtor(&constructReturn);

	MODULE_END
}

PHP_METHOD(CRedis,set)
{

	char	*key,
			*value;

	int		keyLen = 0,
			valueLen = 0;

	long	timeout = 3600;

	zval	*redisZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|l",&key,&keyLen,&value,&valueLen,&timeout) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException]Redis execution method set params error,must send params as (string,string,int)", 12012 TSRMLS_CC);
		return;
	}

	//redis对象
	redisZval = zend_read_property(CRedisCe,getThis(),ZEND_STRL("_redisConn"),0 TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(redisZval)){
		zend_throw_exception(CRedisExceptionCe, "[CRedisException] CQuickFramework internal error, CRedis connection object is lost", 12013 TSRMLS_CC);
		return;
	}

	MODULE_BEGIN
		zval	actionVal,
				actionReturn,
				*paramsList[3],
				*returnStatus;

		INIT_ZVAL(actionVal);
		ZVAL_STRING(&actionVal,"setex",0);

		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],key,1);

		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_LONG(paramsList[1],timeout);

		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_STRING(paramsList[2],value,1);


		if(SUCCESS != call_user_function(NULL, &redisZval, &actionVal, &actionReturn,3, paramsList TSRMLS_CC)){
			ZVAL_BOOL(return_value,0);
			return;
		}

		returnStatus = &actionReturn;
		ZVAL_ZVAL(return_value,returnStatus,1,0);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_dtor(&actionReturn);

	MODULE_END

}