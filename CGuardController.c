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
#include "php_CHooks.h"
#include "php_CController.h"
#include "php_CException.h"
#include "php_CSmtp.h"
#include "php_CWatcher.h"
#include "php_CWebApp.h"
#include "php_CRequest.h"
#include "php_CGuardController.h"

//zend类方法
zend_function_entry CGuardController_functions[] = {
	PHP_ME(CGuardController,Action_run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CGuardController,Action_monitor,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CGuardController,Action_watchTelnet,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CGuardController,Action_watchHttp,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CGuardController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CGuardController",CGuardController_functions);
	CGuardControllerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);

	zend_declare_property_double(CGuardControllerCe, ZEND_STRL("lastMail"),0.00,ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}

void CGuardController_sendMail(char *string TSRMLS_DC){

	zval	*lastMail,
			*nowTimestamp,
			*cconfigInstanceZval,
			*mailConfg,
			**delayZval,
			**toZval,
			**fromZval,
			**hostZval,
			**portZval,
			**userZval,
			**passZval,
			**titleZval,
			*toList,
			*smtpObject,
			**nowMail,
			*toListStirngZval;

	char	*from,
			*host,
			*user,
			*pass,
			*title;

	int		delay = 300,
			port = 0,
			i,h;

	CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("notify",cconfigInstanceZval,&mailConfg TSRMLS_CC);
	zval_ptr_dtor(&cconfigInstanceZval);
	
	//lost notify config , set log and return
	if(IS_ARRAY != Z_TYPE_P(mailConfg)){
		char	*thisMothTime,
				logString[1024];
		php_date("Y-m-d h:i:s",&thisMothTime);
		sprintf(logString,"%s%s%s%s%s%s","#LogTime:",thisMothTime,PHP_EOL,"[CGuardController] need to send warn email , but system lost notify config",PHP_EOL,PHP_EOL);
		CLog_writeSystemFile(logString TSRMLS_CC);
		efree(thisMothTime);
		zval_ptr_dtor(&mailConfg);
		return;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"delay",strlen("delay")+1,(void**)&delayZval) && IS_LONG == Z_TYPE_PP(delayZval) ){
		delay = Z_LVAL_PP(delayZval);
	}


	lastMail = zend_read_static_property(CGuardControllerCe,ZEND_STRL("lastMail"),1 TSRMLS_CC);
	microtime(&nowTimestamp);

	//check need to delay send mail
	if(Z_DVAL_P(lastMail) + delay > Z_DVAL_P(nowTimestamp)){
		php_printf("not need to send");
		zval_ptr_dtor(&nowTimestamp);
		zval_ptr_dtor(&mailConfg);
		return;
	}
	


	//get mail from
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"from",strlen("from")+1,(void**)&fromZval) && IS_STRING == Z_TYPE_PP(fromZval) ){
		from = estrdup(Z_STRVAL_PP(fromZval));
	}else{
		from = estrdup("");
	}

	//host
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"host",strlen("host")+1,(void**)&hostZval) && IS_STRING == Z_TYPE_PP(hostZval) ){
		host = estrdup(Z_STRVAL_PP(hostZval));
	}else{
		host = estrdup("");
	}

	//user
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"user",strlen("user")+1,(void**)&userZval) && IS_STRING == Z_TYPE_PP(userZval) ){
		user = estrdup(Z_STRVAL_PP(userZval));
	}else{
		user = estrdup("");
	}

	//pass
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"pass",strlen("pass")+1,(void**)&passZval) && IS_STRING == Z_TYPE_PP(passZval) ){
		pass = estrdup(Z_STRVAL_PP(passZval));
	}else{
		pass = estrdup("");
	}

	//title
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"title",strlen("title")+1,(void**)&titleZval) && IS_STRING == Z_TYPE_PP(titleZval) ){
		title = estrdup(Z_STRVAL_PP(titleZval));
	}else{
		title = estrdup("");
	}

	//port
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"port",strlen("port")+1,(void**)&portZval) && IS_LONG == Z_TYPE_PP(portZval) ){
		port = Z_LVAL_PP(portZval);
	}

	//to
	MAKE_STD_ZVAL(toList);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(mailConfg),"to",strlen("to")+1,(void**)&toZval)){
		if(IS_ARRAY == Z_TYPE_PP(toZval)){
			ZVAL_ZVAL(toList,*toZval,1,0);
		}else if(IS_STRING == Z_TYPE_PP(toZval)){
			array_init(toList);
			add_next_index_string(toList,Z_STRVAL_PP(toZval),1);
		}else{
			array_init(toList);
		}
	}else{
		array_init(toList);
	}

	//check params is right
	if(
		IS_ARRAY == Z_TYPE_P(toList) && zend_hash_num_elements(Z_ARRVAL_P(toList)) > 0
		&& port > 0 
		&& strlen(pass) > 0 
		&& strlen(user) > 0 
		&& strlen(host) > 0 
		&& strlen(title) > 0
	){}else{

		//params has error
		zval_ptr_dtor(&mailConfg);
		zval_ptr_dtor(&toList);
		efree(host);
		efree(user);
		efree(pass);
		efree(from);
		efree(title);
		zval_ptr_dtor(&nowTimestamp);
		return;
	}


	//init smtp
	MAKE_STD_ZVAL(smtpObject);
	object_init_ex(smtpObject,CSmtpCe);
	if (CSmtpCe->constructor) {
		zval	constructVal,
				smtpReturn,
				*params[4];

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, CSmtpCe->constructor->common.function_name, 0);

		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],host,1);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[1],port);
		MAKE_STD_ZVAL(params[2]);
		ZVAL_STRING(params[2],user,1);
		MAKE_STD_ZVAL(params[3]);
		ZVAL_STRING(params[3],pass,1);
		call_user_function(NULL, &smtpObject, &constructVal, &smtpReturn, 4, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_dtor(&smtpReturn);
	}

	//shezhi title
	zend_update_property_string(CSmtpCe,smtpObject,ZEND_STRL("displayname"),title TSRMLS_CC);

	//send mail to this mail
	MODULE_BEGIN
		zval	constructVal,
				constructReturn,
				*paramsList[4];

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "send", 0);

		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],toList,1,0);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_STRING(paramsList[1],from,1);
		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_STRING(paramsList[2],title,1);
		MAKE_STD_ZVAL(paramsList[3]);
		ZVAL_STRING(paramsList[3],string,1);
		call_user_function(NULL, &smtpObject, &constructVal, &constructReturn, 4, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		zval_dtor(&constructReturn);
	MODULE_END
	


	//set last send mail time
	zend_update_static_property_double(CGuardControllerCe,ZEND_STRL("lastMail"),Z_DVAL_P(nowTimestamp) TSRMLS_CC);


	//destory
	zval_ptr_dtor(&mailConfg);
	zval_ptr_dtor(&toList);
	efree(host);
	efree(user);
	efree(pass);
	efree(from);
	efree(title);
	zval_ptr_dtor(&smtpObject);
	zval_ptr_dtor(&nowTimestamp);
}

int CGuardController_Action_watchHTTP(zval *config,zval *object TSRMLS_DC){

	char	*host,
			*method;

	zval	**hostZval,
			**methodZval,
			**paramsZval,
			**timeoutZval,
			*params,
			*curlReturn,
			*header;

	int		timeout = 3;

	//host
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"host",strlen("host")+1,(void**)&hostZval) && IS_STRING == Z_TYPE_PP(hostZval)){
		host = estrdup(Z_STRVAL_PP(hostZval));
	}else{
		host = estrdup("");
	}

	//method
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"method",strlen("method")+1,(void**)&methodZval) && IS_STRING == Z_TYPE_PP(methodZval)){
		method = estrdup(Z_STRVAL_PP(methodZval));
	}else{
		method = estrdup("GET");
	}

	//params
	MAKE_STD_ZVAL(params);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"params",strlen("params")+1,(void**)&paramsZval) && IS_ARRAY == Z_TYPE_PP(paramsZval)){
		ZVAL_ZVAL(params,*paramsZval,1,0);
	}else{
		array_init(params);
	}

	//timeout
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"timeout",strlen("timeout")+1,(void**)&timeoutZval) && IS_LONG == Z_TYPE_PP(timeoutZval)){
		timeout = Z_LVAL_PP(timeoutZval);
	}

	//check params all rights;
	if(strlen(host) <= 0 ){
		efree(host);
		efree(method);
		zval_ptr_dtor(&params);
		return;
	}

	//test curl
	MAKE_STD_ZVAL(header);
	array_init(header);
	CResponse_sendHttpRequest(host,params,method,header,timeout,&curlReturn TSRMLS_CC);

	//get httpcode
	MODULE_BEGIN
		zval	**infoZval,
				**httpCode;

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(curlReturn),"info",strlen("info")+1,(void**)&infoZval) && IS_ARRAY == Z_TYPE_PP(infoZval)){
		}else{
			efree(host);
			efree(method);
			zval_ptr_dtor(&header);
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&curlReturn);
			return -1;
		}

		//check httpcode is 200
		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(infoZval),"http_code",strlen("http_code")+1,(void**)&httpCode) && IS_LONG == Z_TYPE_PP(httpCode)){
		}else{
			efree(host);
			efree(method);
			zval_ptr_dtor(&header);
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&curlReturn);
			return -1;
		}

		if(Z_LVAL_PP(httpCode) != 200){
			int returnCode = Z_LVAL_PP(httpCode);
			efree(host);
			efree(method);
			zval_ptr_dtor(&header);
			zval_ptr_dtor(&params);
			zval_ptr_dtor(&curlReturn);
			return returnCode;
		}

	MODULE_END


	//destory
	efree(host);
	efree(method);
	zval_ptr_dtor(&header);
	zval_ptr_dtor(&params);
	zval_ptr_dtor(&curlReturn);
	return 200;
}

PHP_METHOD(CGuardController,Action_watchHttp)
{
	zval	*cconfigInstanceZval,
			*httpList,
			**thisHttpConfig,
			*httpConfig,
			**sendMailZval;

	char	*httpKey,
			*thisMothTime;

	ulong	ukey,
			httpUkey;

	int		i,h,httpStatus = 0;

	//read http config to test curl
	CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("http",cconfigInstanceZval,&httpList TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(httpList) && zend_hash_num_elements(Z_ARRVAL_P(httpList)) > 0){
	}else{
		//no need to test
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&httpList);
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(httpList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(httpList));
	
	for(i = 0; i < h;i++){

		zend_hash_get_current_key(Z_ARRVAL_P(httpList),&httpKey,&httpUkey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(httpList),(void**)&thisHttpConfig);

		MAKE_STD_ZVAL(httpConfig);
		ZVAL_ZVAL(httpConfig,*thisHttpConfig,1,0);

		httpStatus = CGuardController_Action_watchHTTP(httpConfig,getThis() TSRMLS_CC);

		php_date("Y-m-d h:i:s",&thisMothTime);
		php_printf("%s%s%s%s%s%s%d","#LogTime:",thisMothTime,PHP_EOL,"[CGuardController] Run HTTP Test task end ,[",httpKey,"] return httpcode is ",httpStatus);

		//log checkLog
		if(httpStatus != 200){
			char	logString[10240];
			sprintf(logString,"%s%s%s%s%s%s%d%s%s","#LogTime:",thisMothTime,PHP_EOL,"[CGuardController] Run HTTP Test task end ,[",httpKey,"] return httpcode is ",httpStatus,PHP_EOL,PHP_EOL);
			CLog_writeSystemFile(logString TSRMLS_CC);

			//check need to mail
			if(SUCCESS == zend_hash_find(Z_ARRVAL_P(httpConfig),"sendMail",strlen("sendMail")+1,(void**)&sendMailZval) && IS_BOOL == Z_TYPE_PP(sendMailZval) && 1 == Z_LVAL_PP(sendMailZval) ){
				//send mail
				CGuardController_sendMail(logString TSRMLS_CC);
			}
		}
		
		efree(thisMothTime);
		zval_ptr_dtor(&httpConfig);
		zend_hash_move_forward(Z_ARRVAL_P(httpList));
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&httpList);
}

PHP_METHOD(CGuardController,Action_run)
{
	
	zval	*appPath,
			*cconfigInstanceZval,
			*telnetList,
			*watchObject,
			**sendMailZval,
			*processList,
			**thisProcessZval,
			**processConfigItem;

	char	runScript[10240],
			scriptParams[10240],
			*key,
			*thisMothTime;

	ulong	ukey;

	int		i,h;


	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//run script
	sprintf(runScript,"%s%s",Z_STRVAL_P(appPath),"/index.php");

	//get telnet config list
	CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("telnet",cconfigInstanceZval,&telnetList TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(telnetList) && zend_hash_num_elements(Z_ARRVAL_P(telnetList))){

		//foreach list
		h = zend_hash_num_elements(Z_ARRVAL_P(telnetList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(telnetList));
		for(i = 0 ; i < h ; i++){

			zend_hash_get_current_key(Z_ARRVAL_P(telnetList),&key,&ukey,0);

			sprintf(scriptParams,"%s%s","CGuardController/watchTelnet/taskName/",key);

			//check this task
			MAKE_STD_ZVAL(watchObject);
			object_init_ex(watchObject,CWatcherCe);
			if (CWatcherCe->constructor) {
				zval	constructVal,
						constructReturn,
						*params[2];

				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, CWatcherCe->constructor->common.function_name, 0);
				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				ZVAL_STRING(params[0],runScript,1);
				ZVAL_STRING(params[1],scriptParams,1);
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 2, params TSRMLS_CC);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				zval_dtor(&constructReturn);
			}

			//call run
			MODULE_BEGIN
				zval	constructVal,
						constructReturn;
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "run", 0);
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				zval_dtor(&constructReturn);
			MODULE_END
			zval_ptr_dtor(&watchObject);


			zend_hash_move_forward(Z_ARRVAL_P(telnetList));
		}
	}
	zval_ptr_dtor(&telnetList);

	//get process watch
	CConfig_load("process",cconfigInstanceZval,&processList TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(processList) && zend_hash_num_elements(Z_ARRVAL_P(processList))){
		
		char	*scriptName,
				*scriptParams;

		h = zend_hash_num_elements(Z_ARRVAL_P(processList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(processList));
		for(i = 0 ; i < h ; i++){
			
			zend_hash_get_current_key(Z_ARRVAL_P(processList),&key,&ukey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(processList),(void**)&thisProcessZval);

			//host
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisProcessZval),"host",strlen("host")+1,(void**)&processConfigItem) && IS_STRING == Z_TYPE_PP(processConfigItem) ){
				scriptName = estrdup(Z_STRVAL_PP(processConfigItem));
			}else{
				scriptName = estrdup("");
			}

			//params
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisProcessZval),"params",strlen("params")+1,(void**)&processConfigItem) && IS_STRING == Z_TYPE_PP(processConfigItem) ){
				scriptParams = estrdup(Z_STRVAL_PP(processConfigItem));
			}else{
				scriptParams = estrdup("");
			}

			if(strlen(scriptParams) <= 0 || strlen(scriptName) <= 0){
				efree(scriptName);
				efree(scriptParams);
				zend_hash_move_forward(Z_ARRVAL_P(processList));
				continue;
			}

			//check this task
			MAKE_STD_ZVAL(watchObject);
			object_init_ex(watchObject,CWatcherCe);
			if (CWatcherCe->constructor) {
				zval	constructVal,
						constructReturn,
						*params[2];

				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, CWatcherCe->constructor->common.function_name, 0);
				MAKE_STD_ZVAL(params[0]);
				MAKE_STD_ZVAL(params[1]);
				ZVAL_STRING(params[0],scriptName,1);
				ZVAL_STRING(params[1],scriptParams,1);
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 2, params TSRMLS_CC);
				zval_ptr_dtor(&params[0]);
				zval_ptr_dtor(&params[1]);
				zval_dtor(&constructReturn);
			}

			//check num setRunNum
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisProcessZval),"num",strlen("num")+1,(void**)&processConfigItem) && IS_LONG == Z_TYPE_PP(processConfigItem) ){
				zval	constructVal,
						constructReturn,
						*paramsList[1];
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "setRunNum", 0);
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_LONG(paramsList[0],Z_LVAL_PP(processConfigItem));
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
				zval_dtor(&constructReturn);
				zval_ptr_dtor(&paramsList[0]);
			}

			//setLogPath
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisProcessZval),"logs",strlen("logs")+1,(void**)&processConfigItem) && IS_STRING == Z_TYPE_PP(processConfigItem) ){
				zval	constructVal,
						constructReturn,
						*paramsList[1];
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "setLogFile", 0);
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_STRING(paramsList[0],Z_STRVAL_PP(processConfigItem),1);
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
				zval_dtor(&constructReturn);
				zval_ptr_dtor(&paramsList[0]);
			}

			//call run
			MODULE_BEGIN
				zval	constructVal,
						constructReturn;
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "run", 0);
				call_user_function(NULL, &watchObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				zval_dtor(&constructReturn);
			MODULE_END
			zval_ptr_dtor(&watchObject);
			efree(scriptName);
			efree(scriptParams);

			zend_hash_move_forward(Z_ARRVAL_P(processList));
		}

	}
	zval_ptr_dtor(&processList);
	

	//destroy
	zval_ptr_dtor(&cconfigInstanceZval);
}

int CGuardController_Action_watchTelnet(zval *config,zval *object TSRMLS_DC){

	//cmd
	char	telnetCmd[1024],
			*telentHost;

	int		endStatus = 0;

	zval	**checkTypeZval;



	//the checktype is telnet
	MODULE_BEGIN

		char	*host,
				*shellReturnString;
		int		port = 0;

		zval	**hostZval,
				**portZval;

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"host",strlen("host")+1,(void**)&hostZval) && IS_STRING == Z_TYPE_PP(hostZval)){
			host = estrdup(Z_STRVAL_PP(hostZval));
		}else{
			//host error
			return -1;	//lost host
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"port",strlen("port")+1,(void**)&portZval) && IS_LONG == Z_TYPE_PP(portZval)){
			port = Z_LVAL_PP(portZval);
		}else{
			//port error
			efree(host);
			return -2; //list post
		}


		sprintf(telnetCmd,"%s%s%s%d","/usr/bin/telnet ",host," ",port);

		exec_shell_return(telnetCmd,&shellReturnString);

		if(strstr(shellReturnString,"Connected") != NULL){
			endStatus = 1;
		}else{
			endStatus = -3;
		}

		efree(host);
		efree(shellReturnString);
	MODULE_END

	return endStatus;
}


PHP_METHOD(CGuardController,Action_watchTelnet)
{

	zval	*cconfigInstanceZval,
			*thisConfig,
			**startCmdZval,
			**sendMailZval,
			*taskNameZval,
			**intervalZval;

	int		checkStatus = 0,
			checkNum = 0,
			interval = 5;

	char	*startCmd,
			*taskName,
			taskConfigKey[1024];

	//get task name
	CRequest_Args("taskName","string","GET",1,&taskNameZval TSRMLS_CC);

	if(IS_STRING != Z_TYPE_P(taskNameZval)){
		zval_ptr_dtor(&taskNameZval);
		zend_throw_exception(CShellExceptionCe, "[CWatchException] system begin to watch process , but the taskName not given in GET params ", "10010" TSRMLS_CC);
		return;
	}
	taskName = Z_STRVAL_P(taskNameZval);
	if(strlen(taskName) <= 0){
		zval_ptr_dtor(&taskNameZval);
		zend_throw_exception(CShellExceptionCe, "[CWatchException] system begin to watch process , but the taskName not given in GET params ", "10010" TSRMLS_CC);
		return;
	}


	CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
	sprintf(taskConfigKey,"%s%s","telnet.",taskName);
	CConfig_load(taskConfigKey,cconfigInstanceZval,&thisConfig TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(thisConfig)){
		char errorMessage[1024];
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&thisConfig);
		sprintf(errorMessage,"%s%s%s","[CWatchException] system begin to watch php process , but can not find config[telnet->",taskName,"] ");
		zend_throw_exception(CShellExceptionCe, errorMessage, "10010" TSRMLS_CC);
		RETURN_FALSE;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(thisConfig),"start",strlen("start")+1,(void**)&startCmdZval) && IS_STRING == Z_TYPE_PP(startCmdZval) ){
		startCmd = estrdup(Z_STRVAL_PP(startCmdZval));
	}else{
		startCmd = estrdup("");
	}

	//get configs's interval
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(thisConfig),"interval",strlen("interval")+1,(void**)&intervalZval) && IS_LONG == Z_TYPE_PP(intervalZval) ){
		interval = Z_LVAL_PP(intervalZval);
	}

	while(1){

		char	logString[10240],
				*thisMothTime;

		//runCheck
		checkStatus = CGuardController_Action_watchTelnet(thisConfig,getThis() TSRMLS_CC);

		//log checkLog
		if(checkStatus != 1){
			
			int		hasRestart = 0;
			char	*returnString;

			//check need restart
			if(strlen(startCmd) > 0){
				exec_shell_return(startCmd,&returnString);
				hasRestart = 1;
			}else{
				returnString = estrdup("no start cmd");
			}

			//set log
			php_date("Y-m-d h:i:s",&thisMothTime);
			sprintf(logString,"%s%s%s%s%s%s%s%s%s%s%s","#LogTime:",thisMothTime,PHP_EOL,"[CGuardController] Run Checked task [",taskName,"] event end , this check Status is stoped , the guarder do restart status : ", (hasRestart == 1 ? "succeess " : "failure "),startCmd,returnString,PHP_EOL,PHP_EOL);
			php_printf("%s%s%s%s","[CGuardController] Run Checked task [",taskName,"] end , this check Status is stoped , the guarder do restart status :",(hasRestart == 1 ? "succeess\n" : "failure\n"));
			CLog_writeSystemFile(logString TSRMLS_CC);
			efree(thisMothTime);

			//check need to mail
			if(SUCCESS == zend_hash_find(Z_ARRVAL_P(thisConfig),"sendMail",strlen("sendMail")+1,(void**)&sendMailZval) && IS_BOOL == Z_TYPE_PP(sendMailZval) && 1 == Z_LVAL_PP(sendMailZval) ){
				//send mail
				CGuardController_sendMail(logString TSRMLS_CC);
			}
			efree(returnString);

		}else{
			php_printf("%s%d%s","[CGuardController] Run Checked php event end , this check Status is running , the process will sleep ",interval," secound\n");
		}

		//sleep sometime sec
		php_sleep(interval);
	}

	//destory
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&thisConfig);
	efree(startCmd);
	zval_ptr_dtor(&taskNameZval);
}

PHP_METHOD(CGuardController,Action_monitor)
{


}
