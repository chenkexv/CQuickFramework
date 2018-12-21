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
#include "php_CApplication.h"
#include "php_CInitApplication.h"


//zend类方法
zend_function_entry CApplication_functions[] = {
	PHP_ME(CApplication,GetRequest,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CApplication,setBootParams,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CApplication,setTimeLimit,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CApplication,setMemoryLimit,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
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

//类方法:获取请求
PHP_METHOD(CApplication,GetRequest)
{
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
			*returnString;
	int		commandLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&command,&commandLen) == FAILURE){
		RETURN_FALSE;
	}

	if(0 == commandLen){
		RETURN_FALSE;
	}

	exec_shell_return(command,&returnString);

	RETVAL_STRING(returnString,1);
	efree(returnString);
}