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
#include "php_CThread.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CThread_functions[] = {
	PHP_ME(CThread,run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CThread,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CThread,abort,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CThread,isWatch,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CThread)
{
	//注册CThread类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CThread",CThread_functions);
	CThreadCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_string(CThreadCe, ZEND_STRL("scriptPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CThreadCe, ZEND_STRL("scriptParams"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CThreadCe, ZEND_STRL("runNum"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CThreadCe, ZEND_STRL("isWatch"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CThread,run)
{

	zval	*isWatchZval,
			*runNumZval,
			*scriptPathZval,
			*scriptParamsZval;

	int		systemNowRuns = 0,
			needCreateNum = 0,
			i,
			resetRunNum = 1;

	char	*shellCommond,
			*createShellCommond;

	//读取类参数
	isWatchZval = zend_read_property(CThreadCe,getThis(),ZEND_STRL("isWatch"),0 TSRMLS_CC);
	runNumZval = zend_read_property(CThreadCe,getThis(),ZEND_STRL("runNum"),0 TSRMLS_CC);
	scriptPathZval = zend_read_property(CThreadCe,getThis(),ZEND_STRL("scriptPath"),0 TSRMLS_CC);
	scriptParamsZval = zend_read_property(CThreadCe,getThis(),ZEND_STRL("scriptParams"),0 TSRMLS_CC);

	//判断脚本文件是否存在
	if(SUCCESS != fileExist(Z_STRVAL_P(scriptPathZval))){
		zend_throw_exception(CShellExceptionCe, "[CShellException] run script is not exists", "10010" TSRMLS_CC);
		return;
	}


	if(Z_TYPE_P(isWatchZval) == IS_LONG && Z_LVAL_P(isWatchZval) == 1){

		//读取当前系统中运行此脚本的数据
		strcat2(&shellCommond,"/bin/ps -ef|/bin/grep '",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval),"'|/bin/grep -v 'grep'|wc -l",NULL);
		systemNowRuns = exec_shell(shellCommond);
		systemNowRuns = systemNowRuns > 0 ? systemNowRuns - 1 : systemNowRuns;
		efree(shellCommond);

		//默认为监听模式
		if(systemNowRuns >= Z_LVAL_P(runNumZval)){
			return;
		}

		needCreateNum = Z_LVAL_P(runNumZval) - systemNowRuns;

	}else{

		needCreateNum = Z_LVAL_P(runNumZval);

	}


	//需要创建的进程数
	strcat2(&createShellCommond,"nohup php ",Z_STRVAL_P(scriptPathZval)," ",Z_STRVAL_P(scriptParamsZval)," >/dev/null 2>&1 &",NULL);

	for(i = 0 ; i < needCreateNum ; i++){

		//启用脚本
		exec_shell(createShellCommond);

	}

	efree(createShellCommond);

}

PHP_METHOD(CThread,__construct)
{
	char	*scriptPath,
			*scriptParams = "";
	int		scriptPathLen = 0,
			scriptParamsLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&scriptPath,&scriptPathLen,&scriptParams,&scriptParamsLen) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] create Thread Object must give 1 or 2 Parameters : the script path is required, and the script exec params is optional", "10010" TSRMLS_CC);
		return;
	}

	//更新类变量
	zend_update_property_string(CThreadCe,getThis(),ZEND_STRL("scriptPath"),scriptPath TSRMLS_CC);
	zend_update_property_string(CThreadCe,getThis(),ZEND_STRL("scriptParams"),scriptParams TSRMLS_CC);

#ifdef PHP_WIN32
	zend_throw_exception(CShellExceptionCe, "[CShellException] The server is windows , so can not use this function!", "1" TSRMLS_CC);
#endif
}

PHP_METHOD(CThread,abort)
{
}

PHP_METHOD(CThread,isWatch)
{
	int runNum = 0,
		setisWatch = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b|l",&setisWatch,&runNum) == FAILURE){
		zend_throw_exception(CShellExceptionCe, "[CShellException] Given a boolean meaning is watch or create, Given an integer value, the number of processes that maintain the value", "10010" TSRMLS_CC);
		return;
	}

	//设置为监听模式
	if(setisWatch == 1){
		zend_update_property_long(CThreadCe,getThis(),ZEND_STRL("isWatch"),1 TSRMLS_CC);
	}

	//更新类属性
	if(runNum != 0){
		zend_update_property_long(CThreadCe,getThis(),ZEND_STRL("runNum"),runNum TSRMLS_CC);
	}

	RETVAL_ZVAL(getThis(),1,0);
}