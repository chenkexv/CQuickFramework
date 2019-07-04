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

static int le_CQuickFramework;
static int le_resource_httpPool;


zend_function_entry CQuickFramework_functions[] = {
	PHP_FE(CDump,	NULL)
	PHP_FE(CQF_help,NULL)
	PHP_FE(CQF_createProject,NULL) //create a empty project
	PHP_FE(CQF_createPlugin,NULL) //create a plugin demo
	PHP_FE(CQF_createConsumer,NULL) //create a consumer demo
	PHP_FE(CQF_dumpClassMapForIDE,NULL)
	{NULL, NULL, NULL}
};

zend_module_entry CQuickFramework_module_entry = {
	STANDARD_MODULE_HEADER,
	"CQuickFramework",
	CQuickFramework_functions,
	PHP_MINIT(CQuickFramework),
	PHP_MSHUTDOWN(CQuickFramework),
	PHP_RINIT(CQuickFramework),
	PHP_RSHUTDOWN(CQuickFramework),
	PHP_MINFO(CQuickFramework),
	"0.1", 
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CQUICKFRAMEWORK
	ZEND_GET_MODULE(CQuickFramework)
#endif

//set php.ini items
PHP_INI_BEGIN()
	PHP_INI_ENTRY("CQuickFramework.open_trace","0",PHP_INI_ALL,NULL)
	PHP_INI_ENTRY("CQuickFramework.open_shell_check","1",PHP_INI_ALL,NULL)
PHP_INI_END()


//hooks to get function call strace
static int frameworkDoCall(zend_execute_data *execute_data TSRMLS_DC)
{

	const   char  *class_name = "",
				  *space = "",
				  *funname,
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
	add_assoc_string(thisSave,"class_name",estrdup(class_name),0);
	add_assoc_string(thisSave,"function_name",estrdup(funname),0);
	add_assoc_string(thisSave,"file_name",estrdup(filePath),0);
	add_assoc_long(thisSave,"function_type",execute_data->function_state.function->type);
	add_next_index_zval(traceSave,thisSave);
	
	zval_ptr_dtor(&timenow);

    return ZEND_USER_OPCODE_DISPATCH; 
}


void frameworkDoInternalCall(zend_execute_data *execute_data_ptr, int return_value_used TSRMLS_DC)
{
	const char	*funcname;
	char	*filename,
			*disableFunction,
			*shell_check;

	const char	*class_name = "",
				*space = "";

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

	ini_get("CQuickFramework.open_shell_check",&shell_check);
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
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CQuickFrameworkSafeException] The system has blocked these behaviors");
			efree(shell_check);
			efree(filename);
			return;
		}
	}
	efree(shell_check);

	//read php.ini's CQuickFramework.open_trace
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
			add_assoc_string(thisSave,"class_name",estrdup(class_name),0);
			add_assoc_string(thisSave,"function_name",estrdup(funcname),0);
			add_assoc_string(thisSave,"file_name",estrdup(filename),0);
			add_assoc_long(thisSave,"function_type",execute_data_ptr->function_state.function->type);
			add_next_index_zval(traceSave,thisSave);
			zval_ptr_dtor(&timenow);
		}
	MODULE_END

	efree(filename);

	execute_internal(execute_data_ptr, return_value_used TSRMLS_CC);
}


//模块被加载时
PHP_MINIT_FUNCTION(CQuickFramework)
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
	CMYFRAME_REGISTER_CLASS(CRedisMessage);
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
	CMYFRAME_REGISTER_CLASS(CHashTable);
	CMYFRAME_REGISTER_CLASS(CString);
	CMYFRAME_REGISTER_CLASS(CValidate);
	CMYFRAME_REGISTER_CLASS(CHash);
	CMYFRAME_REGISTER_CLASS(CSecurityCode);
	CMYFRAME_REGISTER_CLASS(CFtp);
	CMYFRAME_REGISTER_CLASS(CDate);
	CMYFRAME_REGISTER_CLASS(CWord);
	CMYFRAME_REGISTER_CLASS(CImage);
	CMYFRAME_REGISTER_CLASS(CTree);
	CMYFRAME_REGISTER_CLASS(CFile);
	CMYFRAME_REGISTER_CLASS(CHttpResponse);
	CMYFRAME_REGISTER_CLASS(CHttp);

#ifndef PHP_WIN32
	CMYFRAME_REGISTER_CLASS(CMicroServer);
	CMYFRAME_REGISTER_CLASS(CMicroRequest);
	CMYFRAME_REGISTER_CLASS(CMicroResponse);
	CMYFRAME_REGISTER_CLASS(CHttpPool);
#endif

#if CQUICKFRAMEWORK_USE_QRENCODE
	CMYFRAME_REGISTER_CLASS(CQrcode);
#endif


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
	CMYFRAME_REGISTER_CLASS(CMicroServerException);
	CMYFRAME_REGISTER_CLASS(CRejectException);

	//controller
	CMYFRAME_REGISTER_CLASS(CGuardController);
	CMYFRAME_REGISTER_CLASS(CCrontabController);

	//regsiter ini config
	REGISTER_INI_ENTRIES();

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(CQuickFramework)
{

	//unregister ini config
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}

PHP_RINIT_FUNCTION(CQuickFramework)
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

PHP_RSHUTDOWN_FUNCTION(CQuickFramework)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(CQuickFramework)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "CQuickFramework 3.0 support", "enabled");
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
	php_printf("</pre>");
}


//show all command list
void CQuickFramework_showCommandList(){
	php_printf("\nother command list:\n");
	php_printf("\tphp -r \"CQF_createProject();\";\n");
	php_printf("\tphp -r \"CQF_createConsumer('baseConsumer');\";\n");
	php_printf("\tphp -r \"CQF_createPlugin('basePlugin');\";\n");
	php_printf("\tphp -r \"CQF_dumpClassMapForIDE();\";\n");
	php_printf("\n");
}

//create a emtpy string
PHP_FUNCTION(CQF_createProject)
{
	
	zval	*sapiZval,
			*dirList,
			**nowDir;

	char	*workPath;

	int		i,h;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		return;
	}

	//get now work dir , use pwd
	exec_shell_return("pwd",&workPath);

	//dir list
	MAKE_STD_ZVAL(dirList);
	array_init(dirList);
	add_next_index_string(dirList,"application/controllers",1);
	add_next_index_string(dirList,"application/classes",1);
	add_next_index_string(dirList,"application/configs",1);
	add_next_index_string(dirList,"application/models",1);
	add_next_index_string(dirList,"application/views",1);
	add_next_index_string(dirList,"application/languages",1);
	add_next_index_string(dirList,"__runtime",1);
	add_next_index_string(dirList,"logs",1);
	add_next_index_string(dirList,"data",1);
	add_next_index_string(dirList,"plugins",1);
	add_next_index_string(dirList,"vendors",1);
	

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirList));
	h = zend_hash_num_elements(Z_ARRVAL_P(dirList));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(dirList),(void**)&nowDir);
		if(SUCCESS == fileExist(Z_STRVAL_PP(nowDir))){
			php_printf("create dir [%s] : ignore\n",Z_STRVAL_PP(nowDir));
		}else{
			php_mkdir(Z_STRVAL_PP(nowDir));
			php_printf("create dir [%s] : success\n",Z_STRVAL_PP(nowDir));
		}
		zend_hash_move_forward(Z_ARRVAL_P(dirList));
	}

	//change dir chmod
	exec_shell("chown apache:apache __runtime");
	exec_shell("chown apache:apache logs");
	exec_shell("chown apache:apache data");


	//create index file
	if(SUCCESS != fileExist("index.php")){
		char	*fileContent = "PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqLwoKLy9TZXQgQXBwbGljYXRpb24gUGF0aAppZighZGVmaW5lZCgnQVBQX1BBVEgnKSkJCQlkZWZpbmUoJ0FQUF9QQVRIJywgZGlybmFtZShfX0ZJTEVfXykpOwppZighZGVmaW5lZCgnQ09ERV9QQVRIJykpCQkJZGVmaW5lKCdDT0RFX1BBVEgnLCBBUFBfUEFUSC4nL2FwcGxpY2F0aW9uJyk7CgovL0JlZ2luIHRvIGdldCBSZXF1ZXN0CkNXZWJBcHA6OmNyZWF0ZUFwcCgpLT5HZXRSZXF1ZXN0KCk7Cg==";
		char	*fileCode;
		base64Decode(fileContent,&fileCode);
		file_put_contents("index.php",fileCode);
		efree(fileCode);
		php_printf("create file [%s] : success\n","index.php");
	}else{
		php_printf("create file [%s] : ignore\n","index.php");
	}

	if(SUCCESS != fileExist("application/controllers/base.php")){
		char	*fileContent = "PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqLwoKY2xhc3MgYmFzZSBleHRlbmRzIENDb250cm9sbGVyewoJCglwdWJsaWMgZnVuY3Rpb24gQWN0aW9uX2luZGV4KCl7CgkJZWNobyAnaGVsbG8gQ1F1aWNrRnJhbWV3b3JrJzsKCX0KfQ==";
		char	*fileCode;
		base64Decode(fileContent,&fileCode);
		file_put_contents("application/controllers/base.php",fileCode);
		efree(fileCode);
		php_printf("create file [%s] : success\n","application/controllers/base.php");
	}else{
		php_printf("create file [%s] : ignore\n","application/controllers/base.php");
	}

	if(SUCCESS != fileExist("application/configs/main.php")){
		char	*fileContent = (char*)emalloc(sizeof(char)*9200);
		char	*fileCode;

		//the strlen is 9184
		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqLwoKcmV0dXJuIGFycmF5KAoKICAgIC8qKgogICAgICog5pWw5o2u5bqT6YWN572uICDpu5jorqTphY3nva7plK7kuLogbWFpbgogICAgICog6LCD55SoIENEYXRhYmFzZTo6Z2V0SW5zdGFuY2UoKSDml7bpu5jorqTojrflj5ZtYWlu55qE6YWN572uCiAgICAgKiDlj6/phY3nva5O5LiqIOS7pSBDRGF0YWJhc2U6OmdldEluc3RhbmNlKCdkYjInKSDlvaLlvI/ojrflj5YKICAgICAqLwogICAgJ0RCJz0+YXJyYXkoCgogICAgICAgICAgICAnbWFpbic9PmFycmF5KAogICAgICAgICAgICAgICAgICAgICdzbGF2ZXMnPT5hcnJheSgKICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjb25uZWN0aW9uU3RyaW5nJyA9PiAnbXlzcWw6aG9zdD0xMjcuMC4wLjE7ZGJuYW1lPXRlc3QnLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgJ21hc3RlclJlYWQnID0+IGZhbHNlLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgJ3VzZXJuYW1lJyA9PiAneHh4eCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAncGFzc3dvcmQnID0+ICd4eHh4JywKICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjaGFyc2V0JyA9PiAndXRmOCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAndGFibGVQcmVmaXgnID0+ICd4eF8nLAogICAgICAgICAgICAgICAgICAgICksCiAgICAgICAgICAgICAgICAgICAgJ21hc3Rlcic9PmFycmF5KAogICAgICAgICAgICAg"
			"ICAgICAgICAgICAgICAgJ2Nvbm5lY3Rpb25TdHJpbmcnID0+ICdteXNxbDpob3N0PTEyNy4wLjAuMTtkYm5hbWU9dGVzdCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAnc2xhdmVzV3JpdGUnID0+IGZhbHNlLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgJ3VzZXJuYW1lJyA9PiAneHh4eCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAncGFzc3dvcmQnID0+ICd4eHh4JywKICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjaGFyc2V0JyA9PiAndXRmOCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAndGFibGVQcmVmaXgnID0+ICd4eF8nLAogICAgICAgICAgICAgICAgICAgICksCiAgICAgICAgICAgICksCiAgICApLAogICAgCiAgICAvKioKICAgICAqIOinhuWbvuebuOWFs+mFjee9rgogICAgICog5qGG5p625L2/55SoQ+ivreiogOWunueOsOS6huS4gOS4que8luivkeWei+inhuWbvuW8leaTjizmraTop4blm77mlK/mjIHnu53lpKfpg6jliIZzbWFydHnor63oqIAu5L2/55Soc21hcnR555qE5qih5p2/5Y+v55u05o6l56e75qSNLgogICAgICogVVNFX1FVSUNLVEVNUExBVEUg5Li6IDHml7bkvb/nlKjns7vnu5/oh6rluKblvJXmk44uCiAgICAgKi8KICAgICdVU0VfUVVJQ0tURU1QTEFURScgPT4gMSwKICAgICdURU1QTEFURSc9PmFycmF5KAogICAgICAgICAgICAKICAgICAgICAgICAgLy9zbWFydHnmqKHmnb8KICAgICAgICAgICAgJ3NtYXJ0eScgPT4gYXJyYXkoCiAgICAgICAgICAgICAgICAgICAgJ1RFTVBMQVRFX1BBVEgnPT4gQVBQX1BBVEguJy92ZW5kb3JzL3NtYXJ0eS9saWJzL1NtYXJ0eS5jbGFzcy5waHAnLAogICAgICAgICAgICAgICAgICAgICdDT05GX0lORk8nPT5hcnJheSgKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAndG"
			"VtcGxhdGVfZGlyJyA9PiBBUFBfUEFUSC4nL2FwcGxpY2F0aW9uL3ZpZXdzL2RlZmF1bHQnLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjb21waWxlX2RpcicgPT4gQVBQX1BBVEguJy9fX3J1bnRpbWUvc2NyaXB0JywKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAnY2FjaGVfZGlyJyA9PiBBUFBfUEFUSC4nL19fcnVudGltZS9odG1sJywKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAnbGVmdF9kZWxpbWl0ZXInID0+ICc8eycsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJ3JpZ2h0X2RlbGltaXRlcicgPT4gJ30+JywKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAnYWxsb3dfcGhwX3RhZycgPT4gdHJ1ZSwKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAnY2FjaGluZyc9PmZhbHNlLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjYWNoZV9saWZldGltZSc9PjEwMCwKICAgICAgICAgICAgICAgICAgICApCiAgICAgICAgICAgICksCiAgICAgICAgICAgIAogICAgICAgICAgICAnQ1F1aWNrVGVtcGxhdGUnID0+IGFycmF5KAogICAgICAgICAgICAgICAgICAgICdDT05GX0lORk8nPT5hcnJheSgKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAndGVtcGxhdGVfZGlyJyA9PiBBUFBfUEFUSC4nL2FwcGxpY2F0aW9uL3ZpZXdzL2RlZmF1bHQnLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICdjb21waWxlX2RpcicgPT4gQVBQX1BBVEguJy9fX3J1bnRpbWUvc2NyaXB0JywgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJ2NhY2hlX2RpcicgPT4gQVBQX1BBVEguJy9fX3J1bnRpbWUvaHRtbCcsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJ2xlZnRfZGVsaW1pdGVyJyA9PiAnPHsnLAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICdyaWdodF9kZWxpbWl0ZXInID0+ICd9PicsCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgJ2NhY2hpbmcnPT5mYWxzZSwKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAnY2FjaGVfbGlmZXRpbWUnPT4xMDAsCiAgICAgICAgICAgI"
			"CAgICAgICAgKQogICAgICAgICAgICApLAogICAgICAgICAgICAKICAgICksCiAgICAKICAgIC8qKgogICAgICog6Lev55Sx6YeN5YaZCiAgICAgKiBPUEVOIOaYr+WQpuW8gOmHjeWGmQogICAgICogVFlQRSDpu5jorqTnmoTph43lhpnop4TliJks5Y2z5aaC5p6c6YeN5YaZ6KeE5YiZ5LiN5ZCr5pyJ5p+QVVJMLOS9huahhuaetuWNtOWPiOW8gOWQr+mHjeWGmeaXtueahOm7mOiupOinhOWImSww5Li6aW5kZXgucGhwP2E9MSZiPTI7MeS4uiBhLzEvYi8yIOagvOW8jzsy5Li6IGEtMS1iLTIuaHRtbOagvOW8jzsKICAgICAqLwogICAgJ1VSTFJld3JpdGUnPT5hcnJheSgKICAgICAgICAgICAgJ09QRU4nID0+ICdvbicsCiAgICAgICAgICAgICdUWVBFJyA9PiAxLAogICAgICAgICAgICAnTElTVCcgPT4gYXJyYXkoCiAgICAgICAgICAgICAgICAgICAgJ2xvZ2luLmh0bWwnID0+ICdiYXNlQGxvZ2luJywKICAgICAgICAgICAgICAgICAgICAnc3BhY2UvPHVzZXJuYW1lOlxcdys+Lmh0bWwnID0+J3NwYWNlQGRldGFpbCcsCiAgICAgICAgICAgICAgICAgICAgJ25ld3MtPGlkOlxcZCs+Lmh0bWwnID0+ICduZXdzQGRldGFpbCcsCiAgICAgICAgICAgICksCiAgICApLAogICAgCiAgICAvKioKICAgICAqIOazqOWGjOWIsOezu+e7n+exu+ebruW9lSDmraTnm67lvZXkuIvnmoTnsbvoh6rliqjovb3lhaUgCiAgICAgKi8KICAgICdJTVBPUlQnPT5hcnJheSgKICAgICAgICAgICAgJ2FwcGxpY2F0aW9uLm1vZGVscy4qJywKICAgICAgICAgICAgJ2FwcGxpY2F0aW9uLmNsYXNzZXMuKicsCiAgICApLAogICAgCiAgICAvKioKICAgICAqIHNlc2lvbiBjb29raWVw6YWN572uCiAgICAgKiBBdXRvU2Vzc2lvbuS4unRydWXml7Yg5bCG6Ieq5Yqo5byA5ZCvc2Vzc2lvbiAKICAgICAqIHNlc3Npb27mmK/m"
			"noHlpKfpmY3kvY7ns7vnu5/mlYjnjocsQVBJ5LmL57G755qE57O757uf5bqU5YWz6Zetc2Vzc2lvbgogICAgICovIAogICAgJ0FVVE9fU0VTU0lPTicgICAgICAgICAgICAgICAgICAgICAgPT4gZmFsc2UsCiAgICAnU0VTU0lPTl9NRU1DQUNIRScgICAgICAgICAgICAgICAgICA9PiBmYWxzZSwKICAgICdTRVNTSU9OX01FTUNBSENFX0hPU1QnID0+ICcxMjcuMC4wLjE6MTEyMTEnLAogICAgCiAgICAvKioKICAgICAqIOiwg+ivlemFjee9rgogICAgICogZGVidWfmiZPlvIDml7Yg5bCG5oql6ZSZ6ZSZ6K+v55WM6Z2iLOW5tuiusOW9lXdhcm7jgIFub3RpY2XnrYnkv6Hmga8KICAgICAqIOivt+axgklQ5L2N5LqOREVCVUdfSVDkuK3ml7YsIOWPr+mZhOWKoGdldOWPguaVsGRlYnVnIOaYvuekuuS/oeaBr+iwg+ivleS/oeaBrwogICAgICogY2xvc2VfZGVidWdlciDpu5jorqTkuLpmYWxzZSzorr7kuLp0cnVl5pe2LOezu+e7n+S4jeWGjeWKoOi9veS9v+eUqEPmianlsZXnvJblhpnnmoREZWJ1Z+aPkuS7tgogICAgICovCiAgICAnREVCVUcnICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgPT4gdHJ1ZSwKICAgICdERUJVR19JUCcgICAgICAgICAgICAgICAgICAgICAgICAgICA9PiBhcnJheSgnMTAuMC4wLjEnLCcxMjcuMC4wLjEnKSwKICAgICdDTE9TRV9ERUJVR0VSJyAgICAgICAgICAgICAgICAgICAgICA9PiBmYWxzZSwKICAgIAogICAgLyoqCiAgICAgKiDmj5Lku7bphY3nva4gCiAgICAgKiBsb2FkX3BsdWdpbuS4unRydWXml7Yg57O757uf5bCG5Yqg6L295L2N5LqOcGx1Z2luX3BhdGjkuIvnmoTlkIjms5Xmj5Lku7YKICAgICAqLwogICAgJ0xPQURfUExVR0lOJyAgICAgICAgICAgICAgICAgICAgICAgID0+IHRydWUsCiAgICAnUExVR0lOX1BBVEgnICAgICAgICAgICAgICAgICAgICAgICAgPT4gJ3BsdWdpbnMnLAogICAgCiAgICAKICAgIC8qKgogICAgICogR1pJUOmFjee9riDpu5jorqTlhbPpl60KICAgICAqIGd6aXAg5biD5bCU5Z6LLOaYr+WQpuW8gOWQrwogICAgICogR1pJUF9MRVZFTCDmlbDlrZcxLTEwIOWOi+e8qee6p+WIqwogI"
			"CAgICog6YCa5bi45ZyobmdpbnjlvIDlkK/lkI4g5LiN6ZyA5Zyo56iL5bqP5Lit5byA5ZCvR1pJUAogICAgICovCiAgICAnR1pJUCcgICAgICAgICAgICAgICAgICAgICAgICAgICAgICA9PiB0cnVlLAogICAgJ0daSVBfTEVWRUwnICAgICAgICAgICAgICAgICAgICAgICAgPT4gNiwKICAgIAogICAgCiAgICAvKioKICAgICAqIOm7mOiupOi3r+eUsemFjee9rgogICAgICogZGVmYXVsdF9jb250cm9sbGVyIOmmlumhteaOp+WItuWZqAogICAgICogZGVmYXVsdF9hY3Rpb24g6aaW6aG15pa55rOVCiAgICAgKiBhY3Rpb25fcHJlZml4IOaOp+WItuWZqOWJjee8gAogICAgICogdXNlX21vZHVsZSDmmK/lkKblkK/liqjlpJrmqKHlnZcg5a2Q5Z+f5ZCN6aG555uuCiAgICAgKiBkZWZhdWx0X2luZGV4IOeUn+aIkOi3r+eUseWcsOWdgOaXtizpu5jorqTnlJ/miJDnmoTpppbpobXmoLflvI8KICAgICAqIGFsbG93X2luZGV4IOWFgeiuuOmAmui/h+eahOmmlumhteWcsOWdgCwKICAgICAqLwogICAgJ0RFRkFVTFRfQ09OVFJPTExFUicgICAgICAgICAgICAgID0+ICd3ZWJzaXRlJywgICAgICAgIAogICAgJ0RFRkFVTFRfQUNUSU9OJyAgICAgICAgICAgICAgICAgID0+ICdpbmRleCcsCiAgICAnQUNUSU9OX1BSRUZJWCcgICAgICAgICAgICAgICAgICAgPT4gJ0FjdGlvbl8nLAogICAgJ0RFRkFVTFRfTU9ETFVFJyAgICAgICAgICAgICAgICAgID0+ICd0ZXN0JywKICAgICdVU0VfTU9EVUxFJyAgICAgICAgICAgICAgICAgICAgICA9PiBmYWxzZSwKICAgICdERUZBTFVUX0lOREVYJyAgICAgICAgICAgICAgICAgICA9PiAnaW5kZXgnLCAgICAgICAgCiAgICAnQUxMT1dfSU5ERVgnICAgICAgICAgICAgICAgICAgICAgPT4gYXJyYXkoJ2luZGV4Lmh0bWwnLCdpbmRleC5hc3AnLCdpbmRleCcpLC"
			"AgICAgICAgCiAgICAKICAgIC8qKgogICAgICog5p2C6aG56YWN572uCiAgICAgKiDlrZfnrKbpm4blkozml7bljLoKICAgICAqLwogICAgJ0NIQVJTRVQnICAgICAgICAgICAgICAgICAgICAgICAgICA9PiAndXRmLTgnLCAgICAgICAgCiAgICAnVElNRV9aT05FJyAgICAgICAgICAgICAgICAgICAgICAgID0+ICdQUkMnLAogICAgCiAgICAKICAgIC8qKgogICAgICogUmVkaXPphY3nva4KICAgICAqIHJlZGlzX2hvc3QgcmVkaXNwb3N0IHJlZGlzcGFzc3dvcmQg5a6a5LmJ5L2/55So55qEcmVkaXMg5a+55bqUQ1JlZGlzOjpnZXRJbnN0YW5jZSgpCiAgICAgKiDlpJrkuKpyZWRpc+WPr+mFjee9ruWcqHJlZGlzX3NlcnZlcuS4rSDlr7nlupRDUmVkaXM6OmdldEluc3RhbmNlKCdyZWRpczInKQogICAgICog5rKh5pyJ5a+G56CB5pe2IOaXoOmcgOmFjee9rnJlZGlzX3Bhc3N3b3Jk6ZSu5Y2z5Y+vCiAgICAgKi8KICAgICdSRURJU19IT1NUJyAgICAgICAgICAgICAgICAgICAgICAgPT4gJzEyNy4wLjAuMScsCiAgICAnUkVESVNfUE9SVCcgICAgICAgICAgICAgICAgICAgICAgID0+IDYzNzksCiAgICAnUkVESVNfUEFTU1dPUkQnICAgICAgICAgICAgICAgICAgID0+ICd4eHh4JywKICAgICdSRURJU19USU1FT1VUJyAgICAgICAgICAgICAgICAgICAgPT4zLAogICAgJ1JFRElTX1NFUlZFUicgICAgICAgICAgICAgICAgICAgICA9PiBhcnJheSgKICAgICAgICAgICAgICdyZWRpczInID0+IGFycmF5KCdob3N0Jz0+JzEyNy4"
			"wLjAuMScsJ3BvcnQnPT42Mzc5LCd0aW1lb3V0Jz0+MywncGFzc3dyb2QnPT4neHh4eCcpLAogICAgICksCiAgICAKICAgIC8qKgogICAgICog5raI5oGv6Zif5YiX6YWN572uCiAgICAgKiDlpJrkuKrpmJ/liJfnlKjlpJrkuKrplK7phY3nva4KICAgICAqIOm7mOiupOS4um1haW4g5a+55bqUIENSYWJiaXQ6OmdldEluc3RhbmNlKCk7CiAgICAgKiDlhbbku5bnmoRVSeWvueW6lCAgQ1JhYmJpdDo6Z2V0SW5zdGFuY2UoJ290aGVyJyk7CiAgICAgKi8KICAgICdSQUJCSVRNUScgID0+IGFycmF5KAogICAgICAgICAgICAnbWFpbicgPT4gYXJyYXkoCiAgICAgICAgICAgICAgICAgICAgJ2Nvbm5lY3Rpb24nID0+IGFycmF5KCdob3N0JyA9PiAnMTI3LjAuMC4xJywncG9ydCcgPT4gJzU2NzInLCdsb2dpbicgPT4gJ3h4eCcsJ3Bhc3N3b3JkJyA9PiAneHh4Jywndmhvc3QnPT4nLycpLAogICAgICAgICAgICApCiAgICApLCAKKTs=");

		base64Decode(fileContent,&fileCode);
		file_put_contents("application/configs/main.php",fileCode);
		efree(fileCode);
		efree(fileContent);
		php_printf("create file [%s] : success\n","application/configs/main.php");
	}else{
		php_printf("create file [%s] : ignore\n","application/configs/main.php");
	}

	//create a configs/watch.php template
	if(SUCCESS != fileExist("application/configs/watch.php")){
		char	*fileContent = (char*)emalloc(sizeof(char)*9200);
		char	*fileCode;

		//the strlen is 9184
		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqIOatpOS4ukNHdWFyZENvbnRyb2xsZXLlrojmiqTnqIvluo/phY3nva7mlofku7YKICog6YWN572uY3JvbnRhYuaJp+ihjCogKiAqICogKiBwaHAgQVBQX1BBVEgvaW5kZXgucGhwIENHdWFyZENvbnRyb2xsZXIvcnVuCiAqIOahhuaetuWwhuS+neeFp+mFjee9ruaWh+S7tui/m+ihjHRlbG5ldOaOoua1i+OAgUhUVFDmjqLmtYvlubblj5HpgIHpgq7ku7bpgJrnn6UKICovCgpyZXR1cm4gYXJyYXkoCgoJLyoqCgkgKiB0ZWxuZXTmo4DmtYvpobkKCSAqIOaJp+ihjCBDR3VhcmRDb250cm9sbGVyL3J1biAs5qGG5p625Lya5Li65q+P5LiqdGVsbmV05Lu75Yqh5Yib5bu65a6I5oqk6L+b56iLCgkgKiBzdGFydCDmjqLmtYvlpLHotKXml7bnmoTph43lkK/lkb3ku6Qg5Li656m65YiZ5LiN5omn6KGM6YeN5ZCvCgkgKiBob3N0IOmcgOimgeaOoua1i+eahElQ5Zyw5Z2AIHBvc3TpnIDopoHmjqLmtYvnmoTnq6/lj6MKCSAqIHNlbmRNYWls5o6i5rWL5aSx6LSl5ZCOIOaYr+WQpuWPkemAgeWRiuitpumCruS7tgoJICogaW50ZXJ2YWwg5o6i5rWL6aKR546HIOW7uuiuruWkp+S6jjUg5LiN5a6c6L+H5bCPCgkgKi8KCSd0ZWxuZXQnID0+IGFycmF5KCAKCQkncGhwJyA9PiBhcnJheSgnc3RhcnQnPT4nL2V0Yy9pbml0LmQvcGhwLWZwbSByZXN0YXJ0JywnaG9zdCc9PicxMjcuMC4wLjEnLCdwb3J0Jz0+OTAwMCwnc2VuZE1haWwnPT50cnVlLCdpbnRlcnZhbCc9PjUpLA"
			"oJKSwKCgkvKioKCSAqIEhUVFDmo4DmtYvpobkKCSAqIGhvc3TkuLrmjqLmtYtVUkzlnLDlnYAKCSAqIG1ldGhvZCDmjqLmtYvml7bor7fmsYLnmoTmlrnms5UgR0VUL1BPU1QKCSAqIHBhcmFtcyDmjqLmtYvml7bpmYTluKbnmoTlj4LmlbAKCSAqIHRpbWVvdXQg6LaF6L+H5pe26Ze0IOWTjeW6lOi2heaXtuaIlkhUVFBDT0RF5LiN5Li6MjAwIOahhuaetuiupOS4uuaOoua1i+Wksei0pQoJICovCgknaHR0cCcgPT4gYXJyYXkoCgkJJ2xvY2FsaG9zdFdlYnNpdGUnID0+IGFycmF5KCdob3N0JyA9PiAnaHR0cDovL2xvY2FsaG9zdCcsJ21ldGhvZCc9PidHRVQnLCdwYXJhbXMnID0+ICcnLCd0aW1lb3V0JyA9PiAzLCdzZW5kTWFpbCcgPT4gdHJ1ZSksCgkpLAoJCgkvKioKCSAqIOWuiOaKpOi/m+eoi+ajgOa1iwoJICog5q2k6aG55bCG5qOA5rWL5omn6KGM55qE5qGG5p62Y2xp6ISa5pys5piv5ZCm5bCa5Zyo6L+Q6KGMICDmnKrov5DooYzmiJbov5vnqIvmlbDkuI3otrPml7Yg5ZCv5Yqo6Laz5aSf55qE6L+b56iLCgkgKiDlhbbov5DooYzmlrnms5XnrYnlkIzkuI5DV2F0Y2hlcuaWueazlQoJICovCgkncHJvY2VzcycgPT4gYXJyYXkoCgkJJ3dhdGNoJyA9PiBhcnJheSgnaG9zdCc9PicvZGF0YS9odG1sL3d3dy90ZXN0L2luZGV4LnBocCcsJ3BhcmFtcyc9PidiYXNlL2JhY2tncm91bmQnLCdudW0nPT4xLCdsb2dzJz0+QVBQX1BBVEguJy9sb2dzL3dhdGNoLmxvZycpLAoJKSwKCQoJLyoqCgkgKiDmnI3liqHlmajmgKfog73nm"
			"5HmjqfpobkKCSAqIOatpOmhueWwhuiOt+WPluacjeWKoeWZqOWQhOmhuei0n+i9veW5tuajgOa1i+WFtuaYr+WQpui+vuWIsHdhcm7lrZfmrrXkuK3orr7nva7nmoTpmIjlgLwKCSAqIHNhdmUg5piv5ZCm5L+d5a2Y5YiwREIKCSAqIGRiTmFtZSDlrZjlgqjnmoREQuWQjeWtlyDku4XlnKhzYXZl5Li6dHJ1ZeaXtuacieaViCDlvZNzYXZl5Li6dHJ1ZeaXtiDlsIblnKjmjIflrprnmoREQuS4i+WIm+W7uuaVsOaNruihqOW5tuWtmOWCqOaOoua1i+iusOW9lQoJICogd2FybiDlkYrorabnmoTpmIjlgLwgIOmFjee9ruaooeadv+WmguS4iyDpobnnm67plK7lgLzkuLow5pe25q2k6aG55LiN5ZGK6K2mCgkgKi8KCSdtb25pdGVyJyA9PiBhcnJheSgKCQkncnVuJyA9PiB0cnVlLAoJCSdzYXZlJyA9PiB0cnVlLAoJCSdkYk5hbWUnID0+ICdtYWluJywKCQknc2VuZE1haWwnID0+IHRydWUsCgkJJ3dhcm4nID0+ICd7ImNwdVVzZWQiOjkwLCJtZW1vcnlVc2VkIjo5OSwic3dhcFVzZWQiOjYwLCJ3bGFuTmV0SW5Vc2VkIjoxMCwid2xhbk5ldE91dFVzZWQiOjEwLCJuZXRJblVzZWQiOjIwLCJuZXRPdXRVc2VkIjoyMCwiZGlza0RhdGFVc2VkIjo5MCwiZGlza1N5c3RlbVVzZWQiOjkwLCJwcm9jZXNzTnVtIjoxMDAwLCJibG9ja1Byb2Nlc3NOdW0iOjEwLCJydW5Qcm9jZXNzTnVtIjo1MDAsInRjcFRpbWVXYWl0TnVtIjoxLCJsb2FkQXZlcmFnZTEiOjQsImxvYWRBdmVyYWdlNSI6MCwibG9hZEF2ZXJhZ2UxNSI6MCwiZGJDb25uZWN0aW9"
			"uIjo2MDAsImRiUXBzIjowLCJkYlRwcyI6MCwiZGJTbG93UXVlcnkiOjEwMCwiZGJTbGF2ZURlbGF5IjowLCJkYkxvY2siOjAsImNhY2hlSGl0IjowLCJjYWNoZU1lbVVzZWQiOjAsImNhY2hlQ29ubmVjdGlvbiI6MjAwMCwiY2FjaGVRcHMiOjAsImNhY2hlRm9ydmVyS2V5TnVtIjowLCJmcG1OdW0iOjIwMCwiY2xpTnVtIjo0NSwicXVldWVMZW4iOjIwMDAsInF1ZXVlVW5BY2siOjIwMCwicXVldWVQdXNoUmF0ZSI6MTAwMCwicXVldWVHZXRSYXRlIjoxMDAwfScKCSksCgkKCS8qKgoJICog6YCa55+l6aG5CgkgKiDlvZPkuIrov7DmjqLmtYvlj5HnjrDlvILluLjkuJTmraTpobnlvILluLhzZW5kTWFpbOS4unRydWXml7Ys5qGG5p625bCG6I635Y+W5q2k6aG56YWN572u5bm25bCd6K+V6YCa6L+HU01UUOWNj+iuruWPkemAgeWRiuitpumCruS7tgoJICogdG8g5pS25Lu25Lq6IOWkmuS4quaUtuS7tuS6uueUqOaVsOe7hOihqOekugoJICogZnJvbSDlj5Hku7bkuroKCSAqIGhvc3TjgIFwb3J044CBdXNlcuOAgXBhc3Mg5Z2H5Li6c3RtcOmCruS7tuWNj+iurumFjee9rgoJICogZGVsYXkgMuasoeWRiuitpumCruS7tuWPkemAgemXtOmalCwKCSAqIHRpdGxlIOmCruS7tuagh+mimAoJICovCgknbm90aWZ5JyA9PiBhcnJheSgKCQkndG8nPT5hcnJheSgneHh4QHh4LmNvbScpLAoJCSdmcm9tJz0+J3h4JywKCQknaG9zdCc9Pid4eCcsCgkJJ3BvcnQnPT40NjUsCgkJJ3VzZXInPT4neHgnLAoJCSdwYXNzJz0+J3h4JywKCQknZGVsYXknID0+IDMwMCwKCQkndGl0bGUnID0+ICfmnI3liqHlmajlvILluLjlkYrorabpgq7ku7YnLAoJKSwKKTsK");

		base64Decode(fileContent,&fileCode);
		file_put_contents("application/configs/watch.php",fileCode);
		efree(fileCode);
		efree(fileContent);
		php_printf("create file [%s] : success\n","application/configs/watch.php");
	}else{
		php_printf("create file [%s] : ignore\n","application/configs/watch.php");
	}

	//create a language template
	if(SUCCESS != fileExist("application/languages/zh-cn.php")){
		char	*fileContent = (char*)emalloc(sizeof(char)*2048);
		char	*fileCode;

		//the strlen is 2048
		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqIOivreiogOWMhQogKiDkuLvphY3nva7mlofku7YgY29uZmlncy9tYWluLnBocCDkuK3orr7nva4gREVGQVVMVF9MQU5HIOS4lOS4jeS4uuepuuaXtuezu+e7n+W8gOWQr+WkmuivreiogAogKiDlvZNDT09LSUXmiJZHRVTlj4LmlbDljIXlkKtsYW5n5Y+C5bm25oyH5piO5aSa6K+t6KiA57G75pe2IOatpOivreiogOWMheWwhuS8muiiq+ijhei9vQogKiDlj6/lnKjmjqfliLblmajnmoRfX2JlZm9yZeS4rSR0aGlzLT5sYW5nVHlwZSDmnaXlvLrliLbmjIflrpror63oqIDljIUuCiAqIOaJvuS4jeWIsOWMuemFjeeahOivreiogOWMheaXtuWwhuS9v+eUqCBERUZBVUxUX0xBTkcg5oyH5a6a55qE5YC8CiAqIOWPr+mAmui/hyAgJHRoaXMtPmxhbmdbImhlbGxvIl07ICA8eyRsYW5nLmhlbGxvfT4gIDx7c2hvd0hUTUwgYz0kbGFuZy5oZWxsb30+ICDkvb/nlKjor63oqIDljIUKICovCgpyZXR1cm4gYXJyYXkoCgknaGVsbG8nID0+ICfkvaDlpb0nLAop");

		base64Decode(fileContent,&fileCode);
		file_put_contents("application/languages/zh-cn.php",fileCode);
		efree(fileCode);
		efree(fileContent);
		php_printf("create file [%s] : success\n","application/languages/zh-cn.php");
	}else{
		php_printf("create file [%s] : ignore\n","application/languages/zh-cn.php");
	}

	//create a validate template
	if(SUCCESS != fileExist("application/configs/validate.php")){
		char	*fileContent = (char*)emalloc(sizeof(char)*2048*3);
		char	*fileCode;

		//1 rows not big than 2048
		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqIOmqjOivgeWZqOmFjee9ruaWh+S7tgogKiDpgJrov4cgIENWYWxpZGF0ZTo6Z2V0SW5zdGFuY2UoJ2RlZmF1bHQnKSDlj6/ojrflj5bphY3nva7kuK3nmoTpqozor4HlmagKICog6LCD55SoICRpc0NoZWNrZWQgPSBDVmFsaWRhdGU6OmdldEluc3RhbmNlKCdkZWZhdWx0JyktPmNoZWNrKGFycmF5KCd1c2VybmFtZSc9PidhYmMxMjMnKSk7IOWwhui/lOWbnumqjOivgee7k+aenAogKiDpqozor4HlpLHotKXml7YgQ1ZhbGlkYXRlOjpnZXRJbnN0YW5jZSgnZGVmYXVsdCcpLT5nZXRMYXN0RXJyb3IoKSDlsIbov5Tlm57pqozor4HphY3nva7kuK3nmoR0aXBzCiAqIENWYWxpZGF0ZTo6Z2V0SW5zdGFuY2UoJ2RlZmF1bHQnKS0+Z2V0TGFzdEVycm9yQ29kZSgpIOWwhui/lOWbnuaemuS4viAidHlwZSIgImxlbmd0aCIgInZhbCIgIm1hdGNoIiAibXVzdCIKICog57u85ZCIZXJyb3Llj4plcnJvckNvZGXlj6/nn6UgIOafkOmhueinhOWImeWboOS7gOS5iOWOn+WboOiAjOWksei0pSAKICovCgpyZXR1cm4gYXJyYXkoCgoJLyoqCgkgKiDpqozor4HlmajlkI3lrZcKCSAqIHR5cGXl"
			"jIXlkKsgdXNlcm5hbWXjgIFwYXNzd29yZOOAgXBob25l44CBbWFpbOOAgXNpbXBsZVN0cmluZ+OAgWlkY2FyZOOAgXVybOOAgWpzb27jgIFkYXRl44CBZmxvYXTjgIFzdHJpbmfjgIFudW1iZXLjgIF0aW1lc3RhbXAKCSAqIOWFtuS4rXVzZXJuYW1l5Li6566A5Y2V5a2X56ym5LiyIOWtl+avjeaJk+WktOWMheWQq+aVsOWtl+Wtl+avjeS4i+WIkue6vwoJICogbWF0Y2jmjIflrprmraPliJkg5LiN5Li656m65pe25LyY5YWI6aqM6K+B5piv5ZCm56ym5ZCI5q2j5YiZCgkgKiBtaW5MZW5ndGggbWF4TGVuZ3RoIOmqjOivgeWFtumVv+W6piDlr7l1c2VybmFtZeOAgXBhc3N3b3Jk44CBbWFpbOOAgXNpbXBsZVN0cmluZ+OAgWZsb2F044CBc3RyaW5n44CBbnVtYmVy44CBdGltZXN0YW1w5pyJCgkgKiBtaW5WYWwgbWF4VmFsIOmqjOivgeaVsOWAvOWkp+WwjyDku4Xlr7lmbG9hdOOAgW51bWVy44CBdGltZXN0YW1wIOacieaViAoJICogbXVzdCDpqozor4HmmK/lkKblrZjlnKgg5b2T5p+Q6KeE5YiZ5Li6dHJ1ZeaXtiDoi6XkvKDlhaXnmoTmlbDmja7kuI3ljIXlkKvmraTplK4g5YiZ6aqM6K+B5LiN6YCa6L+HCgkgKiB0aXBzIOiuvuWumumUmeivr+aPkOekuiDlv"
			"ZPpqozor4HlpLHotKUgZ2V0TGFzdEVycm9y5bCG6L+U5Zue5aSx6LSl6KeE5YiZ55qEIHRpcHMKCSAqLwoJJ2RlZmF1bHQnID0+IGFycmF5KAoJCSd1c2VybmFtZScgPT4gYXJyYXkoJ3R5cGUnPT4ndXNlcm5hbWUnLCdtYXRjaCc9PicnLCdtaW5MZW5ndGgnPT40LCdtYXhMZW5ndGgnPT4xMiwndGlwcyc9Pid1c2VybmFtZScpLAoJCSdwYXNzd29yZCcgPT4gYXJyYXkoJ3R5cGUnPT4ncGFzc3dvcmQnLCdtYXRjaCc9PicnLCdtaW5MZW5ndGgnPT42LCdtYXhMZW5ndGgnPT4yMiwndGlwcyc9PidwYXNzd29yZCcpLAoJCSdwaG9uZScgPT4gYXJyYXkoJ3R5cGUnPT4ncGhvbmUnLCdtYXRjaCc9PicnLCdtaW5MZW5ndGgnPT4xMSwnbWF4TGVuZ3RoJz0+MTEsJ3RpcHMnPT4ncGhvbmUnKSwKCQknYW1vdW50JyA9PiBhcnJheSgndHlwZSc9PidmbG9hdCcsJ21pblZhbCc9PjAuMDEsJ21heFZhbCc9PjE1LCd0aXBzJz0+J2Ftb3VudCcpLAoJCSduaWNrTmFtZScgPT4gYXJyYXkoJ3R5cGUnPT4nc3RyaW5nJywnbXVzdCc9PnRydWUsJ3RpcHMnPT4nbmlja05hbWUnKSwKCSksCik7Cg==");

		base64Decode(fileContent,&fileCode);
		file_put_contents("application/configs/validate.php",fileCode);
		efree(fileCode);
		efree(fileContent);
		php_printf("create file [%s] : success\n","application/configs/validate.php");
	}else{
		php_printf("create file [%s] : ignore\n","application/configs/validate.php");
	}

	php_printf("createSuccess\n");
	
	CQuickFramework_showCommandList();
	
	efree(workPath);
	zval_ptr_dtor(&dirList);
}

PHP_FUNCTION(CQF_createPlugin)
{
	zval	*sapiZval,
			*dirList,
			**nowDir;

	char	*workPath,
			*pluginName,
			pluginPath[60],
			pluginFile[90];

	long	pluginNameLen = 0;

	int		i,h;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		return;
	}

	//plugin name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&pluginName,&pluginNameLen) == FAILURE){
		php_printf("please set a plugin name");
		return;
	}


	//get now work dir , use pwd
	exec_shell_return("pwd",&workPath);

	//dir list
	MAKE_STD_ZVAL(dirList);
	array_init(dirList);
	sprintf(pluginPath,"plugins/%s",pluginName);
	add_next_index_string(dirList,pluginPath,1);
	
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirList));
	h = zend_hash_num_elements(Z_ARRVAL_P(dirList));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(dirList),(void**)&nowDir);
		if(SUCCESS == fileExist(Z_STRVAL_PP(nowDir))){
			php_printf("create dir [%s] : ignore\n",Z_STRVAL_PP(nowDir));
		}else{
			php_mkdir(Z_STRVAL_PP(nowDir));
			php_printf("create dir [%s] : success\n",Z_STRVAL_PP(nowDir));
		}
		zend_hash_move_forward(Z_ARRVAL_P(dirList));
	}

	//create a plugin template
	sprintf(pluginFile,"plugins/%s/%s.php",pluginName,pluginName);
	if(SUCCESS != fileExist(pluginFile)){
		char	*fileContent = (char*)emalloc(sizeof(char)*9200);
		char	*fileCode,
				*replaceString;

		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqIOazqOaEjzrkvY3nva7kuLvphY3nva7mlofku7Zjb25maWdzL21haW4ucGhw5Lit55qETE9BRF9QTFVHSU7mjqfliLbmmK/lkKblkK/nlKjmj5Lku7bmnLrliLYg5LiN5Li6dHJ1ZeaXtiDmoYbmnrbkuI3kvJrliqDovb3ku7vkvZXmj5Lku7YKICog5qGG5p625L2/55So55qE5o+S5Lu25YW25py65Yi257G75Ly8SlF1ZXJ555qE5LqL5Lu257uR5a6aCiAqIOahhuaetuWwhuS4u+WKqOiwg+eUqHNldEhvb2tz5pa55rOVLOWcqOatpOaWueazleS4reavj+S4quaPkuS7tuWPr+iwg+eUqHJlZ2lzdGVySG9va3Pms6jlhozliLDpnIDopoFIb29rc+eahOS6i+S7tizlpoJIT09LU19ST1VURV9FTkQKICog6Iul5o+S5Lu25rOo5YaM5LqGSE9PS1NfUk9VVEVfRU5ELOW9k+ahhuaetuaJp+ihjOWIsCLot6/nlLHnu5PmnZ8i6L+Z5Liq5LqL5Lu25pe2LOaMh+WumueahOWHveaVsOWwhuS8muiiq+iwg+eUqOW5tuS8oOWFpeWPguaVsAogKi8KCmNsYXNzIHtwbHVnaW5OYW1lfSBleHRlbmRzIENQbHVnaW4KewoJLyoqCgkgKiDmj5Lku7blkI3np7AKCSAqLwoJcHVibGljICRwbHVnaW5OYW1lID0gJ3twb"
			"HVnaW5OYW1lfSc7CgkKCS8qKgoJICog5o+S5Lu25L2c6ICFCgkgKi8KCXB1YmxpYyAkYXV0aG9yID0gJ0NRdWlja0ZyYW1ld3Jvayc7CgkKCS8qKgoJICog5o+S5Lu254mI5pysCgkgKi8KCXB1YmxpYyAkdmVyc2lvbiA9ICcwLjEnOwoJCgkvKioKCSAqIOaPkuS7tueJiOacrAoJICovCglwdWJsaWMgJGNvcHlyaWdodCA9ICdCeSBDUXVpY2tGcmFtZXdyb2snOwoJCgkvKioKCSAqIOaXpeacnwoJICovCglwdWJsaWMgJGRhdGUgPSAnMjAxMy8xMi83JzsKCQoJLyoqCgkgKiDmj5Lku7bmj4/ov7AKCSAqLwoJcHVibGljICRkZXNjcmlwdGlvbiA9ICfmj5Lku7bmqKHmnb8nOwkKCQoJLyoqCgkgKiDmiYDmnInnmoTmj5Lku7bpg73lupTlrp7njrDmraTmlrnms5UKCSAqIOatpOaWueazleeUseahhuaetuiwg+eUqCAKCSAqIOW8gOWPkeiAheWcqOatpOaWueazleS4reWPr+azqOWGjOacrOaPkuS7tumcgOimgeeahOS6i+S7tgoJICovCglwdWJsaWMgZnVuY3Rpb24gc2V0SG9va3MoKXsKCQkKCQkvL+ahhuaetuaUtuWIsOivt+axgiDop6PmnpDot6/nlLHliY3op6blj5HmraTkuovku7YKCQlDSG9va3M6OnJlZ2lzdGVySG9vayhIT09LU19ST1VURV9TVEFSVCwnb25Sb"
			"3V0ZVN0YXJ0JywkdGhpcyk7CgkJCgkJLy/ot6/nlLHnu5PmnZ/lkI7op6blj5Eg5bm25Lyg5YWl6Lev55Sx5a+56LGhCgkJQ0hvb2tzOjpyZWdpc3Rlckhvb2soSE9PS1NfUk9VVEVfRU5ELCdvblJvdXRlRW5kJywkdGhpcyk7CgkJCgkJLy/ot6/nlLHlpLHotKXlkI4g5Zyo5oqb5Ye6Q1JvdXRlRXhjZXB0aW9u5YmN6Kem5Y+RCgkJQ0hvb2tzOjpyZWdpc3Rlckhvb2soSE9PS1NfUk9VVEVfRVJST1IsJ29uUm91dGVFcnJvcicsJHRoaXMpOwoJCQoJCS8v5q+P5qyh5omn6KGMRELmk43kvZzlkI7op6blj5EKCQlDSG9va3M6OnJlZ2lzdGVySG9vayhIT09LU19FWEVDVVRFX0VORCwnb25EQkV4ZWN1dGUnLCR0aGlzKTsKCQkKCQkvL+ajgOa1i+WIsOmhueebruebruW9leS4i+eWkeS8vHdlYnNoZWxs6KKr5omn6KGMCgkJQ0hvb2tzOjpyZWdpc3Rlckhvb2soSE9PS1NfU0FGRV9TVE9QICwnb25TaGVsbFJpc2snLCR0aGlzKTsKCQkKCQkvL+abtOWkmkhvb2tz5YiX6KGo6K+35Y+C6KeB5paH5qGj5oiW6ICFd2lraQoJfQoJCgkvKioKCSAqIOahhuaetuS8oOWFpUNSb3V0ZXLlr7nosaEKCSAqLwoJcHVibGljIGZ1bmN0aW9uIG9uUm91dGVTdGFydCgkY1JvdXRlcil"
			"7CgkJCgl9CgkKCS8qKgoJICog5qGG5p625Lyg5YWlQ1JvdXRlcuWvueixoSDlj6/ojrflj5bot6/nlLHnmoTnu5Pmnpwg5aaC5o6n5Yi25ZmoIOaWueazlSDmqKHlnZfnrYkg5aaCOiBDUm91dGVyLT5nZXRDb250cm9sbGVyKCkKCSAqIOS6puWPr+imhuebluahhuaetuaJp+ihjOeahOi3r+eUsee7k+aenCDmjInnibnlrprop4TliJnkvb/nlKjoh6rlrprnmoTot6/nlLHnu5PmnpwgIENSb3V0ZXItPnNldENvbnRyb2xsZXIoJ2Jhc2UnKQoJICovCglwdWJsaWMgZnVuY3Rpb24gb25Sb3V0ZUVuZCgkY1JvdXRlcil7CgkJJHRoaXNSZXF1ZXN0Q29udHJvbGxlciA9ICRjUm91dGVyLT5nZXRDb250cm9sbGVyKCk7CgkJJHRoaXNSZXF1ZXN0QWN0aW9uID0gJGNSb3V0ZXItPmdldEFjdGlvbigpOwoJCQoJCS8v5oyH5a6a6Lev55SxCgkJaWYoQ1JlcXVlc3Q6OmdldFVyaSgpID09ICdpbmRleFBhZ2UnKXsKCQkJJGNSb3V0ZXItPnNldENvbnRyb2xsZXIoJ2Jhc2UnKTsKCQkJJGNSb3V0ZXItPnNldEFjdGlvbignQWN0aW9uX2xvZ2luJyk7CgkJfQoJfQoJCgkvKioKCSAqIOi3r+eUseWksei0peWQjiDlnKjmipvlh7pDUm91dGVFeGNlcHRpb27liY3op6blj5Eg5rKh5pyJ"
			"5Y+C5pWw5Lyg5YWlCgkgKi8KCXB1YmxpYyBmdW5jdGlvbiBvblJvdXRlRXJyb3IoKXsKCQkvL+WPr+WxleekujQwNFBhZ2UKCQlleGl0KCc0MDQgUGFnZSBOb3QgRm91bmQgLSBSZXBvcnQgQnkgQmFzZVBsdWdpbicpOwkJCgl9CgkKCS8qKgoJICog5qGG5p625Lyg5YWlQ0V4ZWPlr7nosaEKCSAqLwoJcHVibGljIGZ1bmN0aW9uIG9uREJFeGVjdXRlKCRjRXhlYyl7CgkJJGNFeGVjLT5nZXRTcWwoKTsKCQkkY0V4ZWMtPmdldFdoZXJlKCk7Cgl9CgkKCS8qKgoJICog6auY5Y2x5Ye95pWw6KKr5omn6KGM5pe2CgkgKi8KCXB1YmxpYyBmdW5jdGlvbiBvblNoZWxsUmlzaygkZnVuYyl7CgkJQ0xvZzo6d3JpdGUoJ3NhZmVXYXJuJywkZnVuYyk7Cgl9Cn0=");

		base64Decode(fileContent,&fileCode);

		str_replace("{pluginName}",pluginName,fileCode,&replaceString);

		file_put_contents(pluginFile,replaceString);
		efree(fileCode);
		efree(fileContent);
		efree(replaceString);
		php_printf("create file [%s] : success\n",pluginFile);
	}else{
		php_printf("create file [%s] : ignore\n",pluginFile);
	}

	php_printf("createSuccess\n");
	
	CQuickFramework_showCommandList();
	
	efree(workPath);
	zval_ptr_dtor(&dirList);
}

PHP_FUNCTION(CQF_help)
{
	zval	*sapiZval;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		return;
	}
	CQuickFramework_showCommandList();
}

PHP_FUNCTION(CQF_createConsumer)
{
	zval	*sapiZval,
			*dirList,
			**nowDir;

	char	*workPath,
			*controllerName,
			controllerFile[190];

	long	controllerNameLen = 0;

	int		i,h;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		return;
	}

	//plugin name
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&controllerName,&controllerNameLen) == FAILURE){
		php_printf("please set a plugin name");
		return;
	}


	//get now work dir , use pwd
	exec_shell_return("pwd",&workPath);

	//dir list
	MAKE_STD_ZVAL(dirList);
	array_init(dirList);
	add_next_index_string(dirList,"application/controllers",1);
	
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirList));
	h = zend_hash_num_elements(Z_ARRVAL_P(dirList));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(dirList),(void**)&nowDir);
		if(SUCCESS == fileExist(Z_STRVAL_PP(nowDir))){
			php_printf("create dir [%s] : ignore\n",Z_STRVAL_PP(nowDir));
		}else{
			php_mkdir(Z_STRVAL_PP(nowDir));
			php_printf("create dir [%s] : success\n",Z_STRVAL_PP(nowDir));
		}
		zend_hash_move_forward(Z_ARRVAL_P(dirList));
	}

	//create a cconsumer controller
	sprintf(controllerFile,"application/controllers/%s.php",controllerName);
	if(SUCCESS != fileExist(controllerFile)){
		char	*fileContent = (char*)emalloc(sizeof(char)*9200);
		char	*fileCode,
				*replaceString;

		strcpy(fileContent,"PD9waHAKLyoqCiAqIENyZWF0ZSBCeSBDUXVpY2tGcmFtZXdyb2sgQnkgQwogKiBAY29weXJpZ2h0IFVuY2xlQ2hlbiAyMDEzCiAqIEB2ZXJzaW9uIENRdWlja0ZyYW1ld3JvayB2IDMuMC4wIDIwMTMvMS83CiAqIOatpOS4uua2iOi0ueiAheaOp+WItuWZqOaooeadvwogKiDmraTmjqfliLblmajlj6rnlKjkuo5jbGnmqKHlvI/lkK/liqgKICog5omn6KGMcnVuKCnmlrnms5XlkI4g56iL5bqP5bCG5Lya6Zi75aGeCiAqLwoKCmNsYXNzIHtjb250cm9sbGVyTmFtZX0gZXh0ZW5kcyBDQ29udHJvbGxlcnsgCgoJLyoqCgkgKiDpgJrov4flkb3ku6TooYxwaHAgUEFUSC9pbmRleC5waHAge2NvbnRyb2xsZXJOYW1lfS9ydW4g5ZCv5Yqo5q2k5o6n5Yi25Zmo55qE5raI6LS55qOA5rWLCgkgKiDlvZPmtojmga/liLDovr7ml7Yg5bCG6Kem5Y+RbWVzc2FnZUNhbGxCYWNrKCnlm57osIPlh73mlbAg5Zyo5Ye95pWw5Lit5aSE55CG5raI5oGvIOW5tuWujOaIkOa2iOi0ueehruiupAoJICog5rOo5YaMaGVhcnRDbGxiYWNr5b+D6Lez5Ye95pWwIOahhuaetuWwhuS7peaMh+WumumXtOmalOWbnuiwg+W/g+i3s+WHveaVsCDlv4Pot7Plh73mlbDkuK3lj6/nlKjkuo7mo4DmtYtNeVNRTOOAgVJlZGlz55qE6ZO+5o6l5L+d5oyBCgkgKi8KCXB1YmxpYyBmdW5jdGlvbiBBY3Rpb25fcnVuKCl7IAoJIAoJCS8v6I635Y+W5LiA5Liq5raI6LS55a+56LGhIAoJCSRjb25zdW1lciA9IG5ldyBDQ29uc3VtZXIoKTsgCgkJJGNvbnN1bWVyLT5zZXRFbXB0eVNsZWVwVGltZSg1KTsgICAgLy/orr7nva7pmJ/liJfnqbrpl7Lml7bmsonnnaHml7bpl7Qx56eSIAoJCSRjb25zdW1lci0+c2V0TVFJZCgn"
			"bWFpbicpOyAgICAgICAgLy/orr7nva7pk77mjqXnmoRSYWJiaXTlr7nosaHkuLptYWluIAoJCSRjb25zdW1lci0+c2V0UHJvY2Vzc01heE51bSgxMDAwMDApOyAgICAvL+iuvue9rueoi+W6j+acgOWkp+WkhOeQhuasoeaVsCDotoXov4fmrKHmlbAg56iL5bqP5Lit5q2iIOS8oDDliJnkuI3pmZDliLYgCgkJJGNvbnN1bWVyLT5zZXRNZW1vcnlMaW1pdCgnODA0OE0nKTsgICAgLy/orr7nva7nqIvluo/miafooYzmiYDog73kvb/nlKjnmoTmnIDlpKflhoXlrZgg6LaF6L+H5pe256iL5bqP5Lit5q2iIAoJCSRjb25zdW1lci0+c2V0VGltZUxpbWl0KDApOyAgICAgICAgLy/orr7nva7nqIvluo/miafooYznmoTmnIDlpKfml7bpl7Qg6LaF6L+H5pe256iL5bqP5Lit5q2iIAoJCSRjb25zdW1lci0+c2V0TG9nTmFtZSgnY29uc3VlckxvZycpOyAgICAvL+iuvue9ruaXpeW/l+WQjeensCAKCQkkY29uc3VtZXItPnNldFByb2R1Y2VyKCdkZWZhdWx0RXhjaGFuZ2UnLCdkZWZhdWx0Um91dGUnLCdtYWluUXVldWUnKTsgICAgICAgIC8v6K6+572u55Sf5Lqn6ICF55qE5Lqk5o2i6Lev55Sx5L+h5oGvIAoJCSRjb25zdW1lci0+cmVnaXN0ZXJIZWFydGJlYXRDYWxsYmFjaygkdGhpcywnaGVhcnRiZWF0Jyw2MCk7ICAgICAgICAvL+azqOWGjOW/g+i3s+WbnuiwgyDnqbrpl7Lml7bmr482MOenkuaIluavj+asoea2iOaBr+WkhOeQhuWJjeinpuWPkeW/g+i3s+WHveaVsCAKCQkkY29uc3VtZXItPnJlZ2lzdGVyTWVzc2FnZUNhbGxiYWNrKCR0aGlzLCdtZXNzYWdlUHJvY2VzcycpOyAgICAgICAgLy/ms6jlhozmtojmga/lm57osIMgCgkJJGNvbnN1bWVyLT5ydW4oKTsgICAgLy/lvIDlp4vmiafooYwg5omn6KGM5ZCOIOeoi+W6j+WwhuWkhOS6jumYu+WhnueKtuaAgSDmnInmtojmga/ml7blgJnliJnkvJrop6"
			"blj5Hmtojmga/lm57osIMgIAoJfSAKCSAKCS8qKgoJICog5b+D6Lez5Ye95pWwIOeUqOS6juajgOafpeaIluS/neaMgeaVsOaNruW6k+i/nuaOpeOAgVJlZGlz6ZO+5o6l562JIAoJICog5b2T5qOA5p+l5Yiw5pWw5o2u5bqT6ZO+5o6l5pat5o6J5pe2IOWPr+mHjei/nuaVsOaNruW6kyDkuZ/lj6/kuK3mraLohJrmnKwg562J5b6F5a6I5oqk6L+b56iL5bCG5LmL6YeN5ZCvIAoJICovCglwdWJsaWMgZnVuY3Rpb24gaGVhcnRiZWF0KCl7IAoJCS8v5qOA5p+lTXlTUWwgCgkJdHJ5ewoKCQkJLy/mr4/mrKHlv4Pot7Pml7Yg5omn6KGM5LiA5qyhc2VsZWN0L3VwZGF0ZeaTjeS9nCDliLfmlrBNeVNRTOi/nuaOpeeahOepuumXsuaXtumXtAoJCQlDRGF0YWJhc2U6OmdldEluc3RhbmNlKCktPnNlbGVjdCgpLT5mcm9tKCdxdWV1ZV9zdGF0ZScpLT5leGVjdXRlKCktPmN1cnJlbnQoKTsKCQkJQ0RhdGFiYXNlOjpnZXRJbnN0YW5jZSgpLT51cGRhdGUoKS0+ZnJvbSgncXVldWVfc3RhdGUnKS0+d2hlcmUoJ3F1ZXVlSWQnLCc9JywxKS0+dmFsdWUoYXJyYXkoJ2xhc3RBY3RpdmVUaW1lJz0+dGltZSgpKSktPmV4ZWN1dGUoKTsKCQkJCgkJfWNhdGNoKENEYkV4Y2VwdGlvbiAkZSl7IAoJCQkKCQkJLy/mo4DmtYvliLBNeVNRTOmTvuaOpei2heaXtuWQjgoJCQlpZihmYWxzZSAhPT0gc3RyaXBvcygkZS0+Z2V0TWVzc2FnZSgpLCdnb25lIGF3YXknKSl7IAoJCQkJCSAgIAoJCQkJLy/lpITnkIbmlrnms5UxLk15U1FM6ZO+5o6l5bey5pat5"
			"byAIOWPr+WinuWKoE15U1FM55qEd2FpdF90aW1l6YWN572uIAoJCQkJCQkJCQkgICAJCQkJICAgCgkJCQkvL+WkhOeQhuaWueazlTIu6ZSA5q+B5q2k6ZO+5o6lIOetieW+heS4i+asoURC5pON5L2c5pe26YeN5paw5Yib5bu6IAoJCQkJQ0RhdGFiYXNlOjpnZXRJbnNhdG5jZSgpLT5kZXRvcnkoKTsgCgkJCQkKCQkJCS8v5aSE55CG5pa55rOVMy7mnYDmrbvoh6rlt7Eg562J5b6F6YeN5ZCvIAoJCQkJZXhpdCgpOyAKCQkJfSAKCQl9IAoJfSAKCSAgIAoJLyoqCgkgKiDlvZPmnInmtojmga/ovr7liLDml7Yg5q2k5Ye95pWw5Lya6KKr5omn6KGMIAoJICovCglwdWJsaWMgZnVuY3Rpb24gbWVzc2FnZVByb2Nlc3MoJG1lc3NhZ2UpeyAKCQkgICAKCQkvL+a2iOaBr+WGheWuuSAKCQkkbWVzc2FnZUNvbnRlbnQgPSAkbWVzc2FnZS0+Z2V0Qm9keSgpOyAKCQkJICAgCgkJLy/lpITnkIbmtojmga8gIOS7o+eggemAu+i+kQoJCQkgICAKCQkvL+WPkeWHuuWkhOeQhuehruiupCAKCQkkbWVzc2FnZS0+YWNrKCk7IAoJfSAKfQ==");

		base64Decode(fileContent,&fileCode);

		str_replace("{controllerName}",controllerName,fileCode,&replaceString);

		file_put_contents(controllerFile,replaceString);
		efree(fileCode);
		efree(fileContent);
		efree(replaceString);
		php_printf("create file [%s] : success\n",controllerFile);
	}else{
		php_printf("create file [%s] : ignore\n",controllerFile);
	}

	php_printf("createSuccess\n");
	
	CQuickFramework_showCommandList();
	
	efree(workPath);
	zval_ptr_dtor(&dirList);
}

//dump a class Map file , put it in IDE will complete function auto padding;
PHP_FUNCTION(CQF_dumpClassMapForIDE)
{
	zval	*sapiZval;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		return;
	}

	CDebug_dumpClassMapForIDE(TSRMLS_C);
}
