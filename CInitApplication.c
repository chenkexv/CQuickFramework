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
#include "php_CInitApplication.h"
#include "php_CConfig.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include "php_CHooks.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CWebApp.h"

#ifdef PHP_WIN32
#define PATH_SEPARATOR ";"
#else
#define PATH_SEPARATOR ":"
#endif

//zend类方法
zend_function_entry CInitApplication_functions[] = {
	PHP_ME(CInitApplication,__construct,NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)
	PHP_ME(CInitApplication,_initAutoLoad,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initTimeZone,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initIncludeClass,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initTopException,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initHooks,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initGzip,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initCookie,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,_initSession,NULL, ZEND_ACC_PRIVATE)
	PHP_ME(CInitApplication,webShutdown,NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CInitApplication,GetRequest,NULL, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
	PHP_ME(CInitApplication,classAutoLoad,NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CInitApplication)
{
	//注册类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CInitApplication",CInitApplication_functions);
	CInitApplicationCe = zend_register_internal_class(&funCe TSRMLS_CC);
	
	//抽象类
	CInitApplicationCe->ce_flags = ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;

	//类属性
	zend_declare_property_null(CInitApplicationCe, ZEND_STRL("_initData"), ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//自动加载
void CInitApplication_initAutoLoad(TSRMLS_D){

	zval *	autoload, 
			*method, 
			*function,
			*loader,
			*ret = NULL,
			**params[1] = {&autoload};

	MAKE_STD_ZVAL(autoload);
	array_init(autoload);

	add_next_index_string(autoload,"CInitApplication",1);
	add_next_index_string(autoload,"classAutoLoad",1);

	MAKE_STD_ZVAL(function);
	ZVAL_STRING(function, "spl_autoload_register", 0);

	MODULE_BEGIN
		zend_fcall_info fci = {
			sizeof(fci),
			EG(function_table),
			function,
			NULL,
			&ret,
			1,
			(zval ***)params,
			NULL,
			1
		};

		if (zend_call_function(&fci, NULL TSRMLS_CC) == FAILURE) {
			if (ret) {
				zval_ptr_dtor(&ret);
			}
			efree(function);
			zval_ptr_dtor(&autoload);
			return;
		}

		if (ret) {
			zval_ptr_dtor(&ret);
		}
		efree(function);
		zval_ptr_dtor(&autoload);
	MODULE_END
}

//设置自动加载
PHP_METHOD(CInitApplication,_initAutoLoad)
{
	CInitApplication_initAutoLoad(TSRMLS_C); 
}

void CInitApplication_initTimeZone(TSRMLS_D)
{
	zval	*cconfigInstanceZval,
			*timeZoneZval,
			*returnZval;

	//加载时区设置
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("TIME_ZONE",cconfigInstanceZval,&timeZoneZval TSRMLS_CC);

	if(IS_STRING == Z_TYPE_P(timeZoneZval) && strlen(Z_STRVAL_P(timeZoneZval)) > 0 ){
		date_default_timezone_set(Z_STRVAL_P(timeZoneZval));
	}

	zval_ptr_dtor(&timeZoneZval);
	zval_ptr_dtor(&cconfigInstanceZval);
}

//设置时区
PHP_METHOD(CInitApplication,_initTimeZone)
{
	CInitApplication_initTimeZone(TSRMLS_C);
}

//设置include路径
void CInitApplication_initIncludeClass(TSRMLS_D){
	
}

//设置include路径
PHP_METHOD(CInitApplication,_initIncludeClass)
{
	CInitApplication_initIncludeClass(TSRMLS_C);
}

//设置异常处理
void CInitApplication_initTopException(TSRMLS_D)
{
	set_error_handler("CException::getTopErrors");
	register_shutdown_function("CInitApplication::webShutdown");
	error_reporting(0);
}

//设置异常处理
PHP_METHOD(CInitApplication,_initTopException)
{
	CInitApplication_initTopException(TSRMLS_C);
}

//启用事件驱动器
void CInitApplication_initHooks(TSRMLS_D)
{
	zval	*loadPluginZval,
			*cconfigInstanceZval,
			*returnZval;

	//获取配置文件中的import项目
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("LOAD_PLUGIN",cconfigInstanceZval,&loadPluginZval TSRMLS_CC);

	//记录registerEvent时间
	MODULE_BEGIN
		zval	**startTimeZval,
				*startTime;

		if(zend_hash_find(&EG(symbol_table),"SYSTEM_INIT",strlen("SYSTEM_INIT")+1,(void**)&startTimeZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(startTimeZval)){
			microtimeTrue(&startTime);
			add_assoc_zval_ex(*startTimeZval,"registerEvent",strlen("registerEvent")+1,startTime);
		}
	MODULE_END


	//load systemPlugin
	CHooks_loadSystemPlugin(TSRMLS_C);

	//加载插件
	if(IS_BOOL == Z_TYPE_P(loadPluginZval) && 1 == Z_LVAL_P(loadPluginZval)){
	
		//加载插件
		CHooks_loadPlugin(TSRMLS_C);
	}
	

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&loadPluginZval);
}

//设置插件启动
PHP_METHOD(CInitApplication,_initHooks)
{
	CInitApplication_initIncludeClass(TSRMLS_C);
}

//设置Gzip
void CInitApplication_initGzip(TSRMLS_D)
{
	zval	*cconfigInstanceZval,
			*useGzipZval,
			*returnZval;

	//加载cookie
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	CConfig_load("GZIP",cconfigInstanceZval,&useGzipZval TSRMLS_CC);

	//使用Gzip
	if(IS_BOOL == Z_TYPE_P(useGzipZval) && 1 == Z_LVAL_P(useGzipZval) ){
		zval *gzipLevelZval;
		CConfig_load("GZIP_LEVEL",cconfigInstanceZval,&gzipLevelZval TSRMLS_CC);

		ini_set("zlib.output_compression", "On");
		ini_seti("zlib.output_compression_level", Z_LVAL_P(gzipLevelZval));

		zval_ptr_dtor(&gzipLevelZval);
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useGzipZval);
}

//设置Gzip
PHP_METHOD(CInitApplication,_initGzip)
{
	CInitApplication_initGzip(TSRMLS_C);
}

//设置cookie
void CInitApplication_initCookie(TSRMLS_D)
{
	zval	*cconfigInstanceZval,
			*cookieZval,
			*returnZval;

	//加载cookie
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("COOKIE_DOMAIN",cconfigInstanceZval,&cookieZval TSRMLS_CC);

	if(IS_STRING == Z_TYPE_P(cookieZval)){
		ini_set("session.cookie_domain", Z_STRVAL_P(cookieZval));
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&cookieZval);
}

//设置cookie
PHP_METHOD(CInitApplication,_initCookie)
{
	CInitApplication_initCookie(TSRMLS_C);
}

//设置session
void CInitApplication_initSession(TSRMLS_D)
{
	zval	*cconfigInstanceZval,
			*sessionMemcacheZval,
			*returnZval,
			*autoSession;

	//加载cookie
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	CConfig_load("SESSION_MEMCACHE",cconfigInstanceZval,&sessionMemcacheZval TSRMLS_CC);
	

	//判断是否加载memcache
	if(IS_BOOL == Z_TYPE_P(sessionMemcacheZval) && 1 == Z_LVAL_P(sessionMemcacheZval) ){
	
		if(1 == extension_loaded("memcache") ){
		
			//获取memcache的地址
			zval *memcacheHost;

			CConfig_load("SESSION_MEMCAHCE_HOST",cconfigInstanceZval,&memcacheHost TSRMLS_CC);

			if(IS_STRING == Z_TYPE_P(memcacheHost)){
				char memcacheConnHost[10240];
				sprintf(memcacheConnHost,"%s%s","tcp://",Z_STRVAL_P(memcacheHost));

				ini_set("session.save_handler", "memcache"); 
				ini_set("session.save_path", memcacheConnHost); 

				zval_ptr_dtor(&memcacheHost);
			}
		}
	}

	//是否自动启用session
	CConfig_load("AUTO_SESSION",cconfigInstanceZval,&autoSession TSRMLS_CC);
	if(IS_BOOL == Z_TYPE_P(autoSession) && Z_LVAL_P(autoSession) == 1){
		php_session_start(TSRMLS_C);
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&autoSession);
	zval_ptr_dtor(&sessionMemcacheZval);
}

//初始化注册组件
void CInitApplication_initCDiContainer(TSRMLS_D)
{
	zval	*cconfigInstanceZval,
			*componentsZval,
			*diInstanceZval,
			*setParams,
			*requsetObject,
			*responseZval,
			*pageZval;			

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	CConfig_load("COMPONENTS",cconfigInstanceZval,&componentsZval TSRMLS_CC);

	//获取Di容器对象
	CDiContainer_getInstance(&diInstanceZval TSRMLS_CC);

	//绑定CRequest系统服务
	MAKE_STD_ZVAL(setParams);
	array_init(setParams);
	CRequest_getInstance(&requsetObject TSRMLS_CC);
	CDiContainer_set("CRequest",requsetObject,setParams,diInstanceZval TSRMLS_CC);


	//绑定CResponse系统服务
	MAKE_STD_ZVAL(setParams);
	array_init(setParams);
	CResponse_getInstance(&responseZval TSRMLS_CC);
	CDiContainer_set("CResponse",responseZval,setParams,diInstanceZval TSRMLS_CC);


	//绑定分页系统服务
	MAKE_STD_ZVAL(setParams);
	array_init(setParams);
	CPagination_getInstance(&pageZval TSRMLS_CC);
	CDiContainer_set("CPagination",pageZval,setParams,diInstanceZval TSRMLS_CC);



	//绑定配置文件中的服务
	if(IS_ARRAY == Z_TYPE_P(componentsZval)){
		int i,
			m;
		zval **thisVal;
		char *key;
		ulong ikey;

		m = zend_hash_num_elements(Z_ARRVAL_P(componentsZval));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(componentsZval));
		for(i = 0 ; i < m ; i++){

			zend_hash_get_current_key(Z_ARRVAL_P(componentsZval),&key,&ikey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(componentsZval),(void**)&thisVal);

			MAKE_STD_ZVAL(setParams);
			array_init(setParams);
			zval_add_ref(thisVal);
			CDiContainer_set(key,*thisVal,setParams,diInstanceZval TSRMLS_CC);

			zend_hash_move_forward(Z_ARRVAL_P(componentsZval));
		}
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&componentsZval);
	zval_ptr_dtor(&diInstanceZval);
}

//设置session
PHP_METHOD(CInitApplication,_initSession)
{
	CInitApplication_initSession(TSRMLS_C);
}

//触发自动加载
PHP_METHOD(CInitApplication,classAutoLoad)
{
	char	*className;
	int		classNameLen;
	zval	*cLoaderInstace,
			*getStatus;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&className,&classNameLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_STRING(paramsList[0],className,1);
		CHooks_callHooks("HOOKS_AUTO_LOAD",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END


	//获取CLoader单例对象
	CLoader_getInstance(&cLoaderInstace TSRMLS_CC);

	//调用CLoader的load方法
	CLoader_load(className,&getStatus TSRMLS_CC);

	zval_ptr_dtor(&cLoaderInstace);
	zval_ptr_dtor(&getStatus);
}

//清理框架保存的数组
void CInitApplication_clearFrame(TSRMLS_D){
	zend_update_static_property_null(CHooksCe,ZEND_STRL("_pluginList") TSRMLS_CC);
	zend_update_static_property_null(CHooksCe,ZEND_STRL("_failLoadPluginList") TSRMLS_CC);
}

//结束时函数
PHP_METHOD(CInitApplication,webShutdown)
{
	zval	*errorList = NULL,
			*lastError = NULL,
			*errorInstance = NULL,
			*sapiZval;

	zend_class_entry **cexception;


	zval *responseInstace;
	CResponse_getInstance(&responseInstace TSRMLS_CC);
	CResponse_send(responseInstace TSRMLS_CC);
	zval_ptr_dtor(&responseInstace);


	error_get_last(&lastError);

	//cli模式输出致命错误
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
		php_var_dump(&lastError,1 TSRMLS_CC);
	}

	if(IS_ARRAY == Z_TYPE_P(lastError)){
		zval **errorType;
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(lastError),"type",strlen("type")+1,(void**)&errorType)){

			if(IS_LONG == Z_TYPE_PP(errorType) && (Z_LVAL_PP(errorType) == E_USER_ERROR || Z_LVAL_PP(errorType) == E_PARSE || Z_LVAL_PP(errorType) == E_CORE_ERROR || Z_LVAL_PP(errorType) == E_COMPILE_ERROR || Z_LVAL_PP(errorType) == E_ERROR ) ){
			
				//触发getTop函数
				zval	*code,
						*content,
						*file,
						*line,
						**errContentZval,
						**errorFileZval,
						**errorLineZval;


				MAKE_STD_ZVAL(code);
				zend_hash_find(Z_ARRVAL_P(lastError),"code",strlen("code")+1,(void**)&errorType);
				ZVAL_ZVAL(code,*errorType,1,0);

				MAKE_STD_ZVAL(content);
				zend_hash_find(Z_ARRVAL_P(lastError),"message",strlen("message")+1,(void**)&errContentZval);
				ZVAL_ZVAL(content,*errContentZval,1,0);

				MAKE_STD_ZVAL(file);
				zend_hash_find(Z_ARRVAL_P(lastError),"file",strlen("file")+1,(void**)&errorFileZval);
				ZVAL_ZVAL(file,*errorFileZval,1,0);
				
				MAKE_STD_ZVAL(line);
				zend_hash_find(Z_ARRVAL_P(lastError),"line",strlen("line")+1,(void**)&errorLineZval);
				ZVAL_ZVAL(line,*errorLineZval,1,0);

				CException_getTopErrors(code,content,file,line TSRMLS_CC);

			}
		}
	}


	//获取CException保存的所有错误列表
	zend_hash_find(EG(class_table),"cexception",strlen("cexception")+1,(void**)&cexception);
	CException_getInstance(&errorInstance TSRMLS_CC);
	errorList = zend_read_property(*cexception,errorInstance,ZEND_STRL("errorList"),0 TSRMLS_CC);
	zval_ptr_dtor(&errorInstance);

	//触发执行结束HOOKS_SYSTEM_SHUTDOWN钩子函数
	CHooks_callHooks("HOOKS_SYSTEM_SHUTDOWN",NULL,0 TSRMLS_CC);

	if(errorList == NULL){
		zval_ptr_dtor(&lastError);
		CInitApplication_clearFrame(TSRMLS_C);
		return;
	}


	if(IS_NULL == Z_TYPE_P(errorList)){
		zval_ptr_dtor(&lastError);
		CInitApplication_clearFrame(TSRMLS_C);
		return;
	}


	//将错误记录到系统日志中
	MODULE_BEGIN
		zval	*cconfigInstanceZval,
				*closeLogZval,
				*debugStatus,
				*loadPlugin;

		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("CLOSE_ERRORLOG",cconfigInstanceZval,&closeLogZval TSRMLS_CC);
		CConfig_load("DEBUG",cconfigInstanceZval,&debugStatus TSRMLS_CC);

		if(IS_NULL == Z_TYPE_P(debugStatus)){
			ZVAL_BOOL(debugStatus,1);
		}

		if(IS_NULL == Z_TYPE_P(closeLogZval)){
			ZVAL_BOOL(closeLogZval,0);
		}

		if(IS_BOOL == Z_TYPE_P(debugStatus) && Z_LVAL_P(debugStatus) == 0 && IS_BOOL == Z_TYPE_P(closeLogZval) && Z_LVAL_P(closeLogZval) == 1){
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&closeLogZval);
			zval_ptr_dtor(&debugStatus);
			zval_ptr_dtor(&lastError);
			CInitApplication_clearFrame(TSRMLS_C);
			return;
		}
	
		//将错误记录到logs/error目录
		if(IS_ARRAY == Z_TYPE_P(errorList)){

			int errorNum,i;
			zval **errorDetail,
				 **errId,
				 **errContent,
				 **errFile,
				 **errLine;

			char *errorType,
				 *errorAll,
				 *lineNumStr,
				 *thisMothTime;

			errorNum = zend_hash_num_elements(Z_ARRVAL_P(errorList));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(errorList));

			for(i = 0;i < errorNum ; i++){
				zend_hash_get_current_data(Z_ARRVAL_P(errorList),(void**)&errorDetail);
				if(IS_ARRAY != Z_TYPE_PP(errorDetail)){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}

				if(zend_hash_num_elements(Z_ARRVAL_PP(errorDetail)) != 4){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}


				//取错误ID
				zend_hash_internal_pointer_reset(Z_ARRVAL_PP(errorDetail));
				zend_hash_get_current_data(Z_ARRVAL_PP(errorDetail),(void**)&errId);
				if(IS_LONG != Z_TYPE_PP(errId)){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}

				//错误内容
				zend_hash_move_forward(Z_ARRVAL_PP(errorDetail));
				zend_hash_get_current_data(Z_ARRVAL_PP(errorDetail),(void**)&errContent);
				if(IS_STRING != Z_TYPE_PP(errContent)){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}

				//错误文件
				zend_hash_move_forward(Z_ARRVAL_PP(errorDetail));
				zend_hash_get_current_data(Z_ARRVAL_PP(errorDetail),(void**)&errFile);
				if(IS_STRING != Z_TYPE_PP(errFile)){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}

				//行号
				zend_hash_move_forward(Z_ARRVAL_PP(errorDetail));
				zend_hash_get_current_data(Z_ARRVAL_PP(errorDetail),(void**)&errLine);
				if(IS_LONG != Z_TYPE_PP(errLine)){
					zend_hash_move_forward(Z_ARRVAL_P(errorList));
					continue;
				}

				//获取错误类型
				CException_getErrorLevel(Z_LVAL_PP(errId),&errorType);

				//行号
				toChar(Z_LVAL_PP(errLine),&lineNumStr);
				
				//日期
				php_date("Y-m-d h:i:s",&thisMothTime);

				//内容
				strcat2(&errorAll,"#LogTime:",thisMothTime,PHP_EOL,"[",errorType,"] ",Z_STRVAL_PP(errContent)," ",PHP_EOL,"-File:",Z_STRVAL_PP(errFile)," -Line:",lineNumStr,PHP_EOL,PHP_EOL,NULL);
				efree(thisMothTime);

				if(IS_BOOL == Z_TYPE_P(closeLogZval) && Z_LVAL_P(closeLogZval) == 1){
				}else{
					CLog_writeSystemFile(errorAll TSRMLS_CC);
				}

				//读取是否开启Debug 开启时输出错误提示
				if(IS_BOOL == Z_TYPE_P(debugStatus) && 1 == Z_LVAL_P(debugStatus)){

					//错误输出未被关闭时
					zval *errorPut;
					char *htmlErrorStr;


					CException_getInstance(&errorInstance TSRMLS_CC);
					errorPut = zend_read_property(CExceptionCe,errorInstance,ZEND_STRL("errorOutput"),0 TSRMLS_CC);
					zval_ptr_dtor(&errorInstance);

					if(IS_LONG == Z_TYPE_P(errorPut) && Z_LVAL_P(errorPut) == 1){

							//检查是否致命错误
							if(IS_LONG == Z_TYPE_PP(errId) && 
									( 
										Z_LVAL_PP(errId) == E_ERROR || 
										Z_LVAL_PP(errId) == E_USER_ERROR || 
										Z_LVAL_PP(errId) == E_PARSE || 
										Z_LVAL_PP(errId) == E_CORE_ERROR || 
										Z_LVAL_PP(errId) == E_COMPILE_ERROR
									) 
							){
								str_replace(PHP_EOL,"<br>",errorAll,&htmlErrorStr);
								php_printf("%s<br><br>",htmlErrorStr);
								efree(htmlErrorStr);
							}
					}
				}
			
				efree(errorType);
				efree(lineNumStr);
				efree(errorAll);

				zend_hash_move_forward(Z_ARRVAL_P(errorList));
			}
		}
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&closeLogZval);
		zval_ptr_dtor(&debugStatus);
	MODULE_END

	zval_ptr_dtor(&lastError);

	CInitApplication_clearFrame(TSRMLS_C);
}


//处理请求
PHP_METHOD(CInitApplication,GetRequest)
{

}

//判断权限
void CInitApplication_checkFrameAuth(TSRMLS_D)
{
	//checkVersionAuth(TSRMLS_C);
}

//类方法:构造器
PHP_METHOD(CInitApplication,__construct)
{

	//初始化自动加载
	CInitApplication_initAutoLoad(TSRMLS_C);

	//初始化加载目录
	CInitApplication_initIncludeClass(TSRMLS_C);

	//初始化事件驱动器
	CInitApplication_initHooks(TSRMLS_C);

	//初始化异常处理
	CInitApplication_initTopException(TSRMLS_C);

	//初始化时区等设置
	CInitApplication_initTimeZone(TSRMLS_C);

	//判断权限
	CInitApplication_checkFrameAuth(TSRMLS_C);

	//初始化CookieDomain等
	CInitApplication_initCookie(TSRMLS_C);

	//初始化session
	CInitApplication_initSession(TSRMLS_C);

	//初始化Gzip压缩
	CInitApplication_initGzip(TSRMLS_C);

	//初始化Di容器
	CInitApplication_initCDiContainer(TSRMLS_C);
}
