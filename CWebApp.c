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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


#include "php_CQuickFramework.h"
#include "php_CWebApp.h"
#include "php_CConfig.h"
#include "php_CException.h"
#include "php_CApplication.h"


//zend类方法
zend_function_entry CWebApp_functions[] = {
	PHP_ME(CWebApp,createApp,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CWebApp)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CWebApp",CWebApp_functions);
	CWebAppCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//全局变量
	zend_declare_property_string(CWebAppCe, ZEND_STRL("app_path"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CWebAppCe, ZEND_STRL("code_path"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CWebAppCe, ZEND_STRL("cplugin_path"),"",ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CWebAppCe, ZEND_STRL("disable_function"),"",ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	
	return SUCCESS;
}

//create app
void CWebApp_createApp(zval **object TSRMLS_DC){

	zval								*appPath,
										*codePath,
										*applicationObject;

	char								*pluginPath;

	//appPath
	if(zend_hash_find(EG(zend_constants),"APP_PATH",strlen("APP_PATH")+1,(void**)&appPath) == SUCCESS){
		if(IS_STRING == Z_TYPE_P(appPath)){
			strcat2(&pluginPath,Z_STRVAL_P(appPath),"/plugins",NULL);
			zend_update_static_property_string(CWebAppCe,ZEND_STRL("app_path"),Z_STRVAL_P(appPath) TSRMLS_CC);
			zend_update_static_property_string(CWebAppCe,ZEND_STRL("cplugin_path"),pluginPath TSRMLS_CC);
			efree(pluginPath);
		}
	}

	//codePath
	if(zend_hash_find(EG(zend_constants),"CODE_PATH",strlen("CODE_PATH")+1,(void**)&codePath) == SUCCESS){
		if(IS_STRING == Z_TYPE_P(codePath)){
			zend_update_static_property_string(CWebAppCe,ZEND_STRL("code_path"),Z_STRVAL_P(codePath) TSRMLS_CC);
		}
	}

	//reset CConfig
	zend_update_static_property_null(CConfigCe,ZEND_STRL("instance") TSRMLS_CC);

	//实例化至Zval结构体
	MAKE_STD_ZVAL(applicationObject);
	object_init_ex(applicationObject,CApplicationCe);

	//执行其构造器 并传入参数
	if (CApplicationCe->constructor) {
		zval	constructReturn,
				constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, CApplicationCe->constructor->common.function_name, 0);
		call_user_function(NULL, &applicationObject, &constructVal, &constructReturn, 0,NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	MAKE_STD_ZVAL(*object);
	ZVAL_ZVAL(*object,applicationObject,1,1);
}

//create app from frame , need set boot path constenst;
void CWebApp_createAppFromFramework(zval **object TSRMLS_DC){
	
	char	*documentRoot,
			*codePath;

	//define
	getServerParam("DOCUMENT_ROOT",&documentRoot TSRMLS_CC);
	spprintf(&codePath,0,"%s%s",documentRoot,"/application");
	php_define("APP_PATH",documentRoot TSRMLS_CC);
	php_define("CODE_PATH",codePath TSRMLS_CC);
	efree(documentRoot);
	efree(codePath);

	CWebApp_createApp(object TSRMLS_CC);
}

//类方法:创建应用对象
PHP_METHOD(CWebApp,createApp)
{

	zval				*createParams = NULL,
						*object;


	//获取参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|a",&createParams) == SUCCESS){
		if(createParams != NULL && Z_TYPE_P(createParams) == IS_ARRAY){
			//获取CConfig对象
			zval	*cconfigInstanceZval,
					*saveParams;
			MAKE_STD_ZVAL(saveParams);
			ZVAL_ZVAL(saveParams,createParams,1,0);
			CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
			CConfig_setConfigs(saveParams,cconfigInstanceZval TSRMLS_CC);
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&saveParams);
		}
	}


	CWebApp_createApp(&object TSRMLS_CC);
	RETVAL_ZVAL(object,1,1);
}
