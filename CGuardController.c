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
#include "php_CMonitor.h"
#include "php_CActiveRecord.h"
#include "php_CHooks.h"
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

void CGuardController_sendMail(zval *toList,char *string TSRMLS_DC){

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
			**sendMailZval,
			*notifyConfig,
			**node1,
			*toList,
			**toZval;

	char	*httpKey,
			*thisMothTime,
			*templatePath;

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

	//notifyConf
	CConfig_load("notify",cconfigInstanceZval,&notifyConfig TSRMLS_CC);

	//tempPath
	if(IS_ARRAY == Z_TYPE_P(notifyConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyConfig),"template",strlen("template")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)){
		templatePath = estrdup(Z_STRVAL_PP(node1));
	}else{
		templatePath = estrdup("");
	}
	MAKE_STD_ZVAL(toList);
	if(IS_ARRAY == Z_TYPE_P(notifyConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyConfig),"to",strlen("to")+1,(void**)&toZval)){
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
				
				//mail content
				zval	*mailContent,
						*warnItem,
						*warnItemList,
						*htmlContent,
						*saveToList;
	
				char	*httpInterCheck,
						*nohttp200,
						*nowWarnVal,
						*hostIdString,
						itemTrueName[1024];

				MAKE_STD_ZVAL(warnItemList);
				array_init(warnItemList);
				getHostName(&hostIdString);
				MAKE_STD_ZVAL(mailContent);
				array_init(mailContent);
				MAKE_STD_ZVAL(warnItem);
				array_init(warnItem);
				base64Decode("SFRUUOaOpeWPo+aOoua1iw==",&httpInterCheck);
				base64Decode("54q25oCB6Z2eMjAw",&nohttp200);
				sprintf(itemTrueName,"%s%s%s%s",httpInterCheck,"(",httpKey,")");
				add_assoc_string(warnItem,"item",itemTrueName,1);
				efree(httpInterCheck);
				add_assoc_string(warnItem,"condition",nohttp200,1);
				efree(nohttp200);
				add_assoc_string(warnItem,"note","",1);
				add_assoc_long(warnItem,"value",httpStatus);
				add_assoc_long(warnItem,"flag",1);
				add_assoc_string(warnItem,"unit","",1);
				add_assoc_string(mailContent,"mailTye","httpCheckWarn",1);
				add_assoc_string(mailContent,"mailDate",thisMothTime,1);
				add_assoc_string(mailContent,"mailSender","CGuardController/watchHttp",1);
				add_assoc_string(mailContent,"hostId",hostIdString,1);
				add_next_index_zval(warnItemList,warnItem);
				add_assoc_zval(mailContent,"item",warnItemList);
				MAKE_STD_ZVAL(saveToList);
				ZVAL_ZVAL(saveToList,toList,1,0);
				add_assoc_zval(mailContent,"to",saveToList);
				
				//call assign
				MODULE_BEGIN
					zval	callFunction,
							returnFunction,
							*paramsList[2];
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_STRING(paramsList[0],"data",1);
					MAKE_STD_ZVAL(paramsList[1]);
					ZVAL_ZVAL(paramsList[1],mailContent,1,0);
					INIT_ZVAL(callFunction);
					ZVAL_STRING(&callFunction, "assign", 0);
					call_user_function(NULL, &getThis(), &callFunction, &returnFunction, 2, paramsList TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
					zval_ptr_dtor(&paramsList[1]);
					zval_dtor(&returnFunction);
				MODULE_END

				//get a view object
				MODULE_BEGIN
					zval	callFunction,
							returnFunction,
							*paramsList[1],
							*saveHtml;
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_STRING(paramsList[0],templatePath,1);
					INIT_ZVAL(callFunction);
					ZVAL_STRING(&callFunction, "fetch", 0);
					call_user_function(NULL, &getThis(), &callFunction, &returnFunction, 1, paramsList TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
					if(IS_STRING == Z_TYPE(returnFunction)){
						add_assoc_string(mailContent,"html",Z_STRVAL(returnFunction),1);
					}
					zval_dtor(&returnFunction);
				MODULE_END

				//callHooks
				MODULE_BEGIN
					zval	*paramsList[1],
							*dataObject,
							*hooksUseData,
							**endSendHtml;
					MAKE_STD_ZVAL(dataObject);
					object_init_ex(dataObject,CDataObjectCe);
					CHooks_setDataObject(dataObject,mailContent TSRMLS_CC);
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],dataObject,1,0);
					CHooks_callHooks("HOOKS_MAIL_BEFORE",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);

					//check return data's type has right
					CHooks_getDataObject(dataObject,&hooksUseData TSRMLS_CC);
					

					if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"html",strlen("html")+1,(void**)&endSendHtml) && IS_STRING == Z_TYPE_PP(endSendHtml)){
						//send mail
						zval	**newToList,
								*endToList;
						MAKE_STD_ZVAL(endToList);
						if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"to",strlen("to")+1,(void**)&newToList) && IS_ARRAY == Z_TYPE_PP(newToList)){
							ZVAL_ZVAL(endToList,*newToList,1,0);
						}else{
							ZVAL_ZVAL(endToList,toList,1,0);
						}
						CGuardController_sendMail(endToList,Z_STRVAL_PP(endSendHtml) TSRMLS_CC);
						zval_ptr_dtor(&endToList);
					}

					zval_ptr_dtor(&dataObject);
					zval_ptr_dtor(&hooksUseData);
				MODULE_END

				efree(hostIdString);
				zval_ptr_dtor(&mailContent);
			}
		}
		
		efree(thisMothTime);
		zval_ptr_dtor(&httpConfig);
		zend_hash_move_forward(Z_ARRVAL_P(httpList));
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&httpList);
	zval_ptr_dtor(&notifyConfig);
	efree(templatePath);
	zval_ptr_dtor(&toList);
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
	

	//run http
	MODULE_BEGIN
		zval	*httpList;
		CConfig_load("http",cconfigInstanceZval,&httpList TSRMLS_CC);
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
			ZVAL_STRING(params[1],"CGuardController/watchHttp",1);
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
		zval_ptr_dtor(&httpList);
	MODULE_END

	//run monitor
	MODULE_BEGIN
		zval	*moniter,
				**checkRun;
		CConfig_load("moniter",cconfigInstanceZval,&moniter TSRMLS_CC);
		if(IS_ARRAY == Z_TYPE_P(moniter) && SUCCESS == zend_hash_find(Z_ARRVAL_P(moniter),"run",strlen("run")+1,(void**)&checkRun) && IS_BOOL == Z_TYPE_PP(checkRun) && 1 == Z_LVAL_PP(checkRun) ){
			
			//begintoRun
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
				ZVAL_STRING(params[1],"CGuardController/monitor",1);
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
		}
		zval_ptr_dtor(&moniter);
	MODULE_END

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
			**intervalZval,
			*notifyConfig,
			**node1,
			**delayZval,
			*toList,
			**toZval;

	int		checkStatus = 0,
			checkNum = 0,
			interval = 5,
			delay = 300;

	char	*startCmd,
			*taskName,
			taskConfigKey[1024],
			*templatePath;

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

	//notifyConfig
	CConfig_load("notify",cconfigInstanceZval,&notifyConfig TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(notifyConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyConfig),"template",strlen("template")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)){
		templatePath = estrdup(Z_STRVAL_PP(node1));
	}else{
		templatePath = estrdup("");
	}
	if(IS_ARRAY == Z_TYPE_P(notifyConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyConfig),"delay",strlen("delay")+1,(void**)&delayZval) && IS_LONG == Z_TYPE_PP(delayZval) ){
		delay = Z_LVAL_PP(delayZval);
	}
	MAKE_STD_ZVAL(toList);
	if(IS_ARRAY == Z_TYPE_P(notifyConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyConfig),"to",strlen("to")+1,(void**)&toZval)){
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

	while(1){

		char	logString[10240],
				*thisMothTime;

		//runCheck
		checkStatus = CGuardController_Action_watchTelnet(thisConfig,getThis() TSRMLS_CC);

		dumpMemory();

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

			//check need to mail
			if(SUCCESS == zend_hash_find(Z_ARRVAL_P(thisConfig),"sendMail",strlen("sendMail")+1,(void**)&sendMailZval) && IS_BOOL == Z_TYPE_PP(sendMailZval) && 1 == Z_LVAL_PP(sendMailZval) ){
				
				//mail content
				zval	*mailContent,
						*warnItem,
						*warnItemList,
						*htmlContent,
						*lastMail,
						*nowTimestamp;
	
				char	*httpInterCheck,
						*nohttp200,
						*nowWarnVal,
						*hostIdString;

				//send in
				lastMail = zend_read_static_property(CGuardControllerCe,ZEND_STRL("lastMail"),1 TSRMLS_CC);
				microtime(&nowTimestamp);
				//check need to delay send mail
				if(Z_DVAL_P(lastMail) + delay > Z_DVAL_P(nowTimestamp)){
					php_printf("send mail delay .. ");
				}else{
					char		telnetName[120];
					zval		*saveToList;
					MAKE_STD_ZVAL(warnItemList);
					array_init(warnItemList);
					getHostName(&hostIdString);
					MAKE_STD_ZVAL(mailContent);
					array_init(mailContent);
					MAKE_STD_ZVAL(warnItem);
					array_init(warnItem);
					base64Decode("VGVsbmV0572R57uc5o6i5rWL",&httpInterCheck);
					base64Decode("5o6i5rWL5aSx6LSl",&nohttp200);
					sprintf(telnetName,"%s%s%s%s",httpInterCheck,"(",taskName,")");
					add_assoc_string(warnItem,"item",telnetName,1);
					efree(httpInterCheck);
					add_assoc_string(warnItem,"condition",nohttp200,1);
					efree(nohttp200);
					add_assoc_string(warnItem,"note",(hasRestart == 1 ? "hasRestart" : "noAction"),1);
					add_assoc_string(warnItem,"value","stopped",1);
					add_assoc_long(warnItem,"flag",1);
					add_assoc_string(warnItem,"unit","",1);
					add_assoc_string(mailContent,"mailTye","httpCheckWarn",1);
					add_assoc_string(mailContent,"mailDate",thisMothTime,1);
					add_assoc_string(mailContent,"mailSender","CGuardController/watchTelnet",1);
					add_assoc_string(mailContent,"hostId",hostIdString,1);
					add_next_index_zval(warnItemList,warnItem);
					add_assoc_zval(mailContent,"item",warnItemList);
					MAKE_STD_ZVAL(saveToList);
					add_assoc_zval(mailContent,"to",saveToList);
									
					//call assign
					MODULE_BEGIN
						zval	callFunction,
								returnFunction,
								*paramsList[2];
						MAKE_STD_ZVAL(paramsList[0]);
						ZVAL_STRING(paramsList[0],"data",1);
						MAKE_STD_ZVAL(paramsList[1]);
						ZVAL_ZVAL(paramsList[1],mailContent,1,0);
						INIT_ZVAL(callFunction);
						ZVAL_STRING(&callFunction, "assign", 0);
						call_user_function(NULL, &getThis(), &callFunction, &returnFunction, 2, paramsList TSRMLS_CC);
						zval_ptr_dtor(&paramsList[0]);
						zval_ptr_dtor(&paramsList[1]);
						zval_dtor(&returnFunction);
					MODULE_END

					//get a view object
					MODULE_BEGIN
						zval	callFunction,
								returnFunction,
								*paramsList[1],
								*saveHtml;
						MAKE_STD_ZVAL(paramsList[0]);
						ZVAL_STRING(paramsList[0],templatePath,1);
						INIT_ZVAL(callFunction);
						ZVAL_STRING(&callFunction, "fetch", 0);
						call_user_function(NULL, &getThis(), &callFunction, &returnFunction, 1, paramsList TSRMLS_CC);
						zval_ptr_dtor(&paramsList[0]);
						add_assoc_string(mailContent,"html",Z_STRVAL(returnFunction),1);
						zval_dtor(&returnFunction);
					MODULE_END

					//callHooks
					MODULE_BEGIN
						zval	*paramsList[1],
								*dataObject,
								*hooksUseData,
								**endSendHtml;

						MAKE_STD_ZVAL(dataObject);
						object_init_ex(dataObject,CDataObjectCe);
						CHooks_setDataObject(dataObject,mailContent TSRMLS_CC);

						MAKE_STD_ZVAL(paramsList[0]);
						ZVAL_ZVAL(paramsList[0],dataObject,1,0);
						CHooks_callHooks("HOOKS_MAIL_BEFORE",paramsList,1 TSRMLS_CC);
						zval_ptr_dtor(&paramsList[0]);

						//check return data's type has right
						CHooks_getDataObject(dataObject,&hooksUseData TSRMLS_CC);

						if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"html",strlen("html")+1,(void**)&endSendHtml) && IS_STRING == Z_TYPE_PP(endSendHtml)){
							//send mail
							zval	**newToList,
									*endToList;
							MAKE_STD_ZVAL(endToList);
							if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"to",strlen("to")+1,(void**)&newToList) && IS_ARRAY == Z_TYPE_PP(newToList)){
								ZVAL_ZVAL(endToList,*newToList,1,0);
							}else{
								ZVAL_ZVAL(endToList,toList,1,0);
							}
							php_printf("send mail now .. ");
							CGuardController_sendMail(endToList,Z_STRVAL_PP(endSendHtml) TSRMLS_CC);
							zval_ptr_dtor(&endToList);
						}

						zval_ptr_dtor(&hooksUseData);
						zval_ptr_dtor(&dataObject);
					MODULE_END

					efree(hostIdString);
					zval_ptr_dtor(&mailContent);
				}
				zval_ptr_dtor(&nowTimestamp);
			}
			efree(returnString);
			efree(thisMothTime);
		}else{
			php_printf("%s%d%s","[CGuardController] Run Checked php event end , this check Status is running , the process will sleep ",interval," secound\n");
		}

		//sleep sometime sec
		php_sleep(interval);
	}

	//destory
	zval_ptr_dtor(&notifyConfig);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&thisConfig);
	efree(startCmd);
	efree(templatePath);
	zval_ptr_dtor(&taskNameZval);
}

//get net status
void CGuardController_getNetUsed(char **in,char **out TSRMLS_DC){

	zval	*point1,
			*point2,
			**point1Val,
			**point1ValInByte,
			**point1ValOutByte,
			**point2Val,
			**point2ValInByte,
			**point2ValOutByte;

	long	point1loIn = 0,
			point1loOut = 0,
			point1outIn = 0,
			point1outOut = 0,
			point2loIn = 0,
			point2loOut = 0,
			point2outIn = 0,
			point2outOut = 0;

	double	loInRate = 0.00,
			loOutRate = 0.00,
			outInRate = 0.00,
			outOutRate = 0.00;

	int		i,h;

	char	*pointKey;

	ulong	pointUkey;

	CMonitor_getNetworkStat(&point1 TSRMLS_CC);
	php_sleep(1);
	CMonitor_getNetworkStat(&point2 TSRMLS_CC);

	//point1 data
	h = zend_hash_num_elements(Z_ARRVAL_P(point1));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(point1),&pointKey,&pointUkey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(point1),(void**)&point1Val);



		//wlan
		if(strcmp(pointKey,"lo") == 0){
			if(
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point1Val),"in_bytes",strlen("in_bytes")+1,(void**)&point1ValInByte) && IS_STRING == Z_TYPE_PP(point1ValInByte) &&
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point1Val),"out_bytes",strlen("out_bytes")+1,(void**)&point1ValOutByte) && IS_STRING == Z_TYPE_PP(point1ValOutByte)
			){
				//point1Data['lo']['in'] = $point1Data['lo']['in'] + $l['in_bytes']
				ulong inByte,outByte;
				inByte = strtol(Z_STRVAL_PP(point1ValInByte),NULL,10);
				outByte = strtol(Z_STRVAL_PP(point1ValOutByte),NULL,10);
				point1loIn = point1loIn + inByte;
				point1loOut = point1loOut + outByte;
			}
		}else{
			if(
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point1Val),"in_bytes",strlen("in_bytes")+1,(void**)&point1ValInByte) && IS_STRING == Z_TYPE_PP(point1ValInByte) &&
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point1Val),"out_bytes",strlen("out_bytes")+1,(void**)&point1ValOutByte) && IS_STRING == Z_TYPE_PP(point1ValOutByte)
			){
				//$point1Data['out']['in'] = $point1Data['out']['in'] + $l['in_bytes'];
				ulong inByte,outByte;
				inByte = strtol(Z_STRVAL_PP(point1ValInByte),NULL,10);
				outByte = strtol(Z_STRVAL_PP(point1ValOutByte),NULL,10);
				point1outIn = point1outIn + inByte;
				point1outOut = point1outOut + outByte;
			}
		}
		zend_hash_move_forward(Z_ARRVAL_P(point1));
	}


	//point2 data
	h = zend_hash_num_elements(Z_ARRVAL_P(point2));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(point2),&pointKey,&pointUkey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(point2),(void**)&point2Val);

		//wlan
		if(strcmp(pointKey,"lo") == 0){
			if(
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point2Val),"in_bytes",strlen("in_bytes")+1,(void**)&point2ValInByte) && IS_STRING == Z_TYPE_PP(point2ValInByte) &&
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point2Val),"out_bytes",strlen("out_bytes")+1,(void**)&point2ValOutByte) && IS_STRING == Z_TYPE_PP(point2ValOutByte)
			){
				//point1Data['lo']['in'] = $point1Data['lo']['in'] + $l['in_bytes']
				ulong inByte,outByte;
				inByte = strtol(Z_STRVAL_PP(point2ValInByte),NULL,10);
				outByte = strtol(Z_STRVAL_PP(point2ValOutByte),NULL,10);
				point2loIn = point2loIn + inByte;
				point2loOut = point2loOut + outByte;
			}
		}else{
			if(
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point2Val),"in_bytes",strlen("in_bytes")+1,(void**)&point2ValInByte) && IS_STRING == Z_TYPE_PP(point2ValInByte) &&
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(point2Val),"out_bytes",strlen("out_bytes")+1,(void**)&point2ValOutByte) && IS_STRING == Z_TYPE_PP(point2ValOutByte)
			){
				//$point1Data['out']['in'] = $point1Data['out']['in'] + $l['in_bytes'];
				ulong inByte,outByte;
				inByte = strtol(Z_STRVAL_PP(point2ValInByte),NULL,10);
				outByte = strtol(Z_STRVAL_PP(point2ValOutByte),NULL,10);
				point2outIn = point2outIn + inByte;
				point2outOut = point2outOut + outByte;
			}
		}
		zend_hash_move_forward(Z_ARRVAL_P(point2));
	}

	loInRate = ((double)(point2loIn - point1loIn))/1024;
	loOutRate = ((double)(point2loOut - point1loOut))/1024;
	outInRate = ((double)(point2outIn - point1outIn))/1024;
	outOutRate = ((double)(point2outOut - point1outOut))/1024;

	spprintf(in,0,"%.2f%s%.2f",loInRate,",",loOutRate);
	spprintf(out,0,"%.2f%s%.2f",outInRate,",",outOutRate);

	zval_ptr_dtor(&point1);
	zval_ptr_dtor(&point2);

}

void CGuardController_getDiskUsed(char **data,char **system TSRMLS_DC){

	zval	*diskInfo,
			**thisVal,
			**mountTo,
			**used;

	int		i,h;

	char	*dataString = NULL,
			*systemString = NULL;

	CMonitor_getDisk(&diskInfo TSRMLS_CC);

	h = zend_hash_num_elements(Z_ARRVAL_P(diskInfo));
	for(i = 0 ; i < h ; i ++){

		zend_hash_get_current_data(Z_ARRVAL_P(diskInfo),(void**)&thisVal);

		// mount to /
		if(IS_ARRAY != Z_TYPE_PP(thisVal)){
			zend_hash_move_forward(Z_ARRVAL_P(diskInfo));
			continue;
		}
		
		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"mounted",strlen("mounted")+1,(void**)&mountTo) && IS_STRING == Z_TYPE_PP(mountTo) ){
			
			//check is moun to / or /data
			if(strcmp(Z_STRVAL_PP(mountTo),"/") == 0){
				
				if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"useRate",strlen("useRate")+1,(void**)&used) && IS_STRING == Z_TYPE_PP(used)){
					systemString = estrdup(Z_STRVAL_PP(used));
				}
			}
			if(strcmp(Z_STRVAL_PP(mountTo),"/data") == 0){

				if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"useRate",strlen("useRate")+1,(void**)&used) && IS_STRING == Z_TYPE_PP(used)){
					dataString = estrdup(Z_STRVAL_PP(used));
				}
			}
		}

		zend_hash_move_forward(Z_ARRVAL_P(diskInfo));
	}

	if(dataString == NULL){
		dataString = estrdup("0%");
	}
	if(systemString == NULL){
		systemString = estrdup("0%");
	}

	str_replace("%","",dataString,data);
	str_replace("%","",systemString,system);

	zval_ptr_dtor(&diskInfo);
	efree(dataString);
	efree(systemString);
}

void CGuardController_getProcess(char **all,char **block,char **run TSRMLS_DC){

	zval	*info,
			**node1,
			**node2;

	int		allNum = 0,
			blockNum = 0,
			runNum = 0;

	CMonitor_getProcess(&info TSRMLS_CC);

	//total
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"total",strlen("total")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2)
	){
		allNum = toInt(Z_STRVAL_PP(node2));
	}

	//block
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"zombie",strlen("zombie")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2)
	){
		blockNum = toInt(Z_STRVAL_PP(node2));
	}

	//runing
	if(
		SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"running",strlen("running")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
		SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2)
	){
		runNum = toInt(Z_STRVAL_PP(node2));
	}

	spprintf(all,0,"%d",allNum);
	spprintf(block,0,"%d",blockNum);
	spprintf(run,0,"%d",runNum);

	zval_ptr_dtor(&info);
}

void CGuardController_getDBStatus(zval *saveLogs TSRMLS_DC){

	zval	*fullProcessZval,
			*qps1,
			*tps1CommitZval,
			*tps1RollbackZval,
			*qps2,
			*tps2CommitZval,
			*tps2RollbackZval,
			**firstNode,
			**firstVal,
			*dbLockZval;

	long	qps1Num = 0,
			tps1Commit = 0,
			tps1Rollback = 0,
			qps2Num = 0,
			tps2Commit = 0,
			tps2Rollback = 0;

	//process num
	CActiveRecord_getSQL("show full processlist",&fullProcessZval TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(fullProcessZval)){
		int pnum;
		pnum = zend_hash_num_elements(Z_ARRVAL_P(fullProcessZval));
		add_assoc_long(saveLogs,"dbConnection",pnum);
	}
	zval_ptr_dtor(&fullProcessZval);

	//slow query num
	MODULE_BEGIN
		zval	*timenow,
				*slowArray,
				**node1;
		int		timeLastSec;
		char	slowSql[1024],
				*dateString;
		microtime(&timenow);
		timeLastSec = (int)(Z_DVAL_P(timenow));
		timeLastSec = timeLastSec - 60;
		php_date_ex("Y-m-d H:i:s",timeLastSec,&dateString);
		sprintf(slowSql,"%s%s%s","select count(*) as num from mysql.slow_log where start_time >= '",dateString,"'");
		CActiveRecord_getSQL(slowSql,&slowArray TSRMLS_CC);
		if(IS_ARRAY == Z_TYPE_P(slowArray)){
			int		slownum = 0;
			zval	**numZval;
			if(
				SUCCESS == zend_hash_index_find(Z_ARRVAL_P(slowArray),0,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
				SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"num",strlen("num")+1,(void**)&numZval) && IS_STRING == Z_TYPE_PP(numZval) 
			){
				slownum = toInt(Z_STRVAL_PP(numZval));
			}
			add_assoc_long(saveLogs,"dbSlowQuery",slownum);
		}
		zval_ptr_dtor(&timenow);
		zval_ptr_dtor(&slowArray);
		efree(dateString);
	MODULE_END

	//qps first get
	CActiveRecord_getSQL("show global status like 'questions'",&qps1 TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(qps1) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(qps1),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		qps1Num = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&qps1);


	//tps commit
	CActiveRecord_getSQL("show global status like 'com_commit'",&tps1CommitZval TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(tps1CommitZval) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(tps1CommitZval),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		tps1Commit = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&tps1CommitZval);

	//tps rollback
	CActiveRecord_getSQL("show global status like 'com_rollback'",&tps1RollbackZval TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(tps1RollbackZval) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(tps1RollbackZval),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		tps1Rollback = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&tps1RollbackZval);
	

	php_sleep(1);

	//qps sec get
	CActiveRecord_getSQL("show global status like 'questions'",&qps2 TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(qps2) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(qps2),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		qps2Num = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&qps2);


	//tps commit sec
	CActiveRecord_getSQL("show global status like 'com_commit'",&tps2CommitZval TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(tps2CommitZval) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(tps2CommitZval),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		tps2Commit = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&tps2CommitZval);

	//tps rollback sec
	CActiveRecord_getSQL("show global status like 'com_rollback'",&tps2RollbackZval TSRMLS_CC);
	if(
		IS_ARRAY == Z_TYPE_P(tps2RollbackZval) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(tps2RollbackZval),0,(void**)&firstNode) && IS_ARRAY == Z_TYPE_PP(firstNode) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_PP(firstNode),"Value",strlen("Value")+1,(void**)&firstVal) && IS_STRING == Z_TYPE_PP(firstVal)
	){
		tps2Rollback = strtol(Z_STRVAL_PP(firstVal),NULL,10);
	}
	zval_ptr_dtor(&tps2RollbackZval);

	MODULE_BEGIN
		long	dbQps,
				dbTps;
		dbQps = qps2Num - qps1Num;
		add_assoc_long(saveLogs,"dbQps",dbQps);
		dbTps = (tps2Rollback+tps2Commit) - (tps1Rollback+tps1Commit);
		add_assoc_long(saveLogs,"dbTps",dbTps);
	MODULE_END


	//get dblock
	MODULE_BEGIN
		long	Table_locks_waited = 0,
				Table_locks_immediate = 0;
		int		di,dh;
		zval	**thisWaitVal,
				**Variable_name,
				**nodeVal;
		char	dbLockString[128];
		double	lockRate;

		CActiveRecord_getSQL("show global status like '%lock%'",&dbLockZval TSRMLS_CC);
		dh = zend_hash_num_elements(Z_ARRVAL_P(dbLockZval));
		for(di = 0 ; di < dh ; di++){
			zend_hash_get_current_data(Z_ARRVAL_P(dbLockZval),(void**)&thisWaitVal);
			if(IS_ARRAY != Z_TYPE_PP(thisWaitVal)){
				zend_hash_move_forward(Z_ARRVAL_P(dbLockZval));
				continue;
			}

			//Variable_name
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisWaitVal),"Variable_name",strlen("Variable_name")+1,(void**)&Variable_name) && IS_STRING == Z_TYPE_PP(Variable_name)){
				if(strcmp(Z_STRVAL_PP(Variable_name),"Table_locks_waited") == 0){	
					if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisWaitVal),"Value",strlen("Value")+1,(void**)&nodeVal) && IS_STRING == Z_TYPE_PP(nodeVal)){
						Table_locks_waited = strtol(Z_STRVAL_PP(nodeVal),NULL,10);
					}
				}
				if(strcmp(Z_STRVAL_PP(Variable_name),"Table_locks_immediate") == 0){	
					if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisWaitVal),"Value",strlen("Value")+1,(void**)&nodeVal) && IS_STRING == Z_TYPE_PP(nodeVal)){
						Table_locks_immediate = strtol(Z_STRVAL_PP(nodeVal),NULL,10);
					}
				}
			}

			zend_hash_move_forward(Z_ARRVAL_P(dbLockZval));
		}

		//$returnData['dbLock'] = round($Table_locks_waited/$Table_locks_immediate*100,4);
		lockRate = (double)Table_locks_waited/(double)Table_locks_immediate*100;
		sprintf(dbLockString,"%.2f",lockRate);
		add_assoc_string(saveLogs,"dbLock",dbLockString,1);
		zval_ptr_dtor(&dbLockZval);
	MODULE_END
}

void CGuardController_getCacheStatus(zval *saveLogs TSRMLS_DC){

	zval	*fristInfo,
			*callParams,
			*redisBack,
			*redisBack2;

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	CRedis_callFunction("info",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);

	//get cache hit
	MODULE_BEGIN
		zval	**keyspace_misses,
				**keyspace_hits,
				**connected_clients,
				**used_memory_rss,
				**db0;
		long	miss = 0,
				hit = 0;
		char	hitRateString[20];
		double	hitRate = 0.0;

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"keyspace_misses",strlen("keyspace_misses")+1,(void**)&keyspace_misses) && IS_LONG == Z_TYPE_PP(keyspace_misses) ){
			miss = Z_LVAL_PP(keyspace_misses);
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"keyspace_hits",strlen("keyspace_hits")+1,(void**)&keyspace_hits) && IS_LONG == Z_TYPE_PP(keyspace_hits) ){
			hit = Z_LVAL_PP(keyspace_hits);
		}
		hitRate = (double)miss / ((double)miss + (double)hit)*100;
		sprintf(hitRateString,"%.2f",hitRate);
		add_assoc_string(saveLogs,"cacheHit",hitRateString,1);

		//cacheConnection
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"connected_clients",strlen("connected_clients")+1,(void**)&connected_clients) && IS_LONG == Z_TYPE_PP(connected_clients) ){
			add_assoc_long(saveLogs,"cacheConnection",Z_LVAL_PP(connected_clients));
		}

		//memoryuse
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"used_memory_rss",strlen("used_memory_rss")+1,(void**)&used_memory_rss) && IS_LONG == Z_TYPE_PP(used_memory_rss) ){
			double	memUse;
			char	memUseString[40];
			memUse = Z_LVAL_PP(used_memory_rss)/1024;
			sprintf(memUseString,"%.0f",memUse);
			add_assoc_string(saveLogs,"cacheMemUsed",memUseString,1);
		}

		//unexpirekey
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"db0",strlen("db0")+1,(void**)&db0) && IS_STRING == Z_TYPE_PP(db0) ){
			zval	*expireKeys,
					*keysParse,
					**keysVal,
					**nodeKeys,
					**nodeVal;
			int		ki,kh,
					keyLen = 0,expireLen = 0,unExpireLen = 0;
			php_explode(",",Z_STRVAL_PP(db0),&expireKeys);
			kh = zend_hash_num_elements(Z_ARRVAL_P(expireKeys));
			for(ki = 0 ; ki < kh ; ki++){
				zend_hash_get_current_data(Z_ARRVAL_P(expireKeys),(void**)&keysVal);
				if(IS_STRING != Z_TYPE_PP(keysVal)){
					zend_hash_move_forward(Z_ARRVAL_P(expireKeys));
					continue;
				}
				php_explode("=",Z_STRVAL_PP(keysVal),&keysParse);
				if(
					IS_ARRAY == Z_TYPE_P(keysParse) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(keysParse),0,(void**)&nodeKeys) && IS_STRING == Z_TYPE_PP(nodeKeys) && 
					SUCCESS == zend_hash_index_find(Z_ARRVAL_P(keysParse),1,(void**)&nodeVal) && IS_STRING == Z_TYPE_PP(nodeVal)
				){
					if(strcmp(Z_STRVAL_PP(nodeKeys),"keys") == 0){
						keyLen = toInt(Z_STRVAL_PP(nodeVal));
					}
					if(strcmp(Z_STRVAL_PP(nodeKeys),"expires") == 0){
						expireLen = toInt(Z_STRVAL_PP(nodeVal));
					}
				}
				zval_ptr_dtor(&keysParse);
				zend_hash_move_forward(Z_ARRVAL_P(expireKeys));
			}
			if(keyLen - expireLen >= 0){
				add_assoc_long(saveLogs,"cacheForverKeyNum",keyLen - expireLen);
			}
			zval_ptr_dtor(&expireKeys);
		}
	MODULE_END

	//sleep 1 sec to get sec point
	php_sleep(1);

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	CRedis_callFunction("info",callParams,&redisBack2 TSRMLS_CC);
	zval_ptr_dtor(&callParams);

	//cache QPS
	MODULE_BEGIN
		zval	**keyspace_hits1,
				**keyspace_misses1,
				**keyspace_hits2,
				**keyspace_misses2;
		long	hit1 = 0,
				hit2 = 0,
				miss1 = 0,
				miss2 = 0,
				qpsRate;

		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"keyspace_hits",strlen("keyspace_hits")+1,(void**)&keyspace_hits1) && IS_LONG == Z_TYPE_PP(keyspace_hits1) ){
			hit1 = Z_LVAL_PP(keyspace_hits1);
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack2),"keyspace_hits",strlen("keyspace_hits")+1,(void**)&keyspace_hits2) && IS_LONG == Z_TYPE_PP(keyspace_hits2) ){
			hit2 = Z_LVAL_PP(keyspace_hits2);
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack),"keyspace_misses",strlen("keyspace_misses")+1,(void**)&keyspace_misses1) && IS_LONG == Z_TYPE_PP(keyspace_misses1) ){
			miss1 = Z_LVAL_PP(keyspace_misses1);
		}
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(redisBack2),"keyspace_misses",strlen("keyspace_misses")+1,(void**)&keyspace_misses2) && IS_LONG == Z_TYPE_PP(keyspace_misses2) ){
			miss2 = Z_LVAL_PP(keyspace_misses2);
		}
		qpsRate = (miss2+hit2)-(miss1+hit1);
		add_assoc_long(saveLogs,"cacheQps",qpsRate);
	MODULE_END

	zval_ptr_dtor(&redisBack);
	zval_ptr_dtor(&redisBack2);
}

void CGuardController_getFpmStatus(zval *saveLogs TSRMLS_DC){ 
	char	*fpmLogs,
			*cliLogs,
			*cliNum;
	zval	*returnArray,
			**thisVal;
	int		i,h,fpmNum = 0,cliNumInt = 2;

	//php-fpm num
	exec_shell_return("ps aux|grep php",&fpmLogs);
	php_explode("\n",fpmLogs,&returnArray);
	h = zend_hash_num_elements(Z_ARRVAL_P(returnArray));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(returnArray),(void**)&thisVal);
		if(IS_STRING == Z_TYPE_PP(thisVal) && strstr(Z_STRVAL_PP(thisVal),"php-fpm: pool") != NULL){
			fpmNum++;
		}
		zend_hash_move_forward(Z_ARRVAL_P(returnArray));
	}
	add_assoc_long(saveLogs,"fpmNum",fpmNum);
	efree(fpmLogs);
	zval_ptr_dtor(&returnArray);


	//cli num
	exec_shell_return("ps axu|grep \"\\.php\"|wc -l",&cliLogs);
	php_trim(cliLogs,"\n",&cliNum);
	cliNumInt = toInt(cliNum);
	if(cliNumInt >= 2){
		cliNumInt = cliNumInt - 2;
	}else{
		cliNumInt = 0;
	}
	add_assoc_long(saveLogs,"cliNum",cliNumInt);
	efree(cliLogs);
	efree(cliNum);
}

void CGuardController_getMQStatus(zval *saveLogs TSRMLS_DC){ 

	zval	*rabbitHelper,
			*queueLenZval,
			*overviewData;

	//overview info
	CRabbitHelper_getInstance(&rabbitHelper,"main" TSRMLS_CC);
	CRabbitHelper_callApi_overview(rabbitHelper,&overviewData TSRMLS_CC);

	//get rabbit queueLen	
	MODULE_BEGIN
		zval	**node1,
				**node2;
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(overviewData),"queue_totals",strlen("queue_totals")+1,(void**)&node1) && 
			IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"messages",strlen("messages")+1,(void**)&node2) && 
			IS_LONG == Z_TYPE_PP(node2) 
		){
			add_assoc_long(saveLogs,"queueLen",Z_LVAL_PP(node2));
		}
	MODULE_END

	//unack len
	MODULE_BEGIN
		zval	**node1,
				**node2;
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(overviewData),"queue_totals",strlen("queue_totals")+1,(void**)&node1) && 
			IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"messages_unacknowledged",strlen("messages_unacknowledged")+1,(void**)&node2)
		){
			add_assoc_long(saveLogs,"queueUnAck",Z_LVAL_PP(node2));
		}
	MODULE_END

	//other rate to be a json to save
	MODULE_BEGIN
		zval	**node1,
				**node2,
				**node3,
				*rateList;

		char	*moreRateString;

		MAKE_STD_ZVAL(rateList);
		array_init(rateList);

		//publish rate
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(overviewData),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
			IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"publish_details",strlen("publish_details")+1,(void**)&node2) && 
			IS_ARRAY == Z_TYPE_PP(node2) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
		){
			if(IS_LONG == Z_TYPE_PP(node3)){
				add_assoc_long(rateList,"publishRate",Z_LVAL_PP(node3));
			}else if(IS_DOUBLE == Z_TYPE_PP(node3)){
				int thisInt;
				thisInt = (int)(Z_DVAL_PP(node3));
				add_assoc_long(rateList,"publishRate",thisInt);
			}
		}

		//ack rate
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(overviewData),"message_stats",strlen("message_stats")+1,(void**)&node1) && 
			IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node1),"ack_details",strlen("ack_details")+1,(void**)&node2) && 
			IS_ARRAY == Z_TYPE_PP(node2) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(node2),"rate",strlen("rate")+1,(void**)&node3)
		){
			if(IS_LONG == Z_TYPE_PP(node3)){
				add_assoc_long(rateList,"ackRate",Z_LVAL_PP(node3));
			}else if(IS_DOUBLE == Z_TYPE_PP(node3)){
				int thisInt;
				thisInt = (int)(Z_DVAL_PP(node3));
				add_assoc_long(rateList,"ackRate",thisInt);
			}
		}

		json_encode(rateList,&moreRateString);
		add_assoc_string(saveLogs,"queueRate",moreRateString,1);
		zval_ptr_dtor(&rateList);
		efree(moreRateString);
	MODULE_END


	zval_ptr_dtor(&rabbitHelper);
	zval_ptr_dtor(&overviewData);
}

void CGuardController_createSaveTable(char *dbConfig TSRMLS_DC){

	zval	*cconfigInstanceZval,
			*tablePreZval,
			*createReturn;
	char	dbConfigTablePirfix[80],
			*tablePreFix,
			*createTableSql;

	//get db table prefix
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	sprintf(dbConfigTablePirfix,"%s%s%s","DB.",dbConfig,".master.tablePrefix");
	CConfig_load(dbConfigTablePirfix,cconfigInstanceZval,&tablePreZval TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tablePreZval)){
		tablePreFix = estrdup(Z_STRVAL_P(tablePreZval));
	}else{
		tablePreFix = estrdup("");
	}

	//createTableSq;
	spprintf(&createTableSql,0,"%s%s%s","CREATE TABLE IF NOT EXISTS `",tablePreFix,"monitor` (`id` int(11) unsigned NOT NULL AUTO_INCREMENT,`serverId` varchar(20) NOT NULL DEFAULT '',`date` date NOT NULL DEFAULT '0000-00-00',`createTime` int(11) unsigned NOT NULL DEFAULT '0',`cpuUsed` decimal(6,2) unsigned NOT NULL DEFAULT '0.00',`memoryUsed` decimal(6,2) unsigned NOT NULL DEFAULT '0.00', `swapUsed` decimal(6,2) unsigned NOT NULL, `wlanNetUsed` varchar(120) NOT NULL DEFAULT '0.00,0.00', `netUsed` varchar(120) NOT NULL DEFAULT '0.00,0.00', `diskDataUsed` decimal(4,2) unsigned NOT NULL DEFAULT '0.00', `diskSystemUsed` decimal(4,2) unsigned NOT NULL, `processNum` int(11) unsigned NOT NULL,  `blockProcessNum` int(11) unsigned NOT NULL,  `runProcessNum` int(11) unsigned NOT NULL, `tcpTimeWaitNum` int(10) unsigned NOT NULL DEFAULT '0', `loadAverage` varchar(150) NOT NULL DEFAULT '', `dbConnection` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'dbcontent', `dbQps` decimal(10,2) unsigned NOT NULL DEFAULT '0.00' COMMENT 'dbQPS', `dbTps` decimal(10,2) unsigned NOT NULL DEFAULT '0.00' COMMENT 'dbtps', `dbSlowQuery` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'slowquery', `dbSlaveDelay` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'salve delay', `dbLock` decimal(10,4) unsigned NOT NULL DEFAULT '0.0000' COMMENT 'lock wait', `cacheHit` decimal(10,4) unsigned NOT NULL DEFAULT '0.0000' COMMENT 'missrate', `cacheMemUsed` int(11) unsigned NOT NULL DEFAULT '0', `cacheConnection` int(11) unsigned NOT NULL DEFAULT '0', `cacheQps` decimal(10,2) unsigned NOT NULL DEFAULT '0.00', `cacheForverKeyNum` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'unexpirekey', `fpmNum` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'webnum',"\
"`cliNum` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'cli task num', `queueLen` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'mqlen', `queueUnAck` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'mq fail', `queueRate` text, `netCheck1` varchar(30) NOT NULL DEFAULT '' COMMENT 'pinglost', `netCheck2` varchar(30) NOT NULL DEFAULT '' COMMENT 'pinglost',`netCheck3` varchar(30) NOT NULL DEFAULT '' COMMENT 'pinglist',PRIMARY KEY (`id`),KEY `serverId` (`serverId`),KEY `date` (`date`),KEY `createTime` (`createTime`)) ENGINE=MyISAM AUTO_INCREMENT=107 DEFAULT CHARSET=utf8");

	//run sql
	CActiveRecord_getDBSQL(dbConfig,createTableSql,&createReturn TSRMLS_CC);

	zval_ptr_dtor(&createReturn);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&tablePreZval);
	efree(tablePreFix);
	efree(createTableSql);
}

void CGuardController_checkClearLogs(char *dbConfig TSRMLS_DC){
	zval	*cconfigInstanceZval,
			*tablePreZval,
			*createReturn,
			*callParams,
			*redisBack;
	char	dbConfigTablePirfix[80],
			*tablePreFix,
			*createTableSql;
	int		beforeTime;


	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,"monitor",1);
	CRedis_callFunction("get",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);
	if(IS_STRING == Z_TYPE_P(redisBack)){
		zval_ptr_dtor(&redisBack);
		return;
	}
	zval_ptr_dtor(&redisBack);

	//get db table prefix
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	sprintf(dbConfigTablePirfix,"%s%s%s","DB.",dbConfig,".master.tablePrefix");
	CConfig_load(dbConfigTablePirfix,cconfigInstanceZval,&tablePreZval TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tablePreZval)){
		tablePreFix = estrdup(Z_STRVAL_P(tablePreZval));
	}else{
		tablePreFix = estrdup("");
	}

	//createTableSq;
	beforeTime = php_strtotime("-15 days");
	spprintf(&createTableSql,0,"%s%s%s%d","delete from ",tablePreFix,"monitor where createTime <= ",beforeTime);

	//run sql
	CActiveRecord_getDBSQL(dbConfig,createTableSql,&createReturn TSRMLS_CC);

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,"monitor",1);
	add_next_index_string(callParams,"1",1);
	add_next_index_long(callParams,86400);
	CRedis_callFunction("set",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);
	zval_ptr_dtor(&redisBack);


	zval_ptr_dtor(&createReturn);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&tablePreZval);
	efree(tablePreFix);
	efree(createTableSql);
}

void CGuardController_checkWarnMail(zval *logs,zval *object TSRMLS_DC){

	zval	*cconfigInstanceZval,
			*moniterZval,
			**thisVal,
			**nowVal,
			**node1,
			*warnArray,
			*reportData,
			*saveItem,
			*keyNameArray,
			**thisWarnDesc,
			*warnDesc;

	int		i,h,needReport = 0;

	char	*key,
			*warnString,
			*keyNameBase,
			*keyNameJson,
			*thisItemCondition,
			*itemName,
			*itemUnit;
	ulong	ukey;

	//get config
	CConfig_getInstance("watch",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("moniter",cconfigInstanceZval,&moniterZval TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(moniterZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(moniterZval),"warn",strlen("warn")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)){
		warnString = estrdup(Z_STRVAL_PP(node1));
	}else{
		warnString = estrdup("{\"cpuUsed\":90,\"memoryUsed\":99,\"swapUsed\":60,\"wlanNetInUsed\":1,\"wlanNetOutUsed\":1,\"netInUsed\":20,\"netOutUsed\":20,\"diskDataUsed\":90,\"diskSystemUsed\":90,\"processNum\":0,\"blockProcessNum\":0,\"runProcessNum\":0,\"tcpTimeWaitNum\":0,\"loadAverage1\":4,\"loadAverage5\":0,\"loadAverage15\":0,\"dbConnection\":600,\"dbQps\":0,\"dbTps\":0,\"dbSlowQuery\":100,\"dbSlaveDelay\":0,\"dbLock\":0,\"cacheHit\":0,\"cacheMemUsed\":0,\"cacheConnection\":2000,\"cacheQps\":0,\"cacheForverKeyNum\":0,\"fpmNum\":200,\"cliNum\":45,\"queueLen\":2000,\"queueUnAck\":200,\"queuePushRate\":1000,\"queueGetRate\":1000}");
	}
	json_decode(warnString,&warnArray);
	efree(warnString);

	keyNameBase = "eyJjcHVVc2VkIjp7Im5hbWUiOiJDUFXkvb/nlKjnjociLCJ1bml0IjoiJSJ9LCJtZW1vcnlVc2VkIjp7Im5hbWUiOiLlhoXlrZjkvb/nlKjnjociLCJ1bml0IjoiJSJ9LCJzd2FwVXNlZCI6eyJuYW1lIjoic3dhcOS9v+eUqOeOhyIsInVuaXQiOiIlIn0sIndsYW5OZXRJblVzZWQiOnsibmFtZSI6IuWGhee9keW4puWuvSjlhaXnvZEpIiwidW5pdCI6Ik1iIn0sIndsYW5OZXRPdXRVc2VkIjp7Im5hbWUiOiLlhoXnvZHluKblrr0o5Ye6572RKSIsInVuaXQiOiJNYiJ9LCJuZXRJblVzZWQiOnsibmFtZSI6IuWklue9keW4puWuvSjlhaXnvZEpIiwidW5pdCI6Ik1iIn0sIm5ldE91dFVzZWQiOnsibmFtZSI6IuWklue9keW4puWuvSjlh7rnvZEpIiwidW5pdCI6Ik1iIn0sImxvYWRBdmVyYWdlMSI6eyJuYW1lIjoiMW1pbiBsb2FkQXZlcmFnZSIsInVuaXQiOiIifSwibG9hZEF2ZXJhZ2U1Ijp7Im5hbWUiOiI1bWluIGxvYWRBdmVyYWdlIiwidW5pdCI6IiJ9LCJsb2FkQXZlcmFnZTE1Ijp7Im5hbWUiOiIxNW1pbiBsb2FkQXZlcmFnZSIsInVuaXQiOiIifSwiZGlza0RhdGFVc2VkIjp7Im5hbWUiOiLmlbDmja7nm5jkvb/nlKjnjociLCJ1bml0IjoiJSJ9LCJkaXNrU3lzdGVtVXNlZCI6eyJuYW1lIjoi57O757uf55uY5L2/55So546HIiwidW5pdCI6IiUifSwidGNwVGltZVdhaXROdW0iOnsibmFtZSI6IlRDUOi/nuaOpeaVsCIsInVuaXQiOiIifSwicHJvY2Vzc051bSI6eyJuYW1lIjoi5oC76L+b56iL5pWwIiwidW5pdCI6IiJ9LCJydW5Qcm9jZXNzTnVtIjp7Im5hbWUiOiLov5DooYzov5vnqIvmlbAiLCJ1bml0IjoiIn0sImJsb2NrUHJvY2Vzc051bSI6eyJuYW1lIjoi6Zi75aGe6L+b56iL5pWwIiwidW5pdCI6IiJ9LCJkYkNvbm5lY3Rpb24iOnsibmFtZSI6IuaVsOaNruW6k+i/nuaOpeaVsCIsInVuaXQiOiIifSwiZGJRcHMiOnsibmFtZSI6IuaVsOaNruW6k1FQUyIsInVuaXQiOiIifSwiZGJUcHMiOnsibmFtZSI6IuaVsOaN"\
		"ruW6k1RQUyIsInVuaXQiOiIifSwiZGJTbG93UXVlcnkiOnsibmFtZSI6IuaVsOaNruW6k+aFouafpeivouaVsCIsInVuaXQiOiIifSwiZGJTbGF2ZURlbGF5Ijp7Im5hbWUiOiLku47lupPlu7bov58iLCJ1bml0IjoicyJ9LCJkYkxvY2siOnsibmFtZSI6IuaVsOaNruW6k+mUgeihqOetieW+hSIsInVuaXQiOiIlIn0sImNhY2hlQ29ubmVjdGlvbiI6eyJuYW1lIjoi57yT5a2Y57O757uf6L+e5o6l5pWwIiwidW5pdCI6IiJ9LCJjYWNoZUhpdCI6eyJuYW1lIjoi57yT5a2Y57O757ufTWlzcyIsInVuaXQiOiIlIn0sImNhY2hlTWVtVXNlZCI6eyJuYW1lIjoi57yT5a2Y57O757uf5YaF5a2Y5L2/55SoIiwidW5pdCI6Ik1iIn0sImNhY2hlUXBzIjp7Im5hbWUiOiLnvJPlrZjns7vnu59RUFMiLCJ1bml0IjoiIn0sImNhY2hlRm9ydmVyS2V5TnVtIjp7Im5hbWUiOiLnvJPlrZjns7vnu5/msLjkuYVLZXkiLCJ1bml0IjoiIn0sImZwbU51bSI6eyJuYW1lIjoid2Vi6L+b56iL5pWw55uuIiwidW5pdCI6IuS4qiJ9LCJjbGlOdW0iOnsibmFtZSI6IuWQjuWPsOS7u+WKoeaVsOebriIsInVuaXQiOiLkuKoifSwicXVldWVMZW4iOnsibmFtZSI6Iua2iOaBr+mYn+WIl+mVv+W6piIsInVuaXQiOiIifSwicXVldWVVbkFjayI6eyJuYW1lIjoi5aSE55CG5aSx6LSl5raI5oGvIiwidW5pdCI6IiJ9LCJxdWV1ZVB1c2hSYXRlIjp7Im5hbWUiOiLpmJ/liJfnlJ/kuqfpgJ/luqYiLCJ1bml0IjoiIn0sInF1ZXVlR2V0UmF0ZSI6eyJuYW1lIjoi6Zif5YiX5raI6LS56YCf5bqmIiwidW5pdCI6IiJ9fQ==";
	base64Decode(keyNameBase,&keyNameJson);
	json_decode(keyNameJson,&keyNameArray);
	efree(keyNameJson);

	//reportData
	MAKE_STD_ZVAL(reportData);
	array_init(reportData);

	//foreach warn to check
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(warnArray));
	h = zend_hash_num_elements(Z_ARRVAL_P(warnArray));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(warnArray),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(warnArray),&key,&ukey,0);

		if(IS_LONG != Z_TYPE_PP(thisVal)){
			zend_hash_move_forward(Z_ARRVAL_P(warnArray));
			continue;
		}

		//find this thisWarnDesc
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(keyNameArray),key,strlen(key)+1,(void**)&thisWarnDesc) && IS_ARRAY == Z_TYPE_PP(thisWarnDesc)){
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisWarnDesc),"name",strlen("name")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)  ){
				itemName = estrdup(Z_STRVAL_PP(node1));
			}else{
				itemName = estrdup(key);
			}
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisWarnDesc),"unit",strlen("unit")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)  ){
				itemUnit = estrdup(Z_STRVAL_PP(node1));
			}else{
				itemUnit = estrdup("");
			}
		}else{
			itemName = estrdup(key);
			itemUnit = estrdup("");
		}

		MAKE_STD_ZVAL(saveItem);
		array_init(saveItem);

		//set this item yuzhi
		if(Z_LVAL_PP(thisVal) == 0){
			thisItemCondition = estrdup("None");
		}else{
			spprintf(&thisItemCondition,0,"%d",Z_LVAL_PP(thisVal));
		}
		add_assoc_string(saveItem,"condition",thisItemCondition,1);
		add_assoc_string(saveItem,"item",itemName,1);
		add_assoc_string(saveItem,"note","",1);
		add_assoc_long(saveItem,"flag",0);
		add_assoc_string(saveItem,"unit",itemUnit,1);
		add_assoc_long(saveItem,"value",0);



		//cpuUsed memoryUsed swapUsed  dbLock cacheHit
		if(
			strcmp(key,"cpuUsed") == 0 || 
			strcmp(key,"memoryUsed") == 0 || 
			strcmp(key,"swapUsed") == 0 || 
			strcmp(key,"swapUsed") == 0 || 
			strcmp(key,"diskDataUsed") == 0 || 
			strcmp(key,"diskSystemUsed") == 0 || 
			strcmp(key,"processNum") == 0 || 
			strcmp(key,"blockProcessNum") == 0 || 
			strcmp(key,"runProcessNum") == 0 || 
			strcmp(key,"tcpTimeWaitNum") == 0 ||
			strcmp(key,"dbLock") == 0 || 
			strcmp(key,"cacheHit") == 0 
		){
			//get now val
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),key,strlen(key)+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				int	thisCpuUsed = toInt(Z_STRVAL_PP(nowVal));
				add_assoc_string(saveItem,"value",Z_STRVAL_PP(nowVal),1);
				if(thisCpuUsed >= Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
					needReport = 1;
					add_assoc_long(saveItem,"flag",1);
				}
			}
		}

		//dbConnection dbQps dbTps dbSlowQuery cacheConnection cacheQps cacheForverKeyNum fpmNum cliNum queueLen queueUnAck
		if(
			strcmp(key,"dbConnection") == 0 || 
			strcmp(key,"dbQps") == 0 || 
			strcmp(key,"dbTps") == 0 || 
			strcmp(key,"dbSlowQuery") == 0 || 
			strcmp(key,"cacheConnection") == 0 || 
			strcmp(key,"cacheQps") == 0 || 
			strcmp(key,"cacheForverKeyNum") == 0 || 
			strcmp(key,"fpmNum") == 0 || 
			strcmp(key,"cliNum") == 0 || 
			strcmp(key,"queueUnAck") == 0 || 
			strcmp(key,"queueLen") == 0 

		){
			//get now val
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),key,strlen(key)+1,(void**)&nowVal) && IS_LONG == Z_TYPE_PP(nowVal)){
				int	thisUsed = Z_LVAL_PP(nowVal);
				add_assoc_long(saveItem,"value",thisUsed);
				if(thisUsed >= Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
					needReport = 1;
					add_assoc_long(saveItem,"flag",1);
				}
			}
		}

		//wlanNetInUsed
		if(strcmp(key,"wlanNetInUsed") == 0){
			add_assoc_string(saveItem,"unit","Mb",1);
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"wlanNetUsed",strlen("wlanNetUsed")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 2){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),0,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					thisCutValInt = thisCutValInt / 1024;
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
						needReport = 1;
						add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//wlanNetOutUsed
		if(strcmp(key,"wlanNetOutUsed") == 0){
			add_assoc_string(saveItem,"unit","Mb",1);
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"wlanNetUsed",strlen("wlanNetUsed")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 2){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),1,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					thisCutValInt = thisCutValInt / 1024;
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
						needReport = 1;
						add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//netInUsed
		if(strcmp(key,"netInUsed") == 0){
			add_assoc_string(saveItem,"unit","Mb",1);
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"netUsed",strlen("netUsed")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 2){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),0,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					thisCutValInt = thisCutValInt / 1024;
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
							needReport = 1;
							add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//netOutUsed
		if(strcmp(key,"netOutUsed") == 0){
			add_assoc_string(saveItem,"unit","Mb",1);
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"netUsed",strlen("netUsed")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 2){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),1,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					thisCutValInt = thisCutValInt / 1024;
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
							needReport = 1;
							add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//loadAverage1
		if(strcmp(key,"loadAverage1") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"loadAverage",strlen("loadAverage")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 3){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),0,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
							needReport = 1;
							add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//loadAverage5
		if(strcmp(key,"loadAverage1") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"loadAverage",strlen("loadAverage")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 3){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),1,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
							needReport = 1;
							add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//loadAverage15
		if(strcmp(key,"loadAverage1") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"loadAverage",strlen("loadAverage")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*nowValArray;
				php_explode(",",Z_STRVAL_PP(nowVal),&nowValArray);
				if(IS_ARRAY == Z_TYPE_P(nowValArray) && zend_hash_num_elements(Z_ARRVAL_P(nowValArray)) == 3){
					zval **thisCutVal;
					double	thisCutValInt = 0;
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(nowValArray),2,(void**)&thisCutVal) && IS_STRING == Z_TYPE_PP(thisCutVal)){
						thisCutValInt = atof(Z_STRVAL_PP(thisCutVal));
					}
					add_assoc_double(saveItem,"value",thisCutValInt);
					if(thisCutValInt >= (double)Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
							needReport = 1;
							add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&nowValArray);
			}
		}

		//cacheMemUsed
		if(strcmp(key,"cacheMemUsed") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),key,strlen(key)+1,(void**)&nowVal) && IS_LONG == Z_TYPE_PP(nowVal)){
				double	thisUsed;
				thisUsed = ((double)Z_LVAL_PP(nowVal))/1024;
				add_assoc_double(saveItem,"value",thisUsed);
				if((int)thisUsed >= Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
					needReport = 1;
					add_assoc_long(saveItem,"flag",1);
				}
			}
		}

		//queuePushRate
		if(strcmp(key,"queuePushRate") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"queueRate",strlen("queueRate")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*jsonRateArray,
						**thisNode1;
				json_decode(Z_STRVAL_PP(nowVal),&jsonRateArray);
				if(IS_ARRAY == Z_TYPE_P(jsonRateArray) && SUCCESS == zend_hash_find(Z_ARRVAL_P(jsonRateArray),"publishRate",strlen("publishRate")+1,(void**)&thisNode1) && IS_LONG == Z_TYPE_PP(thisNode1)){
					add_assoc_long(saveItem,"value",Z_LVAL_PP(thisNode1));
					if(Z_LVAL_PP(thisNode1) >= Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
						needReport = 1;
						add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&jsonRateArray);
			}
		}

		//queueGetRate
		if(strcmp(key,"queueGetRate") == 0){
			if(IS_ARRAY == Z_TYPE_P(logs) && SUCCESS == zend_hash_find(Z_ARRVAL_P(logs),"queueRate",strlen("queueRate")+1,(void**)&nowVal) && IS_STRING == Z_TYPE_PP(nowVal)){
				zval	*jsonRateArray,
						**thisNode1;
				json_decode(Z_STRVAL_PP(nowVal),&jsonRateArray);
				if(IS_ARRAY == Z_TYPE_P(jsonRateArray) && SUCCESS == zend_hash_find(Z_ARRVAL_P(jsonRateArray),"ackRate",strlen("ackRate")+1,(void**)&thisNode1) && IS_LONG == Z_TYPE_PP(thisNode1)){
					add_assoc_long(saveItem,"value",Z_LVAL_PP(thisNode1));
					if(Z_LVAL_PP(thisNode1) >= Z_LVAL_PP(thisVal) && Z_LVAL_PP(thisVal) > 0){
						needReport = 1;
						add_assoc_long(saveItem,"flag",1);
					}
				}
				zval_ptr_dtor(&jsonRateArray);
			}
		}

		//insert report
		add_next_index_zval(reportData,saveItem);
		
		//destroy
		efree(itemName);
		efree(itemUnit);
		efree(thisItemCondition);
	
		zend_hash_move_forward(Z_ARRVAL_P(warnArray));
	}

	//need to send warn mail
	if(needReport == 1 && IS_ARRAY == Z_TYPE_P(moniterZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(moniterZval),"sendMail",strlen("sendMail")+1,(void**)&node1) && IS_BOOL == Z_TYPE_PP(node1) && 1 == Z_LVAL_PP(node1) ){

		//mail content
		zval	*mailContent,
				*warnItem,
				*warnItemList,
				*htmlContent,
				*saveToList,
				*notifyZval,
				*toList,
				**delayZval,
				*callParams,
				*redisBack;

		char	*httpInterCheck,
				*nohttp200,
				*nowWarnVal,
				*hostIdString,
				itemTrueName[1024],
				*thisMothTime,
				*templatePath;

		int		delay = 300;

		//notify
		CConfig_load("notify",cconfigInstanceZval,&notifyZval TSRMLS_CC);

		//check delay
		if(IS_ARRAY == Z_TYPE_P(notifyZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyZval),"delay",strlen("delay")+1,(void**)&delayZval) && IS_LONG == Z_TYPE_PP(delayZval)){
			delay = 300;
		}

		//checkRedis
		MAKE_STD_ZVAL(callParams);
		array_init(callParams);
		add_next_index_string(callParams,"sendMonitorWarnMail",1);
		CRedis_callFunction("get",callParams,&redisBack TSRMLS_CC);
		zval_ptr_dtor(&callParams);

		if(IS_STRING == Z_TYPE_P(redisBack)){
			//destroy
			zval_ptr_dtor(&redisBack);
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&moniterZval);
			zval_ptr_dtor(&keyNameArray);
			zval_ptr_dtor(&warnArray);
			zval_ptr_dtor(&notifyZval);
			zval_ptr_dtor(&reportData);
			return;
		}
		zval_ptr_dtor(&redisBack);

		MAKE_STD_ZVAL(toList);
		if(IS_ARRAY == Z_TYPE_P(notifyZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyZval),"to",strlen("to")+1,(void**)&node1) ){
			if(IS_STRING == Z_TYPE_PP(node1)){
				array_init(toList);
				add_next_index_string(toList,Z_STRVAL_PP(node1),1);
			}else if(IS_ARRAY == Z_TYPE_PP(node1)){
				ZVAL_ZVAL(toList,*node1,1,0);
			}else{
				array_init(toList);
			}
		}else{
			array_init(toList);
		}

		//tempplate
		if(IS_ARRAY == Z_TYPE_P(notifyZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(notifyZval),"template",strlen("template")+1,(void**)&node1) && IS_STRING == Z_TYPE_PP(node1)){
			templatePath = estrdup(Z_STRVAL_PP(node1));
		}else{
			templatePath = estrdup("");
		}

		getHostName(&hostIdString);
		MAKE_STD_ZVAL(mailContent);
		array_init(mailContent);

		php_date("Y-m-d H:i:s",&thisMothTime);
		add_assoc_string(mailContent,"mailTye","serverLoadCheck",1);
		add_assoc_string(mailContent,"mailDate",thisMothTime,1);
		add_assoc_string(mailContent,"mailSender","CGuardController/monitor",1);
		add_assoc_string(mailContent,"hostId",hostIdString,1);
		add_assoc_zval(mailContent,"item",reportData);
		add_assoc_zval(mailContent,"to",toList);
		
		//call assign
		MODULE_BEGIN
			zval	callFunction,
					returnFunction,
					*paramsList[2];
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],"data",1);
			MAKE_STD_ZVAL(paramsList[1]);
			ZVAL_ZVAL(paramsList[1],mailContent,1,0);
			INIT_ZVAL(callFunction);
			ZVAL_STRING(&callFunction, "assign", 0);
			call_user_function(NULL, &object, &callFunction, &returnFunction, 2, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_ptr_dtor(&paramsList[1]);
			zval_dtor(&returnFunction);
		MODULE_END

		//get a view object
		MODULE_BEGIN
			zval	callFunction,
					returnFunction,
					*paramsList[1],
					*saveHtml;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],templatePath,1);
			INIT_ZVAL(callFunction);
			ZVAL_STRING(&callFunction, "fetch", 0);
			call_user_function(NULL, &object, &callFunction, &returnFunction, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			if(IS_STRING == Z_TYPE(returnFunction)){
				add_assoc_string(mailContent,"html",Z_STRVAL(returnFunction),1);
			}
			zval_dtor(&returnFunction);
		MODULE_END

		//callHooks
		MODULE_BEGIN
			zval	*paramsList[1],
					*dataObject,
					*hooksUseData,
					**endSendHtml;
			MAKE_STD_ZVAL(dataObject);
			object_init_ex(dataObject,CDataObjectCe);
			CHooks_setDataObject(dataObject,mailContent TSRMLS_CC);
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],dataObject,1,0);
			CHooks_callHooks("HOOKS_MAIL_BEFORE",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);

			//check return data's type has right
			CHooks_getDataObject(dataObject,&hooksUseData TSRMLS_CC);

			if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"html",strlen("html")+1,(void**)&endSendHtml) && IS_STRING == Z_TYPE_PP(endSendHtml)){
				//send mail
				zval	**newToList,
						*endToList;
				MAKE_STD_ZVAL(endToList);
				if(IS_ARRAY == Z_TYPE_P(hooksUseData) && SUCCESS == zend_hash_find(Z_ARRVAL_P(hooksUseData),"to",strlen("to")+1,(void**)&newToList) && IS_ARRAY == Z_TYPE_PP(newToList)){
					ZVAL_ZVAL(endToList,*newToList,1,0);
				}else{
					ZVAL_ZVAL(endToList,toList,1,0);
				}
				CGuardController_sendMail(endToList,Z_STRVAL_PP(endSendHtml) TSRMLS_CC);
				zval_ptr_dtor(&endToList);

				MAKE_STD_ZVAL(callParams);
				array_init(callParams);
				add_next_index_string(callParams,"sendMonitorWarnMail",1);
				add_next_index_string(callParams,"1",1);
				add_next_index_long(callParams,delay);
				CRedis_callFunction("set",callParams,&redisBack TSRMLS_CC);
				zval_ptr_dtor(&callParams);
				zval_ptr_dtor(&redisBack);

			}

			zval_ptr_dtor(&dataObject);
			zval_ptr_dtor(&hooksUseData);
		MODULE_END

		efree(hostIdString);
		efree(thisMothTime);
		zval_ptr_dtor(&mailContent);
		zval_ptr_dtor(&notifyZval);
		efree(templatePath);
	}else{
		zval_ptr_dtor(&reportData);
	}


	//destroy
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&moniterZval);
	zval_ptr_dtor(&keyNameArray);
	zval_ptr_dtor(&warnArray);
}

PHP_METHOD(CGuardController,Action_monitor)
{

	zval	*cconfigInstanceZval,
			*sitehostName,
			*saveLogs;

	char	*hostname;

	int		saveStatus = 0;


	//get host name
	CConfig_getInstance("site",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("serverId",cconfigInstanceZval,&sitehostName TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(sitehostName)){
		hostname = estrdup(Z_STRVAL_P(sitehostName));
	}else{
		getHostName(&hostname);
	}
	zval_ptr_dtor(&sitehostName);

	//init saveLogs
	MAKE_STD_ZVAL(saveLogs);
	array_init(saveLogs);

	//serverId
	add_assoc_string(saveLogs,"serverId",hostname,1);
	efree(hostname);

	//cpu info
	MODULE_BEGIN
		zval	*cpuInfo,
				**node1,
				**node2;

		char	*idleString;
		double	cpuUsedInt;

		CMonitor_getCPU(&cpuInfo,2 TSRMLS_CC);

		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(cpuInfo),"id",strlen("id")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),1,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2) 
		){
			double  idleInt;
			char	cpuUseString[20];
			str_replace("%","",Z_STRVAL_PP(node2),&idleString);
			idleInt = atof(idleString);
			cpuUsedInt = 100.00 - idleInt;
			sprintf(cpuUseString,"%.2f",cpuUsedInt);
			add_assoc_string(saveLogs,"cpuUsed",cpuUseString,1);
			efree(idleString);
		}
		zval_ptr_dtor(&cpuInfo);
	MODULE_END


	//memoryInfo
	MODULE_BEGIN
		zval	*info,
				**node1,
				**node2,
				**node3,
				**node4;

		CMonitor_getMemory(&info,1 TSRMLS_CC);

		//used total
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"used",strlen("used")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"total",strlen("total")+1,(void**)&node3) && IS_ARRAY == Z_TYPE_PP(node3) && 
			SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node3),0,(void**)&node4) && IS_STRING == Z_TYPE_PP(node4)
		){
			int used = 0 , total = 0;
			double useRate = 0.0;
			char	useRateString[20];
			used = toInt(Z_STRVAL_PP(node2));
			total = toInt(Z_STRVAL_PP(node4));
			useRate = (double)((double)used / (double)total*100);
			sprintf(useRateString,"%.2f",useRate);
			add_assoc_string(saveLogs,"memoryUsed",useRateString,1);
		}
		zval_ptr_dtor(&info);
	MODULE_END

	//swap
	MODULE_BEGIN
		zval	*info,
				**node1,
				**node2,
				**node3,
				**node4;

		CMonitor_getSwap(&info TSRMLS_CC);

		//used total
		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"used",strlen("used")+1,(void**)&node1) && IS_ARRAY == Z_TYPE_PP(node1) && 
			SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node1),0,(void**)&node2) && IS_STRING == Z_TYPE_PP(node2) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_P(info),"total",strlen("total")+1,(void**)&node3) && IS_ARRAY == Z_TYPE_PP(node3) && 
			SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(node3),0,(void**)&node4) && IS_STRING == Z_TYPE_PP(node4)
		){
			int used = 0 , total = 0;
			double useRate = 0.0;
			char	useRateString[20];
			used = toInt(Z_STRVAL_PP(node2));
			total = toInt(Z_STRVAL_PP(node4));
			useRate = (double)((double)used / (double)total*100);
			sprintf(useRateString,"%.2f",useRate);
			add_assoc_string(saveLogs,"swapUsed",useRateString,1);
		}
		zval_ptr_dtor(&info);
	MODULE_END

	//getNetUsed
	MODULE_BEGIN
		char	*in,
				*out;
		CGuardController_getNetUsed(&in,&out TSRMLS_CC);
		add_assoc_string(saveLogs,"wlanNetUsed",in,1);
		add_assoc_string(saveLogs,"netUsed",out,1);
		efree(in);
		efree(out);
	MODULE_END

	//getdisk
	MODULE_BEGIN
		char	*data,
				*system;
		CGuardController_getDiskUsed(&data,&system TSRMLS_CC);
		add_assoc_string(saveLogs,"diskDataUsed",data,1);
		add_assoc_string(saveLogs,"diskSystemUsed",system,1);
		efree(data);
		efree(system);
	MODULE_END

	//getProcess
	MODULE_BEGIN
		char	*all,
				*block,
				*run;
		CGuardController_getProcess(&all,&block,&run TSRMLS_CC);
		add_assoc_string(saveLogs,"processNum",all,1);
		add_assoc_string(saveLogs,"blockProcessNum",block,1);
		add_assoc_string(saveLogs,"runProcessNum",run,1);
		efree(all);
		efree(block);
		efree(run);
	MODULE_END

	//get tcp wait_time
	MODULE_BEGIN
		zval	*tcpinfo,
				**waitTime;
		CMonitor_getNetConnection(&tcpinfo TSRMLS_CC);
		if(zend_hash_find(Z_ARRVAL_P(tcpinfo),"TIME_WAIT",strlen("TIME_WAIT")+1,(void**)&waitTime) && IS_STRING == Z_TYPE_PP(waitTime) ){
			add_assoc_string(saveLogs,"tcpTimeWaitNum",Z_STRVAL_PP(waitTime),1);
		}
		zval_ptr_dtor(&tcpinfo);
	MODULE_END

	//loadverage
	MODULE_BEGIN
		zval	*loadAverage,
				*loadAllAverage;
		CMonitor_getLoadAverage(&loadAverage TSRMLS_CC);
		php_implode(",",loadAverage,&loadAllAverage);
		if(IS_STRING == Z_TYPE_P(loadAllAverage)){
			add_assoc_string(saveLogs,"loadAverage",Z_STRVAL_P(loadAllAverage),1);
		}
		zval_ptr_dtor(&loadAverage);
		zval_ptr_dtor(&loadAllAverage);
	MODULE_END

	//create Time
	MODULE_BEGIN
		zval	*createTime;
		char	*createDate;
		microtime(&createTime);	
		php_date("Y-m-d",&createDate);
		add_assoc_long(saveLogs,"createTime",(int)Z_DVAL_P(createTime));
		add_assoc_string(saveLogs,"date",createDate,1);
		zval_ptr_dtor(&createTime);
		efree(createDate);
	MODULE_END


	//DB info
	CGuardController_getDBStatus(saveLogs TSRMLS_CC);

	//cache info
	CGuardController_getCacheStatus(saveLogs TSRMLS_CC);

	//php fpm info
	CGuardController_getFpmStatus(saveLogs TSRMLS_CC);

	//mq info
	CGuardController_getMQStatus(saveLogs TSRMLS_CC);

	//call HOOKS_MONITOR_END 
	MODULE_BEGIN
		zval	*paramsList[1],
				*dataObject,
				*hooksUseData;
		MAKE_STD_ZVAL(dataObject);
		object_init_ex(dataObject,CDataObjectCe);
		CHooks_setDataObject(dataObject,saveLogs TSRMLS_CC);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],dataObject,1,0);
		CHooks_callHooks("HOOKS_MONITOR_END",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);

		//get data
		CHooks_getDataObject(dataObject,&hooksUseData TSRMLS_CC);
		zval_ptr_dtor(&saveLogs);
		MAKE_STD_ZVAL(saveLogs);
		ZVAL_ZVAL(saveLogs,hooksUseData,1,1);
		zval_ptr_dtor(&dataObject);
	MODULE_END

	//insert into
	MODULE_BEGIN
		zval	*exception,
				*watchConfigInstance,
				*monitorConfig,
				**saveKey;

		//get watch config to 
		CConfig_getInstance("watch",&watchConfigInstance TSRMLS_CC);
		CConfig_load("moniter",watchConfigInstance,&monitorConfig TSRMLS_CC);

		//check save key is true
		if(IS_ARRAY == Z_TYPE_P(monitorConfig) && SUCCESS == zend_hash_find(Z_ARRVAL_P(monitorConfig),"save",strlen("save")+1,(void**)&saveKey) && IS_BOOL == Z_TYPE_PP(saveKey) && 1 == Z_LVAL_PP(saveKey)){
			
			zval	**dbNameKey;
			char	*dbName;
			if(IS_ARRAY == Z_TYPE_P(monitorConfig) && zend_hash_find(Z_ARRVAL_P(monitorConfig),"dbName",strlen("dbName")+1,(void**)&dbNameKey) && IS_STRING == Z_TYPE_PP(dbNameKey)){
				dbName = estrdup(Z_STRVAL_PP(dbNameKey));
			}else{
				dbName = estrdup("main");
			}

			saveStatus = CActiveRecord_insert(dbName,"monitor",saveLogs,&exception TSRMLS_CC);
			if(IS_STRING == Z_TYPE_P(exception)){
				if(strstr(Z_STRVAL_P(exception),"doesn't exist") != NULL){
					CGuardController_createSaveTable(dbName TSRMLS_CC);
					zval_ptr_dtor(&exception);
					saveStatus = CActiveRecord_insert(dbName,"monitor",saveLogs,&exception TSRMLS_CC);
				}
			}

			//checkClearLast15days'log
			CGuardController_checkClearLogs(dbName TSRMLS_CC);

			zval_ptr_dtor(&exception);
			efree(dbName);
		}

		zval_ptr_dtor(&watchConfigInstance);
		zval_ptr_dtor(&monitorConfig);
	MODULE_END


	//check to send warn
	CGuardController_checkWarnMail(saveLogs,getThis() TSRMLS_CC);

	//destory
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&saveLogs);
}
