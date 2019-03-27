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
#include "php_CDebug.h"
#include "php_CHooks.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CDebug_functions[] = {
	PHP_ME(CDebug,debug,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpDBExecute,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpErrors,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpIncludeFiles,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpMemoryUsed,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpTimeCast,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpPluginLoaded,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpRequestData,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,setHooks,NULL,ZEND_ACC_PUBLIC)

	//hooks function 
	PHP_ME(CDebug,getDatabaseExecuteEnd,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDebug,getErrorsData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDebug,getRequestShutdown,NULL,ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDebug)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CDebug",CDebug_functions);
	CDebugCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CDebugCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("pluginName"),"debugTool",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("author"),"uncleChen",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("version"),"0.1",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("copyright"),"by UncleChen",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("date"),"2013/12/7",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CDebugCe, ZEND_STRL("description"),"Quick output of debugging information",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CDebugCe, ZEND_STRL("_sqlList"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CDebugCe, ZEND_STRL("_errorList"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CDebugCe, ZEND_STRL("_lastError"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CDebug,debug)
{
	int		status = 0;
	zval	*cconfigInstanceZval,
			callFunction,
			returnFunction,
			*paramsList[2];

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&status) == FAILURE){
		return;
	}

	//reset config's debug 
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);
	ZVAL_STRING(paramsList[0],"DEBUG",1);
	ZVAL_BOOL(paramsList[1],status);
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"set",0);
	call_user_function(NULL, &cconfigInstanceZval, &callFunction, &returnFunction, 2, paramsList TSRMLS_CC);

	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	zval_dtor(&returnFunction);
	zval_ptr_dtor(&cconfigInstanceZval);
}

PHP_METHOD(CDebug,dumpDBExecute)
{
	zval	*object,
			*sqlList;

	int		isOurput = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isOurput) == FAILURE){
	}

	object = zend_read_static_property(CDebugCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//read errorList
	sqlList = zend_read_property(CDebugCe,object,ZEND_STRL("_sqlList"),1 TSRMLS_CC);

	if(isOurput){
		php_printf("<pre>");
		php_var_dump(&sqlList,1 TSRMLS_CC);
	}

	RETVAL_ZVAL(sqlList,1,0);
}

PHP_METHOD(CDebug,dumpErrors)
{
	zval	*object,
			*errorList;

	int		isOurput = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isOurput) == FAILURE){
	}

	object = zend_read_static_property(CDebugCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//read errorList
	errorList = zend_read_property(CDebugCe,object,ZEND_STRL("_errorList"),1 TSRMLS_CC);

	if(isOurput){
		php_printf("<pre>");
		php_var_dump(&errorList,1 TSRMLS_CC);
	}

	RETVAL_ZVAL(errorList,1,0);
}

PHP_METHOD(CDebug,dumpIncludeFiles)
{
	zval	callFunction,
			returnFunction,
			*returnData;

	int		isOurput = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isOurput) == FAILURE){
	}

	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"get_included_files",0);
	call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
	returnData = &returnFunction;

	if(isOurput){
		php_printf("<pre>");
		php_var_dump(&returnData,1 TSRMLS_CC);
	}

	ZVAL_ZVAL(return_value,returnData,1,0);
	zval_dtor(&returnFunction);
}

PHP_METHOD(CDebug,dumpMemoryUsed)
{
	zval	callFunction,
			returnFunction,
			*returnData;

	int		isOurput = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isOurput) == FAILURE){
	}

	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"memory_get_usage",0);
	call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
	returnData = &returnFunction;

	if(isOurput){
		php_printf("<pre>");
		php_var_dump(&returnData,1 TSRMLS_CC);
	}

	ZVAL_ZVAL(return_value,returnData,1,0);
	zval_dtor(&returnFunction);
}

PHP_METHOD(CDebug,dumpTimeCast)
{

}

PHP_METHOD(CDebug,dumpPluginLoaded)
{

}

PHP_METHOD(CDebug,dumpRequestData)
{

}

//register Hooks
PHP_METHOD(CDebug,setHooks)
{
	CHooks_registerHooks("HOOKS_EXECUTE_END","getDatabaseExecuteEnd",getThis(),0 TSRMLS_CC);
	CHooks_registerHooks("HOOKS_ERROR_HAPPEN","getErrorsData",getThis(),0 TSRMLS_CC);
	CHooks_registerHooks("HOOKS_SYSTEM_SHUTDOWN","getRequestShutdown",getThis(),0 TSRMLS_CC);
}


PHP_METHOD(CDebug,getDatabaseExecuteEnd)
{
	zval	*cbuilderObject,
			*saveData,
			*sqlList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&cbuilderObject) == FAILURE){
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(cbuilderObject)){
		return;
	}

	sqlList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_sqlList"),1 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(sqlList)){
		zval	*saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CDebugCe,getThis(),ZEND_STRL("_sqlList"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		sqlList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_sqlList"),0 TSRMLS_CC);
	}

	//create save array
	MAKE_STD_ZVAL(saveData);
	array_init(saveData);

	//get sql;
	MODULE_BEGIN
		zval	callFunction,
				returnFunction;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getSql", 0);
		call_user_function(NULL, &cbuilderObject, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
		if(IS_STRING == Z_TYPE(returnFunction)){
			add_assoc_string(saveData,"sql",Z_STRVAL(returnFunction),1);
		}
		zval_dtor(&returnFunction);
	MODULE_END

	//getTime cast
	MODULE_BEGIN
		zval	callFunction,
				returnFunction;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getCastTime", 0);
		call_user_function(NULL, &cbuilderObject, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
		if(IS_DOUBLE == Z_TYPE(returnFunction)){
			char	timeCast[60];
			sprintf(timeCast,"%.6f",Z_DVAL(returnFunction));
			add_assoc_string(saveData,"time",timeCast,1);
		}
		zval_dtor(&returnFunction);
	MODULE_END

	//is from master
	MODULE_BEGIN
		zval	callFunction,
				returnFunction;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getIsMaster", 0);
		call_user_function(NULL, &cbuilderObject, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
		if(IS_BOOL == Z_TYPE(returnFunction)){
			add_assoc_long(saveData,"master",Z_LVAL(returnFunction));
		}
		zval_dtor(&returnFunction);
	MODULE_END

	//no cache
	add_assoc_long(saveData,"cache",0);

	//wherevalue
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*returnWhere,
				*saveWhere;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getWhere", 0);
		call_user_function(NULL, &cbuilderObject, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);
		returnWhere = &returnFunction;
		MAKE_STD_ZVAL(saveWhere);
		ZVAL_ZVAL(saveWhere,returnWhere,1,0);
		add_assoc_zval(saveData,"whereValue",saveWhere);
		zval_dtor(&returnFunction);
	MODULE_END

	add_next_index_zval(sqlList,saveData);
}

void CDebug_getErrorLevelSay(int code,char **say TSRMLS_DC){

	if(code == 1){
		*say = estrdup("FatalErrors");
	}else if(code == 2){
		*say = estrdup("RuntimeWarning");
	}else if(code == 4){
		*say = estrdup("ZendPaseErrors");
	}else if(code == 8){
		*say = estrdup("RuntimeNotices");
	}else if(code == 16){
		*say = estrdup("CoreFatalErrors");
	}else if(code == 32){
		*say = estrdup("CoreWarning");
	}else if(code == 64){
		*say = estrdup("ZendCompileErrors");
	}else if(code == 128){
		*say = estrdup("ZendCompileWarning");
	}else if(code == 256){
		*say = estrdup("UserErrors");
	}else if(code == 512){
		*say = estrdup("UserWarning");
	}else if(code == 1024){
		*say = estrdup("UserNotices");
	}else if(code == 2048){
		*say = estrdup("RuntimeNotices");
	}else if(code == 4096){
		*say = estrdup("CatchableFatalErrors");
	}else{
		*say = estrdup("MyFrameException");
	}
}

PHP_METHOD(CDebug,getErrorsData)
{
	int		errorCode,
			messageLen = 0,
			fileLen = 0,
			lineLen = 0,
			line = 0;

	char	*message,
			*file,
			*messsageString,
			*errorType;

	zval	*errorList,
			*saveData;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"lssl",&errorCode,&message,&messageLen,&file,&fileLen,&line) == FAILURE){
		return;
	}

	errorList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_errorList"),1 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(errorList)){
		zval	*saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CDebugCe,getThis(),ZEND_STRL("_errorList"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		errorList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_errorList"),0 TSRMLS_CC);
	}

	MAKE_STD_ZVAL(saveData);
	array_init(saveData);

	//save message
	str_replace(PHP_EOL,"<br>",message,&messsageString);
	add_assoc_string(saveData,"message",messsageString,1);
	efree(messsageString);

	//save code
	add_assoc_long(saveData,"code",errorCode);

	//save file
	add_assoc_string(saveData,"file",file,1);

	//save line
	add_assoc_long(saveData,"line",line);

	//get errorType
	CDebug_getErrorLevelSay(errorCode,&errorType TSRMLS_CC);
	add_assoc_string(saveData,"type",errorType,1);

	//get debugTrace
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*returnZval,
				*saveTrace;

		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"debug_backtrace",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 0, NULL TSRMLS_CC);	
		returnZval = &returnFunction;
		MAKE_STD_ZVAL(saveTrace);
		ZVAL_ZVAL(saveTrace,returnZval,1,0);
		add_assoc_zval(saveData,"trace",saveTrace);
		zval_dtor(&returnFunction);
	MODULE_END

	//errorList
	add_next_index_zval(errorList,saveData);
}

PHP_METHOD(CDebug,getRequestShutdown)
{
	//php_printf("====================000000000000000========================");
}