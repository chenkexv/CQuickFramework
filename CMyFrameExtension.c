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
	PHP_FE(CMyFrameExtension_createProject,NULL) //create a empty project
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

//create a emtpy string
PHP_FUNCTION(CMyFrameExtension_createProject)
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

	php_printf("createSuccess\n");
	
	efree(workPath);
	zval_ptr_dtor(&dirList);
}