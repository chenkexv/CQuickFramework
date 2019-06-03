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
#include "php_CPlugin.h"


//zend类方法
zend_function_entry CPlugin_functions[] = {
	PHP_ME(CPlugin,getView,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CPlugin)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CPlugin",CPlugin_functions);
	CPluginCe = zend_register_internal_class(&funCe TSRMLS_CC);

	return SUCCESS;
}

//类方法:创建应用对象
PHP_METHOD(CPlugin,getView)
{
	zval	*viewObjectZval,
			*cconfigInstanceZval,
			*useQuickTemplate;
	char	*templateUsed;

	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);	
	RETVAL_ZVAL(viewObjectZval,1,1);
}
