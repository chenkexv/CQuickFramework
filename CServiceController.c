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

#ifndef PHP_WIN32
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"


#include "php_CQuickFramework.h"
#include "php_CServiceController.h"
#include "php_CController.h"
#include "php_CWebApp.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CException.h"
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>


//zend类方法
zend_function_entry CServiceController_functions[] = {
	PHP_ME(CServiceController,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CServiceController,Action_run,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CServiceController,__dispatch,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,run,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(CServiceController,serviceWorker,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,serviceName,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,serviceLogs,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_install,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_serviceRunning,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CServiceController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CServiceController",CServiceController_functions);
	CServiceControllerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);

	zend_declare_property_string(CServiceControllerCe, ZEND_STRL("name"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CServiceControllerCe, ZEND_STRL("logs"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CServiceControllerCe, ZEND_STRL("processNum"),1,ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CServiceControllerCe, ZEND_STRL("pidList"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void writeServiceLog(char *log,zval *object TSRMLS_DC){

	char	*logsPath;
	zval *logs;

	logs = zend_read_property(CServiceControllerCe,object,ZEND_STRL("logs"),0 TSRMLS_CC);
	
	//确定日志存放目录
	if(IS_STRING == Z_TYPE_P(logs) && strlen(Z_STRVAL_P(logs)) > 0){

		CLog_writeFileContent(Z_STRVAL_P(logs),log TSRMLS_CC);

	}else{

		//未指定日志 则直接写入系统日志中
		zval *appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
		char *saveFiles;
		spprintf(&saveFiles,0,"%s/logs/systemlog/CService.log",Z_STRVAL_P(appPath));
		CLog_writeFileContent(saveFiles,log TSRMLS_CC);
		efree(saveFiles);
	}
	

}

PHP_METHOD(CServiceController,__dispatch){

	char	*data;
	int		dataLen = 0 ;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	if(strcmp(data,"Action_serviceRunning") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_serviceRunning",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}if(strcmp(data,"Action_install") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_install",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else{

		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_run",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}
	
}

PHP_METHOD(CServiceController,__construct){

	zval	*sapiZval;

	zend_class_entry *nowClass;

	//check is cli
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] 403 Forbidden ");
		return;
	}

	//set memlimit 
	ini_set("memory_limit","4048M");

	//获取进程数
	MODULE_BEGIN
		zval	callFunction,
				callReturn;

		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"serviceWorker",0);
		call_user_function(NULL, &getThis(), &callFunction, &callReturn, 0, NULL TSRMLS_CC);
		zval	*returnZval;
		returnZval = &callReturn;
		if(IS_LONG == Z_TYPE_P(returnZval) && Z_LVAL_P(returnZval) > 0){
			zend_update_property_long(CServiceControllerCe,getThis(),ZEND_STRL("processNum"),Z_LVAL_P(returnZval) TSRMLS_CC);
		}
		zval_dtor(&callReturn);
	MODULE_END

	//获取服务名
	MODULE_BEGIN
		zval	callFunction,
				callReturn;

		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"serviceName",0);
		call_user_function(NULL, &getThis(), &callFunction, &callReturn, 0, NULL TSRMLS_CC);
		zval	*returnZval;
		returnZval = &callReturn;
		if(IS_STRING == Z_TYPE_P(returnZval) && strlen(Z_STRVAL_P(returnZval)) > 0){
			zend_update_property_string(CServiceControllerCe,getThis(),ZEND_STRL("name"),Z_STRVAL_P(returnZval) TSRMLS_CC);
		}
		zval_dtor(&callReturn);
	MODULE_END

	//获取日志名
	MODULE_BEGIN
		zval	callFunction,
				callReturn;

		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"serviceLogs",0);
		call_user_function(NULL, &getThis(), &callFunction, &callReturn, 0, NULL TSRMLS_CC);
		zval	*returnZval;
		returnZval = &callReturn;
		if(IS_STRING == Z_TYPE_P(returnZval) && strlen(Z_STRVAL_P(returnZval)) > 0){
			zend_update_property_string(CServiceControllerCe,getThis(),ZEND_STRL("logs"),Z_STRVAL_P(returnZval) TSRMLS_CC);
		}
		zval_dtor(&callReturn);
	MODULE_END
}


void checkChildStatusOnce(zval *object TSRMLS_DC){

	//检查子进程数量  不足则重新fork
	zval	*pidList,
			*processNum;
	pidList = zend_read_property(CServiceControllerCe,object,ZEND_STRL("pidList"),1 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(pidList)){
		return;
	}
	processNum = zend_read_property(CServiceControllerCe,object,ZEND_STRL("processNum"),1 TSRMLS_CC);

	int		i,h,processStatus=-1;
	char	*key;
	ulong	ukey;
	h = zend_hash_num_elements(Z_ARRVAL_P(pidList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pidList));
	int		aliveNum = 0 ;
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(pidList),&key,&ukey,0);

		//做存活检查
		processStatus = -1;
		processStatus = kill(ukey,0);
		if(processStatus == 0){
			aliveNum++;
		}else{
			zend_hash_index_del(Z_ARRVAL_P(pidList),ukey);
		}

		zend_hash_move_forward(Z_ARRVAL_P(pidList));
	}

	int needNewFork = Z_LVAL_P(processNum) - aliveNum;
	if(needNewFork <= 0){
		return;
	}
	
	//重新fork
	zend_class_entry *nowClass = Z_OBJCE_P(object);
	for(i = 0 ; i < needNewFork ;i++){

		int forkPid = -1;
		forkPid=fork();
		if(forkPid==-1){

			continue;

		}else if(forkPid == 0){

			//子进程逻辑
			//child process action exec a php request
			zval	**SERVER,
					*appPath,
					**phpPath;
			char	*phpTruePath,
					*runCommand,
					*runParams;
			int		status = 0;

			(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
			if(zend_hash_find(Z_ARRVAL_PP(SERVER),"_",strlen("_")+1,(void**)&phpPath) == SUCCESS && IS_STRING == Z_TYPE_PP(phpPath)){
				phpTruePath = estrdup(Z_STRVAL_PP(phpPath));
			}else{
				phpTruePath = estrdup("php");
			}
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			
			//run command
			spprintf(&runCommand,0,"%s/index.php",Z_STRVAL_P(appPath));
			spprintf(&runParams,0,"%s/serviceRunning",nowClass->name);

			status = execl(phpTruePath, "/usr/bin/php", runCommand,runParams, NULL); 

			efree(runCommand);
			efree(phpTruePath);
			efree(runParams);

		}else{

			add_index_long(pidList,forkPid,1);
		}
	}
}

static zval *serviceObject = NULL;
void catchChildSig(int sig){

	char tips1[1024];
	sprintf(tips1,"[CServiceMaster] [%d-%d] Receive a child process exit signal [%d]\n",getppid(),getpid(),sig);
	writeServiceLog(tips1,serviceObject TSRMLS_CC);

	int endPid = wait(NULL);
	char tips2[1024];
	sprintf(tips2,"[CServiceMaster] The process for determining the unexpected termination is [%d]\n",endPid);
	writeServiceLog(tips2,serviceObject TSRMLS_CC);


	//remove now pid
	zval *pidList = zend_read_property(CServiceControllerCe,serviceObject,ZEND_STRL("pidList"),1 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(pidList)){
		zend_hash_index_del(Z_ARRVAL_P(pidList),endPid);
	}
	checkChildStatusOnce(serviceObject TSRMLS_CC);



}


void handleMaster_signal(int signo){
	if (signo == SIGHUP){  
		zend_bailout();
	}
}


void checkChildStatus(zval *object TSRMLS_DC){

	while(1){

		//检查子进程数量
		checkChildStatusOnce(object TSRMLS_CC);

		//控制频率 5秒检测一次
		usleep(5000000);
	}

}

//按设置启动服务
void CServiceController_start(zval *object TSRMLS_DC){

	zval	*processNum,
			*logs,
			*pidList,
			*serviceName;
	int		forkNum = 1,
			i;
	char	*logsPath;
	zend_class_entry	*nowClass;

	processNum = zend_read_property(CServiceControllerCe,object,ZEND_STRL("processNum"),1 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(processNum) && Z_LVAL_P(processNum) > 1){
		forkNum = Z_LVAL_P(processNum);
	}

	logs = zend_read_property(CServiceControllerCe,object,ZEND_STRL("logs"),1 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(logs) && strlen(Z_STRVAL_P(logs)) > 0){
		logsPath = Z_STRVAL_P(logs);
	}

	pidList = zend_read_property(CServiceControllerCe,object,ZEND_STRL("pidList"),1 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(pidList)){
		zval *saveData;
		MAKE_STD_ZVAL(saveData);
		array_init(saveData);
		zend_update_property(CServiceControllerCe,object,ZEND_STRL("pidList"),saveData TSRMLS_CC);
		zval_ptr_dtor(&saveData);
		pidList = zend_read_property(CServiceControllerCe,object,ZEND_STRL("pidList"),1 TSRMLS_CC);
	}

	//以守护进程运行
	int s = daemon(1, 0);

	//记录当前 pid
	zval *appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	char *pidPath;
	spprintf(&pidPath,0,"%s/logs/pid",Z_STRVAL_P(appPath));
	if(FAILURE == fileExist(pidPath)){
		//尝试创建文件夹
		php_mkdir(pidPath);

		//创建失败
		if(FAILURE == fileExist(pidPath)){
			efree(logsPath);
			char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uQ2VdIOWQr+WKqOWuiOaKpOi/m+eoi++8jOWIm+W7umxvZ3MvcGlk55uu5b2V5aSx6LSl77yM6K+35qOA5p+l56Gs55uY5oiW5p2D6ZmQ";
			char *tipsSay;
			base64Decode(tips,&tipsSay);
			zend_throw_exception(CServiceExceptionCe, tipsSay, 7001 TSRMLS_CC);
			efree(tipsSay);
			efree(pidPath);
			return;
		}

		//如果是cli 将目录所有设为apache:apche
		MODULE_BEGIN
			char    *command,
					*returnString;
			spprintf(&command, 0,"chown apache:apache -R %s", pidPath);
			exec_shell_return(command,&returnString);
			efree(command);
			efree(returnString);
		MODULE_END
	}
	
	//将当前任务的pid写入
	char *pidFileName;
	serviceName = zend_read_property(CServiceControllerCe,object,ZEND_STRL("name"),1 TSRMLS_CC);
	spprintf(&pidFileName,0,"%s/%s.pid",pidPath,Z_STRVAL_P(serviceName));
	char nowpid[64];
	sprintf(nowpid,"%d",getpid());
	file_put_contents(pidFileName,nowpid);
	efree(pidPath);
	efree(pidFileName);


	//读取当前控制器的名称
	nowClass = Z_OBJCE_P(object);
	serviceObject = object;

	char tips[1024];
	sprintf(tips,"%s%d","fork",forkNum);
	writeServiceLog(tips,object TSRMLS_CC);

	//启动进程数
	for(i = 0 ; i < forkNum ;i++){

		php_printf("forked:%d\n",i);

		int forkPid = -1;
		forkPid=fork();
		if(forkPid==-1){

			continue;

		}else if(forkPid == 0){

			//子进程逻辑
			//child process action exec a php request
			zval	**SERVER,
					*appPath,
					**phpPath;
			char	*phpTruePath,
					*runCommand,
					*runParams;
			int		status = 0;

			(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
			if(zend_hash_find(Z_ARRVAL_PP(SERVER),"_",strlen("_")+1,(void**)&phpPath) == SUCCESS && IS_STRING == Z_TYPE_PP(phpPath)){
				phpTruePath = estrdup(Z_STRVAL_PP(phpPath));
			}else{
				phpTruePath = estrdup("php");
			}
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			
			//run command
			spprintf(&runCommand,0,"%s/index.php",Z_STRVAL_P(appPath));
			spprintf(&runParams,0,"%s/serviceRunning",nowClass->name);

			status = execl(phpTruePath, "/usr/bin/php", runCommand,runParams, NULL); 

			efree(runCommand);
			efree(phpTruePath);
			efree(runParams);

		}else{

			//signal(SIGCHLD, SIG_IGN);
			signal(SIGCHLD, catchChildSig);

			add_index_long(pidList,forkPid,1);

			//启动最后一个进程后  主进程进入检测状态
			if(i == forkNum - 1){
				checkChildStatus(object TSRMLS_CC);
			}
		}
	}
}

//服务启动入口文件
PHP_METHOD(CServiceController,Action_run)
{
	
	CServiceController_start(getThis() TSRMLS_CC);

}

PHP_METHOD(CServiceController,run)
{
}

//控制器子类可以覆盖此方法指定需要启动的进程数量
PHP_METHOD(CServiceController,serviceWorker)
{
	RETURN_LONG(1);
}

//控制器子类可以覆盖此方法指定服务器名字
PHP_METHOD(CServiceController,serviceName)
{
	//获当前控制器名字
	zend_class_entry *nowClass;
	nowClass = Z_OBJCE_P(getThis());
	RETURN_STRING(nowClass->name,1);
}

//控制器子类可以覆盖此方法指定log存放路径
PHP_METHOD(CServiceController,serviceLogs)
{
	zval *appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	char	*savePath;
	spprintf(&savePath,0,"%s/logs/systemlog/CService.log",Z_STRVAL_P(appPath));
	RETURN_STRING(savePath,0);
}

void CServiceController_action_install(char *path,char *serviceName,char *controller TSRMLS_DC){

	char *templateString = "IyEvYmluL3NoCi4gL2V0Yy9yYy5kL2luaXQuZC9mdW5jdGlvbnMKbmFtZT0iPHskc2VydmVyTmFtZX0+IgpleGVjPSIvdXNyL2Jpbi9waHAiCmlucHV0UGF0aD0iPHskaHRtbFBhdGh9PiIKY29udHJvbGxlcj0iPHskY29udHJvbGxlcn0+IgpzdGFydCgpIHsKICAgIGVjaG8gLW4gJCJTdGFydGluZyBDUXVpY2tGcmFtZXdvcmsgU2VydmljZSBbJG5hbWVdOiAiCiAgICBkYWVtb24gLS11c2VyIHJvb3QgIiRleGVjICRpbnB1dFBhdGgvaW5kZXgucGhwICRjb250cm9sbGVyL3J1biIKICAgIHJldHZhbD0kPwogICAgZWNobwogICAgcmV0dXJuICRyZXR2YWwKfQpzdG9wKCkgewogICAgZWNobyAtbiAkIlN0b3BwaW5nIENRdWlja0ZyYW1ld29yayBTZXJ2aWNlIFskbmFtZV06ICIKICAgIGtpbGxwcm9jIC1wICIkaW5wdXRQYXRoL2xvZ3MvcGlkLyRuYW1lLnBpZCIgJG5hbWUKICAgIHJldHZhbD0kPwogICAgZWNobwogICAgcmV0dXJuICRyZXR2YWwKfQpyZXN0YXJ0KCkgewogICAgc3RvcAogICAgc3RhcnQKfQpyaF9zdGF0dXMoKSB7CiAgICBzdGF0dXMgLXAgIiRpbnB1dFBhdGgvbG9ncy9waWQvJG"
		"5hbWUucGlkIiAkbmFtZQp9CnJoX3N0YXR1c19xKCkgewogICAgcmhfc3RhdHVzID4vZGV2L251bGwgMj4mMQp9CmNhc2UgIiQxIiBpbgogICAgc3RhcnQpCiAgICAgICAgcmhfc3RhdHVzX3EgJiYgZXhpdCAwCiAgICAgICAgJDEKICAgICAgICA7OwogICAgc3RvcCkKICAgICAgICByaF9zdGF0dXNfcSB8fCBleGl0IDAKICAgICAgICAkMQogICAgICAgIDs7CiAgICByZXN0YXJ0KQogICAgICAgICQxCiAgICAgICAgOzsKICAgIHN0YXR1cykKICAgICAgICByaF9zdGF0dXMKICAgICAgICA7OwogICAgKikKICAgICAgICBlY2hvICQiVXNhZ2U6ICQwIHtzdGFydHxzdG9wfHN0YXR1c3xyZXN0YXJ0fSIKICAgICAgICBleGl0IDIKZXNhYwpleGl0ICQ/";
	char *templateContent;


	//检查是否存在同名服务
	char *servicePath;
	spprintf(&servicePath,0,"/etc/init.d/%s",serviceName);
	
	if(SUCCESS == fileExist(servicePath)){
		char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uXSDliJvlu7pMaW51eOacjeWKoeWksei0pe+8jC9ldGMvaW5pdC5kLyDkuIvlt7LlrZjlnKjlkIzlkI3mnI3liqHvvIzoi6Xku43pnIDlronoo4XmraTmnI3liqHor7flhYjliKDpmaTlkIzlkI3mnI3liqHvvJo=";
		char *tipsSay;
		char *tipsEnd;
		base64Decode(tips,&tipsSay);
		spprintf(&tipsEnd,0,"%s%s",tipsSay,serviceName);
		zend_throw_exception(CServiceExceptionCe, tipsEnd, 7001 TSRMLS_CC);
		efree(tipsEnd);
		efree(tipsSay);
		efree(servicePath);
		return;
	}

	//创建同名服务
	base64Decode(templateString,&templateContent);

	//替换其中变量
	zval	*replace,
			*newVal;
	char	*serviceCode;

	MAKE_STD_ZVAL(replace);
	MAKE_STD_ZVAL(newVal);
	array_init(replace);
	array_init(newVal);
	add_next_index_string(replace,"<{$serverName}>",1);
	add_next_index_string(replace,"<{$htmlPath}>",1);
	add_next_index_string(replace,"<{$controller}>",1);
	add_next_index_string(newVal,serviceName,1);
	add_next_index_string(newVal,path,1);
	add_next_index_string(newVal,controller,1);
	
	str_replaceArray(replace,newVal,templateContent,&serviceCode);

	//写入文件
	file_put_contents(servicePath,serviceCode);

	if(SUCCESS != fileExist(servicePath)){
		char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uXSDlkJEgL2V0Yy9pbml0LmQvIOWGmeWFpeacjeWKoeaWh+S7tuWksei0pe+8jOivt+ajgOafpeW9k+WJjWxpbnV46LSm5Y+35p2D6ZmQ5piv5ZCm5YWB6K645re75Yqg57O757uf5pyN5Yqh";
		char *tipsSay;
		char *tipsEnd;
		base64Decode(tips,&tipsSay);
		spprintf(&tipsEnd,0,"%s%s",tipsSay,serviceName);
		zend_throw_exception(CServiceExceptionCe, tipsEnd, 7001 TSRMLS_CC);
		efree(tipsEnd);
		efree(tipsSay);
		efree(servicePath);
		zval_ptr_dtor(&replace);
		zval_ptr_dtor(&newVal);
		efree(serviceCode);
		efree(templateContent);
		return;
	}

	//添加可执行权限
	char *command;
	spprintf(&command,0,"chmod +x %s",servicePath);
	exec_shell(command);

	//判断是否可执行
	if(0 == access(servicePath, 6)){

		char	*successTips = "Q1NlcnZpY2VDb250cm9sbGVy5Yib5bu6TGludXjmnI3liqHmiJDlip8K5ZCv5Yqo5pyN5YqhOjwkc3RhcnQ+CuWBnOatouacjeWKoTo8JHN0b3A+CumHjeWQr+acjeWKoTo8JHJlc3RhcnQ+Cg==";
		char	*tipsDecode,
				*tipsPrint;
		
		zval	*replaceZval,
				*newValZval;

		char	*startCommand,
				*stopCommand,
				*restartCommand;

		spprintf(&startCommand,0,"service %s start",serviceName);
		spprintf(&stopCommand,0,"service %s stop",serviceName);
		spprintf(&restartCommand,0,"service %s restart",serviceName);

		MAKE_STD_ZVAL(replaceZval);
		MAKE_STD_ZVAL(newValZval);
		array_init(replaceZval);
		array_init(newValZval);
		add_next_index_string(replaceZval,"<$start>",1);
		add_next_index_string(newValZval,startCommand,0);
		add_next_index_string(replaceZval,"<$stop>",1);
		add_next_index_string(newValZval,stopCommand,0);
		add_next_index_string(replaceZval,"<$restart>",1);
		add_next_index_string(newValZval,restartCommand,0);
		base64Decode(successTips,&tipsDecode);
		str_replaceArray(replaceZval,newValZval,tipsDecode,&tipsPrint);
		php_printf("\n%s\n",tipsPrint);
		zval_ptr_dtor(&replaceZval);
		zval_ptr_dtor(&newValZval);
		efree(tipsDecode);
		efree(tipsPrint);

	}else{

		char *failTips = "5Yib5bu6TGludXjmnI3liqHlpLHotKUK6K+35qOA5p+l5aaC5LiL5Y+v6IO9OgoxLuaJp+ihjOWIm+W7uuWuieijheWRveS7pOeahOeUqOaIt+aYr+WQpuaciei2s+Wkn+eahOadg+mZkOS7peWIm+W7uuacjeWKoQoyLuezu+e7n+aYr+WQpuS4ukNlbnRPU++8jOW9k+WJjeacjeWKoeiEmuacrOS7hemAgumFjUNlbnRPUwozLuacjeWKoeaOp+WItuWZqOS7o+eggeaYr+WQpuacieivrwo=";
		char *failShow;
		base64Decode(failTips,&failShow);
		php_printf("\n%s\n",failShow);
		efree(failShow);
	}

	efree(command);
	zval_ptr_dtor(&replace);
	zval_ptr_dtor(&newVal);
	efree(serviceCode);
	efree(templateContent);
	efree(servicePath);

}

PHP_METHOD(CServiceController,Action_install)
{
	zval *serviceName = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("name"),1 TSRMLS_CC);
	zval *appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	zend_class_entry *nowClass = Z_OBJCE_P(getThis());

	//读取服务名
	if(IS_STRING == Z_TYPE_P(serviceName) && strlen(Z_STRVAL_P(serviceName)) > 0){
	}else{
		char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uXSDlronoo4VMaW51eOacjeWKoeWksei0pe+8jOiwg+eUqENTZXJ2aWNlQ29udHJvbGxlci0+c2VydmljZU5hbWUoKeiOt+WPluW9k+WJjeacjeWKoeWQjeWksei0pe+8jOWPr+WcqOaOp+WItuWZqOS4remHjeWGmUNTZXJ2aWNlQ29udHJvbGxlcuaPkOS+m+eahOatpOWHveaVsO+8jOm7mOiupOWwhuWOu+ivu+aOp+WItuWQjeWBmuS4uuacjeWKoeWQjQ==";
		char *tipsSay;
		base64Decode(tips,&tipsSay);
		zend_throw_exception(CServiceExceptionCe, tipsSay, 7001 TSRMLS_CC);
		efree(tipsSay);
		return;
	}

	//读取当前文件路径失败
	if(IS_STRING == Z_TYPE_P(appPath) && strlen(Z_STRVAL_P(appPath)) > 0){
	}else{
		char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uXSDor7vlj5bnqIvluo/nm67lvZXlj5jph49BUFBfUEFUSOWksei0pe+8jOivt+iwg+eUqENXZWJBcHA6OmNyZWF0ZUFwcCgp5Lul5ZCv5Yqo5qGG5p625bm26K6+572u546v5aKD5Y+Y6YeP";
		char *tipsSay;
		base64Decode(tips,&tipsSay);
		zend_throw_exception(CServiceExceptionCe, tipsSay, 7001 TSRMLS_CC);
		efree(tipsSay);
		return;
	}

	//读取当前控制器名字
	if(strlen(nowClass->name) > 0){
	}else{
		char *tips = "W0NTZXJ2aWNlRXhjZXB0aW9uXSDor7vlj5blvZPliY3mjqfliLblmajlkI3lpLHotKXvvIzpgJrluLjmraTlvILluLjkuLrmoYbmnrblhoXpg6jlvILluLg=";
		char *tipsSay;
		base64Decode(tips,&tipsSay);
		zend_throw_exception(CServiceExceptionCe, tipsSay, 7001 TSRMLS_CC);
		efree(tipsSay);
		return;
	}

	char *className = estrdup(nowClass->name);
	CServiceController_action_install(Z_STRVAL_P(appPath),Z_STRVAL_P(serviceName),className TSRMLS_CC);
	efree(className);
}

PHP_METHOD(CServiceController,Action_serviceRunning)
{

	//执行run
	zval	constructVal,
			constructReturn;



	signal(SIGHUP, handleMaster_signal);
	prctl(PR_SET_PDEATHSIG, SIGHUP);
	prctl(PR_SET_NAME, "CServiceWorker"); 


	INIT_ZVAL(constructVal);
	ZVAL_STRING(&constructVal,"run",0);
	call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
	zval_dtor(&constructReturn);
}
#endif