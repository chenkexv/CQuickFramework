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
	PHP_ME(CServiceController,serviceNote,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_install,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,status,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,stop,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,start,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,restart,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_help,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_list,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CServiceController,Action_serviceRunning,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CServiceController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CServiceController",CServiceController_functions);
	CServiceControllerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);

	zend_declare_property_string(CServiceControllerCe, ZEND_STRL("name"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CServiceControllerCe, ZEND_STRL("note"),"No description of related services",ZEND_ACC_PUBLIC TSRMLS_CC);
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

	}else if(strcmp(data,"Action_install") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_install",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"status") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"status",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"start") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"start",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"stop") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"stop",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"restart") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"restart",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"Action_help") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_help",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"Action_list") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_list",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else if(strcmp(data,"Action_run") == 0){
		
		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_run",0);
		call_user_function(NULL, &getThis(), &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);

	}else{

		zval	constructVal,
				constructReturn;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"Action_help",0);
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

	//获取服务说明
	MODULE_BEGIN
		zval	callFunction,
				callReturn;

		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"serviceNote",0);
		call_user_function(NULL, &getThis(), &callFunction, &callReturn, 0, NULL TSRMLS_CC);
		zval	*returnZval;
		returnZval = &callReturn;
		if(IS_STRING == Z_TYPE_P(returnZval) && strlen(Z_STRVAL_P(returnZval)) > 0){
			zend_update_property_string(CServiceControllerCe,getThis(),ZEND_STRL("note"),Z_STRVAL_P(returnZval) TSRMLS_CC);
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

void CServiceController_action_install(char *path,char *serviceName,char *controller,char *note TSRMLS_DC){

	char *templateString = "IyEvYmluL3NoCi4gL2V0Yy9yYy5kL2luaXQuZC9mdW5jdGlvbnMKbmFtZT0iPHskc2VydmVyTmFtZX0+IgpleGVjPSIvdXNyL2Jpbi9waHAiCmlucHV0UGF0aD0iPHskaHRtbFBhdGh9PiIKY29udHJvbGxlcj0iPHskY29udHJvbGxlcn0+Igpub3RlPSI8eyRub3RlfT4iCnN0YXJ0KCkgewogICAgZWNobyAtbiAkIlN0YXJ0aW5nIENRdWlja0ZyYW1ld29yayBTZXJ2aWNlIFskbmFtZV06ICIKICAgIGRhZW1vbiAtLXVzZXIgcm9vdCAiJGV4ZWMgJGlucHV0UGF0aC9pbmRleC5waHAgJGNvbnRyb2xsZXIvcnVuIgogICAgcmV0dmFsPSQ/CiAgICBlY2hvCiAgICByZXR1cm4gJHJldHZhbAp9CnN0b3AoKSB7CiAgICBlY2hvIC1uICQiU3RvcHBpbmcgQ1F1aWNrRnJhbWV3b3JrIFNlcnZpY2UgWyRuYW1lXTogIgogICAga2lsbHByb2MgLXAgIiRpbnB1dFBhdGgvbG9ncy9waWQvJG5hbWUucGlkIiAkbmFtZQogICAgcmV0dmFsPSQ/CiAgICBlY2hvCiAgICByZXR1cm4gJHJldHZhbAp9CnJlc3RhcnQoKSB7CiAgICBzdG9wCiAgICBzdGFydAp9CnJoX3N0YXR1cygpIHsKICAgIHN0YXR1cyAtcCAiJGlucHV0UGF0aC9sb2dzL3BpZC8kbmFtZS5waWQiICRuYW1lCn0Kcmhfc3RhdHVzX3EoKSB7CiAgICByaF9zdGF0dXMgPi9kZXYvbnVsbCAyPiYxCn0KY2FzZSAiJDEiIGluCiAgICBzdGFydCkKICAgICAgICByaF9zdGF0dXNfcSAmJiBleGl0IDAKICAgICAgICAkMQogICAgICAgIDs7CiAgICBzdG9wKQogICAgICAgIHJoX3N0YXR1c19xIHx8IGV4aXQgMAogICAgICAgICQxCiAgICAgICAgOzsKICAgIHJlc3RhcnQpCiAgICAgICAgJDEKICAgICAgICA7OwogICA"
							"gc3RhdHVzKQogICAgICAgIHJoX3N0YXR1cwogICAgICAgIDs7CglpbmZvKQoJCWVjaG8gIiRub3RlIgoJCTs7CiAgICAqKQogICAgICAgIGVjaG8gJCJVc2FnZTogJDAge3N0YXJ0fHN0b3B8c3RhdHVzfHJlc3RhcnR9IgogICAgICAgIGV4aXQgMgplc2FjCmV4aXQgJD8=";
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
	add_next_index_string(replace,"<{$note}>",1);
	add_next_index_string(newVal,serviceName,1);
	add_next_index_string(newVal,path,1);
	add_next_index_string(newVal,controller,1);
	add_next_index_string(newVal,note,1);
	
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
	zval *note = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("note"),1 TSRMLS_CC);
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
	CServiceController_action_install(Z_STRVAL_P(appPath),Z_STRVAL_P(serviceName),className,Z_STRVAL_P(note) TSRMLS_CC);
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

int CServiceController_status(zval *object TSRMLS_DC){

	zval	*serviceName = zend_read_property(CServiceControllerCe,object,ZEND_STRL("name"),1 TSRMLS_CC);

	//检查服务存在
	char	*command,
			*result;

	spprintf(&command,0,"/etc/init.d/%s",Z_STRVAL_P(serviceName));
	
	if(0 != access(command, 6)){
		char *tips = "5b2T5YmN5o6n5Yi25Zmo5bCa5pyq5a6J6KOF5oiQbGludXjmnI3liqHvvIzor7fosIPnlKhpbnN0YWxs5pa55rOV5a6J6KOF5pyN5Yqh77yM6ICM5ZCO6LCD55Soc3RhdHVz5Y+v6L+U5Zue5b2T5YmN5pyN5Yqh54q25oCB";
		char *tipsSay;
		base64Decode(tips,&tipsSay);
		zend_throw_exception(CServiceExceptionCe, tipsSay, 7001 TSRMLS_CC);
		efree(command);
		efree(tipsSay);
		return -1;
	}
	efree(command);

	char *statusCommand;
	spprintf(&statusCommand,0,"/sbin/service %s status",Z_STRVAL_P(serviceName));
	exec_shell_return(statusCommand,&result);

	if(strstr(result,"is running...") != NULL){
		efree(statusCommand);
		efree(result);
		return 1;
	}else{
		efree(statusCommand);
		efree(result);
		return 0;
	}
}

PHP_METHOD(CServiceController,status)
{
	int status = CServiceController_status(getThis() TSRMLS_CC);

	if(status == 1){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CServiceController,stop)
{

	//先执行stop
	zval	*serviceName = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("name"),1 TSRMLS_CC);
	char	*command,
			*result;

	//执行start
	spprintf(&command,0,"/sbin/service %s stop",Z_STRVAL_P(serviceName));
	exec_shell_return(command,&result);

	if(strstr(result,"[  OK  ]") != NULL){
		efree(result);
		efree(command);
		RETURN_BOOL(1);
	}

	efree(result);
	efree(command);

	//而后读取pid 判断进程是否存在
	zval *appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	char *pidPath;
	spprintf(&pidPath,0,"%s/logs/pid/%s.pid",Z_STRVAL_P(appPath),Z_STRVAL_P(serviceName));

	//没有pid也返回成功
	if(FAILURE == fileExist(pidPath)){
		efree(pidPath);
		RETURN_BOOL(1);
	}

	//找pid则检查进程
	char *pidString;
	file_get_contents(pidPath,&pidString);
	if(strlen(pidString) <= 0){
		efree(pidString);
		efree(pidPath);
		RETURN_BOOL(1);
	}

	//检查进程存在
	int pidInt = toInt(pidString);
	int processStatus = kill(pidInt,0);
	if(processStatus == 0){
		efree(pidString);
		efree(pidPath);
		RETURN_BOOL(0);
	}

	efree(pidString);
	efree(pidPath);
	RETURN_BOOL(1);
}

PHP_METHOD(CServiceController,start)
{
	zval	*serviceName = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("name"),1 TSRMLS_CC);
	char	*command,
			*result;

	//执行start
	spprintf(&command,0,"/sbin/service %s start",Z_STRVAL_P(serviceName));
	exec_shell_return(command,&result);
	efree(result);
	efree(command);

	//判断是否已启动
	int status = CServiceController_status(getThis() TSRMLS_CC);
	
	if(status == 1){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CServiceController,restart)
{
	zval	*serviceName = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("name"),1 TSRMLS_CC);
	char	*command,
			*result;

	//执行start
	spprintf(&command,0,"/sbin/service %s restart",Z_STRVAL_P(serviceName));
	exec_shell_return(command,&result);
	efree(result);
	efree(command);

	//判断是否已启动
	int status = CServiceController_status(getThis() TSRMLS_CC);
	
	if(status == 1){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CServiceController,Action_help)
{
	char	*tips = "CkNMSeWRveS7pOihjOaPkOekujoK5a6J6KOF5pyN5YqhOiBwaHAgPCRwYXRoPi9pbmRleC5waHAgPCRjb250cm9sbGVyPi9pbnN0YWxsCui/kOihjOacjeWKoTogcGhwIDwkcGF0aD4vaW5kZXgucGhwIDwkY29udHJvbGxlcj4vcnVuCuWIl+WHuuacjeWKoTogcGhwIDwkcGF0aD4vaW5kZXgucGhwIDwkY29udHJvbGxlcj4vbGlzdAoK5a6J6KOF5pyN5Yqh5Y+v5L2/55So5aaC5LiL5ZG95LukOgrlkK/liqjmnI3liqE6IHNlcnZpY2UgPCRuYW1lPiBzdGFydArlgZzmraLmnI3liqE6IHNlcnZpY2UgPCRuYW1lPiBzdG9wCumHjeWQr+acjeWKoTogc2VydmljZSA8JG5hbWU+IHJlc3RhcnQK5Yig6Zmk5pyN5YqhOiBybSAtcmYgL2V0Yy9pbml0LmQvPCRuYW1lPgoK";
	char	*decodeTips,
			*replaceString;

	zval	*appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	zval	*serviceName = zend_read_property(CServiceControllerCe,getThis(),ZEND_STRL("name"),1 TSRMLS_CC);
	zend_class_entry	*nowClass = Z_OBJCE_P(getThis());

	zval	*replace,
			*newVal;

	MAKE_STD_ZVAL(replace);
	MAKE_STD_ZVAL(newVal);
	array_init(replace);
	array_init(newVal);
	add_next_index_string(replace,"<$path>",1);
	add_next_index_string(replace,"<$name>",1);
	add_next_index_string(replace,"<$controller>",1);
	add_next_index_string(newVal,Z_STRVAL_P(appPath),1);
	add_next_index_string(newVal,Z_STRVAL_P(serviceName),1);
	add_next_index_string(newVal,nowClass->name,1);


	base64Decode(tips,&decodeTips);
	str_replaceArray(replace,newVal,decodeTips,&replaceString);


	php_printf("%s",replaceString);

	zval_ptr_dtor(&replace);
	zval_ptr_dtor(&newVal);
	efree(decodeTips);
	efree(replaceString);
}

PHP_METHOD(CServiceController,Action_list)
{

	char	*path;
	zval	*dirFile;
	zval	*appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);

	spprintf(&path,0,"%s/logs/pid",Z_STRVAL_P(appPath));

	printfBase64("5b2T5YmN6aG555uu5a6J6KOF6L+H55qE5pyN5YqhOgo=");

	//列出pid目录下曾经安装过的服务
	php_scandir(path,&dirFile);

	int		i,h,allNum = 0,serviceStatus=0;
	zval	**nowData;
	char	*servicePath,
			*serverName;
	h = zend_hash_num_elements(Z_ARRVAL_P(dirFile));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirFile));
	for(i = 0 ; i < h ;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(dirFile),(void**)&nowData);

		if(strstr(Z_STRVAL_PP(nowData),".pid") == NULL){
			zend_hash_move_forward(Z_ARRVAL_P(dirFile));
			continue;
		}

		//判断此服务是否还存在
		str_replace(".pid","",Z_STRVAL_PP(nowData),&serverName);
		spprintf(&servicePath,0,"/etc/init.d/%s",serverName);

		//检查是否存在且可执行
		if(0 == access(servicePath, 6)){
			
			char	*serviceNote,
					*getNoteCommand;
			spprintf(&getNoteCommand,0,"service %s info",serverName);
			exec_shell_return(getNoteCommand,&serviceNote);
			php_printf("Service [%s] has available ; Note: %s\n",serverName,serviceNote);
			efree(getNoteCommand);
			efree(serviceNote);

		}else{
			php_printf("Service [%s] has been removed\n",serverName);
		}


		efree(serverName);
		efree(servicePath);

		zend_hash_move_forward(Z_ARRVAL_P(dirFile));
	}



	efree(path);
	zval_ptr_dtor(&dirFile);
}

PHP_METHOD(CServiceController,serviceNote){
	RETVAL_FALSE;
}


#endif