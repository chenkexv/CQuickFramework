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
#include "php_CApplication.h"
#include "php_CWebApp.h"
#include "php_CHooks.h"
#include "php_CInitApplication.h"
#ifndef PHP_WIN32
#include <unistd.h>
#endif

//zend类方法
zend_function_entry CApplication_functions[] = {
	PHP_ME(CApplication,GetRequest,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CApplication,setBootParams,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CApplication,setTimeLimit,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CApplication,setMemoryLimit,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CApplication,setDaemon,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CApplication,runBash,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CApplication)
{
	//注册类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CApplication",CApplication_functions);
	CApplicationCe = zend_register_internal_class_ex(&funCe,CInitApplicationCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//run as a deamon process
PHP_METHOD(CApplication,setDaemon){

	long	nochangeDir = 1,
			noclose = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|ll",&nochangeDir,&noclose) == FAILURE){
		return;
	}


#ifdef PHP_WIN32
	RETVAL_FALSE;
#else
	int s = daemon(nochangeDir, noclose);
	RETVAL_BOOL(s);
#endif
}

//设置启动参数
PHP_METHOD(CApplication,setBootParams){

	zval	*bootParams,
			*cconfigInstanceZval;

	//返回对象
	RETVAL_ZVAL(getThis(),1,0);

	//获取参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&bootParams) == FAILURE){
		return;
	}

	//获取Config的main的配置对象
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//设置参数
	CConfig_setConfigs(bootParams,cconfigInstanceZval TSRMLS_CC);
	zval_ptr_dtor(&cconfigInstanceZval);

}

void CApplication_GetRequest(TSRMLS_D){

	zval	*requestInstace,
			*routeDataZval,
			*responseInstace;

	//处理请求
	CRequest_getInstance(&requestInstace TSRMLS_CC);
	CRequest_run(requestInstace,&routeDataZval TSRMLS_CC);

	zval_ptr_dtor(&routeDataZval);
	zval_ptr_dtor(&requestInstace);


	//发送响应
	CResponse_getInstance(&responseInstace TSRMLS_CC);
	zval_ptr_dtor(&responseInstace);

}

//类方法:获取请求
PHP_METHOD(CApplication,GetRequest)
{
	CApplication_GetRequest(TSRMLS_C);
}


PHP_METHOD(CApplication,setTimeLimit)
{
	long timeout = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&timeout) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	set_time_limit(timeout);
}

PHP_METHOD(CApplication,setMemoryLimit)
{
	char *memoryLimit = "1024M";
	int		memLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&memoryLimit,&memLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	ini_set("memory_limit",memoryLimit);
}

//执行linuxShell的方法 不受disable_function限制
PHP_METHOD(CApplication,runBash)
{

	char	*command,
			*returnString,
			*runCommand;
	int		commandLen = 0;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&command,&commandLen) == FAILURE){
		RETURN_FALSE;
	}

	if(0 == commandLen){
		RETURN_FALSE;
	}

	//call beginHooks
	MODULE_BEGIN
		zval	*paramsList[1],
				*dataObject,
				*cmdZval,
				*hooksEnd;
		MAKE_STD_ZVAL(dataObject);
		object_init_ex(dataObject,CDataObjectCe);
		MAKE_STD_ZVAL(cmdZval);
		ZVAL_STRING(cmdZval,command,1);
		CHooks_setDataObject(dataObject,cmdZval TSRMLS_CC);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],dataObject,1,0);
		CHooks_callHooks("HOOKS_BASH_BEFORE",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		//check return data
		CHooks_getDataObject(dataObject,&hooksEnd TSRMLS_CC);
		convert_to_string(hooksEnd);
		runCommand = estrdup(Z_STRVAL_P(hooksEnd));
		zval_ptr_dtor(&hooksEnd);
		zval_ptr_dtor(&dataObject);
		zval_ptr_dtor(&cmdZval);
	MODULE_END


	exec_shell_return(runCommand,&returnString);

	//all shell run will write a log
	MODULE_BEGIN
		char	errorPath[1024],
				errorFile[1024],
				*errorTips,
				*thisMothTime;

		zval	*appPath;

		php_date("Y-m-d H:i:s",&thisMothTime);

		appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
		sprintf(errorPath,"%s%s",Z_STRVAL_P(appPath),"/logs/safe/");
		if(FAILURE == fileExist(errorPath)){
			//尝试创建文件夹
			php_mkdir(errorPath);
		}
		spprintf(&errorTips,0,"%s%s%s%s%s%s%s","#LogTime:",thisMothTime,"\nrunBash:",runCommand,"\nrunResult:\n============================================\n",returnString,"\n============================================\n\n");
		sprintf(errorFile,"%s%s",errorPath,"runBash.log");
		CLog_writeFile(errorFile,errorTips TSRMLS_CC);

		//call hooks
		MODULE_BEGIN
			zval	*paramsList[1],
					*dataObject,
					*saveObject;

			MAKE_STD_ZVAL(saveObject);
			array_init(saveObject);
			add_assoc_string(saveObject,"command",runCommand,1);
			add_assoc_string(saveObject,"result",returnString,1);

			MAKE_STD_ZVAL(dataObject);
			object_init_ex(dataObject,CDataObjectCe);
			CHooks_setDataObject(dataObject,saveObject TSRMLS_CC);
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],dataObject,1,0);
			CHooks_callHooks("HOOKS_BASH_END",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_ptr_dtor(&saveObject);
			zval_ptr_dtor(&dataObject);
		MODULE_END

		efree(errorTips);
		efree(thisMothTime);

	MODULE_END

	RETVAL_STRING(returnString,1);
	efree(returnString);
	efree(runCommand);
}