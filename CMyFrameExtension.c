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
#include "php_CDebug.h"
#include "php_CGuardController.h"

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

//set php.ini items
PHP_INI_BEGIN()
	PHP_INI_ENTRY("CMyFrameExtension.open_trace","0",PHP_INI_ALL,NULL)
	PHP_INI_ENTRY("CMyFrameExtension.open_shell_check","1",PHP_INI_ALL,NULL)
PHP_INI_END()


//hooks to get function call strace
static int frameworkDoCall(zend_execute_data *execute_data TSRMLS_DC)
{

	char	*funname,
			*class_name,
			*space,
			*filePath;

	zval	*timenow,
			*thisSave;

	zval	*traceSave;

	int		isDebug = 0;

	isDebug = CDebug_getIsDebugStats(TSRMLS_C);
	if(isDebug == 0){
		return ZEND_USER_OPCODE_DISPATCH; 
	}
	
	//get save
	traceSave = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(traceSave)){
		zval	*save;
		MAKE_STD_ZVAL(save);
		array_init(save);
		zend_update_static_property(CDebugCe,ZEND_STRL("functionTrace"),save TSRMLS_CC);
		zval_ptr_dtor(&save);
		traceSave = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);
	}

	microtime(&timenow);
	class_name = get_active_class_name(&space TSRMLS_CC);
	funname = get_active_function_name(TSRMLS_C);

	filePath = zend_get_executed_filename(TSRMLS_C);

	if(strstr(filePath,".lo") != NULL){
		zval_ptr_dtor(&timenow);
		return ZEND_USER_OPCODE_DISPATCH; 
	}

	MAKE_STD_ZVAL(thisSave);
	array_init(thisSave);

	add_assoc_double(thisSave,"time",Z_DVAL_P(timenow));
	add_assoc_string(thisSave,"class_name",class_name,1);
	add_assoc_string(thisSave,"function_name",funname,1);
	add_assoc_string(thisSave,"file_name",filePath,1);
	add_assoc_long(thisSave,"function_type",execute_data->function_state.function->type);
	add_next_index_zval(traceSave,thisSave);
	
	zval_ptr_dtor(&timenow);

    return ZEND_USER_OPCODE_DISPATCH; 
}


void frameworkDoInternalCall(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC)
{
	char	*funcname;
	char	*filename,
			*class_name,
			*space,
			*disableFunction,
			*shell_check;


	zval	*traceSave,
			*thisSave,
			*timenow,
			*disableArray;


	if (EG(current_execute_data) && EG(current_execute_data)->op_array) {
		filename = estrdup(EG(current_execute_data)->op_array->filename);
    } else {
		filename = estrdup(zend_get_executed_filename(TSRMLS_C));
    }

	class_name = get_active_class_name(&space TSRMLS_CC);
	funcname = get_active_function_name(TSRMLS_C);

	ini_get("CMyFrameExtension.open_shell_check",&shell_check);
	if(strcmp(shell_check,"1") == 0){

		//"exec,passthru,phpinfo,popen,proc_open,shell_exec,system,parse_ini_file,show_source,assert,pcntl_exec,proc_get_status,dl,putenv";
		if(
			strcmp(funcname,"system") == 0 || 
			strcmp(funcname,"exec") == 0 || 
			strcmp(funcname,"passthru") == 0 || 
			strcmp(funcname,"phpinfo") == 0 || 
			strcmp(funcname,"popen") == 0 || 
			strcmp(funcname,"proc_open") == 0 || 
			strcmp(funcname,"shell_exec") == 0 || 
			strcmp(funcname,"parse_ini_file") == 0 || 
			strcmp(funcname,"show_source") == 0 || 
			strcmp(funcname,"assert") == 0 || 
			strcmp(funcname,"pcntl_exec") == 0 || 
			strcmp(funcname,"proc_get_status") == 0 || 
			strcmp(funcname,"putenv") == 0 || 
			strcmp(funcname,"dl") == 0
		){
			char	errorTips[10240],
					errorPath[1024],
					errorFile[1024],
					*savePath,
					*thisMothTime;
			zval	*appPath;

			//tips content
			php_date("Y-m-d H:i:s",&thisMothTime);
			sprintf(errorTips,"%s%s%s%s%s%s%s","#",thisMothTime,"Detection of possible attacks,exception function calls :",funcname," in files:",filename,"\n\n");
			efree(thisMothTime);

			//save Path
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			if(Z_STRLEN_P(appPath) <= 0){
				//not use frame and now create a web app and init env params
				zval	*object;
				CWebApp_createAppFromFramework(&object TSRMLS_CC);
				zval_ptr_dtor(&object);
			}


			savePath = estrdup(Z_STRVAL_P(appPath));
			sprintf(errorPath,"%s%s",savePath,"/logs/safe/");
			efree(savePath);

			if(FAILURE == fileExist(errorPath)){
				//尝试创建文件夹
				php_mkdir(errorPath);
			}
			sprintf(errorFile,"%s%s%s",errorPath,funcname,".log");
			CLog_writeFile(errorFile,errorTips TSRMLS_CC);

			//call hooks
			MODULE_BEGIN
				zval	*paramsList[1],
						param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_STRING(paramsList[0],funcname,1);
				CHooks_callHooks("HOOKS_SAFE_STOP",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END
			
			//call CGuardController to check send warn Mail
			CGuardController_safeStopWarn(funcname TSRMLS_CC);

			//stop the webapp
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CMyFrameSafeException] The system has blocked these behaviors");
			efree(shell_check);
			efree(filename);
			return;
		}
	}
	efree(shell_check);

	//read php.ini's CMyFrameExtension.open_trace
	MODULE_BEGIN
		int isDebug = 0;
		isDebug = CDebug_getIsDebugStats(TSRMLS_C);
		if(isDebug == 1 && strstr(filename,".lo") == NULL){
			//get save
			traceSave = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);
			if(IS_NULL == Z_TYPE_P(traceSave)){
				zval	*save;
				MAKE_STD_ZVAL(save);
				array_init(save);
				zend_update_static_property(CDebugCe,ZEND_STRL("functionTrace"),save TSRMLS_CC);
				zval_ptr_dtor(&save);
				traceSave = zend_read_static_property(CDebugCe,ZEND_STRL("functionTrace"),0 TSRMLS_CC);
			}
			microtime(&timenow);
			MAKE_STD_ZVAL(thisSave);
			array_init(thisSave);
			add_assoc_double(thisSave,"time",Z_DVAL_P(timenow));
			add_assoc_string(thisSave,"class_name",class_name,1);
			add_assoc_string(thisSave,"function_name",funcname,1);
			add_assoc_string(thisSave,"file_name",filename,1);
			add_assoc_long(thisSave,"function_type",execute_data_ptr->function_state.function->type);
			add_next_index_zval(traceSave,thisSave);
			zval_ptr_dtor(&timenow);
		}
	MODULE_END

	efree(filename);

	execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
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
	CMYFRAME_REGISTER_CLASS(CDataObject);
	CMYFRAME_REGISTER_CLASS(CModel);
	CMYFRAME_REGISTER_CLASS(CResult);
	CMYFRAME_REGISTER_CLASS(CActiveRecord);
	CMYFRAME_REGISTER_CLASS(CEmptyModel);
	CMYFRAME_REGISTER_CLASS(CExec);
	CMYFRAME_REGISTER_CLASS(CSession);
	CMYFRAME_REGISTER_CLASS(CDebug);
	CMYFRAME_REGISTER_CLASS(CInitApplication);
	CMYFRAME_REGISTER_CLASS(CDiContainer);
	CMYFRAME_REGISTER_CLASS(CApplication);
	CMYFRAME_REGISTER_CLASS(CRequset);
	CMYFRAME_REGISTER_CLASS(CRoute);
	CMYFRAME_REGISTER_CLASS(CQuickTemplate);
	CMYFRAME_REGISTER_CLASS(CSmarty);
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
	CMYFRAME_REGISTER_CLASS(CRabbitHelper);
	CMYFRAME_REGISTER_CLASS(CRabbitMessage);
	CMYFRAME_REGISTER_CLASS(CEncrypt);
	CMYFRAME_REGISTER_CLASS(CCookie);
	CMYFRAME_REGISTER_CLASS(CSmtp);
	CMYFRAME_REGISTER_CLASS(CConsumer);
	CMYFRAME_REGISTER_CLASS(CMonitor);
	CMYFRAME_REGISTER_CLASS(CArrayHelper);
	CMYFRAME_REGISTER_CLASS(CValidate);
	CMYFRAME_REGISTER_CLASS(CHash);
	CMYFRAME_REGISTER_CLASS(CSecurityCode);
	CMYFRAME_REGISTER_CLASS(CFtp);
	CMYFRAME_REGISTER_CLASS(CTime);
	CMYFRAME_REGISTER_CLASS(CWord);
	CMYFRAME_REGISTER_CLASS(CImage);

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
	CMYFRAME_REGISTER_CLASS(CFtpException);

	//controller
	CMYFRAME_REGISTER_CLASS(CGuardController);
	CMYFRAME_REGISTER_CLASS(CCrontabController);

	//regsiter ini config
	REGISTER_INI_ENTRIES();

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(CMyFrameExtension)
{

	//unregister ini config
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}

PHP_RINIT_FUNCTION(CMyFrameExtension)
{
	zval	*sapiZval;

	//记录框架启动时间
	register_cmyframe_begin(module_number TSRMLS_CC);

	//cli not save calltrace
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{

		//Hooks zend_do_call ZEND_INIT_METHOD_CALL
		zend_set_user_opcode_handler(ZEND_DO_FCALL, frameworkDoCall);

		//hooks internal function run
		zend_execute_internal = frameworkDoInternalCall;
	}

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

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &arg) == FAILURE) {
		RETURN_FALSE;
		return;
	}

	php_printf("<pre>");
	php_var_dump(&arg, 1 TSRMLS_CC);
}

//获取CPUID
PHP_FUNCTION(CGetServiceExpire)
{

}