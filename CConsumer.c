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
#include "php_CConsumer.h"
#include "php_CLog.h"
#include "php_CRabbit.h"
#include "php_CRedis.h"
#include "php_CRabbitMessage.h"
#include "php_CException.h"
#include "php_CWebApp.h"

#ifndef PHP_WIN32
#include <unistd.h>
#endif



//zend类方法
zend_function_entry CConsumer_functions[] = {
	PHP_ME(CConsumer,setEmptySleepTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setProcessMaxNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setMemoryLimit,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setTimeLimit,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,registerHeartbeatCallback,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,registerMessageCallback,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setProducer,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setLogName,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,ack,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setMQId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setMQType,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CConsumer,setListKey,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CConsumer)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CConsumer",CConsumer_functions);
	CConsumerCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量
	zend_declare_property_string(CConsumerCe, ZEND_STRL("listKey"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("listCommand"),"lpop",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("mqType"),1,ZEND_ACC_PUBLIC TSRMLS_CC); //MQ Type 1.rabbitmq 2.redis->list
	zend_declare_property_long(CConsumerCe, ZEND_STRL("emptySleepTime"),3,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("processMaxNum"),1000000,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("processNum"),0,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("memoryLimit"),"8048M",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("timeLimit"),0,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CConsumerCe, ZEND_STRL("heartbeatObject"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CConsumerCe, ZEND_STRL("heartbeatFunction"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("heartBeatTime"),60,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CConsumerCe, ZEND_STRL("processMessageObject"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CConsumerCe, ZEND_STRL("processMessageFunction"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("emptySec"),0,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("producerExchange"),"default",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("producerRoute"),"default",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("producerQueue"),"default",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CConsumerCe, ZEND_STRL("producerAutoAck"),0,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("logName"),"consumer",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CConsumerCe, ZEND_STRL("mqId"),"main",ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}


PHP_METHOD(CConsumer,setEmptySleepTime)
{
	int		sleepTime = 3;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&sleepTime) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setEmptySleepTime] the 1 parameter type error , must be int ", 1 TSRMLS_CC);
		return;
	}

	zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("emptySleepTime"),sleepTime TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setProcessMaxNum)
{
	int		maxTime = 100000;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&maxTime) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setProcessMaxNum] the 1 parameter type error , must be int ", 1 TSRMLS_CC);
		return;
	}
	zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("processMaxNum"),maxTime TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setMemoryLimit)
{
	char	*memLimit = NULL,
			*trueMemLimit;
	int		memLimitLen = 0;
	zval	*memZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&memLimit,&memLimitLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setMemoryLimit] the 1 parameter type error , must be string ", 1 TSRMLS_CC);
		return;
	}

	if(memLimitLen == 0){
		trueMemLimit = estrdup("8084M"); 
	}else{
		trueMemLimit = estrdup(memLimit);
	}

	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("memoryLimit"),trueMemLimit TSRMLS_CC);

	memZval = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("memoryLimit"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(memZval)){
		ini_set("memory_limit",Z_STRVAL_P(memZval));
	}

	RETVAL_ZVAL(getThis(),1,0);
	efree(trueMemLimit);
}

PHP_METHOD(CConsumer,setTimeLimit)
{
	int		maxTime = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&maxTime) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setTimeLimit] the 1 parameter type error , must be int ", 1 TSRMLS_CC);
		return;
	}
	set_time_limit(maxTime);
	zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("timeLimit"),maxTime TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,registerHeartbeatCallback)
{
	zval	*callObject;
	char	*callFun,
			*functionName;
	int		callFunLen = 0,
			timeWhich = 0;

	zend_class_entry *classEntry;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs|l",&callObject,&callFun,&callFunLen,&timeWhich) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->registerHeartbeatCallback] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	//判断是否为对象
	if(IS_OBJECT != Z_TYPE_P(callObject)){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] register heartbeat callback the params is not an object", 1 TSRMLS_CC);
		return;
	}

	//判断对象是否包含方法
	classEntry = Z_OBJCE_P(callObject);
	functionName = estrdup(callFun);
	php_strtolower(functionName,strlen(functionName)+1);
	if(!zend_hash_exists(&classEntry->function_table,callFun,strlen(callFun)+1)){
		efree(functionName);
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] register heartbeat callback the object has not this function", 1 TSRMLS_CC);
		return;
	}

	//更新类变量
	zend_update_property(CConsumerCe,getThis(),ZEND_STRL("heartbeatObject"),callObject TSRMLS_CC);
	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("heartbeatFunction"),callFun TSRMLS_CC);
	zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("heartBeatTime"),timeWhich TSRMLS_CC);

	efree(functionName);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setListKey)
{
	char	*id,
			*command;
	int		idLen = 0,
			commandLen = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&id,&idLen,&command,&commandLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setListKey] the 1 parameter type error , must be string ", 1 TSRMLS_CC);
		return;
	}

	//listCommand
	if(commandLen > 0){
		php_strtolower(command,strlen(command)+1);
		if(strcmp(command,"rpop") == 0){
			zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("listCommand"),"rpop" TSRMLS_CC);
		}
	}

	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("listKey"),id TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setProducer)
{

	char	*exchange,
			*route,
			*queue;

	int		exchangeLen = 0,
			routeLen = 0,
			queueLen = 0,
			autoAck = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sss|b",&exchange,&exchangeLen,&route,&routeLen,&queue,&queueLen,&autoAck) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setProducer] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	if(exchangeLen > 0){
		zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("producerExchange"),exchange TSRMLS_CC);
	}

	if(routeLen > 0){
		zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("producerRoute"),route TSRMLS_CC);
	}

	if(queueLen > 0){
		zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("producerQueue"),queue TSRMLS_CC);
	}

	zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("producerAutoAck"),autoAck TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setMQId)
{
	char	*id;
	int		idLen = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&id,&idLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setMQId] the 1 parameter type error , must be string ", 1 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("mqId"),id TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setMQType)
{
	char	*id;
	int		idLen = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&id,&idLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setMQType] the 1 parameter type error , must be string ", 1 TSRMLS_CC);
		return;
	}
	
	php_strtolower(id,strlen(id)+1);
	if(strcmp(id,"redis") == 0){
		zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("mqType"),2 TSRMLS_CC);
	}else{
		zend_update_property_long(CConsumerCe,getThis(),ZEND_STRL("mqType"),1 TSRMLS_CC);
	}


	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,registerMessageCallback)
{

	zval	*callObject;
	char	*callFun,
			*functionName;
	int		callFunLen = 0;

	zend_class_entry *classEntry;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&callObject,&callFun,&callFunLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->registerMessageCallback] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	//判断是否为对象
	if(IS_OBJECT != Z_TYPE_P(callObject)){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] register message callback the params is not an object", 1 TSRMLS_CC);
		return;
	}

	//判断对象是否包含方法
	classEntry = Z_OBJCE_P(callObject);
	functionName = estrdup(callFun);
	php_strtolower(functionName,strlen(functionName)+1);
	if(!zend_hash_exists(&classEntry->function_table,functionName,strlen(functionName)+1)){
		efree(functionName);
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] register message callback the object has not this function", 1 TSRMLS_CC);
		return;
	}

	//更新类变量
	zend_update_property(CConsumerCe,getThis(),ZEND_STRL("processMessageObject"),callObject TSRMLS_CC);
	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("processMessageFunction"),callFun TSRMLS_CC);

	efree(functionName);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,setLogName)
{
	char	*logName;
	int		logNameLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&logName,&logNameLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setLogName] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CConsumerCe,getThis(),ZEND_STRL("logName"),logName TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CConsumer,ack)
{

	zval	*messageObject,
			*rabbitObject,
			messageReturn,
			*mqId,
			*exchange,
			*route,
			*queue,
			*autoAck;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&messageObject) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->ack] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(messageObject)){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->ack] the parameter type error", 1 TSRMLS_CC);
		return;
	}

	//mqID
	mqId = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("mqId"), 0 TSRMLS_CC);
	exchange = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("producerExchange"), 0 TSRMLS_CC);
	route = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("producerRoute"), 0 TSRMLS_CC);
	queue = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("producerQueue"), 0 TSRMLS_CC);
	autoAck = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("producerAutoAck"), 0 TSRMLS_CC);

	//获取CRabbit对象
	CRabbit_getInstance(&rabbitObject,Z_STRVAL_P(mqId) TSRMLS_CC);
	if(EG(exception)){
		Z_OBJ_HANDLE_P(EG(exception)) = 0;		
		zval_ptr_dtor(&rabbitObject);
		zend_clear_exception(TSRMLS_C);
		return;
	}

	//调用getExchange
	MODULE_BEGIN
		zval	constructVal,
				contruReturn,
				*paramsList[1],
				params1;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],exchange,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"getExchange", 0);
		call_user_function(NULL, &rabbitObject, &constructVal, &contruReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&contruReturn);
		if(EG(exception)){
			zval_ptr_dtor(&rabbitObject);
			Z_OBJ_HANDLE_P(EG(exception)) = 0;	
			zend_clear_exception(TSRMLS_C);
			return;
		}
	MODULE_END

	//调用ack
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*paramsList[3],
				params1,
				params2,
				params3;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		paramsList[2] = &params3;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_ZVAL(paramsList[0],messageObject,1,0);
		ZVAL_ZVAL(paramsList[1],route,1,0);
		ZVAL_ZVAL(paramsList[2],queue,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"ack", 0);
		call_user_function(NULL, &rabbitObject, &constructVal, &messageReturn, 3, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_dtor(&messageReturn);
	MODULE_END

	//销毁资源
	zval_ptr_dtor(&rabbitObject);

	RETVAL_ZVAL(getThis(),1,0);
}

//设置日志
void setLog(zval *object,char *message TSRMLS_DC){
	
	zval	*logName,
			*exchange,
			*route,
			*queue,
			*appPath;

	char	*messageSave,
			*logTruePath,
			*thisMothTime;

	int		pid = 0;


	//日志文件名字
	logName = zend_read_property(CConsumerCe,object,ZEND_STRL("logName"),0 TSRMLS_CC);
	exchange = zend_read_property(CConsumerCe,object,ZEND_STRL("producerExchange"),0 TSRMLS_CC);
	route = zend_read_property(CConsumerCe,object,ZEND_STRL("producerRoute"),0 TSRMLS_CC);
	queue = zend_read_property(CConsumerCe,object,ZEND_STRL("producerQueue"),0 TSRMLS_CC);
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);

	php_date("Y-m-d H:i:s",&thisMothTime);

#ifndef PHP_WIN32
	pid = getpid();
#endif

	//队列前缀
	spprintf(&messageSave,0,"#LogTime[Pid:%d]:%s%sLogContent:[%s]->[%s]->[%s] : %s%s%s",pid,thisMothTime,PHP_EOL,Z_STRVAL_P(exchange),Z_STRVAL_P(route),Z_STRVAL_P(queue),message,PHP_EOL,PHP_EOL);

	//日志路径
	spprintf(&logTruePath,0,"%s/logs/userlog/%s.log",Z_STRVAL_P(appPath),Z_STRVAL_P(logName));
	CLog_writeFile(logTruePath,messageSave);

	php_printf("%s===%s\n",logTruePath,messageSave);

	efree(messageSave);
	efree(logTruePath);
	efree(thisMothTime);
}


void CConsumer_getRedisMessage(zval *object,zval **returnObject TSRMLS_DC){

	zval	*listKey,
			*mqId,
			*redisInstance,
			*callParams,
			*redisReturnData,
			*redisMessageObject,
			*redisCommand;


	MAKE_STD_ZVAL(*returnObject);
	ZVAL_NULL(*returnObject);

	listKey = zend_read_property(CConsumerCe,object,ZEND_STRL("listKey"), 0 TSRMLS_CC);
	mqId = zend_read_property(CConsumerCe,object,ZEND_STRL("mqId"), 0 TSRMLS_CC);

	if(strlen(Z_STRVAL_P(listKey)) == 0){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->run] when set MQ type is Redis must call [CConsumer->setListKey] to set which list will use", 1 TSRMLS_CC);
		return;
	}

	//listCommand
	redisCommand = zend_read_property(CConsumerCe,object,ZEND_STRL("listCommand"), 0 TSRMLS_CC);
	
	//get Redis Object
	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,Z_STRVAL_P(listKey),1);
	CRedis_callFunction(Z_STRVAL_P(mqId),Z_STRVAL_P(redisCommand),callParams,&redisReturnData TSRMLS_CC);
	zval_ptr_dtor(&callParams);

	//find exception
	if(EG(exception)){
		Z_OBJ_HANDLE_P(EG(exception)) = 0;	
		zend_clear_exception(TSRMLS_C);
		return;
	}

	//no message
	if(IS_NULL == Z_TYPE_P(redisReturnData) || (IS_BOOL == Z_TYPE_P(redisReturnData) && 0 == Z_LVAL_P(redisReturnData) ) || (IS_ARRAY == Z_TYPE_P(redisReturnData) && 0 == zend_hash_num_elements(Z_ARRVAL_P(redisReturnData)))){
		zval_ptr_dtor(&redisReturnData);
		return;
	}

	//create a redisMessage
	MAKE_STD_ZVAL(redisMessageObject);
	object_init_ex(redisMessageObject,CRedisMessageCe);
	zend_update_property(CRedisMessageCe,redisMessageObject,ZEND_STRL("bodyContent"),redisReturnData TSRMLS_CC);
	zval_ptr_dtor(&redisReturnData);

	ZVAL_ZVAL(*returnObject,redisMessageObject,1,1);
}

//重启自身
void CConsumer_restartSelf(TSRMLS_D){

	//获取cli下的argv
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
	if (zend_hash_find(Z_ARRVAL_PP(SERVER),"argv",strlen("argv") + 1,(void **)&ret) == SUCCESS ){

		zval	**file,
				**params;

		if(IS_ARRAY == Z_TYPE_PP(ret) && SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(ret),0,(void**)&file) && SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(ret),1,(void**)&params) ){

			//命令
			char *restartCommand;
			spprintf(&restartCommand,0,"nohup php %s %s >/dev/null 2>log &",Z_STRVAL_PP(file),Z_STRVAL_PP(params));
			exec_shell(restartCommand);
			efree(restartCommand);
		}
	}
	
	//kill self
	zend_bailout();
}

void CConsumer_getRabbitMessage(zval *object,zval **returnObject TSRMLS_DC){

	zval	*rabbitObject,
			*messageObject,
			messageReturn,
			*mqId,
			*exchange,
			*route,
			*queue,
			*autoAck,
			*rabbitMessageObjct,
			*saveAmqp,
			*saveObject;

	MAKE_STD_ZVAL(*returnObject);
	ZVAL_NULL(*returnObject);

	//mqID
	mqId = zend_read_property(CConsumerCe,object,ZEND_STRL("mqId"), 0 TSRMLS_CC);
	exchange = zend_read_property(CConsumerCe,object,ZEND_STRL("producerExchange"), 0 TSRMLS_CC);
	route = zend_read_property(CConsumerCe,object,ZEND_STRL("producerRoute"), 0 TSRMLS_CC);
	queue = zend_read_property(CConsumerCe,object,ZEND_STRL("producerQueue"), 0 TSRMLS_CC);
	autoAck = zend_read_property(CConsumerCe,object,ZEND_STRL("producerAutoAck"), 0 TSRMLS_CC);

	//获取CRabbit对象
	CRabbit_getInstance(&rabbitObject,Z_STRVAL_P(mqId) TSRMLS_CC);

	if(EG(exception)){

		zval	*exceptionMessage;
		zend_class_entry *exceptionCe;
		exceptionCe = Z_OBJCE_P(EG(exception));
		exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
		setLog(object,Z_STRVAL_P(exceptionMessage) TSRMLS_CC);

		zval_ptr_dtor(&rabbitObject);
		Z_OBJ_HANDLE_P(EG(exception)) = 0;	
		zend_clear_exception(TSRMLS_C);
		
		//强行中止等待重启
		CConsumer_restartSelf(TSRMLS_C);
		return;
	}

	//调用getExchange
	MODULE_BEGIN
		zval	constructVal,
				contruReturn,
				*paramsList[1],
				params1;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],exchange,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"getExchange", 0);
		call_user_function(NULL, &rabbitObject, &constructVal, &contruReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&contruReturn);
		if(EG(exception)){
			zval_ptr_dtor(&rabbitObject);
			Z_OBJ_HANDLE_P(EG(exception)) = 0;	
			zend_clear_exception(TSRMLS_C);
			return;
		}
	MODULE_END


	//调用get
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*paramsList[3],
				params1,
				params2,
				params3;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		paramsList[2] = &params3;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_ZVAL(paramsList[0],route,1,0);
		ZVAL_ZVAL(paramsList[1],queue,1,0);
		ZVAL_ZVAL(paramsList[2],autoAck,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"get", 0);
		call_user_function(NULL, &rabbitObject, &constructVal, &messageReturn, 3, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		messageObject = &messageReturn;
	MODULE_END

	if(IS_OBJECT != Z_TYPE_P(messageObject)){
		zval_ptr_dtor(&rabbitObject);
		zval_dtor(&messageReturn);
		return;
	}

	//生成一个CRabbitMessage对象
	MAKE_STD_ZVAL(rabbitMessageObjct);
	object_init_ex(rabbitMessageObjct,CRabbitMessageCe);
	MAKE_STD_ZVAL(saveAmqp);
	ZVAL_ZVAL(saveAmqp,messageObject,1,0);
	MAKE_STD_ZVAL(saveObject);
	ZVAL_ZVAL(saveObject,object,1,0);
	zend_update_property(CRabbitMessageCe,rabbitMessageObjct,ZEND_STRL("amqpObject"),saveAmqp TSRMLS_CC);
	zend_update_property(CRabbitMessageCe,rabbitMessageObjct,ZEND_STRL("consumerObject"),saveObject TSRMLS_CC);
	zval_ptr_dtor(&saveAmqp);
	zval_ptr_dtor(&saveObject);
	
	//返回
	ZVAL_ZVAL(*returnObject,rabbitMessageObjct,1,0);

	//销毁资源
	zval_ptr_dtor(&rabbitObject);
	zval_dtor(&messageReturn);
	zval_ptr_dtor(&rabbitMessageObjct);
}

//沉睡N秒
void CConsumer_sleep(zval *object TSRMLS_DC){
	zval	*sleepSec;
	long		trueSleep = 3;
	sleepSec = zend_read_property(CConsumerCe,object,ZEND_STRL("emptySleepTime"), 0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(sleepSec)){
		trueSleep = Z_LVAL_P(sleepSec);
	}
	php_sleep(trueSleep);
}

//检查是否触发心跳
void CConsumer_checkHeartbeat(zval *object TSRMLS_DC){

	zval	*emptySec,
			*heartBeatTime,
			*heartObject,
			*heartFunction;

	int		needHeart = 0;

	emptySec = zend_read_property(CConsumerCe,object,ZEND_STRL("emptySec"), 0 TSRMLS_CC);
	heartObject = zend_read_property(CConsumerCe,object,ZEND_STRL("heartbeatObject"), 0 TSRMLS_CC);
	heartFunction = zend_read_property(CConsumerCe,object,ZEND_STRL("heartbeatFunction"), 0 TSRMLS_CC);
	heartBeatTime = zend_read_property(CConsumerCe,object,ZEND_STRL("heartBeatTime"), 0 TSRMLS_CC);
	

	if(IS_LONG == Z_TYPE_P(emptySec) && IS_LONG == Z_TYPE_P(heartBeatTime) && Z_LVAL_P(emptySec) % Z_LVAL_P(heartBeatTime) == 0){
		needHeart = 1;
	}

	//触发心跳回调
	if(1 == needHeart){
		zval	constructVal,
				contruReturn;
		char	*callFunctionName;
		callFunctionName = estrdup(Z_STRVAL_P(heartFunction));
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callFunctionName, 0);
		call_user_function(NULL, &heartObject, &constructVal, &contruReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&contruReturn);
		efree(callFunctionName);
		setLog(object,"run heartbeat function ... " TSRMLS_CC);
	}
}

//触发消息回调
void CConsumer_parseMessage(zval *object,zval *message TSRMLS_DC){
	
	zval	*processNum,
			*messageObject,
			*messageFunction;

	int		trueProcessNum = 1;

	processNum = zend_read_property(CConsumerCe,object,ZEND_STRL("processNum"), 0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(processNum)){
		trueProcessNum = Z_LVAL_P(processNum) + 1;
	}
	zend_update_property_long(CConsumerCe,object,ZEND_STRL("processNum"),trueProcessNum TSRMLS_CC);

	//触发消息回调
	messageObject = zend_read_property(CConsumerCe,object,ZEND_STRL("processMessageObject"), 0 TSRMLS_CC);
	messageFunction = zend_read_property(CConsumerCe,object,ZEND_STRL("processMessageFunction"), 0 TSRMLS_CC);

	MODULE_BEGIN
		zval	constructVal,
				contruReturn,
				params1,
				*paramsList[2];
		char	*callFunctionName;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_ZVAL(paramsList[0],message,1,0);
		ZVAL_ZVAL(paramsList[1],object,1,0);
		callFunctionName = estrdup(Z_STRVAL_P(messageFunction));
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callFunctionName, 0);
		call_user_function(NULL, &messageObject, &constructVal, &contruReturn, 2, paramsList TSRMLS_CC);
		zval_dtor(&contruReturn);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		efree(callFunctionName);
	MODULE_END
}

void CConsumer_checkProcessMax(zval *object TSRMLS_DC){

	zval	*processNum,
			*maxProcessNum;

	processNum = zend_read_property(CConsumerCe,object,ZEND_STRL("processNum"), 0 TSRMLS_CC);
	maxProcessNum = zend_read_property(CConsumerCe,object,ZEND_STRL("processMaxNum"), 0 TSRMLS_CC);

	//终结自身
	if(IS_LONG == Z_TYPE_P(processNum) && IS_LONG == Z_TYPE_P(maxProcessNum) && Z_LVAL_P(maxProcessNum) > 0  && Z_LVAL_P(processNum) >= Z_LVAL_P(maxProcessNum)){
		char tips[1024];
		sprintf(tips,"%s%d%s%d%s","process has run (",Z_LVAL_P(processNum),"/",Z_LVAL_P(maxProcessNum),") kill myself ...");
		setLog(object,tips TSRMLS_CC);
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[CQuickFameworkSystem] Framework kill self");
	}
}



//zhixing  RabbitMQ
void CConsumer_run(int mqType,zval *object TSRMLS_DC){

	while(1){

		zval	*message = NULL;

		//get diff mq message
		if(mqType == 1){
			CConsumer_getRabbitMessage(object,&message TSRMLS_CC);
		}else if(mqType == 2){
			CConsumer_getRedisMessage(object,&message TSRMLS_CC);
		}else{
			setLog(object,"The CConsumer cant not support this MQ Type .." TSRMLS_CC);
			break;
		}
	
		//消息为空时沉睡
		if(IS_NULL == Z_TYPE_P(message)){

			//输出日志
			setLog(object,"No Task , System sleep .." TSRMLS_CC);
			zval_ptr_dtor(&message);

			//沉睡N秒
			CConsumer_sleep(object TSRMLS_CC);

			//判断是否触发心跳函数
			CConsumer_checkHeartbeat(object TSRMLS_CC);

			//持续空闲的时间+1
			MODULE_BEGIN
				zval *emptySec;
				int		nowSec = 0;
				emptySec = zend_read_property(CConsumerCe,object,ZEND_STRL("emptySec"), 0 TSRMLS_CC);
				if(IS_LONG == Z_TYPE_P(emptySec)){
					nowSec = Z_LVAL_P(emptySec) + 1;
				}
				zend_update_property_long(CConsumerCe,object,ZEND_STRL("emptySec"),nowSec TSRMLS_CC);
			MODULE_END

			//继续阻塞等待
			continue;
		}


		//重置等待秒数
		zend_update_property_long(CConsumerCe,object,ZEND_STRL("emptySec"),1 TSRMLS_CC);


		//触发消息回调
		CConsumer_parseMessage(object,message TSRMLS_CC);

		//判断处理消息次数
		CConsumer_checkProcessMax(object TSRMLS_CC);

		//销毁资源
		zval_ptr_dtor(&message);
	}
}



PHP_METHOD(CConsumer,run)
{
	zval *mqType;

	//read mqType
	mqType = zend_read_property(CConsumerCe,getThis(),ZEND_STRL("mqType"), 0 TSRMLS_CC);


	CConsumer_run(Z_LVAL_P(mqType),getThis() TSRMLS_CC);

}
