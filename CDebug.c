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
				*debugStatus;

	char		*filePath,
				*tempString;

	smart_str	html = {0};

	int			i,h,hasFatal = 0;

	errorList = zend_read_property(CDebugCe,getThis(),ZEND_STRL("_errorList"),1 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(errorList)){
		return;
	}

	hasFatal = CException_hasFatalErrors(TSRMLS_C);
	if(hasFatal != 1){
		return;
	}

	//read debug config
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("DEBUG",cconfigInstanceZval,&debugStatus TSRMLS_CC);
	if(IS_BOOL == Z_TYPE_P(debugStatus) && 1 == Z_LVAL_P(debugStatus)){
	}else{
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&debugStatus);
		return;
	}
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&debugStatus);


	CException_getInstance(&errorInstance TSRMLS_CC);
	zend_update_property_long(CExceptionCe,errorInstance, ZEND_STRL("errorOutput"), 0 TSRMLS_CC);
	zval_ptr_dtor(&errorInstance);

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//header
	smart_str_appends(&html,"</pre><style>#classTable .maininfo{cursor: pointer;}#classTable .switch{display:inline-block;width:20px; cursor: pointer;}#classTable .none{display:none}#classTable .desc{padding-left:10px}#classTable .desc2{padding-left:60px}#classTable .desc3{padding-left:40px}#classTable{border:solid 1px #ccc; width:100%; border-collapse:collapse; margin-bottom:20px; font-size:14px;}td,th{height:30px; text-align:left; padding-left:8px;border:solid 1px #ccc;}#classTableHead th{background:#F7F7FB}#classTable .name{background:#F7F7FB; width:30%}</style><table id='classTable'><thead id='classTableHead'><tr><th colspan='3'>CQuickFramework Exception Report Page</th></tr></thead>");
	smart_str_appends(&html,"<script>function showTrace(id){var nowstatus=document.getElementById('switch'+id).innerHTML;if(nowstatus=='+'){document.getElementById('switch'+id).innerHTML='-';var needShow=document.getElementsByClassName('stack'+id);for(var i=0;i<needShow.length;i++){needShow[i].classList.remove('none')}}else{document.getElementById('switch'+id).innerHTML='+';var needShow=document.getElementsByClassName('stack'+id);for(var i=0;i<needShow.length;i++){needShow[i].classList.add('none')}}};</script>");
	

	//create HTML
	zend_hash_internal_pointer_end(Z_ARRVAL_P(errorList));
	h = zend_hash_num_elements(Z_ARRVAL_P(errorList));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(errorList),(void**)&thisVal);
		if(IS_ARRAY != Z_TYPE_PP(thisVal)){
			zend_hash_move_forward(Z_ARRVAL_P(errorList));
			continue;
		}

		zend_hash_find(Z_ARRVAL_PP(thisVal),"message",strlen("message")+1,(void**)&message);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"type",strlen("type")+1,(void**)&type);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"line",strlen("line")+1,(void**)&line);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"file",strlen("file")+1,(void**)&file);
		zend_hash_find(Z_ARRVAL_PP(thisVal),"trace",strlen("trace")+1,(void**)&trace);

		str_replace(Z_STRVAL_P(appPath),"APP_PATH",Z_STRVAL_PP(file),&filePath);


		spprintf(&tempString,0,"%s%d%s%d%s%s%s%s%s%s%s%d%s%d%s%d%s","<tr><td class='desc maininfo' onclick=\"showTrace('",i,"')\" style='padding-top:10px;padding-bottom:10px;line-height:24px'> <span id='switch",i,"' class=\"switch\">+</span> [",Z_STRVAL_PP(type),"] : ",Z_STRVAL_PP(message),"&nbsp;&nbsp;&nbsp;&nbsp;  - in script ",filePath," - Line : ",Z_LVAL_PP(line),"</td></tr><tr><td id='func",i,"' class='desc3 none stack",i,"'>FunctionStack : </td></tr>");
		smart_str_appends(&html,tempString);
		efree(tempString);
		efree(filePath);

		if(IS_ARRAY == Z_TYPE_PP(trace)){
			int		k,j;
			zval	**detailClass,
					**detailFunction,
					**detailType,
					**detailFile,
					**defaultLine;
			char	*fileTrue;
			long	fileLine = 0;
			zend_hash_internal_pointer_end(Z_ARRVAL_PP(trace));
			j = zend_hash_num_elements(Z_ARRVAL_PP(trace));
			for(k = 0 ; k < j ; k++){
				zend_hash_get_current_data(Z_ARRVAL_PP(trace),(void**)&traceDetail);

				zend_hash_find(Z_ARRVAL_PP(traceDetail),"function",strlen("function")+1,(void**)&detailFunction);
				zend_hash_find(Z_ARRVAL_PP(traceDetail),"class",strlen("class")+1,(void**)&detailClass);
				zend_hash_find(Z_ARRVAL_PP(traceDetail),"type",strlen("type")+1,(void**)&detailType);

				if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(traceDetail),"file",strlen("file")+1,(void**)&detailFile) && IS_STRING == Z_TYPE_PP(detailFile)){
					str_replace(Z_STRVAL_P(appPath),"APP_PATH",Z_STRVAL_PP(detailFile),&fileTrue);
				}else{
					fileTrue = estrdup("Unknown script");
				}

				if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(traceDetail),"line",strlen("line")+1,(void**)&defaultLine) && IS_LONG == Z_TYPE_PP(defaultLine)){
					fileLine = Z_LVAL_PP(defaultLine);
				}else{
					fileLine = 0;
				}


				spprintf(&tempString,0,"%s%d%s%d%s%s%s%s%s%s%s%d%s","<tr class='stack",i," none'><td class='desc2'>",k+1,".",Z_STRVAL_PP(detailClass),Z_STRVAL_PP(detailType),Z_STRVAL_PP(detailFunction),"()&nbsp;&nbsp;&nbsp;&nbsp; in script : ",fileTrue," - Line : ",fileLine,"</td></tr>");
				smart_str_appends(&html,tempString);
				efree(fileTrue);
				efree(tempString);

				zend_hash_move_backwards(Z_ARRVAL_PP(trace));
			}
		}

		smart_str_appends(&html,"<tr><td class=\"none\">&nbsp;</td></tr>");

		zend_hash_move_backwards(Z_ARRVAL_P(errorList));
	}




	smart_str_0(&html);
	php_printf("%s",html.c);
	smart_str_free(&html);
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