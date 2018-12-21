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
#include "php_CWatcher.h"
#include "php_CWebApp.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CWathcer_functions[] = {
	PHP_ME(CWatcher,run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWatcher,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CWatcher,setRunNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWatcher,setLogFile,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWatcher,stopAll,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWatcher,stop,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWatcher,getRunList,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CWatcher)
{
	//注册CThread类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CWatcher",CWathcer_functions);
	CWatcherCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_string(CWatcherCe, ZEND_STRL("scriptPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CWatcherCe, ZEND_STRL("scriptParams"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CWatcherCe, ZEND_STRL("runNum"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CWatcherCe, ZEND_STRL("logFile"),"/dev/null",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void getProcessData(char *string,zval **returnVal TSRMLS_DC){

	zval *matchZval,
		 *saveZval,
		 **thisVal;

	MAKE_STD_ZVAL(saveZval);
	array_init(saveZval);

	//正则
	if(preg_match("/(\\w+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S+)\\s+(\\S+)\\s+(.*)\\sphp\\s+([\\w\\._\\/]+)\\s*([\\w\\._\\/]*)/",string,&matchZval)){

		//执行用户名
		zend_hash_index_find(Z_ARRVAL_P(matchZval),1,(void**)&thisVal);
		add_assoc_string(saveZval,"user",Z_STRVAL_PP(thisVal),1);

		//pid
		zend_hash_index_find(Z_ARRVAL_P(matchZval),2,(void**)&thisVal);
		add_assoc_string(saveZval,"pid",Z_STRVAL_PP(thisVal),1);

		//ppid
		zend_hash_index_find(Z_ARRVAL_P(matchZval),3,(void**)&thisVal);
		add_assoc_string(saveZval,"ppid",Z_STRVAL_PP(thisVal),1);

		//c
		zend_hash_index_find(Z_ARRVAL_P(matchZval),4,(void**)&thisVal);
		add_assoc_string(saveZval,"c",Z_STRVAL_PP(thisVal),1);

		//stime
		zend_hash_index_find(Z_ARRVAL_P(matchZval),5,(void**)&thisVal);
		add_assoc_string(saveZval,"stime",Z_STRVAL_PP(thisVal),1);

		//TTY
		zend_hash_index_find(Z_ARRVAL_P(matchZval),6,(void**)&thisVal);
		add_assoc_string(saveZval,"tty",Z_STRVAL_PP(thisVal),1);

		//time
		zend_hash_index_find(Z_ARRVAL_P(matchZval),7,(void**)&thisVal);
		add_assoc_string(saveZval,"time",Z_STRVAL_PP(thisVal),1);

		//script
		zend_hash_index_find(Z_ARRVAL_P(matchZval),8,(void**)&thisVal);
		add_assoc_string(saveZval,"script",Z_STRVAL_PP(thisVal),1);
		
		//param
		zend_hash_index_find(Z_ARRVAL_P(matchZval),9,(void**)&thisVal);
		add_assoc_string(saveZval,"param",Z_STRVAL_PP(thisVal),1);
	}

	zval_ptr_dtor(&matchZval);
	MAKE_STD_ZVAL(*returnVal);
	ZVAL_ZVAL(*returnVal,saveZval,1,1);
}

void setRunResultToLog(char *log,zval *object TSRMLS_DC)
{

	char *thisMothTime,
		 *logString,
		 logPath[10240];

	zval *appPath,
		 *logFile;

	//日期
	php_date("Y-m-d h:i:s",&thisMothTime);

	//日志路径
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(appPath)){
		return;
	}

	sprintf(logPath,"%s%s",Z_STRVAL_P(appPath),"/logs/systemlog/CWatcher.log");

	//内容
	strcat2(&logString,"#LogTime:",thisMothTime,PHP_EOL,"[CWatcherExecShellCommand] \"",log,"\"",PHP_EOL,PHP_EOL,NULL);
	CLog_writeFile(logPath,logString);
	efree(thisMothTime);
	efree(logString);
}

PHP_METHOD(CWatcher,run)
{

	zval	*runNumZval,
			*scriptPathZval,
			*scriptParamsZval,
			*logFile;

	int		systemNowRuns = 0,
			needCreateNum = 0,
			i,
			resetRunNum = 1;

	char	*shellCommond,
			*createShellCommond,
			*logString;

	//读取类参数
	runNumZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("runNum"),0 TSRMLS_CC);
	scriptPathZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptPath"),0 TSRMLS_CC);
	scriptParamsZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptParams"),0 TSRMLS_CC);
	logFile = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("logFile"),0 TSRMLS_CC);

	//判断脚本文件是否存在
	if(SUCCESS != fileExist(Z_STRVAL_P(scriptPathZval))){
		zend_throw_exception(CShellExceptionCe, "[CShellException] run script is not exists", "10010" TSRMLS_CC);
		return;
	}

	//读取当前系统中运行此脚本的数据
	if(Z_STRLEN_P(scriptParamsZval) >0 ){
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval),"'|/bin/grep -v 'grep'|wc -l",NULL);
	}else{
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval),"'|/bin/grep -v 'grep'|wc -l",NULL);
	}

	systemNowRuns = exec_shell(shellCommond);
	efree(shellCommond);

	//默认为监听模式
	if(systemNowRuns >= Z_LVAL_P(runNumZval)){
		return;
	}

	needCreateNum = Z_LVAL_P(runNumZval) - systemNowRuns;

	//需要创建的进程数
	strcat2(&createShellCommond,"nohup php ",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval)," > ",Z_STRVAL_P(logFile)," 2>&1 &",NULL);

	for(i = 0 ; i < needCreateNum ; i++){

		//启用脚本
		exec_shell(createShellCommond);

		//记录日志
		setRunResultToLog(createShellCommond,getThis() TSRMLS_CC);
	}

	efree(createShellCommond);
}

PHP_METHOD(CWatcher,__construct)
{
	char	*scriptPath,
			*scriptParams = "";
	int		scriptPathLen = 0,
			scriptParamsLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&scriptPath,&scriptPathLen,&scriptParams,&scriptParamsLen) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] create Watcher Object must give 1 or 2 Parameters : the script path is required, and the script exec params is optional", "10010" TSRMLS_CC);
		return;
	}

	//更新类变量
	zend_update_property_string(CWatcherCe,getThis(),ZEND_STRL("scriptPath"),scriptPath TSRMLS_CC);
	zend_update_property_string(CWatcherCe,getThis(),ZEND_STRL("scriptParams"),scriptParams TSRMLS_CC);

	//判断平台 如果是window平台则抛出异常
#ifdef PHP_WIN32
	zend_throw_exception(CShellExceptionCe, "[CShellException] The server is windows , so can not use this function!", "1" TSRMLS_CC);
#endif
}


PHP_METHOD(CWatcher,setRunNum)
{
	int runNum = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&runNum) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] Given an integer value, the number of processes that maintain the value", "10010" TSRMLS_CC);
		return;
	}

	//更新类属性
	if(runNum != 0){
		zend_update_property_long(CWatcherCe,getThis(),ZEND_STRL("runNum"),runNum TSRMLS_CC);
	}

	RETVAL_ZVAL(getThis(),1,0);
}

//设置日志文件路径
PHP_METHOD(CWatcher,setLogFile)
{
	char *logFile = "";
	int	  logFileInt = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&logFile,&logFileInt) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] Given a string mean the log file saved path", "2" TSRMLS_CC);
		return;
	}

	if(logFileInt > 0){
		zend_update_property_string(CWatcherCe,getThis(),ZEND_STRL("logFile"),logFile TSRMLS_CC);
	}

	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CWatcher,stopAll)
{
	//获取执行的列表
	zval	*runNumZval,
			*scriptPathZval,
			*scriptParamsZval,
			*logFile,
			*lineArray,
			**thisVal,
			*thisArray,
			*tempTable,
			**pidZval;

	int		systemNowRuns = 0,
			needCreateNum = 0,
			i,
			resetRunNum = 1,
			h = 0;

	char	*shellCommond,
			*createShellCommond,
			*logString,
			*returnString,
			cmd[10240],
			*killString;


	//读取类参数
	runNumZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("runNum"),0 TSRMLS_CC);
	scriptPathZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptPath"),0 TSRMLS_CC);
	scriptParamsZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptParams"),0 TSRMLS_CC);
	logFile = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("logFile"),0 TSRMLS_CC);


	//判断脚本文件是否存在
	if(SUCCESS != fileExist(Z_STRVAL_P(scriptPathZval))){
		zend_throw_exception(CShellExceptionCe, "[CShellException] run script is not exists", "10010" TSRMLS_CC);
		return;
	}

	//读取当前系统中运行此脚本的数据
	if(Z_STRLEN_P(scriptParamsZval) >0 ){
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval),"'",NULL);
	}else{
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval),"'",NULL);
	}

	exec_shell_return(shellCommond,&returnString);
	if(returnString == ""){
		efree(returnString);
		efree(shellCommond);
		RETVAL_FALSE;
		return;
	}

	//按行分割
	php_explode(PHP_EOL,returnString,&lineArray);
	if(IS_ARRAY != Z_TYPE_P(lineArray)){
		array_init(return_value);
		efree(returnString);
		zval_ptr_dtor(&lineArray);
		efree(shellCommond);
		RETVAL_FALSE;
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(lineArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(lineArray));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(lineArray),(void**)&thisVal);
		if(Z_STRLEN_PP(thisVal) == 0 || strstr(Z_STRVAL_PP(thisVal),"/bin/ps") != NULL || strstr(Z_STRVAL_PP(thisVal),"/bin/grep") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//提取关键信息
		getProcessData(Z_STRVAL_PP(thisVal),&thisArray TSRMLS_CC);


		//PID
		if(zend_hash_find(Z_ARRVAL_P(thisArray),"pid",4,(void**)&pidZval) == SUCCESS && IS_STRING == Z_TYPE_PP(pidZval)){

			//执行kill
			sprintf(cmd,"%s%s","kill ",Z_STRVAL_PP(pidZval));

			//执行shell
			exec_shell_return(cmd,&killString);
			efree(killString);
		}

		zval_ptr_dtor(&thisArray);

		zend_hash_move_forward(Z_ARRVAL_P(lineArray));
	}

	efree(shellCommond);
	efree(returnString);
	zval_ptr_dtor(&lineArray);
	RETVAL_TRUE;
}

PHP_METHOD(CWatcher,stop)
{
	int pid = 0;
	char cmd[10240],
		 *returnString;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&pid) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] Given an intger mean pid", "3" TSRMLS_CC);
		return;
	}

	if(pid == 0){
		RETURN_FALSE;
	}

	sprintf(cmd,"%s%d","kill ",pid);

	//执行shell
	exec_shell_return(cmd,&returnString);
	if(strlen(returnString) == 0){
		efree(returnString);
		RETURN_TRUE;
	}

	efree(returnString);
	RETURN_FALSE;
}


PHP_METHOD(CWatcher,getRunList)
{
	//获取执行的列表
	zval	*runNumZval,
			*scriptPathZval,
			*scriptParamsZval,
			*logFile,
			*lineArray,
			**thisVal,
			*thisArray,
			*tempTable;

	int		systemNowRuns = 0,
			needCreateNum = 0,
			i,
			resetRunNum = 1,
			h = 0;

	char	*shellCommond,
			*createShellCommond,
			*logString,
			*returnString;


	//读取类参数
	runNumZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("runNum"),0 TSRMLS_CC);
	scriptPathZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptPath"),0 TSRMLS_CC);
	scriptParamsZval = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("scriptParams"),0 TSRMLS_CC);
	logFile = zend_read_property(CWatcherCe,getThis(),ZEND_STRL("logFile"),0 TSRMLS_CC);


	//判断脚本文件是否存在
	if(SUCCESS != fileExist(Z_STRVAL_P(scriptPathZval))){
		zend_throw_exception(CShellExceptionCe, "[CShellException] run script is not exists", "10010" TSRMLS_CC);
		return;
	}

	//读取当前系统中运行此脚本的数据
	if(Z_STRLEN_P(scriptParamsZval) >0 ){
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval),"'",NULL);
	}else{
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval),"'",NULL);
	}

	exec_shell_return(shellCommond,&returnString);
	if(returnString == ""){
		array_init(return_value);
		efree(returnString);
		efree(shellCommond);
		return;
	}

	//按行分割
	php_explode(PHP_EOL,returnString,&lineArray);
	if(IS_ARRAY != Z_TYPE_P(lineArray)){
		array_init(return_value);
		efree(returnString);
		zval_ptr_dtor(&lineArray);
		efree(shellCommond);
		return;
	}

	MAKE_STD_ZVAL(tempTable);
	array_init(tempTable);

	h = zend_hash_num_elements(Z_ARRVAL_P(lineArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(lineArray));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(lineArray),(void**)&thisVal);
		if(Z_STRLEN_PP(thisVal) == 0 || strstr(Z_STRVAL_PP(thisVal),"/bin/ps") != NULL || strstr(Z_STRVAL_PP(thisVal),"/bin/grep") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//提取关键信息
		getProcessData(Z_STRVAL_PP(thisVal),&thisArray TSRMLS_CC);

		if(zend_hash_num_elements(Z_ARRVAL_P(thisArray)) == 0){
			zval_ptr_dtor(&thisArray);
			add_next_index_string(tempTable,Z_STRVAL_PP(thisVal),1);
		}else{
			add_next_index_zval(tempTable,thisArray);
		}
		zend_hash_move_forward(Z_ARRVAL_P(lineArray));
	}

	efree(shellCommond);
	efree(returnString);
	zval_ptr_dtor(&lineArray);
	ZVAL_ZVAL(return_value,tempTable,1,1);
}