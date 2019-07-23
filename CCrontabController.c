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
#include "php_CCrontabController.h"
#include "php_CController.h"
#include "php_CException.h"
#ifndef PHP_WIN32
#include <sys/prctl.h>
#include <signal.h>
#endif


//zendÀà·½·¨
zend_function_entry CCrontabController_functions[] = {
	PHP_ME(CCrontabController,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CCrontabController,Action_recHttpPool,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CCrontabController,Action_poolWoker,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CCrontabController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CCrontabController",CCrontabController_functions);
	CCrontabControllerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);


	return SUCCESS;
}

PHP_METHOD(CCrontabController,__construct)
{
	zval	*sapiZval;

	//check is cli
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] 403 Forbidden ");
		return;
	}

	//set memlimit 
	ini_set("memory_limit","4048M");
}

#ifndef PHP_WIN32
static int do_abort = 0;
void handle_signal(int signo){
	if (signo == SIGHUP){                 
		php_printf("child recv parent process exit sighup..\n");
		do_abort = 1;
	}
}
#endif

PHP_METHOD(CCrontabController,Action_recHttpPool){

#ifndef PHP_WIN32

	//rec parent process exit signal
	signal(SIGHUP, handle_signal);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
	prctl(PR_SET_NAME, "CQuickHttpPool worker"); 

	zval	*llenParams,
			*llen,
			*rangeParams,
			*list,
			*message,
			*messageZval,
			**messageParams,
			**messageContent,
			**messageError,
			*callArr,
			**className,
			**funcName,
			*params[1],
			callUserFunc,
			returnUserFunc;

	int		nowMessageNum = 0,
			i,h,
			callStatus = 0,
			runTime = 0,
			runCheck = 1000;

	suseconds_t	allCastTime = 0;

	long double  btime,
				 etime,
				 pushRate = 0;

	char	*callbakFunctionName,
			*classNameLower;

	struct timeval	t_start,
					t_end;

	zend_class_entry	**classEntryPP,
						*classEntryCe;

	//get getParams "callback"
	getGetParam("callback",&callbakFunctionName TSRMLS_CC);

	if(callbakFunctionName == NULL){
		return;
	}

	//cut this
	php_explode("::",callbakFunctionName,&callArr TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(callArr) && zend_hash_num_elements(Z_ARRVAL_P(callArr))== 2){
	}else{
		zval_ptr_dtor(&callArr);
		efree(callbakFunctionName);
		return;
	}

	//check class exists
	zend_hash_index_find(Z_ARRVAL_P(callArr),0,(void**)&className);
	zend_hash_index_find(Z_ARRVAL_P(callArr),1,(void**)&funcName);
	classNameLower = estrdup(Z_STRVAL_PP(className));
	php_strtolower(classNameLower,strlen(classNameLower)+1);
	if(zend_hash_find(EG(class_table),classNameLower,strlen(classNameLower)+1,(void**)&classEntryPP) == FAILURE){
		
		//try to find file
		zval	*loadStatus;
		CLoader_load(Z_STRVAL_PP(className),&loadStatus TSRMLS_CC);
		if(IS_BOOL == Z_TYPE_P(loadStatus) && 1== Z_LVAL_P(loadStatus)){
			zval_ptr_dtor(&loadStatus);
		}else{
			zval_ptr_dtor(&loadStatus);
			efree(classNameLower);
			zval_ptr_dtor(&callArr);
			efree(callbakFunctionName);
			return;
		}
	}

	if(zend_hash_find(EG(class_table),classNameLower,strlen(classNameLower)+1,(void**)&classEntryPP) == FAILURE){
		zval_ptr_dtor(&callArr);
		efree(callbakFunctionName);
		efree(classNameLower);
		return;
	}

	efree(classNameLower);
	classEntryCe = *classEntryPP;
	if(!zend_hash_exists(&classEntryCe->function_table,Z_STRVAL_PP(funcName),strlen(Z_STRVAL_PP(funcName))+1)){
		zval_ptr_dtor(&callArr);
		efree(callbakFunctionName);
		return;
	}

	//is rec parent process exit , this will exit
	while(!do_abort) {

		//get message
		MAKE_STD_ZVAL(rangeParams);
		array_init(rangeParams);
		add_next_index_string(rangeParams,"CQuickFrameHttpPool",1);
		CRedis_callFunction("main","lpop",rangeParams,&message TSRMLS_DC);
		zval_ptr_dtor(&rangeParams);

		if(IS_STRING != Z_TYPE_P(message)){
			usleep(10000);
			zval_ptr_dtor(&message);
			continue;
		}

		
		//decodeThisMessage
		json_decode(Z_STRVAL_P(message),&messageZval);
		if(IS_ARRAY != Z_TYPE_P(messageZval)){
			zval_ptr_dtor(&messageZval);
			zval_ptr_dtor(&message);
			continue;
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageZval),"params",strlen("params")+1,(void**)&messageParams) && IS_STRING == Z_TYPE_PP(messageParams)){
			//base64decode
			char *base64Params;
			base64Decode(Z_STRVAL_PP(messageParams),&base64Params);
			add_assoc_string(messageZval,"params",base64Params,0);
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageZval),"content",strlen("content")+1,(void**)&messageContent) && IS_STRING == Z_TYPE_PP(messageContent)){
			//base64decode
			char *base64Params;
			base64Decode(Z_STRVAL_PP(messageContent),&base64Params);
			add_assoc_string(messageZval,"content",base64Params,0);
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageZval),"error",strlen("error")+1,(void**)&messageError) && IS_STRING == Z_TYPE_PP(messageError)){
			//base64decode
			char *base64Params;
			base64Decode(Z_STRVAL_PP(messageError),&base64Params);
			add_assoc_string(messageZval,"error",base64Params,0);
		}

		zval *thisObject;
		MAKE_STD_ZVAL(thisObject);
		object_init_ex(thisObject,classEntryCe);
	
		//call user function
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],messageZval,1,0);
		INIT_ZVAL(callUserFunc);
		ZVAL_STRING(&callUserFunc,Z_STRVAL_PP(funcName),0);
		call_user_function(NULL, &thisObject, &callUserFunc, &returnUserFunc, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnUserFunc);
		zval_ptr_dtor(&thisObject);
		
		//destory
		zval_ptr_dtor(&messageZval);
		zval_ptr_dtor(&message);
	}


	efree(callbakFunctionName);
	zval_ptr_dtor(&callArr);

#endif

}

int doRunObject(zval *callObject TSRMLS_DC)
{
	
	zval	**classZval,
			**objectZval,
			*object,
			*loadStatus;

	zend_class_entry	*classCe,
						**classEntryPP;

	if(IS_ARRAY != Z_TYPE_P(callObject)){
		return -1;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(callObject),"class",strlen("class")+1,(void**)&classZval) && 
		IS_STRING == Z_TYPE_PP(classZval) &&
		SUCCESS == zend_hash_find(Z_ARRVAL_P(callObject),"object",strlen("object")+1,(void**)&objectZval) &&
		IS_STRING == Z_TYPE_PP(objectZval)){
	}else{
		return -2;
	}

	php_strtolower(Z_STRVAL_PP(classZval),strlen(Z_STRVAL_PP(classZval))+1);
	if(zend_hash_find(EG(class_table),Z_STRVAL_PP(classZval),strlen(Z_STRVAL_PP(classZval))+1,(void**)&classEntryPP) == FAILURE){

		//load class find className
		CLoader_load(Z_STRVAL_PP(classZval),&loadStatus TSRMLS_CC);
		zval_ptr_dtor(&loadStatus);
	}


	unserialize(*objectZval,&object);
	classCe = Z_OBJCE_P(object);

	//call run function
	MODULE_BEGIN
		zval	callFunction,
				returnFunction;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"run",0);
		call_user_function(NULL, &object, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
		zval_dtor(&returnFunction);
	MODULE_END

	zval_ptr_dtor(&object);
	return 1;
}

PHP_METHOD(CCrontabController,Action_poolWoker){
	
#ifdef PHP_WIN32
#else

	signal(SIGHUP, handle_signal);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
	prctl(PR_SET_NAME, "CQuickPoolWoker"); 

	zval	*rangeParams,
			*message,
			*callObject,
			*jsonDecode;

	int		runStatus;

	while(!do_abort) {
	

		MAKE_STD_ZVAL(rangeParams);
		array_init(rangeParams);
		add_next_index_string(rangeParams,"CQuickFramePool",1);
		CRedis_callFunction("main","lpop",rangeParams,&message TSRMLS_DC);
		zval_ptr_dtor(&rangeParams);

		if(IS_STRING != Z_TYPE_P(message)){
			usleep(10000);
			zval_ptr_dtor(&message);
			continue;
		}

		json_decode(Z_STRVAL_P(message),&jsonDecode);
		runStatus = doRunObject(jsonDecode TSRMLS_CC);
		zval_ptr_dtor(&jsonDecode);
		zval_ptr_dtor(&message);
	}
	

#endif

}