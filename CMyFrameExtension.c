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
#include "php_CWebApp.h"
#include "php_CConfig.h"
#include "php_CApplication.h"
#include "php_CInitApplication.h"
#include "php_CRequest.h"
#include "php_CHooks.h"
#include "php_CPlugin.h"
#include "php_COpcode.h"

static int le_CMyFrameExtension;

zend_function_entry CMyFrameExtension_functions[] = {
	PHP_FE(CDump,	NULL)
	PHP_FE(CGetServiceExpire,	NULL)
	{NULL, NULL, NULL}
};

zend_module_entry CMyFrameExtension_module_entry = {
	STANDARD_MODULE_HEADER,
	"CMyFrameExtension",
	CMyFrameExtension_functions,
	PHP_MINIT(CMyFrameExtension),
	PHP_MSHUTDOWN(CMyFrameExtension),
	PHP_RINIT(CMyFrameExtension),
	PHP_RSHUTDOWN(CMyFrameExtension),
	PHP_MINFO(CMyFrameExtension),
	"0.1", 
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CMYFRAMEEXTENSION
ZEND_GET_MODULE(CMyFrameExtension)
#endif

//用于缓存php代码文件
#define PHP_CODE_SOURCE "php_codes"
static int source_php_code;
static int source_php_code_persist;
typedef struct _php_source_code_data
{
    char *filename;
    zend_op_array *op_code;
}php_source_code_data;


static void php_CMyFrameExtension_globals_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_source_code_data *fdata = (php_source_code_data*)rsrc->ptr;
    pefree(fdata->op_code,1);
    pefree(fdata->filename, 1);
    pefree(fdata, 1);
}

//模块被加载时
PHP_MINIT_FUNCTION(CMyFrameExtension)
{

	//定义宏
	register_cmyframe_macro(module_number TSRMLS_CC);

	//注册基类
	CMYFRAME_REGISTER_CLASS(CWebApp);
	CMYFRAME_REGISTER_CLASS(CLoader);
	CMYFRAME_REGISTER_CLASS(CConfig);
	CMYFRAME_REGISTER_CLASS(CPlugin);
	CMYFRAME_REGISTER_CLASS(CHooks);
	CMYFRAME_REGISTER_CLASS(CModel);
	CMYFRAME_REGISTER_CLASS(CResult);
	CMYFRAME_REGISTER_CLASS(CActiveRecord);
	CMYFRAME_REGISTER_CLASS(CEmptyModel);
	CMYFRAME_REGISTER_CLASS(CExec);
	CMYFRAME_REGISTER_CLASS(CSession);
	CMYFRAME_REGISTER_CLASS(CInitApplication);
	CMYFRAME_REGISTER_CLASS(CDiContainer);
	CMYFRAME_REGISTER_CLASS(CApplication);
	CMYFRAME_REGISTER_CLASS(CRequset);
	CMYFRAME_REGISTER_CLASS(CRoute);
	CMYFRAME_REGISTER_CLASS(CQuickTemplate);
	//CMYFRAME_REGISTER_CLASS(CSmarty);
	CMYFRAME_REGISTER_CLASS(CView);
	CMYFRAME_REGISTER_CLASS(CController);
	CMYFRAME_REGISTER_CLASS(CLog);
	CMYFRAME_REGISTER_CLASS(CResponse);
	CMYFRAME_REGISTER_CLASS(CDatabase);
	CMYFRAME_REGISTER_CLASS(CBuilder);
	CMYFRAME_REGISTER_CLASS(CDbError);
	CMYFRAME_REGISTER_CLASS(CMemcache);
	CMYFRAME_REGISTER_CLASS(Cache);
	CMYFRAME_REGISTER_CLASS(CacheItem);
	CMYFRAME_REGISTER_CLASS(CRedis);
	CMYFRAME_REGISTER_CLASS(CThread);
	CMYFRAME_REGISTER_CLASS(CPagination);
	CMYFRAME_REGISTER_CLASS(CWatcher);
	CMYFRAME_REGISTER_CLASS(CRabbit);
	CMYFRAME_REGISTER_CLASS(CRabbitMessage);
	CMYFRAME_REGISTER_CLASS(CEncrypt);
	CMYFRAME_REGISTER_CLASS(CCookie);
	CMYFRAME_REGISTER_CLASS(CSmtp);
	CMYFRAME_REGISTER_CLASS(CConsumer);
	CMYFRAME_REGISTER_CLASS(CMonitor);
	CMYFRAME_REGISTER_CLASS(CArrayHelper);
	CMYFRAME_REGISTER_CLASS(CValidate);

	//注册异常类
	CMYFRAME_REGISTER_CLASS(CException);
	CMYFRAME_REGISTER_CLASS(CacheException);
	CMYFRAME_REGISTER_CLASS(CRedisException);
	CMYFRAME_REGISTER_CLASS(CClassNotFoundException);
	CMYFRAME_REGISTER_CLASS(CDbException);
	CMYFRAME_REGISTER_CLASS(CModelException);
	CMYFRAME_REGISTER_CLASS(CPluginException);
	CMYFRAME_REGISTER_CLASS(CRouteException);
	CMYFRAME_REGISTER_CLASS(CSessionCookieException);
	CMYFRAME_REGISTER_CLASS(CViewException);
	CMYFRAME_REGISTER_CLASS(CShellException);
	CMYFRAME_REGISTER_CLASS(CHttpException);
	CMYFRAME_REGISTER_CLASS(CMailException);
	CMYFRAME_REGISTER_CLASS(CQueueException);

	//覆盖zend_compile_file方法 添加opcode缓存
    old_compile_file = zend_compile_file;
    zend_compile_file = my_compile_file;
	
	//定义资源
	source_php_code = zend_register_list_destructors_ex(NULL, NULL, PHP_CODE_SOURCE,module_number);
	source_php_code_persist =zend_register_list_destructors_ex(NULL, php_CMyFrameExtension_globals_dtor,PHP_CODE_SOURCE, module_number);



	return SUCCESS;
}



static zend_op_array* my_compile_file(zend_file_handle* h,int type TSRMLS_DC)
{
	
	zend_op_array			*op_array = NULL;
	const char				*filename = NULL;
	char					*saveFileName;
	char					*hash_key;
    int						hash_key_len;

	
	if(h->opened_path) {
        filename = h->opened_path;
    } else {
        filename = h->filename;
    }


	op_array = old_compile_file(h,type TSRMLS_CC);

	php_printf("<pre>%s<br>",filename);
	dump_op_array(op_array);
	php_printf("<br>======================================================<br>");

	return op_array;
}


PHP_MSHUTDOWN_FUNCTION(CMyFrameExtension)
{
#ifndef ZTS
    php_CMyFrameExtension_globals_dtor(&CMyFrameExtension_globals TSRMLS_CC);
#endif
	return SUCCESS;
}

PHP_RINIT_FUNCTION(CMyFrameExtension)
{
	//记录框架启动时间
	register_cmyframe_begin(module_number TSRMLS_CC);

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(CMyFrameExtension)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(CMyFrameExtension)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "CMyFrameExtension 3.0 support", "enabled");
	php_info_print_table_end();
}

//调试打印函数
PHP_FUNCTION(CDump)
{
	zval *arg = NULL;
	int argLen;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg, &argLen) == FAILURE) {
		RETURN_FALSE;
		return;
	}

	php_printf("<pre>");
	php_var_dump(&arg, 1 TSRMLS_CC);
}

//获取CPUID
PHP_FUNCTION(CGetServiceExpire)
{
	/*char *limitTime;
	getServiceExpire(&limitTime);
	ZVAL_STRING(return_value,limitTime,1);
	efree(limitTime);
	*/
}
