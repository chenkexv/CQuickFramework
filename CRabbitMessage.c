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
#include "php_CRabbitMessage.h"
#include "php_CConsumer.h"
#include "php_CException.h"

ZEND_BEGIN_ARG_INFO_EX(CRabbitMessage_call_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

//zend类方法
zend_function_entry CRabbitMessage_functions[] = {
	PHP_ME(CRabbitMessage,setAMQP,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitMessage,__call,CRabbitMessage_call_arginfo,ZEND_ACC_PUBLIC)
	PHP_ME(CRabbitMessage,ack,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRabbitMessage)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CRabbitMessage",CRabbitMessage_functions);
	CRabbitMessageCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CRabbitMessageCe, ZEND_STRL("amqpObject"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRabbitMessageCe, ZEND_STRL("consumerObject"),ZEND_ACC_PRIVATE TSRMLS_CC);
	return SUCCESS;
}


PHP_METHOD(CRabbitMessage,setAMQP)
{
	zval	*object;

	//接入参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&object) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setAMQP] the 1 parameter type error", 1 TSRMLS_CC);
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(object)){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] Call [CConsumer->setAMQP] the 1 parameter type error", 1 TSRMLS_CC);
		return;
	}

	zend_update_property(CRabbitMessageCe,getThis(),ZEND_STRL("amqpObject"),object TSRMLS_CC);
}

PHP_METHOD(CRabbitMessage,__call)
{
	char	*val;
	int		valLen,
			i,
			num;
	zval	*args,
			*paramsList[64],
			param,
			**thisVal,
			*callObject,
			*returnZval;

	zend_function *requsetAction;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&val,&valLen,&args) == FAILURE){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] method parameter error", 12012 TSRMLS_CC);
		return;
	}

	//对象
	callObject = zend_read_property(CRabbitMessageCe,getThis(),ZEND_STRL("amqpObject"),0 TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(callObject)){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] CMyFrame internal error, CRabbitMessage connection object is lost", 12013 TSRMLS_CC);
		return;
	}

	//判断对象是否存在该方法
	php_strtolower(val,strlen(val)+1);
	if(zend_hash_find( &(Z_OBJCE_P(callObject)->function_table),val,strlen(val)+1,(void**)&requsetAction) != SUCCESS){
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQueueException] CRabbitMessage does not exist the method : ",val);
		zend_throw_exception(CQueueExceptionCe, errorMessage, 12014 TSRMLS_CC);
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(args));

	if(num >= 64){
		zend_throw_exception(CQueueExceptionCe, "[CQueueException] CRabbitMessage execution method parameter number too much", 12012 TSRMLS_CC);
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
		call_user_function(NULL, &callObject, &constructVal, &constructReturn, num, paramsList TSRMLS_CC);
		returnZval = &constructReturn;
		
		for(i = 0 ; i < num ; i++){
			zval_ptr_dtor(&paramsList[i]);
		}

		if(EG(exception)){
			char errMessage[1024];
			sprintf(errMessage,"%s%s%s","[CQueueException] Unable to connect to the CRabbit server to CallFunction: ",val,"()");
			zend_clear_exception(TSRMLS_C);
			zend_throw_exception(CRedisExceptionCe, errMessage, 1001 TSRMLS_CC);
			return;
		}

		ZVAL_ZVAL(return_value,returnZval,1,0);
		zval_dtor(&constructReturn);

	MODULE_END
}

PHP_METHOD(CRabbitMessage,ack){

	zval	*rabbitObject,
			*callObject,
			*messageObject,
			messageReturn,
			*mqId,
			*exchange,
			*route,
			*queue,
			*autoAck,
			*consumerObject;

	//对象
	callObject = zend_read_property(CRabbitMessageCe,getThis(),ZEND_STRL("amqpObject"),0 TSRMLS_CC);

	//consumer对象
	consumerObject = zend_read_property(CRabbitMessageCe,getThis(),ZEND_STRL("consumerObject"),0 TSRMLS_CC);

	//mqID
	mqId = zend_read_property(CConsumerCe,consumerObject,ZEND_STRL("mqId"), 0 TSRMLS_CC);
	exchange = zend_read_property(CConsumerCe,consumerObject,ZEND_STRL("producerExchange"), 0 TSRMLS_CC);
	route = zend_read_property(CConsumerCe,consumerObject,ZEND_STRL("producerRoute"), 0 TSRMLS_CC);
	queue = zend_read_property(CConsumerCe,consumerObject,ZEND_STRL("producerQueue"), 0 TSRMLS_CC);
	autoAck = zend_read_property(CConsumerCe,consumerObject,ZEND_STRL("producerAutoAck"), 0 TSRMLS_CC);

	//获取CRabbit对象
	CRabbit_getInstance(&rabbitObject,Z_STRVAL_P(mqId) TSRMLS_CC);
	if(EG(exception)){
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
		ZVAL_ZVAL(paramsList[0],callObject,1,0);
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
}