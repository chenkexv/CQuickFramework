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
#include "php_CRabbit.h"
#include "php_CRabbitHelper.h"
#include "php_CException.h"

//zend类方法
zend_function_entry CRabbitHelper_functions[] = {
	PHP_ME(CRabbitHelper,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRabbitHelper,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CRabbitHelper,getOverview,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueues,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getAckRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getGetRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getReadyLen,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getPublishRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getDeliverGetRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getUnAckRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getUnAckNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueInfo,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueAckRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueGetRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueuePublishRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueDeliverGetRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueUnAckRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueUnAckNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueIdleSince,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueState,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getQueueConsumerNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodes,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodeInfo,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodeMemoryUsedRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodeFdUsedRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodeSocketUsedRate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitHelper,getNodeErlangProcessUsedRate,NULL,ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRabbitHelper)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CRabbitHelper",CRabbitHelper_functions);
	CRabbitHelperCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//class params
	zend_declare_property_null(CRabbitHelperCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CRabbitHelperCe, ZEND_STRL("_configName"),"main",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRabbitHelperCe, ZEND_STRL("_configData"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRabbitHelperCe, ZEND_STRL("_requestHeader"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CRabbitHelperCe, ZEND_STRL("_requestHost"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void CRabbitHelper_getInstance(zval **returnZval,char *groupName TSRMLS_DC)
{
	zval	*instanceZval,
			**instaceSaveZval;

	int hasExistConfig;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CRabbitCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CRabbitCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CRabbitCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(instanceZval), groupName, strlen(groupName)+1);

	//为空时则实例化自身
	if(0 == hasExistConfig ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CRabbitHelperCe);

		//执行构造器
		if (CRabbitCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 params1;
			zval *paramsList[1];
			paramsList[0] = &params1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],groupName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CRabbitCe->constructor->common.function_name, 0);
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
		zend_update_static_property(CRabbitCe, ZEND_STRL("instance"),instanceZval TSRMLS_CC);

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

	zend_throw_exception(CQueueExceptionCe, "[CQueueException] An internal error occurred while CMyFrameExtension was acquired by AMQP ", 1 TSRMLS_CC);
}


PHP_METHOD(CRabbitHelper,getInstance)
{
	zval *instanceZval;
	char *groupName = "main";
	int groupNameLen = 0;

	//判断是否指定序列
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException]The server is not currently installed AMQP extension", 1 TSRMLS_CC);
		return;
	}

	CRabbitHelper_getInstance(&instanceZval,groupName TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	zval_ptr_dtor(&instanceZval);
}

PHP_METHOD(CRabbitHelper,__construct)
{
	char	*groupName,
			thisConfigKey[1024],
			*authString,
			requestHost[10240];

	int		groupNameLen = 0,
			consolePort = 15672;

	zval	*cconfigInstanceZval,
			*connectParamsZval,
			*requestHeader,
			**loginZval,
			**passwordZval,
			**consolePortZval,
			**hostZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&groupName,&groupNameLen) == FAILURE){
		zend_throw_exception(CRedisExceptionCe, "[CQueueException] CRabbitHelper execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

	if(groupNameLen == 0){
		zend_throw_exception(CRedisExceptionCe, "[CQueueException] CRabbitHelper execution method parameter error", 12012 TSRMLS_CC);
		return;
	}

	//获取配置对象
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	
	//当前配置的名称
	sprintf(thisConfigKey,"%s%s%s","RABBITMQ.",groupName,".connection");

	//获取连接配置
	CConfig_load(thisConfigKey,cconfigInstanceZval,&connectParamsZval TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(connectParamsZval)){
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQueueException] There is no specified MQ configuration : ",thisConfigKey);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&connectParamsZval);
		zend_throw_exception(CQueueExceptionCe, errorMessage, 6 TSRMLS_CC);
		return;
	}

	//update to class property
	zend_update_property(CRabbitHelperCe,getThis(),ZEND_STRL("_configData"),connectParamsZval TSRMLS_CC);

	//read login and password from config
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(connectParamsZval),"login",strlen("login")+1,(void**)&loginZval) && IS_STRING == Z_TYPE_PP(loginZval) ){
	}else{
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQueueException] read config error , lose the login key , configuration : ",thisConfigKey);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&connectParamsZval);
		zend_throw_exception(CQueueExceptionCe, errorMessage, 6 TSRMLS_CC);
		return;
	}
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(connectParamsZval),"password",strlen("password")+1,(void**)&passwordZval) && IS_STRING == Z_TYPE_PP(passwordZval) ){
	}else{
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQueueException] read config error , lose the password key , configuration : ",thisConfigKey);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&connectParamsZval);
		zend_throw_exception(CQueueExceptionCe, errorMessage, 6 TSRMLS_CC);
		return;
	}
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(connectParamsZval),"host",strlen("host")+1,(void**)&hostZval) && IS_STRING == Z_TYPE_PP(hostZval) ){
	}else{
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQueueException] read config error , lose the host key , configuration : ",thisConfigKey);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&connectParamsZval);
		zend_throw_exception(CQueueExceptionCe, errorMessage, 6 TSRMLS_CC);
		return;
	}


	//create request http header
	MODULE_BEGIN
		char	*loginAndPass,
				*base64Pass;
		spprintf(&loginAndPass,0,"%s%s%s",Z_STRVAL_PP(loginZval),":",Z_STRVAL_PP(passwordZval));
		base64Encode(loginAndPass,&base64Pass);
		spprintf(&authString, 0,"%s%s","Authorization:Basic ", base64Pass);
		efree(loginAndPass);
		efree(base64Pass);
	MODULE_END
	MAKE_STD_ZVAL(requestHeader);
	array_init(requestHeader);
	add_next_index_string(requestHeader,"Content-Type:application/json",1);
	add_next_index_string(requestHeader,authString,1);
	efree(authString);

	//create requestHost
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(connectParamsZval),"consolePort",strlen("consolePort")+1,(void**)&consolePortZval) && IS_LONG == Z_TYPE_PP(consolePortZval) ){
		consolePort = Z_LVAL_PP(consolePortZval);
	}

	//hostZval
	sprintf(requestHost,"%s%s%d",Z_STRVAL_PP(hostZval),":",consolePort);
	zend_update_property_string(CRabbitHelperCe,getThis(),ZEND_STRL("_requestHost"),requestHost TSRMLS_CC);
	zend_update_property(CRabbitHelperCe,getThis(),ZEND_STRL("_requestHeader"),requestHeader TSRMLS_CC);


	//destory
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&connectParamsZval);
	zval_ptr_dtor(&requestHeader);
}

void CRabbitHelper_callApi_overview(zval *object,zval **returnZval TSRMLS_DC){

	zval	*requestHost,
			*requestHeader,
			*requestParams,
			*curlReturnZval,
			**contentZval,
			*jsonZval;

	char	*requestUrl;

	//get request host and header
	requestHost = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHost"),0 TSRMLS_CC);
	requestHeader = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHeader"),0 TSRMLS_CC);

	//get request url
	spprintf(&requestUrl,0,"%s%s",Z_STRVAL_P(requestHost),"/api/overview");

	//request params is an empty array
	MAKE_STD_ZVAL(requestParams);
	array_init(requestParams);

	//send http request
	CResponse_sendHttpRequest(requestUrl,requestParams,"GET",requestHeader,15,&curlReturnZval TSRMLS_CC);

	//read content item
	MAKE_STD_ZVAL(*returnZval);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(curlReturnZval),"content",strlen("content")+1,(void**)&contentZval) && IS_STRING == Z_TYPE_PP(contentZval)){
	}else{
		//can not get a string , return an emtpy array
		efree(requestUrl);
		zval_ptr_dtor(&requestParams);
		zval_ptr_dtor(&curlReturnZval);
		array_init(*returnZval);
		return;
	}

	//parse json
	json_decode(Z_STRVAL_PP(contentZval),&jsonZval);	
	if(IS_ARRAY == Z_TYPE_P(jsonZval)){
		ZVAL_ZVAL(*returnZval,jsonZval,1,0);
	}else{
		array_init(*returnZval);
	}

	//destroy
	efree(requestUrl);
	zval_ptr_dtor(&requestParams);
	zval_ptr_dtor(&curlReturnZval);
	zval_ptr_dtor(&jsonZval);
}

PHP_METHOD(CRabbitHelper,getOverview)
{
	zval	*returnZval;
	CRabbitHelper_callApi_overview(getThis(),&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}


PHP_METHOD(CRabbitHelper,getAckRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['message_stats']['ack_details']['rate']) ? $data['message_stats']['ack_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"ack_details",strlen("ack_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getGetRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['message_stats']['get_details']['rate']) ? $data['message_stats']['get_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"get_details",strlen("get_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getReadyLen){

	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['queue_totals']['messages']) ? $data['queue_totals']['messages'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"queue_totals",strlen("queue_totals")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"messages",strlen("messages")+1,(void**)&node2) && 
		IS_LONG == Z_TYPE_PP(node2) 
	){
		RETVAL_ZVAL(*node2,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getPublishRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['message_stats']['publish_details']['rate']) ? $data['message_stats']['publish_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"publish_details",strlen("publish_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getDeliverGetRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['message_stats']['deliver_get_details']['rate']) ? $data['message_stats']['deliver_get_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"deliver_get_details",strlen("deliver_get_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getUnAckRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['queue_totals']['messages_unacknowledged_details']['rate']) ? $data['queue_totals']['messages_unacknowledged_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"queue_totals",strlen("queue_totals")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"messages_unacknowledged_details",strlen("messages_unacknowledged_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getUnAckNum)
{
	zval	*data,
			**node1,
			**node2,
			**node3;
	CRabbitHelper_callApi_overview(getThis(),&data TSRMLS_CC);

	//return isset($data['queue_totals']['messages_unacknowledged']) ? $data['queue_totals']['messages_unacknowledged'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"queue_totals",strlen("queue_totals")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"messages_unacknowledged",strlen("messages_unacknowledged")+1,(void**)&node2)
	){
		RETVAL_ZVAL(*node2,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

void CRabbitHelper_callApi_queues(zval *object,zval **returnZval TSRMLS_DC){

	zval	*requestHost,
			*requestHeader,
			*requestParams,
			*curlReturnZval,
			**contentZval,
			*jsonZval;

	char	*requestUrl;

	//get request host and header
	requestHost = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHost"),0 TSRMLS_CC);
	requestHeader = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHeader"),0 TSRMLS_CC);

	//get request url
	spprintf(&requestUrl,0,"%s%s",Z_STRVAL_P(requestHost),"/api/queues");

	//request params is an empty array
	MAKE_STD_ZVAL(requestParams);
	array_init(requestParams);

	//send http request
	CResponse_sendHttpRequest(requestUrl,requestParams,"GET",requestHeader,15,&curlReturnZval TSRMLS_CC);

	//read content item
	MAKE_STD_ZVAL(*returnZval);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(curlReturnZval),"content",strlen("content")+1,(void**)&contentZval) && IS_STRING == Z_TYPE_PP(contentZval)){
	}else{
		//can not get a string , return an emtpy array
		efree(requestUrl);
		zval_ptr_dtor(&requestParams);
		zval_ptr_dtor(&curlReturnZval);
		array_init(*returnZval);
		return;
	}

	//parse json
	json_decode(Z_STRVAL_PP(contentZval),&jsonZval);	
	if(IS_ARRAY == Z_TYPE_P(jsonZval)){
		int		i,h;
		zval	**thisVal,
				**nameZval,
				*saveThisVal;
		char	*name;

		//reset to set name
		array_init(*returnZval);

		h = zend_hash_num_elements(Z_ARRVAL_P(jsonZval));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_data(Z_ARRVAL_P(jsonZval),(void**)&thisVal);
	
			if(IS_ARRAY == Z_TYPE_PP(thisVal) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"name",strlen("name")+1,(void**)&nameZval) && IS_STRING == Z_TYPE_PP(nameZval) ){
				name = estrdup(Z_STRVAL_PP(nameZval));
			}else{
				spprintf(&name,0,"%s%d","queue",i);
			}
		
			MAKE_STD_ZVAL(saveThisVal);
			ZVAL_ZVAL(saveThisVal,*thisVal,1,0);
			add_assoc_zval(*returnZval,name,saveThisVal);

			efree(name);
			zend_hash_move_forward(Z_ARRVAL_P(jsonZval));
		}

	}else{
		array_init(*returnZval);
	}

	//destroy
	efree(requestUrl);
	zval_ptr_dtor(&requestParams);
	zval_ptr_dtor(&curlReturnZval);
	zval_ptr_dtor(&jsonZval);
}

PHP_METHOD(CRabbitHelper,getQueues)
{
	zval	*data;
	CRabbitHelper_callApi_queues(getThis(),&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CRabbitHelper_getQueueInfo(char *queueName,zval *object,zval **returnData TSRMLS_DC){

	zval	*data,
			**thisKeyVal;

	if(strlen(queueName) <= 0){
		MAKE_STD_ZVAL(*returnData);
		array_init(*returnData);
		return;
	}

	CRabbitHelper_callApi_queues(object,&data TSRMLS_CC);

	//get this key
	MAKE_STD_ZVAL(*returnData);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),queueName,strlen(queueName)+1,(void**)&thisKeyVal)){
		ZVAL_ZVAL(*returnData,*thisKeyVal,1,0);
	}else{
		array_init(*returnData);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueInfo)
{
	zval	*data;
	char	*queueName;
	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		array_init(return_value);
		return;
	}

	if(queueNameLen == 0){
		array_init(return_value);
		return;
	}

	//get queue
	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

PHP_METHOD(CRabbitHelper,getQueueAckRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['message_stats']['ack_details']['rate']) ? $queueInfo['message_stats']['ack_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"ack_details",strlen("ack_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueGetRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['message_stats']['get_details']['rate']) ? $queueInfo['message_stats']['get_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"get_details",strlen("get_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueuePublishRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['message_stats']['publish_details']['rate']) ? $queueInfo['message_stats']['publish_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"publish_details",strlen("publish_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueDeliverGetRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['message_stats']['deliver_get_details']['rate']) ? $queueInfo['message_stats']['deliver_get_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"deliver_get_details",strlen("deliver_get_details")+1,(void**)&node2) && 
		IS_ARRAY == Z_TYPE_PP(node2) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
	){
		RETVAL_ZVAL(*node3,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueUnAckRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['messages_unacknowledged_details']['rate']) ? $queueInfo['messages_unacknowledged_details']['rate'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"messages_unacknowledged_details",strlen("messages_unacknowledged_details")+1,(void**)&node1) && 
		IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"deliver_get_details",strlen("rate")+1,(void**)&node2) 
	){
		RETVAL_ZVAL(*node2,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueUnAckNum)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['messages_unacknowledged']) ? $queueInfo['messages_unacknowledged'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"messages_unacknowledged",strlen("messages_unacknowledged")+1,(void**)&node1) 
	){
		RETVAL_ZVAL(*node1,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueIdleSince)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['idle_since']) ? $queueInfo['idle_since'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"idle_since",strlen("idle_since")+1,(void**)&node1) 
	){
		RETVAL_ZVAL(*node1,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueState)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['state']) ? $queueInfo['state'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"state",strlen("state")+1,(void**)&node1) 
	){
		RETVAL_ZVAL(*node1,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getQueueConsumerNum)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getQueueInfo(queueName,getThis(),&data TSRMLS_CC);

	//return isset($queueInfo['consumers']) ? $queueInfo['consumers'] : 0;
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"consumers",strlen("consumers")+1,(void**)&node1) 
	){
		RETVAL_ZVAL(*node1,1,0);
	}else{
		RETVAL_LONG(0);
	}

	zval_ptr_dtor(&data);
}

void CRabbitHelper_callApi_nodes(zval *object,zval **returnZval TSRMLS_DC){

	zval	*requestHost,
			*requestHeader,
			*requestParams,
			*curlReturnZval,
			**contentZval,
			*jsonZval;

	char	*requestUrl;

	//get request host and header
	requestHost = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHost"),0 TSRMLS_CC);
	requestHeader = zend_read_property(CRabbitHelperCe,object,ZEND_STRL("_requestHeader"),0 TSRMLS_CC);

	//get request url
	spprintf(&requestUrl,0,"%s%s",Z_STRVAL_P(requestHost),"/api/nodes");

	//request params is an empty array
	MAKE_STD_ZVAL(requestParams);
	array_init(requestParams);

	//send http request
	CResponse_sendHttpRequest(requestUrl,requestParams,"GET",requestHeader,15,&curlReturnZval TSRMLS_CC);

	//read content item
	MAKE_STD_ZVAL(*returnZval);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(curlReturnZval),"content",strlen("content")+1,(void**)&contentZval) && IS_STRING == Z_TYPE_PP(contentZval)){
	}else{
		//can not get a string , return an emtpy array
		efree(requestUrl);
		zval_ptr_dtor(&requestParams);
		zval_ptr_dtor(&curlReturnZval);
		array_init(*returnZval);
		return;
	}

	//parse json
	json_decode(Z_STRVAL_PP(contentZval),&jsonZval);	
	if(IS_ARRAY == Z_TYPE_P(jsonZval)){
		int		i,h;
		zval	**thisVal,
				**node1,
				**node2,
				**node3,
				*saveThisVal;
		char	*name;

		//reset to set name
		array_init(*returnZval);

		h = zend_hash_num_elements(Z_ARRVAL_P(jsonZval));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_data(Z_ARRVAL_P(jsonZval),(void**)&thisVal);
	
			if(
					IS_ARRAY == Z_TYPE_PP(thisVal) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"cluster_links",strlen("cluster_links")+1,(void**)&node1) && 
					IS_ARRAY == Z_TYPE_PP(node1) && 
					SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && 
					IS_ARRAY == Z_TYPE_PP(node2) && 
					SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"name",strlen("name")+1,(void**)&node3) && 
					IS_STRING == Z_TYPE_PP(node3)
				){
				name = estrdup(Z_STRVAL_PP(node3));
			}else{
				spprintf(&name,0,"%s%d","node",i);
			}
		
			MAKE_STD_ZVAL(saveThisVal);
			ZVAL_ZVAL(saveThisVal,*thisVal,1,0);
			add_assoc_zval(*returnZval,name,saveThisVal);

			efree(name);
			zend_hash_move_forward(Z_ARRVAL_P(jsonZval));
		}

	}else{
		array_init(*returnZval);
	}

	//destroy
	efree(requestUrl);
	zval_ptr_dtor(&requestParams);
	zval_ptr_dtor(&curlReturnZval);
	zval_ptr_dtor(&jsonZval);
}

PHP_METHOD(CRabbitHelper,getNodes)
{
	zval	*data;
	CRabbitHelper_callApi_nodes(getThis(),&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CRabbitHelper_getNodeInfo(char *queueName,zval *object,zval **returnData TSRMLS_DC)
{
	zval	*data,
			**thisKeyVal;

	if(strlen(queueName) <= 0){
		MAKE_STD_ZVAL(*returnData);
		array_init(*returnData);
		return;
	}

	CRabbitHelper_callApi_nodes(object,&data TSRMLS_CC);

	//get this key
	MAKE_STD_ZVAL(*returnData);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),queueName,strlen(queueName)+1,(void**)&thisKeyVal)){
		ZVAL_ZVAL(*returnData,*thisKeyVal,1,0);
	}else{
		array_init(*returnData);
	}

	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getNodeInfo)
{
	zval	*data;
	char	*queueName;
	int		queueNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		array_init(return_value);
		return;
	}

	if(queueNameLen == 0){
		array_init(return_value);
		return;
	}

	//get queue
	CRabbitHelper_getNodeInfo(queueName,getThis(),&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

PHP_METHOD(CRabbitHelper,getNodeMemoryUsedRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	double	memRate = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getNodeInfo(queueName,getThis(),&data TSRMLS_CC);

	//mem_used
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"mem_used",strlen("mem_used")+1,(void**)&node1) && IS_LONG == Z_TYPE_PP(node1)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	//mem_limit
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"mem_limit",strlen("mem_limit")+1,(void**)&node2) && IS_LONG == Z_TYPE_PP(node2)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	
	//round
	memRate = (double)((double)Z_LVAL_PP(node1) / (double)Z_LVAL_PP(node2));	
	RETVAL_DOUBLE(memRate);
	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getNodeFdUsedRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	double	memRate = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getNodeInfo(queueName,getThis(),&data TSRMLS_CC);

	//fd_used
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"fd_used",strlen("fd_used")+1,(void**)&node1) && IS_LONG == Z_TYPE_PP(node1)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	//fd_total
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"fd_total",strlen("fd_total")+1,(void**)&node2) && IS_LONG == Z_TYPE_PP(node2)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	
	//round
	memRate = (double)((double)Z_LVAL_PP(node1) / (double)Z_LVAL_PP(node2));	
	RETVAL_DOUBLE(memRate);
	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getNodeSocketUsedRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	double	memRate = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getNodeInfo(queueName,getThis(),&data TSRMLS_CC);

	//sockets_used
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"sockets_used",strlen("sockets_used")+1,(void**)&node1) && IS_LONG == Z_TYPE_PP(node1)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	//sockets_total
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"sockets_total",strlen("sockets_total")+1,(void**)&node2) && IS_LONG == Z_TYPE_PP(node2)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	
	//round
	memRate = (double)((double)Z_LVAL_PP(node1) / (double)Z_LVAL_PP(node2));	
	RETVAL_DOUBLE(memRate);
	zval_ptr_dtor(&data);
}

PHP_METHOD(CRabbitHelper,getNodeErlangProcessUsedRate)
{
	zval	*data,
			**node1,
			**node2,
			**node3;

	char	*queueName;

	int		queueNameLen = 0;

	double	memRate = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&queueName,&queueNameLen) == FAILURE){
		RETVAL_LONG(0);
		return;
	}

	if(queueNameLen == 0){
		RETVAL_LONG(0);
		return;
	}

	CRabbitHelper_getNodeInfo(queueName,getThis(),&data TSRMLS_CC);

	//proc_used
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"proc_used",strlen("proc_used")+1,(void**)&node1) && IS_LONG == Z_TYPE_PP(node1)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	//proc_total
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"proc_total",strlen("proc_total")+1,(void**)&node2) && IS_LONG == Z_TYPE_PP(node2)){
	}else{
		RETVAL_LONG(0);
		zval_ptr_dtor(&data);
		return;
	}

	
	//round
	memRate = (double)((double)Z_LVAL_PP(node1) / (double)Z_LVAL_PP(node2));	
	RETVAL_DOUBLE(memRate);
	zval_ptr_dtor(&data);
}
