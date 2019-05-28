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
#include "php_CWebApp.h"
#include "php_CRoute.h"
#include "php_CGuardController.h"
#include "php_CException.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CDebug_functions[] = {
	PHP_ME(CDebug,debug,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dump,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpInternalClass,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpClass,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpZval,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDebug,dumpTrace,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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
	zend_declare_property_null(CDebugCe, ZEND_STRL("functionTrace"),ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);


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

void CDebug_dumpDBExecuteHtml(zval *classObject TSRMLS_DC){

	char			*header,
					*tempString;
	zval			**thisVal,
					**time,
					**sql,
					**where,
					**isMaster,
					*whereString;
					
	smart_str		html = {0};
	int				i,h;

	if(IS_ARRAY != Z_TYPE_P(classObject)){
		php_printf("No SQL trace to print");
		return;
	}


	smart_str_appends(&html,"</pre><style>#classTable .note{padding:10px ;}#classTable .methodName{width:30%}#classTable{border:solid 1px #ccc; width:100%; border-collapse:collapse; margin-bottom:20px; font-size:12px;}td,th{height:26px;line-height:20px; text-align:left; padding-left:8px;border:solid 1px #ccc;}#classTableHead th{background:#F7F7FB}#classTable .name{background:#F7F7FB; width:30%}</style><meta charset=\"UTF-8\"><table id='classTable'><thead id='classTableHead'><tr><th colspan='5'>CQuickFramework SQL Profiler</th></tr></thead>");
	
	//header
	base64Decode("PHRyPjx0ZD7luo/lj7c8L3RkPjx0ZD5NL1M8L3RkPjx0ZD5TUUw8L3RkPjx0ZD7mnaHku7Y8L3RkPjx0ZD7ogJfml7Y8L3RkPg==",&header);
	smart_str_appends(&html,header);
	efree(header);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(classObject));
	h = zend_hash_num_elements(Z_ARRVAL_P(classObject));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);

		zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&time);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"sql",strlen("sql")+1,(void**)&sql);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"whereValue",strlen("whereValue")+1,(void**)&where);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"master",strlen("master")+1,(void**)&isMaster);

		php_implode(" , ",*where,&whereString);

		spprintf(&tempString,0,"%s%d%s%s%s%s%s%s%s%.8f%s","<tr><td>",i+1,"</td><td>",(Z_LVAL_PP(isMaster) == 1 ? "Master" : "Slaver"),"</td><td>",Z_STRVAL_PP(sql),"</td><td>",Z_STRVAL_P(whereString),"</td><td>",Z_DVAL_PP(time),"</td></tr>");
		smart_str_appends(&html,tempString);
		efree(tempString);
		zval_ptr_dtor(&whereString);

		zend_hash_move_forward(Z_ARRVAL_P(classObject));
	}



	smart_str_appends(&html,"</table>");
	smart_str_0(&html);
	php_printf("%s",html.c);
	smart_str_free(&html);
}

PHP_METHOD(CDebug,dumpDBExecute)
{
	zval	*object,
			*sqlList;

	int		isOurput = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isOurput) == FAILURE){
	}

	object = zend_read_static_property(CDebugCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//read errorList
	sqlList = zend_read_property(CDebugCe,object,ZEND_STRL("_sqlList"),1 TSRMLS_CC);

	if(isOurput == 0){
		CDebug_dumpDBExecuteHtml(sqlList TSRMLS_CC);
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
	zval	*sapiZval;

	//cli not use
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		CHooks_registerHooks("HOOKS_EXECUTE_END","getDatabaseExecuteEnd",getThis(),0 TSRMLS_CC);
		CHooks_registerHooks("HOOKS_ERROR_HAPPEN","getErrorsData",getThis(),0 TSRMLS_CC);
		CHooks_registerHooks("HOOKS_SYSTEM_SHUTDOWN","getRequestShutdown",getThis(),0 TSRMLS_CC);
	}
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

	//more than 1024 will ignore
	if(IS_ARRAY == Z_TYPE_P(sqlList) && zend_hash_num_elements(Z_ARRVAL_P(sqlList)) >= 1024){
		return;
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

	//more than 64 will ignore
	if(IS_ARRAY == Z_TYPE_P(errorList) && zend_hash_num_elements(Z_ARRVAL_P(errorList)) >= 64){
		return;
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

static char *templateString = "PCFET0NUWVBFIGh0bWwgUFVCTElDICItLy9XM0MvL0RURCBYSFRNTCAxLjAgVHJhbnNpdGlvbmFsLy9FTiIgImh0dHA6Ly93d3cudzMub3JnL1RSL3hodG1sMS9EVEQveGh0bWwxLXRyYW5zaXRpb25hbC5kdGQiPjxodG1sIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5L3hodG1sIj48aGVhZD48bWV0YSBodHRwLWVxdWl2PSJDb250ZW50LVR5cGUiIGNvbnRlbnQ9InRleHQvaHRtbDsgY2hhcnNldD11dGYtOCIgLz48dGl0bGU+Q1F1aWNrRnJhbWV3b3Jr6ZSZ6K+v5oql5ZGKPC90aXRsZT48c3R5bGU+Ym9keXttYXJnaW46MDtwYWRkaW5nOjA7Zm9udC1mYW1pbHk6IuW+rui9r+mbhem7kSI7Zm9udC1zaXplOjE0cHg7b3ZlcmZsb3c6aGlkZGVuO2hlaWdodDoxMDAlfS5lcnJvck1haW57d2lkdGg6MTAwJTtvdmVyZmxvdzpoaWRkZW47aGVpZ2h0OjEwMCV9LmNsZWFye2NsZWFyOmJvdGh9LmVycm9yTGVmdHt3aWR0aDoyOCU7b3ZlcmZsb3cteTphdXRvO2JhY2tncm91bmQtY29sb3I6I2Q0ZDRkNDtvdmVyZmxvdy14OmhpZGRlbjtoZWlnaHQ6MTAwJTtwb3NpdGlvbjphYnNvbHV0ZX0uZXJyb3JSaWdodHt3aWR0aDo3MiU7aGVpZ2h0OjEwMCU7bGVmdDoyOCU7cG9zaXRpb246YWJzb2x1dGU7b3ZlcmZsb3cteTphdXRvO292ZXJmbG93LXg6aGlkZGVufS5lcnJvclR5cGUgc3Bhbntmb250LXNpemU6MTRweDtmb250LXdlaWdodDpub3JtYWw7Y29sb3I6I2ZmZn0uZXJyb3JUeXBlIGF7Y29sb3I6I2ZmZjt0ZXh0LWRlY29yYXRpb246bm9uZTtjdXJzb3I6dGV4dH0uZXJyTWVzc2FnZXtwYWRkaW5nOjEwcHg7YmFja2dyb3VuZC1jb2xvcjojMDAwO2JvcmRlci1sZWZ0OnNvbGlkIDZweCAjY2QzZjNmO2JvcmRlci1ib3R0b206c29saWQgMXB4ICM2NjZ9LmVycm9yVHlwZXtjb2xvcjojY2QzZjNmO2ZvbnQtd2VpZ2h0OmJvbGQ7Zm9udC1zaXplOjE2cHg7Zm9udC1mYW1pbHk6VmVyZGFuYSxBcmlhbCxIZWx2ZXRpY2Esc2Fucy1zZXJpZjtwYWRkaW5nLWxlZnQ6MjBweDtwYWRkaW5nLXRvcDoxMHB4O21hcmdpbi1ib3R0b206MTBweDtsaW5lLWhlaWdodDoyNnB4fS5lcnJvckNvbnRlbnR7Zm9udC1zaXplOjE0cHg7Y29sb3I6I2ZmZjtwYWRkaW5nLWxlZnQ6MzBweDttYXJnaW4tdG9wOjEzcHh9LnNo"
"b3dGaWxle3BhZGRpbmc6N3B4O2JvcmRlci1sZWZ0OnNvbGlkIDZweCAjNDI4OGNlO2JhY2tncm91bmQtY29sb3I6I2JkYmRiZH0uc2hvd0ZpbGVUaXRsZXtiYWNrZ3JvdW5kOiNjNmM2YzY7Y29sb3I6IzUyNTI1Mjt0ZXh0LXNoYWRvdzowIDFweCAwICNlN2U3ZTc7cGFkZGluZzoxMHB4IDEwcHggNXB4IDEwcHg7Ym9yZGVyLXRvcC1yaWdodC1yYWRpdXM6NnB4O2JvcmRlci10b3AtbGVmdC1yYWRpdXM6NnB4O2JvcmRlcjoxcHggc29saWQgcmdiYSgwLDAsMCwuMSk7Ym9yZGVyLWJvdHRvbTowO2JveC1zaGFkb3c6aW5zZXQgMCAxcHggMCAjZGFkYWRhO2ZvbnQtZmFtaWx5OlZlcmRhbmEsQXJpYWwsSGVsdmV0aWNhLHNhbnMtc2VyaWZ9LnNob3dGaWxlRm9vdHtib3gtc2hhZG93Omluc2V0IDAgMCA2cHggcmdiYSgwLDAsMCwuMyk7Ym9yZGVyOjFweCBzb2xpZCByZ2JhKDAsMCwwLC4yKTtib3JkZXItdG9wOjA7Ym9yZGVyLWJvdHRvbS1yaWdodC1yYWRpdXM6NnB4O2JvcmRlci1ib3R0b20tbGVmdC1yYWRpdXM6NnB4O2hlaWdodDoyNXB4O2JhY2tncm91bmQtY29sb3I6IzQwNDA0MH0uc2hvd0ZpbGVDb250ZW50e2JhY2tncm91bmQtY29sb3I6IzI3MjcyNztwYWRkaW5nOjI1cHggMCA1cHggMH0ubGluZU51bXtjb2xvcjojOTk5O3dpZHRoOjQwcHg7ZGlzcGxheTpibG9jazt0ZXh0LWFsaWduOmNlbnRlcjtmbG9hdDpsZWZ0O21hcmdpbi1yaWdodDoxMHB4fS5zaG93T3RoZXJEYXRhe2JvcmRlci1sZWZ0OnNvbGlkIDZweCAjYmRiZGJkO3BhZGRpbmc6NXB4fS5vdGhlclRpdGxle21hcmdpbi10b3A6OHB4O2JvcmRlci1ib3R0b206ZG90dGVkIDFweCAjOTk5O3BhZGRpbmctYm90dG9tOjRweDtwYWRkaW5nLWxlZnQ6NHB4O2ZvbnQtZmFtaWx5OlZlcmRhbmEsQXJpYWwsSGVsdmV0aWNhLHNhbnMtc2VyaWY7Y29sb3I6IzQyODhjZTtmb250LXdlaWdodDpib2xkO21hcmdpbi1ib3R0b206MTBweH0ub3RoZXJDb250ZW50IHRhYmxle2JvcmRlcjpzb2xpZCAxcHggIzA2Zjt3aWR0aDoxMDAlO21hcmdpbi10b3A6MnB4O2JvcmRlcjpzb2xpZCAxcHggI2NjYztib3"
"JkZXItY29sbGFwc2U6Y29sbGFwc2U7Ym9yZGVyLXNwYWNpbmc6MDt0ZXh0LWFsaWduOmxlZnQ7Y29sb3I6IzJiMmIyYn0ub3RoZXJDb250ZW50IHRhYmxlIHRoe2ZvbnQtd2VpZ2h0Om5vcm1hbDtmb250LWZhbWlseTpoZWx2ZXRpY2EsYXJpYWwsc2Fucy1zZXJpZjtiYWNrZ3JvdW5kLWNvbG9yOiNlNGUzZTM7aGVpZ2h0OjI0cHg7Ym9yZGVyOjFweCBzb2xpZCAjQ0NDO3BhZGRpbmctbGVmdDoxMHB4fS5vdGhlckNvbnRlbnQgdGFibGUgdHJ7ZGlzcGxheTp0YWJsZS1yb3c7dmVydGljYWwtYWxpZ246aW5oZXJpdDtib3JkZXItY29sb3I6aW5oZXJpdH0ub3RoZXJDb250ZW50IHRhYmxlIHRke2JvcmRlcjoxcHggc29saWQgI0NDQztjb2xvcjojNDYzYzU0O2ZvbnQtc2l6ZToxMnB4O2hlaWdodDoyNHB4O3BhZGRpbmctbGVmdDoxMHB4O2xpbmUtaGVpZ2h0OjIzcHg7YmFja2dyb3VuZDojZmZmfS5lcnJvckxlZnQgbGl7Ym9yZGVyLWJvdHRvbTpzb2xpZCAxcHggI2Q0ZDRkNDtsaXN0LXN0eWxlOm5vbmU7cGFkZGluZzoxMHB4O2xpbmUtaGVpZ2h0OjIycHh9LmVycm9yTGVmdCBsaTpob3ZlcntiYWNrZ3JvdW5kLWNvbG9yOiMwMDA7ZmlsdGVyOmFscGhhKG9wYWNpdHk9ODApO29wYWNpdHk6Ljg7Y3Vyc29yOnBvaW50ZXI7KmN1cnNvcjpoYW5kfS5lcnJvckxlZnQgbGk6aG92ZXIgcHtjb2xvcjojZmZmfS5lcnJvckxlZnRfbGl7YmFja2dyb3VuZDojZmZmfS5lcnJvckxlZnRfbGlfaG92ZXJ7YmFja2dyb3VuZDojMDAwO2ZpbHRlcjphbHBoYShvcGFjaXR5PTgwKTtvcGFjaXR5Oi44O2N1cnNvcjpwb2ludGVyOypjdXJzb3I6aGFuZH0uZXJyb3JMZWZ0X2xpX2hvdmVyIHB7Y29sb3I6I2ZmZn0uZXJyb3JMZWZ0TGlTcGFue2ZvbnQtc2l6ZToxMnB4O2NvbG9yOiM5OTk7aGVpZ2h0OjIwcHg7b3ZlcmZsb3c6aGlkZGVufS5lcnJvckxlZnRUeXBle2ZvbnQtd2VpZ2h0OmJvbGQ7Y29sb3I6IzkzMH0uZXJyb3JMZWZ0IGxpIHB7bWFyZ2luOjA7cGFkZGluZzowO2hlaWdodDoyMnB4O292ZXJmbG93OmhpZGRlbn06Oi13ZWJraXQtc2Nyb2xsYmFye3BhZGRpbmctbGVmdDoxcHg7Ym9yZGVyLWxlZnQ6MXB4IHNvbGlkICNkNWQ1ZDU7YmFja2dyb3VuZC1jb2xvcjojZmFmYWZhO292ZXJmbG93OnZpc2libGU7d2lkdGg6MTNweH06Oi13ZWJraXQtc2Nyb2xsYmFyLXRodW1ie2JhY2tncm91bmQtY29sb3I6cmdiYSgwLDAsMCwuMik7YmF"
"ja2dyb3VuZC1jbGlwOnBhZGRpbmctYm94O2JvcmRlcjoxcHggc29saWQgdHJhbnNwYXJlbnQ7Ym9yZGVyLWxlZnQtd2lkdGg6MnB4O21pbi1oZWlnaHQ6MTVweDtib3gtc2hhZG93Omluc2V0IDFweCAxcHggMCByZ2JhKDAsMCwwLC4xKSxpbnNldCAwIC0xcHggMCByZ2JhKDAsMCwwLC4wNyl9Ojotd2Via2l0LXNjcm9sbGJhci10aHVtYjp2ZXJ0aWNhbDpob3ZlcntiYWNrZ3JvdW5kLWNvbG9yOnJnYmEoMCwwLDAsLjMpfTo6LXdlYmtpdC1zY3JvbGxiYXItdGh1bWI6dmVydGljYWw6YWN0aXZle2JhY2tncm91bmQtY29sb3I6cmdiYSgwLDAsMCwuNSl9Ojotd2Via2l0LXNjcm9sbGJhci1idXR0b257aGVpZ2h0OjA7d2lkdGg6MH06Oi13ZWJraXQtc2Nyb2xsYmFyLXRyYWNre2JhY2tncm91bmQtY2xpcDpwYWRkaW5nLWJveDtib3JkZXI6c29saWQgdHJhbnNwYXJlbnQ7Ym9yZGVyLXdpZHRoOjAgMCAwIDRweH06Oi13ZWJraXQtc2Nyb2xsYmFyLWNvcm5lcntiYWNrZ3JvdW5kOnRyYW5zcGFyZW50fTwvc3R5bGU+PC9oZWFkPjxib2R5PjxkaXYgaWQ9ImVycm9yTWFpbiIgY2xhc3M9ImVycm9yTWFpbiI+PGRpdiBpZD0iZXJyb3JMZWZ0IiBjbGFzcz0iZXJyb3JMZWZ0Ij48e2ZvcmVhY2ggZnJvbT0kYWxsRXJyb3IgaXRlbT1lcnIga2V5PWVycktleX0+PGxpIGlkPSJsaTx7JGVycktleX0+IiBubz0iPHskZXJyS2V5fT4iIG9uY2xpY2s9ImNoYW5nZVNob3coPHskZXJyS2V5fT4pIiBjbGFzcz0iZXJyb3JMZWZ0X2xpIDx7aWYgJGVycktleSA9PSAwfT5lcnJvckxlZnRfbGlfaG92ZXI8ey9pZn0+Ij48c3BhbiBjbGFzcz0iZXJyb3JMZWZ0VHlwZSI+Wzx7JGVyci50eXBlfT5dPC9zcGFuPiA8cD48eyRlcnIubWVzc2FnZX0+PC9wPjxzcGFuIGNsYXNzPSJlcnJvckxlZnRMaVNwYW4iPjx7JGVyci5maWxlfT4gLSBMaW5lOjx7JGVyci5saW5lfT48L3NwYW4+PC9saT48ey9mb3JlYWNofT48L2Rpdj48ZGl2IGlkPSJlcnJvclJpZ2h0IiBjbGFzcz0iZXJyb3JSaWdodCI+PGRpdiBzdHlsZT0iaGVpZ2h0OjU5M3B4OyBvdmVyZmxvdzpoaWRkZW4iPjxkaXYgaWQ9InJvbGxBcmVhIiBzdHlsZT0ibWFyZ2luLXRvcDowIj48e2ZvcmVhY2ggZnJvbT0kYWxsRXJyb3IgaXRlbT1lcnIga2V5PWVycktleX0+PGRpdiBjbGFzcz0iZGV0YWlsIiBubz0iPHskZXJyS2V5fT4iPHtpZiAkZXJyS2V5ID09IDB9Pjx7ZWxzZX0+c3R5bGU9ImRpc3"
"BsYXk6Ijx7L2lmfT4gaWQ9ImNvbnRlbnRsaTx7JGVycktleX0+Ij48ZGl2IGNsYXNzPSJlcnJNZXNzYWdlIiBpZD0idGl0bGVsaTx7JGVycktleX0+Ij48ZGl2IGNsYXNzPSJlcnJvclR5cGUiPltNeUZyYW1lRXhjZXB0aW9uXSAtPHtzaG93SFRNTCBjPSRlcnIubWVzc2FnZX0+PC9zcGFuPjwvZGl2PiA8L2Rpdj48ZGl2IGNsYXNzPSJzaG93RmlsZSI+PGRpdiBjbGFzcz0ic2hvd0ZpbGVUaXRsZSI+PHskZXJyLmZpbGV9PjwvZGl2PjxkaXYgY2xhc3M9InNob3dGaWxlQ29udGVudCIgc3R5bGU9IiBoZWlnaHQ6NDU1cHgiPjx7Zm9yZWFjaCBmcm9tPSRlcnIuZGV0YWlsIGtleT1saW5lTnVtIGl0ZW09Y29kZX0+PHNwYW4gY2xhc3M9ImxpbmVOdW0iPjx7JGxpbmVOdW19Pjwvc3Bhbj48e3Nob3dIVE1MIGM9JGNvZGV9Pjx7L2ZvcmVhY2h9PjwvZGl2PjxkaXYgY2xhc3M9InNob3dGaWxlRm9vdCI+PC9kaXY+PC9kaXY+PC9kaXY+PHsvZm9yZWFjaH0+PC9kaXY+PC9kaXY+PGRpdiBjbGFzcz0ic2hvd090aGVyRGF0YSI+PGRpdiBjbGFzcz0ib3RoZXJUaXRsZSI+UmVxdWVzdDwvZGl2PjxkaXYgY2xhc3M9Im90aGVyQ29udGVudCI+PHRhYmxlPjx0aGVhZD48dHI+PHRoIHdpZHRoPSIzMyUiPktleTwvdGg+PHRoIHdpZHRoPSI2NyUiPlZhbHVlPC90aD4gIDwvdHI+PC90aGVhZD48e2ZvcmVhY2ggZnJvbT0kcmVxdWVzdCBpdGVtPXJlcSBrZXk9cmVxVHlwZX0+ICA8dHI+PHRkPjx7JHJlcVR5cGV9PjwvdGQ+PHRkPjx7JHJlcX0+PC90ZD4gIDwvdHI+PHsvZm9yZWFjaH0+PC90YWJsZT48L2Rpdj48L2Rpdj48ZGl2IGNsYXNzPSJzaG93T3RoZXJEYXRhIj48ZGl2IGNsYXNzPSJvdGhlclRpdGxlIj5QZXJmb3JtYW5jZU1vbml0b3I8L2Rpdj48ZGl2IGNsYXNzPSJvdGhlckNvbnRlbnQiPjx0YW"
"JsZT48dGhlYWQ+PHRyPjx0aCB3aWR0aD0iMzMlIj5DYXN0PC90aD48dGggd2lkdGg9IjY3JSI+U2NyaXB0PC90aD4gIDwvdHI+PC90aGVhZD48e2ZvcmVhY2ggZnJvbT0kc2VydmVyIGl0ZW09cmVxIGtleT1yZXFUeXBlfT4gIDx0cj48dGQ+PHskcmVxLmNhc3R9PjwvdGQ+PHRkPjx7JHJlcS5zY3JpcHR9PjwvdGQ+ICA8L3RyPjx7L2ZvcmVhY2h9PjwvdGFibGU+PC9kaXY+PC9kaXY+PGRpdiBjbGFzcz0ic2hvd090aGVyRGF0YSI+PGRpdiBjbGFzcz0ib3RoZXJUaXRsZSI+SW5jbHVkZUZpbGVzPC9kaXY+PGRpdiBjbGFzcz0ib3RoZXJDb250ZW50IiBzdHlsZT0ibWFyZ2luLWJvdHRvbToyMHB4Ij48dGFibGU+PHRoZWFkPjx0cj48dGggd2lkdGg9IjUlIj5OdW08L3RoPjx0aCB3aWR0aD0iOTUlIj5GaWxlTmFtZTwvdGg+ICA8L3RyPjwvdGhlYWQ+PHtmb3JlYWNoIGZyb209JGluY2x1ZGVGaWxlIGl0ZW09aWMga2V5PWlub30+ICA8dHI+PHRkIHN0eWxlPSJwYWRkaW5nOjA7IHRleHQtYWxpZ246Y2VudGVyIj48eyRpbm8rMX0+PC90ZD48dGQ+PHskaWN9PjwvdGQ+ICA8L3RyPjx7L2ZvcmVhY2h9PjwvdGFibGU+PC9kaXY+PC9kaXY+PC9kaXY+PC9kaXY+PC9ib2R5PjwvaHRtbD48c2NyaXB0PmZ1bmN0aW9uIGNoYW5nZVNob3coa2V5KXt2YXIgYWxsVGl0bGUgPSBkb2N1bWVudC5nZXRFbGVtZW50c0J5Q2xhc3NOYW1lKCdlcnJvckxlZnRfbGknKTtmb3IodmFyIGkgPSAwIDsgaTwgYWxsVGl0bGUubGVuZ3RoIDsgaSsrKXt2"
"YXIgdGhpc0VsZW1lbnQgPSBhbGxUaXRsZVtpXTt0aGlzRWxlbWVudC5jbGFzc0xpc3QucmVtb3ZlKCJlcnJvckxlZnRfbGlfaG92ZXIiKTt9ZG9jdW1lbnQuZ2V0RWxlbWVudEJ5SWQoImxpIiArIGtleSkuY2xhc3NMaXN0LmFkZCgiZXJyb3JMZWZ0X2xpX2hvdmVyIik7dmFyIHRvVG9wID0gKDAtcGFyc2VJbnQoZG9jdW1lbnQuZ2V0RWxlbWVudEJ5SWQoImNvbnRlbnRsaSIgKyBrZXkpLm9mZnNldFRvcCkpICsgcGFyc2VJbnQoZG9jdW1lbnQuZ2V0RWxlbWVudEJ5SWQoInJvbGxBcmVhIikuc3R5bGUubWFyZ2luVG9wKTtkb2N1bWVudC5nZXRFbGVtZW50QnlJZCgicm9sbEFyZWEiKS5zdHlsZS5jc3NUZXh0PSAibWFyZ2luLXRvcDoiICsgdG9Ub3AgKyAicHgiO308L3NjcmlwdD4=";

static char *debugString = "PC9wcmU+PG1ldGEgY2hhcnNldD0idXRmLTgiLz48c3R5bGU+LmRlYnVnVGFne3Bvc2l0aW9uOmZpeGVkO19wb3NpdGlvbjphYnNvbHV0ZTtib3R0b206MHB4OyByaWdodDo1cHg7IHdpZHRoOjcwcHg7IGhlaWdodDoyN3B4OyBiYWNrZ3JvdW5kLWNvbG9yOiMwMDAwMDA7IGNvbG9yOiNGRkZGRkY7IGJvcmRlci10b3AtbGVmdC1yYWRpdXM6N3B4O2JvcmRlci10b3AtcmlnaHQtcmFkaXVzOjdweDtsaW5lLWhlaWdodDoyN3B4OyB0ZXh0LWFsaWduOmNlbnRlcjsgZm9udC1mYW1pbHk6IuW+rui9r+mbhem7kSI7IGZvbnQtc2l6ZToxMnB4OyBjdXJzb3I6cG9pbnRlcjsgKmN1cnNvcjpoYW5kOyB6LWluZGV4OjEwMDAwMTt9LmRlYnVnRGF0YXtwb3NpdGlvbjpmaXhlZDtfcG9zaXRpb246YWJzb2x1dGU7aGVpZ2h0OjEwMCU7IGxlZnQ6MDt0cmFuc2Zvcm06dHJhbnNsYXRlM2QoMCwxMDAlLDApOyBib3R0b206MDtyaWdodDowOyBiYWNrZ3JvdW5kOiNENEQ0RDQ7IGZvbnQtZmFtaWx5OiLlvq7ova/pm4Xpu5EiOyBmb250LXNpemU6MTRweDsgcGFkZGluZzowOyBtYXJnaW46MDsgZGlzcGxheTo7IHotaW5kZXg6MTAwMDAwOyBib3JkZXItdG9wOnNvbGlkIDFweCAjQkRCREJEfS5zaG93RGVidWd7dHJhbnNpdGlvbi1kdXJhdGlvbjo0MDBtczt0cmFuc2Zvcm06IHRyYW5zbGF0ZTNkKDAsMCwwKTt9LmhpZGVEZWJ1Z3t0cmFuc2l0aW9uLWR1cmF0aW9uOjQwMG1zO3RyYW5zZm9ybTogdHJhbnNsYXRlM2QoMCwxMDAlLDApO30uZGVidWdMZWZ0eyB3aWR0aDoxMCU7IHBvc2l0aW9uOmFic29sdXRlO292ZXJmbG93LXg6aGlkZGVuOyBvdmVyZmxvdy15OmF1dG87IGhlaWdodDoxMDAlfS5sZWZ0VGFnc3toZWlnaHQ6NDVweDsgY29sb3I6IzMzMzsgbGluZS1oZWlnaHQ6NDVweDsgcGFkZGluZy1sZWZ0OjE1cHg7IGZvbnQtZmFtaWx5OlZlcmRhbmEsIEFyaWFsLCBIZWx2ZXRpY2EsIHNhbnMtc2VyaWY7IGZvbnQtc2l6ZToxNHB4OyBiYWNrZ3JvdW5kLWNvbG9yOiNGRkZGRkY7IGN1cnNvcjpwb2ludGVyOyAqY3Vyc29yOmhhbmR9LmxlZnRUYWdzOmhvdmVye2JhY2tncm91bmQtY29sb3I6IzAwMDtjb2xvcjp3aGl0ZX0ubGVmdFRhZ3NPbntoZWlnaHQ6NDVweDtsaW5lLWhlaWdodDo0NXB4OyBmb250LXd"
"laWdodDpib2xkOyBwYWRkaW5nLWxlZnQ6MTVweDsgZm9udC1mYW1pbHk6VmVyZGFuYSwgQXJpYWwsIEhlbHZldGljYSwgc2Fucy1zZXJpZjsgZm9udC1zaXplOjE0cHg7IGJhY2tncm91bmQtY29sb3I6I0ZGRkZGRjsgY3Vyc29yOnBvaW50ZXI7ICpjdXJzb3I6aGFuZDtib3JkZXItcmlnaHQ6IDFweCBzb2xpZCByZ2JhKDAsIDAsIDAsIC4yKTsgYmFja2dyb3VuZC1jb2xvcjojMDAwOyBjb2xvcjp3aGl0ZTtib3gtc2hhZG93OiBpbnNldCAtMnB4IDAgMCByZ2JhKDI1NSwgMjU1LCAyNTUsIC4xKTt9LmRlYnVnUmlnaHR7IHBvc2l0aW9uOmFic29sdXRlOyB3aWR0aDo5MCU7IGxlZnQ6MTAlOyBoZWlnaHQ6MTAwJTtvdmVyZmxvdy14OmF1dG87IG92ZXJmbG93LXk6YXV0bzsgYm9yZGVyLWxlZnQ6c29saWQgNnB4ICNCREJEQkQ7fS5kZWJ1Z1Nob3dPdGhlckRhdGF7cGFkZGluZzo1cHg7IHBhZGRpbmctbGVmdDo4cHh9LmRlYnVnT3RoZXJUaXRsZXttYXJnaW4tdG9wOjhweDsgYm9yZGVyLWJvdHRvbTpkb3R0ZWQgMXB4ICM5OTk5OTk7IHBhZGRpbmctYm90dG9tOjRweDsgcGFkZGluZy1sZWZ0OjRweDsgZm9udC1mYW1pbHk6VmVyZGFuYSwgQXJpYWwsIEhlbHZldGljYSwgc2Fucy1zZXJpZjsgY29sb3I6IzAwMDsgZm9udC13ZWlnaHQ6Ym9sZDsgbWFyZ2luLWJvdHRvbToxMHB4fS5kZWJ1Z090aGVyQ29udGVudCB0YWJsZXsgYm9yZGVyOnNvbGlkIDFweCAjMDA2NkZGOyB3aWR0aDoxMDAlOyBtYXJnaW4tdG9wOjJweDsgYm9yZGVyOnNvbGlkIDFweCAjY2NjO2JvcmRlci1jb2xsYXBzZTogY29sbGFwc2U7Ym9yZGVyLXNwYWNpbmc6IDA7IHRleHQtYWxpZ246bGVmdDsgY29sb3I6IzJCMkIyQn0uZGVidWdPdGhlckNvbnRlbnQgdGFibGUgdGh7Zm9udC13ZWlnaHQ6bm9ybWFsOyBmb250LWZhbWlseTpoZWx2ZXRpY2EsIGFyaWFsLCBzYW5zLXNlcmlmOyBiYWNrZ3JvdW5kLWNvbG9yOiNFNEUzRTM7IGhlaWdodDoyNHB4O2JvcmRlcjogMXB4IHNvbGlkICNDQ0M7IHBhZGRpbmctbGVmdDoxMHB4OyBmb250LXNpemU6MTRweDsgbGluZS1oZWlnaHQ6MjRweH0uZGVidWdPdGhlckNvbnRlbnR7YmFja2dyb3VuZC1jb2xvcjojRkZGRkZGfS5kZWJ1Z090aGVyQ29udGVudCB0YWJsZSB0cntkaXNwbGF5OiB0YWJsZS1yb3c7dmVydGljYWwtYWxpZ246IGluaGVyaXQ7Ym9yZGVyLWNvbG9yOiBpbmhlcml0O30uZGVidWdPdGhlckNvbnRlbnQgdGFibGUgdGR7Ym9yZGVyOiAxcHggc29saWQgI0NDQztjb2xvcjogIzQ2M0M1NDsgZm9udC1zaXplOjEycHg7IGhlaWdodDoyNHB4OyBwYWRkaW5nLWxlZnQ6MTBweDsgbGluZS1oZWlnaHQ6MjNweDsgYmFja2dyb3VuZDojRkZGRkZGO3dvcmQtYnJlYWs6YnJ"
"lYWstYWxsO30uZGVidWdQbHVnaW5Nb3JlRGF0YXtjb2xvcjojOTk5OTk5fS5kZWJ1Z1BsdWdpbkxvYWR7Y29sb3I6IzAwOTkwMH0uZGVidWdQbHVnaW5VbkxvYWR7Y29sb3I6I0NDMDAwMH0uZGVidWdQbHVnaW5EZXNje2NvbG9yOiM0NjNDNTQ7fS5kZWJ1Z1B1YnV7IGJvcmRlcjpzb2xpZCAxcHggI2NjYzsgaGVpZ2h0OjQwMHB4fS5kZWJ1Z0JpbmdMZWZ0e2Zsb2F0OmxlZnQ7IHdpZHRoOjQ0JTsgaGVpZ2h0OjQwMHB4fS5kZWJ1Z0JpbmdSaWdodHtmbG9hdDpyaWdodDsgd2lkdGg6NDQlOyBoZWlnaHQ6NDAwcHh9LmRlYnVnQm90dG9teyBtYXJnaW46MDsgcGFkZGluZzowfTo6LXdlYmtpdC1zY3JvbGxiYXJ7cGFkZGluZy1sZWZ0OjFweDtib3JkZXItbGVmdDoxcHggc29saWQgI2Q1ZDVkNTtiYWNrZ3JvdW5kLWNvbG9yOiNmYWZhZmE7b3ZlcmZsb3c6dmlzaWJsZTt3aWR0aDoxM3B4O306Oi13ZWJraXQtc2Nyb2xsYmFyLXRodW1ie2JhY2tncm91bmQtY29sb3I6cmdiYSgwLCAwLCAwLCAuMik7YmFja2dyb3VuZC1jbGlwOnBhZGRpbmctYm94O2JvcmRlcjoxcHggc29saWQgdHJhbnNwYXJlbnQ7Ym9yZGVyLWxlZnQtd2lkdGg6MnB4O21pbi1oZWlnaHQ6MTVweDtib3gtc2hhZG93Omluc2V0IDFweCAxcHggMCByZ2JhKDAsIDAsIDAsIC4xKSxpbnNldCAwIC0xcHggMCByZ2JhKDAsIDAsIDAsIC4wNyk7fTo6LXdlYmtpdC1zY3JvbGxiYXItdGh1bWI6dmVydGljYWw6aG92ZXJ7YmFja2dyb3VuZC1jb2xvcjpyZ2JhKDAsIDAsIDAsIC4zKTt9Ojotd2Via2l0LXNjcm9sbGJhci10aHVtYjp2ZXJ0aWNhbDphY3RpdmV7YmFja2dyb3VuZC1jb2xvcjpyZ2JhKDAsIDAsIDAsIC41KTt9Ojotd2Via2l0LXNjcm9sbGJhci1idXR0b257aGVpZ2h0OjA7d2lkdGg6MDt9Ojotd2Via2l0LXNjcm9sbGJhci10cmFja3tiYWNrZ3JvdW5kLWNsaXA6cGFkZGluZy1ib3g7Ym9yZGVyOnNvbGlkIHRyYW5zcGFyZW50O2JvcmRlci13aWR0aDowIDAgMCA0cHg7fTo6LXdlYmtpdC1zY3JvbGxiYXItY29ybmVye2JhY2tncm91bmQ6dHJhbnNwYXJlbnQ7fTwvc3R5bGU+PGRpdiBjbG"
"Fzcz0iZGVidWdUYWciIGlkPSJkZWJ1Z0J1dHRvbiIgb25DbGljaz0ib3BlbkRlYnVnKCkiPuWxleW8gOiwg+ivlTwvZGl2PjxkaXYgY2xhc3M9ImRlYnVnRGF0YSIgaWQ9ImRlYnVnTWFpbiI+CTxkaXYgY2xhc3M9ImRlYnVnTGVmdCIgaWQ9ImRlYnVnTGVmdCI+CQk8ZGl2IGlkPSJyZXF1ZXN0RGF0YUtleSIgcmlkPSJyZXF1ZXN0RGF0YSIgb25jbGljaz0iZGVidWdQbHVnaW5DaGFuZ2VUYWdzKCdyZXF1ZXN0RGF0YScpIiBjbGFzcz0iRGVidWdNZW51IGxlZnRDbGljayBsZWZ0VGFnc09uIj5BcHBsaWNhdGlvbjwvZGl2PgkJPGRpdiBpZD0ic2VydmVyc0RhdGFLZXkiIHJpZD0ic2VydmVyc0RhdGEiIG9uY2xpY2s9ImRlYnVnUGx1Z2luQ2hhbmdlVGFncygnc2VydmVyc0RhdGEnKSIgY2xhc3M9IkRlYnVnTWVudSBsZWZ0Q2xpY2sgbGVmdFRhZ3MiPlNlcnZlcnM8L2Rpdj4JCTxkaXYgaWQ9InNxbERhdGFLZXkiIHJpZD0ic3FsRGF0YSIgb25jbGljaz0iZGVidWdQbHVnaW5DaGFuZ2VUYWdzKCdzcWxEYXRhJykiIGNsYXNzPSJEZWJ1Z01lbnUgbGVmdENsaWNrIGxlZnRUYWdzIj5TUUw8L2Rpdj4JCTxkaXYgaWQ9Indhcm5EYXRhS2V5IiByaWQ9InNxbERhdGEiIG9uY2xpY2s9ImRlYnVnUGx1Z2luQ2hhbmdlVGFncygnd2FybkRhdGEnKSIgY2xhc3M9IkRlYnVnTWVudSBsZWZ0Q2xpY2sgbGVmdFRhZ3MiPldhcm48L2Rpdj4JCTxkaXYgaWQ9InRyYWNlRGF0YUtleSIgcmlkPSJ0cmFjZURhdGEiIG9uY2xpY2s9ImRlYnVnUGx1Z2luQ2hhbmdlVGFncygndHJhY2VEYXRhJykiIGNsYXNzPSJEZWJ1Z01lbnUgbGVmdENsaWNrIGxlZnRUYWdzIj5UcmFjZTwvZGl2Pgk8L2Rpdj4JPGRpdiBjbGFzcz0iZGVidWdSaWdodCIgaWQ9ImRlYnVnUmlnaHQiPgkJCQkJPGRpdiBpZD0icmVxdWVzdERhdGEiIGNsYXNzPSJkZWJ1Z1JpZ2h0Q29udGVudCI+CQkJCQkJPGRpdiBjbGFzcz0iZGVidWdTaG93T3RoZXJEYXRhIj4JCQkJPGRpdiBjbGFzcz0iZGVidWdPdGhlclRpdGxlIj5SZXF1ZXN0ICg8eyRyZXF1ZXN0fEBjb3VudH0+KTwvZGl2PgkJCQk8ZGl2IGNsYXNzPSJ"
"kZWJ1Z090aGVyQ29udGVudCI+CQkJCQk8dGFibGU+CQkJCQkJPHRoZWFkPgkJCQkJCTx0cj4JCQkJCQkJPHRoIHdpZHRoPSIyMyUiPktleTwvdGg+CQkJCQkJCTx0aCB3aWR0aD0iNzclIj5WYWx1ZTwvdGg+CQkJCQkJPC90cj4JCQkJCQk8L3RoZWFkPgkJCQkJCTx7Zm9yZWFjaCBmcm9tPSRyZXF1ZXN0IGl0ZW09cmVxIGtleT1yZXFUeXBlfT4JCQkJCQk8dHI+CQkJCQkJCTx0ZD48eyRyZXFUeXBlfT48L3RkPgkJCQkJCQk8dGQ+PHskcmVxfT48L3RkPgkJCQkJCTwvdHI+CQkJCQkJPHsvZm9yZWFjaH0+CQkJCQkJCQkJCQkJCQkJCQkJPHtpZiAhZW1wdHkoJGNvb2tpZSl9PgkJCQkJCTx0cj48dGQgY29sc3Bhbj0iMiI+PC90ZD48L3RyPgkJCQkJCTx7Zm9yZWFjaCBmcm9tPSRjb29raWUgaXRlbT1jb29raWUga2V5PWNvb2tpZUtleX0+CQkJCQkJPHRyPgkJCQkJCQk8dGQ+Q29va2llIC08eyRjb29raWVLZXl9PjwvdGQ+CQkJCQkJCTx0ZD48eyRjb29raWV9PjwvdGQ+CQkJCQkJPC90cj4JCQkJCQk8ey9mb3JlYWNofT4JCQkJCQk8ey9pZn0+CQkJCQkJCQkJCQk8L3RhYmxlPgkJCQk8L2Rpdj4JCQk8L2Rpdj4JCQkJCQkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z1Nob3dPdGhlckRhdGEiPgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyVGl0bGUiPlBsdWdpbiAoPHskcGx1Z2luRGF0YXxAY291bnR9Pik8L2Rpdj4JCQkJPGRpdiBjbGFzcz0iZGVidWdPdGhlckNvbnRlbnQiPgkJCQkJPHRhYmxlPgkJCQkJCTx0aGVhZD4JCQkJCQk8dHI+CQkJCQkJCTx0aCB3aWR0aD0iMjMlIj5OYW1lPC90aD4JCQkJCQkJPHRoIHdpZHRoPSI3NyUiPlN0YXR1czwvdGg+CQkJCQkJPC90cj4JCQkJCQk8L3RoZWFkPgkJCQkJCTx7Zm9yZWFjaCBmcm9tPSRwbHVnaW5EYXRhIGl0ZW09cmVxIGtleT1yZXFUeXBlfT4JCQkJCQk8dHI+CQkJCQkJCTx0ZD48eyRyZXFUeXBlfT48L3RkPgkJCQkJCQk8dGQ+PHskcmVxfT48L3RkPgkJCQkJCTwvdHI+CQkJCQkJPHsvZm9yZWFjaH0+CQkJCQk8L3RhYmxlPgkJCQk8L2Rpdj4JCQk8L2Rpdj4JCQkJCQkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z1Nob3dPdGhlckRhdGEiPgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyVGl0bGUiPkluY2x1ZGVGaWxlcyAoPHskaW5jbHVkZUZpbGV8QGNvdW50fT4pPC9kaXY+CQkJCTxkaXYgY2xhc3M9ImRlYnVnT3RoZXJDb250ZW50Ij4JCQkJCTx0YWJsZT4JCQkJCQk8dGhlYWQ+CQkJCQkJCTx0cj4JCQkJCQkJCTx0aCB3aWR0aD0iNSUiPk51bTwvdGg+CQkJCQkJCQk8dGggd2lkdGg9Ijk1JSI+RmlsZU5hbWU8L3RoPgkJCQkJCQk8L3RyPgkJCQkJCTwvdGhlYWQ+CQkJCQkJPHtmb3JlYWNoIGZyb209JGluY2x1ZGVGaWxlIGl0ZW09aWMga2V5PWlub30+CQkJCQkJPHRyPgkJCQkJCQk8dGQgc3R5bGU9InBhZGRpbmc6MDsgdGV4dC1hbGlnbjpjZW50ZXIiPjx7JGlubysxfT48L3RkPgkJCQkJCQk8dGQ+PHskaWN9PjwvdGQ+CQkJCQkJPC90cj4JCQkJCQk8ey9mb3JlYWNofT4JCQkJCTwvdGFibGU+CQkJCTwvZ"
"Gl2PgkJCTwvZGl2PgkJPC9kaXY+CQkJCQkJPGRpdiBpZD0ic2VydmVyc0RhdGEiIGNsYXNzPSJkZWJ1Z1JpZ2h0Q29udGVudCIgc3R5bGU9ImRpc3BsYXk6bm9uZSI+CQkJCQkJPGRpdiBjbGFzcz0iZGVidWdTaG93T3RoZXJEYXRhIj4JCQkJPGRpdiBjbGFzcz0iZGVidWdPdGhlclRpdGxlIj5TZXJ2ZXJTdGF0dXMgKDx7JHNlcnZlckxvYWR8QGNvdW50fT4pPC9kaXY+CQkJCTxkaXYgY2xhc3M9ImRlYnVnT3RoZXJDb250ZW50Ij4JCQkJCTx0YWJsZT4JCQkJCQk8dGhlYWQ+CQkJCQkJPHRyPgkJCQkJCQk8dGggd2lkdGg9IjMzJSI+S2V5PC90aD4JCQkJCQkJPHRoIHdpZHRoPSI2NyUiPlZhbHVlPC90aD4JCQkJCQk8L3RyPgkJCQkJCTwvdGhlYWQ+CQkJCQkJPHtmb3JlYWNoIGZyb209JHNlcnZlckxvYWQgaXRlbT1yZXEga2V5PXJlcVR5cGV9PgkJCQkJCTx0cj4JCQkJCQkJPHRkPjx7JHJlcVR5cGV9PjwvdGQ+CQkJCQkJCTx0ZD48eyRyZXF9PjwvdGQ+CQkJCQkJPC90cj4JCQkJCQk8ey9mb3JlYWNofT4JCQkJCTwvdGFibGU+CQkJCTwvZGl2PgkJCTwvZGl2PgkJPC9kaXY+CQkJCQkJPGRpdiBpZD0ic3FsRGF0YSIgY2xhc3M9ImRlYnVnUmlnaHRDb250ZW50IiBzdHlsZT0iZGlzcGxheTpub25lIj4JCQkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z1Nob3dPdGhlckRhdGEiPgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyVGl0bGUiPkV4ZWN1dGVTUUwgKDx7JGV4ZWN1dGVEYXRhfEBjb3VudH0+KTwvZGl2PgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyQ29udGVudCI+CQkJCQk8dGFibGU+CQkJCQkJPHRoZWFkPgkJCQkJCTx0cj4JCQkJCQkJPHRoIHdpZHRoPSI4MHB4Ij5EYXRhYmFzZTwvdGg+CQkJCQkJCTx0aCB3aWR0aD0iNjBweCI+RnJvbUNhY2hlPC90aD4JCQkJCQkJPHRoIHdpZHRoPSI5MHB4Ij5DYXN0VGltZTwvdGg+CQkJCQkJCTx0aD5FeGVjdXRlU1FMPC90aD4JCQkJCQk8L3RyPgkJCQkJCTwvdGhlYWQ+CQkJCQkJPHtmb3JlYWNoIGZyb209JGV4ZWN1dGVEYXRhIGl0ZW09c3FsfT4JCQkJCQk8dHI+CQkJCQkJCTx0ZD48e2lmICRzcWwubWFzdGVyID09IDF9Pk1hc3Rlcjx7ZWxzZX0+U2xhdmVyPHsvaWZ9PjwvdGQ+CQkJCQkJCTx0ZD48e2lmICRzcWwuY2FjaGUgPT0gMX0+5pivPHtlbHNlfT7lkKY8ey9pZn0+PC90ZD4JCQkJCQkJPHRkPjx7JHNxbC50aW1lfT4gU2VjPC90ZD4JCQkJCQkJPHRkPjx7JHNxbC5"
"zcWx9Pjxicj5WYWx1ZSA6PHtmb3JlYWNoIGZyb209JHNxbC53aGVyZVZhbHVlIGl0ZW09d30+Wzx7JHd9Pl3jgIE8ey9mb3JlYWNofT48L3RkPgkJCQkJCTwvdHI+CQkJCQkJPHsvZm9yZWFjaH0+CQkJCQk8L3RhYmxlPgkJCQk8L2Rpdj4JCQk8L2Rpdj4JCQk8L2Rpdj4JCQkJCQk8ZGl2IGlkPSJ3YXJuRGF0YSIgY2xhc3M9ImRlYnVnUmlnaHRDb250ZW50IiBzdHlsZT0iZGlzcGxheTpub25lIj4JCQkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z1Nob3dPdGhlckRhdGEiPgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyVGl0bGUiPlJ1bnRpbWVXYXJuICg8eyR3YXJuTGlzdHxAY291bnR9Pik8L2Rpdj4JCQkJPGRpdiBjbGFzcz0iZGVidWdPdGhlckNvbnRlbnQiPgkJCQkJPHRhYmxlPgkJCQkJCTx0aGVhZD4JCQkJCQk8dHI+CQkJCQkJCTx0aCB3aWR0aD0iMzMlIj5LZXk8L3RoPgkJCQkJCQk8dGggd2lkdGg9IjY3JSI+VmFsdWU8L3RoPgkJCQkJCTwvdHI+CQkJCQkJPC90aGVhZD4JCQkJCQk8e2ZvcmVhY2ggZnJvbT0kd2Fybkxpc3QgaXRlbT1yZXEga2V5PXJlcVR5cGV9PgkJCQkJCTx0cj4JCQkJCQkJPHRkPjx7JHJlcS50eXBlfT48L3RkPgkJCQkJCQk8dGQ+PHskcmVxLm1lc3NhZ2V9PiAgJm5ic3A7Jm5ic3A7Jm5ic3A7Jm5ic3A7LSBJbiBTY3JpcHQ8eyRyZXEuZmlsZX0+IExpbmU8eyRyZXEubGluZX0+PC90ZD4JCQkJCQk8L3RyPgkJCQkJCTx7L2ZvcmVhY2h9PgkJCQkJPC90YWJsZT4JCQkJPC9kaXY+CQkJPC9kaXY+CQk8L2Rpdj4JCQkJCQk8ZGl2IGlkPSJ0cmFjZURhdGEiIGNsYXNzPSJkZWJ1Z1JpZ2h0Q29udGVudCIgc3R5bGU9ImRpc3BsYXk6bm9uZSI+CQkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z1Nob3dPdGhlckRhdGEiPgkJCQk8ZGl2IGNsYXNzPSJkZWJ1Z090aGVyVGl0bGUiPkZ1bmN0aW9uQ2FsbFN0YWNrICg8eyRjYWxsU3RhY2t8QGNvdW50fT4pPC9kaXY+CQkJCTxkaXYgY2xhc3M9ImRlYnVnT3RoZXJDb250ZW50Ij4JCQkJCTx0YWJsZT4JCQkJCQk8dGhlYWQ+CQkJCQkJPHRyPgkJCQkJCQk8dGggd2lkdGg9IjMzJSI+Q2FzdDwvdGg+CQkJCQkJCTx0aCB3aWR0aD0iNjclIj5TY3JpcHQ8L3RoPgkJCQkJCTwvdHI+CQkJCQkJPC90aGVhZD4JCQkJCQk8e2ZvcmVhY2ggZnJvbT0kY2FsbFN0YWNrIGl0ZW09ZH0+CQkJCQkJPHRyPgkJCQkJCQk8dGQ+PHskZC5jYXN0fT48L3RkPgkJCQkJCQk8dGQ+PHskZC5zY3J"
"pcHR9PjwvdGQ+CQkJCQkJPC90cj4JCQkJCQk8ey9mb3JlYWNofT4JCQkJCTwvdGFibGU+CQkJCTwvZGl2PgkJCTwvZGl2PgkJCTwvZGl2PgkJCQkJCQkJPHAgY2xhc3M9ImRlYnVnQm90dG9tIj4mbmJzcDs8L3A+CTwvZGl2PjwvZGl2PjxkaXYgY2xhc3M9ImRlYnVnUGx1Z2luRGVzY1Nob3ciPjwvZGl2PjxzY3JpcHQ+ZnVuY3Rpb24gb3BlbkRlYnVnKCl7CWlmKGRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJkZWJ1Z01haW4iKS5jbGFzc0xpc3QuY29udGFpbnMoJ3Nob3dEZWJ1ZycpKXsJCWRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJkZWJ1Z01haW4iKS5jbGFzc0xpc3QucmVtb3ZlKCdzaG93RGVidWcnKTsJCWRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJkZWJ1Z01haW4iKS5jbGFzc0xpc3QuYWRkKCdoaWRlRGVidWcnKTsJfWVsc2V7CQlkb2N1bWVudC5nZXRFbGVtZW50QnlJZCgiZGVidWdNYWluIikuY2xhc3NMaXN0LnJlbW92ZSgnaGlkZURlYnVnJyk7CQlkb2N1bWVudC5nZXRFbGVtZW50QnlJZCgiZGVidWdNYWluIikuY2xhc3NMaXN0LmFkZCgnc2hvd0RlYnVnJyk7CX19ZnVuY3Rpb24gZGVidWdQbHVnaW5DaGFuZ2VUYWdzKGNvbnRlbnRJZCl7CXZhciB0YWdLZXkgPSBjb250ZW50SWQgKyAnS2V5JzsJdmFyIGFsbFRpdGxlID0gZG9jdW1lbnQuZ2V0RWxlbWVudHNCeUNsYXNzTmFtZSgnRGVidWdNZW51Jyk7CWZvcih2YXIgaSA9IDAgOyBpPCBhbGxUaXRsZS5sZW5ndGggOyBpKyspewkJdmFyIHRoaXNFbGVtZW50ID0g"
"YWxsVGl0bGVbaV07CQl0aGlzRWxlbWVudC5jbGFzc0xpc3QucmVtb3ZlKCJsZWZ0VGFnc09uIik7CQl0aGlzRWxlbWVudC5jbGFzc0xpc3QucmVtb3ZlKCJsZWZ0VGFncyIpOwkJdGhpc0VsZW1lbnQuY2xhc3NMaXN0LmFkZCgibGVmdFRhZ3MiKTsJfQlkb2N1bWVudC5nZXRFbGVtZW50QnlJZCh0YWdLZXkpLmNsYXNzTGlzdC5hZGQoJ2xlZnRUYWdzT24nKTsJZG9jdW1lbnQuZ2V0RWxlbWVudEJ5SWQodGFnS2V5KS5jbGFzc0xpc3QucmVtb3ZlKCdsZWZ0VGFncycpOwl2YXIgYWxsQ29udGVudCA9IGRvY3VtZW50LmdldEVsZW1lbnRzQnlDbGFzc05hbWUoJ2RlYnVnUmlnaHRDb250ZW50Jyk7CWZvcih2YXIgaSA9IDAgOyBpPCBhbGxDb250ZW50Lmxlbmd0aCA7IGkrKyl7CQlhbGxDb250ZW50W2ldLnN0eWxlLmNzc1RleHQgPSAnZGlzcGxheTpub25lJzsJfQlkb2N1bWVudC5nZXRFbGVtZW50QnlJZChjb250ZW50SWQpLnN0eWxlLmNzc1RleHQgPSAnZGlzcGxheTpibG9jayc7fTwvc2NyaXB0Pg==";


void CDebug_getLastErrorFileArr(char *file,int line,zval **codeArray TSRMLS_DC){

	int		padding = 10,i,h;
	char	*fileContent;
	zval	*codeParse,
			**thisVal;

	MAKE_STD_ZVAL(*codeArray);
	array_init(*codeArray);

	//check file exists;
	if(SUCCESS != fileExist(file) || line == 0){
		return;
	}

	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[0],file,1);
		ZVAL_BOOL(params[1],1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"highlight_file",0);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		if(IS_STRING == Z_TYPE(callReturn)){
			fileContent = estrdup(Z_STRVAL(callReturn));
		}else{
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			zval_dtor(&callReturn);
			return;
		}
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&callReturn);
	MODULE_END
	

	//explode
	php_explode("<br />",fileContent,&codeParse);

	h = zend_hash_num_elements(Z_ARRVAL_P(codeParse));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(codeParse),(void**)&thisVal);

		if(i < line - padding - 1){
			zend_hash_move_forward(Z_ARRVAL_P(codeParse));
			continue;
		}else if(i >= line - padding  - 1 && i <= line + padding){

			zval	*nowFind,
					*nowReplace;

			char	*thisCodeLine;

			MAKE_STD_ZVAL(nowFind);
			MAKE_STD_ZVAL(nowReplace);
			array_init(nowFind);
			array_init(nowReplace);

			add_next_index_string(nowFind,"<code>",1);
			add_next_index_string(nowFind,"</code>",1);
			add_next_index_string(nowFind,"#0000BB",1);
			add_next_index_string(nowFind,"&lt;?php",1);
			add_next_index_string(nowFind,"?&gt;",1);

			add_next_index_string(nowReplace,"",1);
			add_next_index_string(nowReplace,"",1);
			add_next_index_string(nowReplace,"#0066FF",1);
			add_next_index_string(nowReplace,"<span style=\"color:#DD0000\">&lt;?php</span>",1);
			add_next_index_string(nowReplace,"<span style=\"color:#DD0000\">?&gt;</span>",1);

			str_replaceArray(nowFind,nowReplace,Z_STRVAL_PP(thisVal),&thisCodeLine);

			if(i + 1 == line){
				char *saveString;
				spprintf(&saveString,0,"%s%s%s","<div style=\"border:soild 1px #352C2C;  background:rgba(255, 120, 120, .17)\">",thisCodeLine,"</div>");
				add_index_string(*codeArray,i+1,saveString,1);
				efree(saveString);
			}else{
				char *saveString;
				spprintf(&saveString,0,"%s%s",thisCodeLine,"<br>");
				add_index_string(*codeArray,i+1,saveString,1);
				efree(saveString);
			}

			zval_ptr_dtor(&nowFind);
			zval_ptr_dtor(&nowReplace);
			efree(thisCodeLine);
		}else{
			break;
		}

		zend_hash_move_forward(Z_ARRVAL_P(codeParse));
	}


	//destroy
	efree(fileContent);
	zval_ptr_dtor(&codeParse);
}

void CDebug_getErrorDetail(zval *errorList TSRMLS_DC){

	int		i,h;
	zval	**thisVal,
			**line,
			**file,
			*appPath;
	char	*mohu;

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(errorList));
	h = zend_hash_num_elements(Z_ARRVAL_P(errorList));
	for(i = 0 ; i < h ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(errorList),(void**)&thisVal);

		if(IS_ARRAY == Z_TYPE_PP(thisVal) &&
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"file",strlen("file")+1,(void**)&file) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"line",strlen("line")+1,(void**)&line)  
		){
			zval	*codeArray;
			convert_to_string(*file);
			convert_to_long(*line);
			CDebug_getLastErrorFileArr(Z_STRVAL_PP(file),Z_LVAL_PP(line),&codeArray TSRMLS_CC);
			add_assoc_zval(*thisVal,"detail",codeArray);
		}

		if(IS_ARRAY == Z_TYPE_PP(thisVal) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"file",strlen("file")+1,(void**)&file)){
			str_replace(Z_STRVAL_P(appPath),"APP_PATH",Z_STRVAL_PP(file),&mohu);
			add_assoc_string(*thisVal,"file",mohu,0);
		}


		zend_hash_move_forward(Z_ARRVAL_P(errorList));
	}
}

int CDebug_getIsDebugStats(TSRMLS_D){
	
	char		*getDebug,
				*remoteAdder;

	zval		*cconfigInstanceZval,
				*debugIp;

	//check get params
	getGetParams("debug",&getDebug);
	if(getDebug == NULL){
		return 0;
	}

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("DEBUG_IP",cconfigInstanceZval,&debugIp TSRMLS_CC);
	
	getServerParam("REMOTE_ADDR",&remoteAdder TSRMLS_CC);
	if(remoteAdder == NULL || IS_ARRAY != Z_TYPE_P(debugIp)){
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugIp);
		if(remoteAdder != NULL){
			efree(remoteAdder);
		}
		return 0;
	}

	if(!in_array(remoteAdder,debugIp)){
		efree(remoteAdder);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugIp);
		return 0;
	}


	efree(remoteAdder);
	return 1;
}

void CDebug_createServerLoadData(zval **data TSRMLS_DC){

	MAKE_STD_ZVAL(*data);
	array_init(*data);

	//CGuardController_getCacheStatus(*data TSRMLS_CC);

	//CGuardController_getFpmStatus(*data TSRMLS_CC);

	//CGuardController_getMQStatus(*data TSRMLS_CC);
}

void CDebug_createServerData(zval **data TSRMLS_DC){

	char	*tempString,
			*header,
			*funAllname,
			*funCast,
			*stackString;
	zval	**thisVal,
			**nowTime,
			**classname,
			**functionname,
			**filename,
			*classObject,
			*appPath,
			*thisSaveArray		
			;

	int		i,h;

	double	lastTime = 0,
			castTime = 0,
			beginTime = 0,
			endTime = 0,
			allTime = 0,
			castTimeRate = 0;

	classObject = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(*data);
	array_init(*data);

	if(IS_ARRAY != Z_TYPE_P(classObject)){
		return;
	}

	//firset
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(classObject));
	zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);
	zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
	beginTime = Z_DVAL_PP(nowTime);

	//endTime
	zend_hash_internal_pointer_end(Z_ARRVAL_P(classObject));
	zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);
	zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
	endTime = Z_DVAL_PP(nowTime);
	allTime = endTime - beginTime;

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//foreach array get casttime
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(classObject));
	h = zend_hash_num_elements(Z_ARRVAL_P(classObject));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);

		zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"class_name",strlen("class_name")+1,(void**)&classname);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"function_name",strlen("function_name")+1,(void**)&functionname);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"file_name",strlen("file_name")+1,(void**)&filename);
		if(i == 0){
			castTime = 0;
		}else{
			castTime = Z_DVAL_PP(nowTime) - lastTime;
		}

		//cast Time rate
		castTimeRate = castTime / allTime*100;

		spprintf(&funAllname,0,"%.8f%s%.4f%s",castTime," / ",castTimeRate,"%");


		if(strlen(Z_STRVAL_PP(classname))){
			spprintf(&stackString,0,"%s%d%s%s%s%s%s","[",i+1,"] ",Z_STRVAL_PP(classname),"::",Z_STRVAL_PP(functionname),"()");
		}else{
			spprintf(&stackString,0,"%s%d%s%s%s","[",i+1,"] ",Z_STRVAL_PP(functionname),"()");
		}

		MODULE_BEGIN
			char *mohu;
			str_replace(Z_STRVAL_P(appPath),"APP_PATH",Z_STRVAL_PP(filename),&mohu);
			spprintf(&funCast,0,"%s%s%s",stackString," - In script : ",mohu);
			efree(mohu);
		MODULE_END

		MAKE_STD_ZVAL(thisSaveArray);
		array_init(thisSaveArray);
		add_assoc_string(thisSaveArray,"cast",funAllname,0);
		add_assoc_string(thisSaveArray,"script",funCast,0);

		add_next_index_zval(*data,thisSaveArray);

		efree(stackString);

		lastTime = Z_DVAL_PP(nowTime);
		zend_hash_move_forward(Z_ARRVAL_P(classObject));
	}
}

void CDebug_createRequestData(zval **data,int type TSRMLS_DC){
	
	zval	*cconfigInstanceZval;

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	

	MAKE_STD_ZVAL(*data);
	array_init(*data);


	//sessionId
	MODULE_BEGIN
		zval	callFunction,
				callReturn;
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"session_id",0);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 0, NULL TSRMLS_CC);
		if(IS_STRING == Z_TYPE(callReturn)){
			char *filter;
			htmlspecialchars(Z_STRVAL(callReturn),&filter);
			add_assoc_string(*data,"SessionID",filter,0);
		}
		zval_dtor(&callReturn);
	MODULE_END

	//IP
	MODULE_BEGIN
		char	*clientIp,
				*filter;
		getServerParam("HTTP_CLIENT_IP",&clientIp TSRMLS_CC);
		if(clientIp != NULL){
			htmlspecialchars(clientIp,&filter);
			add_assoc_string(*data,"HTTP_CLIENT_IP",filter,0);
			efree(clientIp);
		}
	MODULE_END

	//REMOTE_ADDR
	MODULE_BEGIN
		char	*clientIp,
				*filter;
		getServerParam("REMOTE_ADDR",&clientIp TSRMLS_CC);
		if(clientIp != NULL){
			htmlspecialchars(clientIp,&filter);
			add_assoc_string(*data,"REMOTE_ADDR",filter,0);
			efree(clientIp);
		}
	MODULE_END


	//HTTP_X_FORWARDED_FOR
	MODULE_BEGIN
		char	*clientIp,
				*filter;
		getServerParam("HTTP_X_FORWARDED_FOR",&clientIp TSRMLS_CC);
		if(clientIp != NULL){
			htmlspecialchars(clientIp,&filter);
			add_assoc_string(*data,"HTTP_X_FORWARDED_FOR",filter,0);
			efree(clientIp);
		}
	MODULE_END

	//chart
	MODULE_BEGIN
		zval	*charst;
		
		CConfig_load("CHARSET",cconfigInstanceZval,&charst TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(charst)){
			add_assoc_string(*data,"Charset",Z_STRVAL_P(charst),1);
		}else{
			add_assoc_string(*data,"Charset","UTF-8",0);
		}
		zval_ptr_dtor(&charst);
	MODULE_END

	//cast Time
	MODULE_BEGIN
		zval	**startTimeZval,
				**begin,
				**end;
		if(zend_hash_find(&EG(symbol_table),"SYSTEM_INIT",strlen("SYSTEM_INIT")+1,(void**)&startTimeZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(startTimeZval) ){
			
			if( SUCCESS == zend_hash_find(Z_ARRVAL_PP(startTimeZval),"frameBegin",strlen("frameBegin")+1,(void**)&begin) && IS_DOUBLE == Z_TYPE_PP(begin) && 
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(startTimeZval),"systemEnd",strlen("systemEnd")+1,(void**)&end) && IS_DOUBLE == Z_TYPE_PP(end)
			){
				char	*castTimeString;
				spprintf(&castTimeString,0,"%.6f%s",Z_DVAL_PP(end) - Z_DVAL_PP(begin)," Sec");
				add_assoc_string(*data,"CastTime",castTimeString,0);
			}
		}
	MODULE_END

	//Meme
	MODULE_BEGIN
		zval	returnZval,
				function;
		TSRMLS_FETCH();
		INIT_ZVAL(function);
		ZVAL_STRING(&function,"memory_get_usage",0);
		call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
		if(IS_LONG == Z_TYPE(returnZval)){
			double	usedFloat;
			char	*useString;
			usedFloat = (float)(Z_LVAL(returnZval)) / 1024.00;
			spprintf(&useString,0,"%.2f%s",usedFloat," Kb");
			add_assoc_string(*data,"Memory",useString,0);
		}
		zval_dtor(&returnZval);
	MODULE_END

	//memory limit
	if(type == 2){
		char	*memLimit;
		ini_get("memory_limit",&memLimit);
		add_assoc_string(*data,"MemoryLimit",memLimit,0);
	}

	//httponly
	MODULE_BEGIN
		char	*httponly;
		ini_get("session.cookie_httponly",&httponly);
		add_assoc_string(*data,"HttpOnly",strcmp(httponly,"1") == 0 ? "On" : "Off",0);
	MODULE_END

	//framework version
	MODULE_BEGIN
		add_assoc_string(*data,"Version","CQuickFramework extension by C , Version 3.0.0",0);
	MODULE_END

	//route
	MODULE_BEGIN
		zval	*controller,
				*action,
				*module;
		char	*routeString,
				*filter,
				*keyName;
		
		controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
		action = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);
		module = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
		spprintf(&routeString,0,"%s%s%s%s%s",Z_STRVAL_P(module),"/",Z_STRVAL_P(controller),"/",Z_STRVAL_P(action));
		htmlspecialchars(routeString,&filter);
		add_assoc_string(*data,"RouteParse",filter,0);
		efree(routeString);
	MODULE_END

	MODULE_BEGIN
		char	*openTrace,
				*keyName;
		ini_get("CMyFrameExtension.open_trace",&openTrace);
		add_assoc_string(*data,"TraceMonitor",strcmp(openTrace,"1") == 0 ? "On" : "Off",0);
	MODULE_END

	//shell check
	MODULE_BEGIN
		char	*openTrace;
		ini_get("CMyFrameExtension.open_shell_check",&openTrace);
		add_assoc_string(*data,"ShellListener",strcmp(openTrace,"1") == 0 ? "On" : "Off",0);
	MODULE_END

	//notifyMail
	MODULE_BEGIN
		zval	*cconfigInstanceZval,
				*mailConfg,
				**thisVal;

		CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("notify",cconfigInstanceZval,&mailConfg TSRMLS_CC);
		if(
			IS_ARRAY == Z_TYPE_P(mailConfg) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"host",strlen("host")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"port",strlen("port")+1,(void**)&thisVal) && IS_LONG == Z_TYPE_PP(thisVal) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"user",strlen("user")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"pass",strlen("pass")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"to",strlen("to")+1,(void**)&thisVal) 
		){
			char	*say;
			add_assoc_string(*data,"NotifyMail","Ready",0);
		}else{
			char	*say;
			add_assoc_string(*data,"NotifyMail","Not Ready",0);
		}
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&mailConfg);
	MODULE_END


	if(type == 2){

	}


	zval_ptr_dtor(&cconfigInstanceZval);
}

void CDebug_createPluginData(zval **data TSRMLS_DC){

	zval	*loadPlugin,
			**thisVal;

	int		i,h;

	MAKE_STD_ZVAL(*data);
	array_init(*data);

	add_assoc_string(*data,"CDebug","Loaded , extension by C",0);

	loadPlugin = zend_read_static_property(CHooksCe,ZEND_STRL("_pluginList"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(loadPlugin)){
		return;
	}
	
	h = zend_hash_num_elements(Z_ARRVAL_P(loadPlugin));
	for(i = 0 ; i < h;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(loadPlugin),(void**)&thisVal);
		add_assoc_string(*data,Z_STRVAL_PP(thisVal),"Loaded",0);
		zend_hash_move_forward(Z_ARRVAL_P(loadPlugin));
	}

	loadPlugin = zend_read_static_property(CHooksCe,ZEND_STRL("_failLoadPluginList"), 0 TSRMLS_CC);
	h = zend_hash_num_elements(Z_ARRVAL_P(loadPlugin));
	for(i = 0 ; i < h;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(loadPlugin),(void**)&thisVal);
		add_assoc_string(*data,Z_STRVAL_PP(thisVal),"UnLoaded",0);
		zend_hash_move_forward(Z_ARRVAL_P(loadPlugin));
	}
}

void CDebug_checkShowDebugInfo(zval *object TSRMLS_DC){
	
	zval	*cconfigInstanceZval,
			*debugIp,
			*viewObjectZval,
			*useQuickTemplate,
			*requestData,
			*serverData,
			*files,
			*appPath,
			*applicationData,
			*pluginData,
			*sqlList,
			*errorList,
			*serverLoad;

	char	*remoteAdder,
			*templateUsed,
			*tempHtml,
			*getDebug = NULL;

	//check get params
	getGetParams("debug",&getDebug);
	if(getDebug == NULL){
		return;
	}

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("DEBUG_IP",cconfigInstanceZval,&debugIp TSRMLS_CC);
	
	getServerParam("REMOTE_ADDR",&remoteAdder TSRMLS_CC);
	if(remoteAdder == NULL || IS_ARRAY != Z_TYPE_P(debugIp)){
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugIp);
		if(remoteAdder != NULL){
			efree(remoteAdder);
		}
		return;
	}


	if(!in_array(remoteAdder,debugIp)){
		efree(remoteAdder);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugIp);
		return;
	}

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//tempType
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	//get view object
	templateUsed = "smarty";
	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}
	zval_ptr_dtor(&useQuickTemplate);

	base64Decode(debugString,&tempHtml);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);

	//get include files
	MODULE_BEGIN
		char	*entry;
		MAKE_STD_ZVAL(files);
		array_init(files);
		zend_hash_internal_pointer_reset(&EG(included_files));
		while (zend_hash_get_current_key(&EG(included_files), &entry, NULL, 1) == HASH_KEY_IS_STRING) {
			char *mohu;
			str_replace(Z_STRVAL_P(appPath),"APP_PATH",entry,&mohu);
			add_next_index_string(files, mohu, 0);
			zend_hash_move_forward(&EG(included_files));
		}
		CQuickTemplate_assign(viewObjectZval,"includeFile",files TSRMLS_CC);
	MODULE_END


	//create requestData
	CDebug_createRequestData(&requestData,2 TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"request",requestData TSRMLS_CC);

	CDebug_createServerData(&serverData TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"server",serverData TSRMLS_CC);

	sqlList = zend_read_property(CDebugCe,object,ZEND_STRL("_sqlList"),0 TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"executeData",sqlList TSRMLS_CC);

	//serverLoad
	CDebug_createServerLoadData(&serverLoad TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"serverLoad",serverLoad TSRMLS_CC);


	//pluginData
	CDebug_createPluginData(&pluginData TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"pluginData",pluginData TSRMLS_CC);

	//callStack
	CDebug_createServerData(&serverData TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"callStack",serverData TSRMLS_CC);

	errorList = zend_read_property(CDebugCe,object,ZEND_STRL("_errorList"),1 TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"warnList",errorList TSRMLS_CC);

	//show HTML
	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],tempHtml,1);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],"internal/debugTemplate",1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"displayHTML",0);
		call_user_function(NULL, &viewObjectZval, &callFunction, &callReturn, 2, params TSRMLS_CC);
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	MODULE_END

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&debugIp);
	efree(remoteAdder);
	efree(tempHtml);
}

PHP_METHOD(CDebug,getRequestShutdown)
{
	zval		*errorList,
				**thisVal,
				**message,
				**type,
				**file,
				**line,
				**trace,
				*appPath,
				**traceDetail,
				*errorInstance,
				*cconfigInstanceZval,
				*useQuickTemplate,
				*debugStatus,
				*viewObjectZval,
				*files,
				*requestData,
				*serverData;

	char		*filePath,
				*tempString,
				*templateUsed,
				*tempHtml;

	smart_str	html = {0};

	int			i,h,hasFatal = 0,needShow = 0;

	errorList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_errorList"),1 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(errorList)){
		CDebug_checkShowDebugInfo(getThis() TSRMLS_CC);
		return;
	}

	hasFatal = CException_hasFatalErrors(TSRMLS_C);
	if(hasFatal != 1){
		CDebug_checkShowDebugInfo(getThis() TSRMLS_CC);
		return;
	}

	//read debug config
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("DEBUG",cconfigInstanceZval,&debugStatus TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);
	if(IS_BOOL == Z_TYPE_P(debugStatus) && 1 == Z_LVAL_P(debugStatus)){
	}else{
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugStatus);
		CDebug_checkShowDebugInfo(getThis() TSRMLS_CC);
		return;
	}
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&debugStatus);

	CException_getInstance(&errorInstance TSRMLS_CC);
	zend_update_property_long(CExceptionCe,errorInstance, ZEND_STRL("errorOutput"), 0 TSRMLS_CC);
	zval_ptr_dtor(&errorInstance);

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//get view object
	templateUsed = "smarty";
	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}
	zval_ptr_dtor(&useQuickTemplate);

	//add error code line
	CDebug_getErrorDetail(errorList TSRMLS_CC);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);

	ob_end_clean();

	//html
	base64Decode(templateString,&tempHtml);

	//assign
	CQuickTemplate_assign(viewObjectZval,"allError",errorList TSRMLS_CC);

	//get include files
	MODULE_BEGIN
		char	*entry;
		MAKE_STD_ZVAL(files);
		array_init(files);
		zend_hash_internal_pointer_reset(&EG(included_files));
		while (zend_hash_get_current_key(&EG(included_files), &entry, NULL, 1) == HASH_KEY_IS_STRING) {
			char *mohu;
			str_replace(Z_STRVAL_P(appPath),"APP_PATH",entry,&mohu);
			add_next_index_string(files, mohu, 0);
			zend_hash_move_forward(&EG(included_files));
		}
		CQuickTemplate_assign(viewObjectZval,"includeFile",files TSRMLS_CC);
	MODULE_END

	//create requestData
	CDebug_createRequestData(&requestData,1 TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"request",requestData TSRMLS_CC);

	CDebug_createServerData(&serverData TSRMLS_CC);
	CQuickTemplate_assign(viewObjectZval,"server",serverData TSRMLS_CC);

	//show HTML
	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],tempHtml,1);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],"internal/errorTemplate",1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"displayHTML",0);
		call_user_function(NULL, &viewObjectZval, &callFunction, &callReturn, 2, params TSRMLS_CC);
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	MODULE_END

	//destroy
	zval_ptr_dtor(&viewObjectZval);
	efree(tempHtml);

	CDebug_checkShowDebugInfo(getThis() TSRMLS_CC);
}


static int dumpClassConstants(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval	*tables = va_arg(args, zval*);
	zval	*save;
	TSRMLS_FETCH();
	
	MAKE_STD_ZVAL(save);
	ZVAL_ZVAL(save,*zv,1,0);
	add_assoc_zval(tables,hash_key->arKey,save);
	return 0;
}

static int dumpClassProperties(zval **zv, int num_args, va_list args, zend_hash_key *hash_key)
{
	zval	*tables = va_arg(args, zval*);
	const	char	*prop_name,*class_name;
	TSRMLS_FETCH();

	if (hash_key->nKeyLength ==0 ) {
		add_next_index_long(tables,hash_key->h);
	} else { 
		zend_unmangle_property_name(hash_key->arKey, hash_key->nKeyLength-1, &class_name, &prop_name);
		if (class_name) {
			char	*endString;
			if (class_name[0]=='*') {
				endString = estrdup("protected");
			} else {
				endString = estrdup("protected");
			}
			add_assoc_string(tables,prop_name,endString,1);
			efree(endString);

		} else {
			add_assoc_string(tables,hash_key->arKey,"public",1);
		}
	}
	return 0;
}

PHP_METHOD(CDebug,dump)
{
	zval	*data;
	int		noPrint = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|b",&data,&noPrint) == FAILURE){
		return;
	}

	if(noPrint == 0){
		php_printf("<pre>");
		php_var_dump(&data,1 TSRMLS_CC);
	}

	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CDebug,dumpZval)
{
	zval	*data;
	int		noPrint = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|b",&data,&noPrint) == FAILURE){
		return;
	}

	php_printf("<pre>");
	php_debug_zval_dump(&data,2 TSRMLS_CC);
}

void CDebug_dumpTraceHtml(zval *classObject TSRMLS_DC){

	char	*tempString,
			*header;
	zval	**thisVal,
			**nowTime,
			**classname,
			**functionname,
			**filename;
	int		i,h;
	smart_str	html = {0};
	double	lastTime = 0,
			castTime = 0,
			beginTime = 0,
			endTime = 0,
			allTime = 0,
			castTimeRate = 0;

	if(IS_ARRAY != Z_TYPE_P(classObject)){
		php_printf("No SQL trace to print,please check the php.ini->[CMyFrameExtension.open_trace] is \"1\"");
		return;
	}

	smart_str_appends(&html,"</pre><style>#classTable .note{padding:10px ;}#classTable .methodName{width:30%}#classTable{border:solid 1px #ccc; width:100%; border-collapse:collapse; margin-bottom:20px; font-size:12px;}td,th{height:26px;line-height:20px; text-align:left; padding-left:8px;border:solid 1px #ccc;}#classTableHead th{background:#F7F7FB}#classTable .name{background:#F7F7FB; width:30%}</style><meta charset=\"UTF-8\"><table id='classTable'><thead id='classTableHead'><tr><th colspan='7'>CQuickFramework Function call strace</th></tr></thead>");
	
	//header
	base64Decode("PHRyPjx0ZD7luo/lj7c8L3RkPjx0ZD7lvIDlp4s8L3RkPjx0ZD7nsbvlkI08L3RkPjx0ZD7mlrnms5U8L3RkPjx0ZD7mlofku7Y8L3RkPjx0ZD7ogJfml7Y8L3RkPjx0ZD7ljaDmr5Q8L3RkPjwvdHI+",&header);
	smart_str_appends(&html,header);
	efree(header);

	//firset
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(classObject));
	zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);
	zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
	beginTime = Z_DVAL_PP(nowTime);

	//endTime
	zend_hash_internal_pointer_end(Z_ARRVAL_P(classObject));
	zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);
	zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
	endTime = Z_DVAL_PP(nowTime);
	allTime = endTime - beginTime;


	//foreach array get casttime
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(classObject));
	h = zend_hash_num_elements(Z_ARRVAL_P(classObject));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(classObject),(void**)&thisVal);

		zend_hash_find(Z_ARRVAL_PP(thisVal),"time",strlen("time")+1,(void**)&nowTime);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"class_name",strlen("class_name")+1,(void**)&classname);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"function_name",strlen("function_name")+1,(void**)&functionname);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"file_name",strlen("file_name")+1,(void**)&filename);
		if(i == 0){
			castTime = 0;
		}else{
			castTime = Z_DVAL_PP(nowTime) - lastTime;
		}

		//cast Time rate
		castTimeRate = castTime / allTime*100;

		spprintf(&tempString,0,"%s%d%s%.8f%s%s%s%s%s%s%s%0.8f%s%0.4f%s","<tr><td>",i+1,"</td><td>",Z_DVAL_PP(nowTime),"</td><td>",Z_STRVAL_PP(classname),"</td><td>",Z_STRVAL_PP(functionname),"</td><td>",Z_STRVAL_PP(filename),"</td><td>",castTime,"</td><td>",castTimeRate,"%</td></tr>");
		smart_str_appends(&html,tempString);
		efree(tempString);

		lastTime = Z_DVAL_PP(nowTime);
		zend_hash_move_forward(Z_ARRVAL_P(classObject));
	}



	smart_str_appends(&html,"</table>");
	smart_str_0(&html);
	php_printf("%s",html.c);
	smart_str_free(&html);
}

void CDebug_dumpClassHTML(zval *classObject TSRMLS_DC){

	char	*tempString;
	zval	**thisVal;
	int		i,h;
	smart_str	html = {0};
	smart_str_appends(&html,"</pre><style>#classTable .note{padding:10px ;}#classTable .methodName{width:30%}#classTable{border:solid 1px #ccc; width:100%; border-collapse:collapse; margin-bottom:20px; font-size:12px;}td,th{height:26px;line-height:20px; text-align:left; padding-left:8px;border:solid 1px #ccc;}#classTableHead th{background:#F7F7FB}#classTable .name{background:#F7F7FB; width:30%}</style><meta charset=\"UTF-8\"><table id='classTable'><thead id='classTableHead'><tr><th colspan='3'>CQuickFramework dump Class Maps</th></tr></thead>");
	
	//className
	zend_hash_find(Z_ARRVAL_P(classObject),"name",strlen("name")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>className</td><td colspan='2'>",Z_STRVAL_PP(thisVal),"</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);


	//parent
	zend_hash_find(Z_ARRVAL_P(classObject),"parentName",strlen("parentName")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>parent</td><td colspan='2'>",Z_STRVAL_PP(thisVal),"</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//isInternal
	zend_hash_find(Z_ARRVAL_P(classObject),"isInternal",strlen("isInternal")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>isInternal</td><td colspan='2'>",Z_LVAL_PP(thisVal) == 1 ? "Yes" : "No","</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//interface
	zend_hash_find(Z_ARRVAL_P(classObject),"isInterface",strlen("isInterface")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>isInterface</td><td colspan='2'>",Z_LVAL_PP(thisVal) == 1 ? "Yes" : "No","</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//isAbstract
	zend_hash_find(Z_ARRVAL_P(classObject),"isAbstract",strlen("isAbstract")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>isAbstract</td><td colspan='2'>",Z_LVAL_PP(thisVal) == 1 ? "Yes" : "No","</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//isFinal
	zend_hash_find(Z_ARRVAL_P(classObject),"isFinal",strlen("isFinal")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>isFinal</td><td colspan='2'>",Z_LVAL_PP(thisVal) == 1 ? "Yes" : "No","</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//construct
	zend_hash_find(Z_ARRVAL_P(classObject),"construct",strlen("construct")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>construct</td><td colspan='2'>",Z_STRVAL_PP(thisVal),"</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//destructor
	zend_hash_find(Z_ARRVAL_P(classObject),"destructor",strlen("destructor")+1,(void**)&thisVal);
	spprintf(&tempString,0,"%s%s%s","<tr><td class='name'>destructor</td><td colspan='2'>",Z_STRVAL_PP(thisVal),"</td></tr>");
	smart_str_appends(&html,tempString);
	efree(tempString);

	//constants
	MODULE_BEGIN
		char	*key;
		ulong	ukey;
		zval	**tableVal;
		zend_hash_find(Z_ARRVAL_P(classObject),"constants_table",strlen("constants_table")+1,(void**)&thisVal);
		h = zend_hash_num_elements(Z_ARRVAL_PP(thisVal));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_key(Z_ARRVAL_PP(thisVal),&key,&ukey,0);	
			zend_hash_get_current_data(Z_ARRVAL_PP(thisVal),(void**)&tableVal);
			convert_to_string(*tableVal);
			spprintf(&tempString,0,"%s%d%s%s%s%s%s","<tr><td class='name'>Constants(",i+1,")</td><td class='methodName'>",key,"</td><td>",Z_STRVAL_PP(tableVal),"</td></tr>");
			smart_str_appends(&html,tempString);
			zend_hash_move_forward(Z_ARRVAL_PP(thisVal));
			efree(tempString);
		}
	MODULE_END

	//properties
	MODULE_BEGIN
		char	*key;
		ulong	ukey;
		zval	**tableVal;
		zend_hash_find(Z_ARRVAL_P(classObject),"properties",strlen("properties")+1,(void**)&thisVal);
		h = zend_hash_num_elements(Z_ARRVAL_PP(thisVal));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_key(Z_ARRVAL_PP(thisVal),&key,&ukey,0);	
			zend_hash_get_current_data(Z_ARRVAL_PP(thisVal),(void**)&tableVal);
			spprintf(&tempString,0,"%s%d%s%s%s%s%s","<tr><td class='name'>Properties(",i+1,")</td><td class='methodName'>",Z_STRVAL_PP(tableVal),"</td><td>",key,"</td></tr>");
			smart_str_appends(&html,tempString);
			zend_hash_move_forward(Z_ARRVAL_PP(thisVal));
			efree(tempString);
		}
	MODULE_END

	//function_table
	MODULE_BEGIN
		char	*key;
		ulong	ukey;
		zval	**tableVal;
		zend_hash_find(Z_ARRVAL_P(classObject),"function_table",strlen("function_table")+1,(void**)&thisVal);
		h = zend_hash_num_elements(Z_ARRVAL_PP(thisVal));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_key(Z_ARRVAL_PP(thisVal),&key,&ukey,0);	
			zend_hash_get_current_data(Z_ARRVAL_PP(thisVal),(void**)&tableVal);
			spprintf(&tempString,0,"%s%d%s%s%s%s%s","<tr><td class='name'>Method(",i+1,")</td><td class='methodName'>",Z_STRVAL_PP(tableVal),"</td><td>",key,"</td></tr>");
			smart_str_appends(&html,tempString);
			zend_hash_move_forward(Z_ARRVAL_PP(thisVal));
			efree(tempString);
		}
	MODULE_END

	smart_str_appends(&html,"</table>");
	smart_str_0(&html);

	php_printf("%s",html.c);
	smart_str_free(&html);
}

void CDebug_dumpClass(char *sclassName,zval **returnData TSRMLS_DC){

	zval				*saveData,
						funstring;

	char				*className;

	zend_class_entry	**classPP,
						*classCe;

	//tolower
	className = estrdup(sclassName);
	php_strtolower(className,strlen(className)+1);

	//find class
	if(SUCCESS != zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&classPP)){
		MAKE_STD_ZVAL(*returnData);
		array_init(*returnData);
		efree(className);
		return;
	}

	//read this class
	classCe = *classPP;

	MAKE_STD_ZVAL(saveData);
	array_init(saveData);

	//className
	add_assoc_string(saveData,"name",sclassName,1);

	//isInternal
	if(classCe->ce_flags & ZEND_INTERNAL_CLASS){
		add_assoc_long(saveData,"isInternal",1);
	}else{
		add_assoc_long(saveData,"isInternal",0);
	}

	//is interface
	if(classCe->ce_flags & ZEND_ACC_INTERFACE){
		add_assoc_long(saveData,"isInterface",1);
	}else{
		add_assoc_long(saveData,"isInterface",0);
	}

	//isAbstract
	if(classCe->ce_flags & ZEND_ACC_IMPLICIT_ABSTRACT_CLASS || classCe->ce_flags & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS){
		add_assoc_long(saveData,"isAbstract",1);
	}else{
		add_assoc_long(saveData,"isAbstract",0);
	}

	//isFinal
	if(classCe->ce_flags & ZEND_ACC_FINAL_CLASS){
		add_assoc_long(saveData,"isFinal",1);
	}else{
		add_assoc_long(saveData,"isFinal",0);
	}


	//constructor
	if(classCe->constructor){
		INIT_ZVAL(funstring);
		ZVAL_STRING(&funstring,classCe->constructor->common.function_name,0);
		add_assoc_string(saveData,"construct",Z_STRVAL(funstring),1);
	}else{
		add_assoc_string(saveData,"construct","",1);
	}

	//destroy
	if(classCe->destructor){
		INIT_ZVAL(funstring);
		ZVAL_STRING(&funstring,classCe->destructor->common.function_name,0);
		add_assoc_string(saveData,"destructor",Z_STRVAL(funstring),1);
	}else{
		add_assoc_string(saveData,"destructor","",1);
	}

	//type
	add_assoc_long(saveData,"type",classCe->type);

	//ce_flags
	add_assoc_long(saveData,"ce_flags",classCe->ce_flags);

	//function_table
	MODULE_BEGIN
		zval				*funtable,
							*thisFunction;
		int					i,h;
		char				*fname;
		ulong				ufunname;
		zend_function		*fun;

		MAKE_STD_ZVAL(funtable);
		array_init(funtable);
		zend_hash_internal_pointer_reset(&classCe->function_table);
		h = zend_hash_num_elements(&classCe->function_table);
		for(i = 0 ; i < h ; i++){
			zend_hash_get_current_key(&classCe->function_table,&fname,&ufunname,0);

			//find function
			if(SUCCESS == zend_hash_find(&classCe->function_table,fname,strlen(fname)+1,(void**)&fun)){
				char	*functionTureName,
						accessRight[120],
						*note;
				functionTureName = estrdup(fun->common.function_name);

				if(fun->common.fn_flags & ZEND_ACC_PROTECTED){
					sprintf(accessRight,"%s","protected");
				}else if(fun->common.fn_flags & ZEND_ACC_PRIVATE){
					sprintf(accessRight,"%s","private");
				}else{
					sprintf(accessRight,"%s","public");
				}

				//static
				if(fun->common.fn_flags & ZEND_ACC_STATIC){
					char	temp[100];
					sprintf(temp,"static %s",accessRight);
					sprintf(accessRight,"%s",temp);
				}

				add_assoc_string(funtable,functionTureName,accessRight,1);
				efree(functionTureName);
			}
			zend_hash_move_forward(&classCe->function_table);
		}
		add_assoc_zval(saveData,"function_table",funtable);
	MODULE_END

	//parent
	if(classCe->parent){
		char	*parentName = estrdup(classCe->parent->name);
		zval	*parentObject;
		CDebug_dumpClass(parentName,&parentObject TSRMLS_CC);
		add_assoc_zval(saveData,"parent",parentObject);
		add_assoc_string(saveData,"parentName",parentName,0);
	}else{
		add_assoc_string(saveData,"parentName","",1);
	}

	//constants_table
	MODULE_BEGIN
		zval	*tables;
		MAKE_STD_ZVAL(tables);
		array_init(tables);
		zend_hash_apply_with_arguments(&classCe->constants_table TSRMLS_CC, (apply_func_arg_t) dumpClassConstants, 1, tables);
		
		add_assoc_zval(saveData,"constants_table",tables);
	MODULE_END

	//properties
	MODULE_BEGIN
		HashTable	*properties;
		zval		*thisObject,
					*tables;
		int			i,h;
		char		*key;
		ulong		ukey;
		MAKE_STD_ZVAL(tables);
		array_init(tables);
		zend_hash_apply_with_arguments(&classCe->properties_info TSRMLS_CC, (apply_func_args_t) dumpClassProperties, 1,tables);
		add_assoc_zval(saveData,"properties",tables);	
	MODULE_END


	MAKE_STD_ZVAL(*returnData);
	ZVAL_ZVAL(*returnData,saveData,1,1);

	efree(className);
}

PHP_METHOD(CDebug,dumpClass)
{
	char				*className;
	int					noPrint = 0,
						classNameLen = 0;
	zval				*saveData;



	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|b",&className,&classNameLen,&noPrint) == FAILURE){
		return;
	}

	CDebug_dumpClass(className,&saveData TSRMLS_CC);

		
	if(noPrint == 0){
		CDebug_dumpClassHTML(saveData TSRMLS_CC);
	}

	RETVAL_ZVAL(saveData,1,1);
}

void CDebug_dumpClassList(zval **returnArray TSRMLS_DC){

	zval				*classList;

	zend_class_entry	**thisVal;

	char				*key,
						*className;

	int					i,h;

	ulong				ukey;

	MAKE_STD_ZVAL(classList);
	array_init(classList);

	zend_hash_internal_pointer_reset(EG(class_table));
	h = zend_hash_num_elements(EG(class_table));
	for(i = 0 ; i < h ; i ++){
		zend_hash_get_current_data(EG(class_table),(void**)&thisVal);
		zend_hash_get_current_key(EG(class_table),&key,&ukey,0);

#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
		if ((*thisVal)->module && !strcasecmp((*thisVal)->module->name, CMyFrameExtension_module_entry.name)) {
			className = estrdup((*thisVal)->name);
			add_next_index_string(classList,className,0);
		}
#else
		if (((*thisVal)->type == ZEND_INTERNAL_CLASS) && (*thisVal)->info.internal.module && !strcasecmp((*thisVal)->info.internal.module->name, CMyFrameExtension_module_entry.name)) {
			className = estrdup((*thisVal)->name);
			add_next_index_string(classList,className,0);
		}
#endif
		
		zend_hash_move_forward(EG(class_table));
	}
	
	MAKE_STD_ZVAL(*returnArray);
	ZVAL_ZVAL(*returnArray,classList,1,1);
}

void CDebug_dumpClassOneForIDE(char *className,char **returnString TSRMLS_DC){

	smart_str  classContent = {0};
	zval	*returnData,
			**propertiesZval,
			**properNameZval,
			**functionZval,
			**functionName,
			**parentNameZval;
	int		i,h;
	char	*key;
	ulong	ukey;

	CDebug_dumpClass(className,&returnData TSRMLS_CC);

	smart_str_appends(&classContent,"\r\nclass ");
	smart_str_appends(&classContent,className);

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(returnData),"parentName",strlen("parentName")+1,(void**)&parentNameZval) && IS_STRING == Z_TYPE_PP(parentNameZval) && Z_STRLEN_PP(parentNameZval) > 0){
		smart_str_appends(&classContent," extends ");
		smart_str_appends(&classContent,Z_STRVAL_PP(parentNameZval));
	}

	smart_str_appends(&classContent,"{\r\n");

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(returnData),"properties",strlen("properties")+1,(void**)&propertiesZval) && IS_ARRAY == Z_TYPE_PP(propertiesZval)){
		h = zend_hash_num_elements(Z_ARRVAL_PP(propertiesZval));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(propertiesZval));
		for(i = 0 ; i < h; i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(propertiesZval),(void**)&properNameZval);
			zend_hash_get_current_key(Z_ARRVAL_PP(propertiesZval),&key,&ukey,0);

			smart_str_appends(&classContent,"    ");
			smart_str_appends(&classContent,Z_STRVAL_PP(properNameZval));
			smart_str_appends(&classContent," $");
			smart_str_appends(&classContent,key);
			smart_str_appends(&classContent,";\r\n");
			zend_hash_move_forward(Z_ARRVAL_PP(propertiesZval));
		}
	}

	//classTable
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(returnData),"function_table",strlen("function_table")+1,(void**)&functionZval) && IS_ARRAY == Z_TYPE_PP(functionZval)){
		h = zend_hash_num_elements(Z_ARRVAL_PP(functionZval));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(functionZval));
		for(i = 0 ; i < h; i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(functionZval),(void**)&functionName);
			zend_hash_get_current_key(Z_ARRVAL_PP(functionZval),&key,&ukey,0);
			
			smart_str_appends(&classContent,"    ");
			smart_str_appends(&classContent,Z_STRVAL_PP(functionName));
			smart_str_appends(&classContent," function ");
			smart_str_appends(&classContent,key);
			smart_str_appends(&classContent,"(){}\r\n");
		
			zend_hash_move_forward(Z_ARRVAL_PP(functionZval));
		}
	}

	smart_str_appends(&classContent,"}");
	smart_str_0(&classContent);
	*returnString = estrdup(classContent.c);
	smart_str_free(&classContent);
	zval_ptr_dtor(&returnData);
}

void CDebug_dumpClassMapForIDE(TSRMLS_D){

	zval		*classList,
				**thisVal;
	int			i,h;
	smart_str	fileContent = {0};
	char		*thisClassFile;

	CDebug_dumpClassList(&classList TSRMLS_CC);

	smart_str_appends(&fileContent,"<?php\r\nexit();\r\n");

	smart_str_appends(&fileContent,"function CDump(){};\r\n");
	smart_str_appends(&fileContent,"function CMyFrameExtension_createProject(){};\r\n");
	smart_str_appends(&fileContent,"function CMyFrameExtension_createPlugin(){};\r\n");
	smart_str_appends(&fileContent,"function CMyFrameExtension_createConsumer(){};\r\n");
	smart_str_appends(&fileContent,"function CMyFrameExtension_dumpClassMapForIDE(){};\r\n");

	h = zend_hash_num_elements(Z_ARRVAL_P(classList));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(classList),(void**)&thisVal);

		CDebug_dumpClassOneForIDE(Z_STRVAL_PP(thisVal),&thisClassFile TSRMLS_CC);
		smart_str_appends(&fileContent,thisClassFile);
		efree(thisClassFile);
		zend_hash_move_forward(Z_ARRVAL_P(classList));
	}

	smart_str_appends(&fileContent,"\r\n?>");
	smart_str_0(&fileContent);
	file_put_contents("CQuickFramework.php",fileContent.c);
	smart_str_free(&fileContent);
	zval_ptr_dtor(&classList);
}

PHP_METHOD(CDebug,dumpInternalClass)
{
	int		i,h,
			noPrint = 0;
	char	*key,
			*className;
	ulong	ukey;
	zend_class_entry	**thisVal;
	zval	*returnClass,
			*classList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&noPrint) == FAILURE){
		return;
	}

	MAKE_STD_ZVAL(classList);
	array_init(classList);

	zend_hash_internal_pointer_reset(EG(class_table));
	h = zend_hash_num_elements(EG(class_table));
	for(i = 0 ; i < h ; i ++){
		zend_hash_get_current_data(EG(class_table),(void**)&thisVal);
		zend_hash_get_current_key(EG(class_table),&key,&ukey,0);

#if ( PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION <= 3 )
		if ((*thisVal)->module && !strcasecmp((*thisVal)->module->name, CMyFrameExtension_module_entry.name)) {
			className = estrdup((*thisVal)->name);
			CDebug_dumpClass(className,&returnClass TSRMLS_CC);
			if(noPrint == 0){
				CDebug_dumpClassHTML(returnClass TSRMLS_CC);
			}
			add_next_index_string(classList,className,1);
			zval_ptr_dtor(&returnClass);
			efree(className);
		}
#else
		if (((*thisVal)->type == ZEND_INTERNAL_CLASS) && (*thisVal)->info.internal.module && !strcasecmp((*thisVal)->info.internal.module->name, CMyFrameExtension_module_entry.name)) {
			className = estrdup((*thisVal)->name);
			CDebug_dumpClass(className,&returnClass TSRMLS_CC);
			if(noPrint == 0){
				CDebug_dumpClassHTML(returnClass TSRMLS_CC);
			}
			add_next_index_string(classList,className,1);
			zval_ptr_dtor(&returnClass);
			efree(className);
		}
#endif
		


		zend_hash_move_forward(EG(class_table));
	}
	
	RETVAL_ZVAL(classList,1,1);
}

PHP_METHOD(CDebug,dumpTrace)
{
	zval	*traceSave;
	int		noPrint = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&noPrint) == FAILURE){
		return;
	}

	traceSave = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);

	if(noPrint == 0){
		CDebug_dumpTraceHtml(traceSave TSRMLS_CC);
	}

	RETVAL_ZVAL(traceSave,1,0);
}