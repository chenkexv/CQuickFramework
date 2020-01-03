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
#include <zend_exceptions.h>


#include "php_CQuickFramework.h"
#include "php_CException.h"
#include "php_CConfig.h"
#include "php_CWebApp.h"

//zend类方法
zend_function_entry CException_functions[] = {
	PHP_ME(CException,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CException,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(CException,getTopException,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CException,getTopErrors,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CException,closeErrorShow,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CException,getErrorShow,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CException,hasFatalErrors,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CException,filterFileTruePath,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//其他异常方法
zend_function_entry COtherException_functions[] = {
	{NULL, NULL, NULL}
};

zend_class_entry *CQuickFrameworkGetExceptionBase(int root);

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CException)
{
	//注册CWebApp类
	zend_class_entry	funCe,
						**exception,
						*exceptionP;


	//查找Exception异常
	INIT_CLASS_ENTRY(funCe,"CException",CException_functions);
	CExceptionCe = zend_register_internal_class_ex(&funCe,CQuickFrameworkGetExceptionBase(0),NULL TSRMLS_CC);

	//注册变量
	zend_declare_property_null(CExceptionCe, ZEND_STRL("instance"),ZEND_ACC_PUBLIC | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CExceptionCe, ZEND_STRL("errorList"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(CExceptionCe, ZEND_STRL("errorOutput"),1,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

//Cache异常
CMYFRAME_REGISTER_CLASS_RUN(CacheException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CacheException",COtherException_functions);
	CacheExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CRedisException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CRedisException",COtherException_functions);
	CRedisExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CFtpException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CFtpException",COtherException_functions);
	CFtpExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//classNotFound
CMYFRAME_REGISTER_CLASS_RUN(CClassNotFoundException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CClassNotFoundException",COtherException_functions);
	CClassNotFoundExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CDbException
CMYFRAME_REGISTER_CLASS_RUN(CDbException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CDbException",COtherException_functions);
	CDbExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CModelException
CMYFRAME_REGISTER_CLASS_RUN(CModelException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CModelException",COtherException_functions);
	CModelExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CPluginException
CMYFRAME_REGISTER_CLASS_RUN(CPluginException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CPluginException",COtherException_functions);
	CPluginExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CRouteException
CMYFRAME_REGISTER_CLASS_RUN(CRouteException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CRouteException",COtherException_functions);
	CRouteExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CSessionCookieException
CMYFRAME_REGISTER_CLASS_RUN(CSessionCookieException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CSessionCookieException",COtherException_functions);
	CSessionCookieExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CViewException
CMYFRAME_REGISTER_CLASS_RUN(CViewException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CViewException",COtherException_functions);
	CViewExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CThreadException
CMYFRAME_REGISTER_CLASS_RUN(CShellException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CShellException",COtherException_functions);
	CShellExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CHttpException
CMYFRAME_REGISTER_CLASS_RUN(CHttpException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CHttpException",COtherException_functions);
	CHttpExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CQueueException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CQueueException",COtherException_functions);
	CQueueExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CMailException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CMailException",COtherException_functions);
	CMailExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CMicroServerException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CMicroServerException",COtherException_functions);
	CMicroServerExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CRejectException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CRejectException",COtherException_functions);
	CRejectExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CIOException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CIOException",COtherException_functions);
	CIOExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CPoolException)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CPoolException",COtherException_functions);
	CPoolExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}

//CServiceExceptionCe
CMYFRAME_REGISTER_CLASS_RUN(CServiceExceptionCe)
{
	zend_class_entry	funCe;
	INIT_CLASS_ENTRY(funCe,"CServiceExceptionCe",COtherException_functions);
	CServiceExceptionCe = zend_register_internal_class_ex(&funCe,CExceptionCe,NULL TSRMLS_CC);
	return SUCCESS;
}


//类方法:创建应用对象
PHP_METHOD(CException,__construct)
{

}

//获取错误级别
void CException_getErrorLevel(int level,char **errType){

	switch(level){

		case 1:
			*errType = estrdup("FatalException");
			break;

		case 2:
			*errType = estrdup("WarnExpcetion");
			break;

		case 4:
			*errType = estrdup("CompileException");
			break;

		case 8:
			*errType = estrdup("NoticeException");
			break;

		case 16:
			*errType = estrdup("CoreException");
			break;

		case 32:
			*errType = estrdup("CoreWarnException");
			break;

		case 64:
			*errType = estrdup("ZendFatalException");
			break;

		case 128:
			*errType = estrdup("ZendWarnException");
			break;

		case 256:
			*errType = estrdup("UserException");
			break;

		case 512:
			*errType = estrdup("UserWarnException");
			break;

		case 1024:
			*errType = estrdup("UserNoticeException");
			break;

		case 2048:
			*errType = estrdup("CodeNoticeException");
			break;

		case 10000:
			*errType = estrdup("SafeWarnException");
			break;

		default:
			*errType = estrdup("FatalException");

	}
}

void CException_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CExceptionCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zval			*object,
						*saveObject;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CExceptionCe);

		//执行构造器
		if (CExceptionCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CExceptionCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CExceptionCe,ZEND_STRL("instance"),object TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;
}

void CException_getTopErrors(zval *code,zval *content,zval *file,zval *line TSRMLS_DC)
{
	zval	*errorList,
			*saveError,
			*saveData,
			*errorInstance,
			*sapiZval;

	int errorNums;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) != SUCCESS){
		return;
	}

	if( strcmp(Z_STRVAL_P(sapiZval),"cli") == 0 ){
		return;
	}

	//不报告
	if(IS_NULL == Z_TYPE_P(content)){
		return;
	}


	//为运行级别错误时不报表
	if(IS_STRING == Z_TYPE_P(content) && IS_STRING == Z_TYPE_P(file) ){
		if(strstr(Z_STRVAL_P(content),"__runtime") != NULL || strstr(Z_STRVAL_P(file),"__runtime") != NULL ){
			if(IS_LONG == Z_TYPE_P(code) && 
				( 
					Z_LVAL_P(code) == E_ERROR || 
					Z_LVAL_P(code) == E_USER_ERROR || 
					Z_LVAL_P(code) == E_PARSE || 
					Z_LVAL_P(code) == E_CORE_ERROR || 
					Z_LVAL_P(code) == E_COMPILE_ERROR
				) 
			){}else{
				return;
			}
		}
	}

	//检查是否致命错误
	if(IS_LONG == Z_TYPE_P(code) && 
			( 
				Z_LVAL_P(code) == E_ERROR || 
				Z_LVAL_P(code) == E_USER_ERROR || 
				Z_LVAL_P(code) == E_PARSE || 
				Z_LVAL_P(code) == E_CORE_ERROR || 
				Z_LVAL_P(code) == E_COMPILE_ERROR
			) 
	){
		//发送500错误
		sendHttpCode(500 TSRMLS_CC);
	}else{
		//check is debug , release will ignore note warn 
		int isDebugNow = 0;
		isDebugNow = CDebug_getIsDebugStats(TSRMLS_C);
		if(isDebugNow != 1){
			return;
		}
	}

	//读取已存错误
	CException_getInstance(&errorInstance TSRMLS_CC);
	errorList = zend_read_property(CExceptionCe,errorInstance,ZEND_STRL("errorList"),0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(errorList)){
		zval *saveErrorList;
		MAKE_STD_ZVAL(saveErrorList);
		array_init(saveErrorList);
		zend_update_property(CExceptionCe,errorInstance,ZEND_STRL("errorList"),saveErrorList TSRMLS_CC);
		zval_ptr_dtor(&saveErrorList);
		errorList = zend_read_property(CExceptionCe,errorInstance,ZEND_STRL("errorList"),0 TSRMLS_CC);
	}

	if(IS_ARRAY != Z_TYPE_P(errorList)){
		zval_ptr_dtor(&errorInstance);
		return;
	}


	//读取错误记录数
	errorNums = zend_hash_num_elements(Z_ARRVAL_P(errorList));
	if(errorNums > 128){
		zval_ptr_dtor(&errorInstance);
		return;
	}

	MAKE_STD_ZVAL(saveError);
	array_init(saveError);
	add_next_index_long(saveError,Z_LVAL_P(code));
	add_next_index_string(saveError,Z_STRVAL_P(content),1);
	add_next_index_string(saveError,Z_STRVAL_P(file),1);
	add_next_index_long(saveError,Z_LVAL_P(line));
	add_next_index_zval(errorList,saveError);

	zend_update_property(CExceptionCe,errorInstance, ZEND_STRL("errorList"), errorList TSRMLS_CC);

	//触发发生错误后HOOKS_ERROR_HAPPEN钩子
	MODULE_BEGIN
		zval	*paramsList[4],
				param1,
				param2,
				param3,
				param4;
		paramsList[0] = &param1;
		paramsList[1] = &param2;
		paramsList[2] = &param3;
		paramsList[3] = &param4;

		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],code,1,0);

		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_ZVAL(paramsList[1],content,1,0);

		MAKE_STD_ZVAL(paramsList[2]);
		ZVAL_ZVAL(paramsList[2],file,1,0);

		MAKE_STD_ZVAL(paramsList[3]);
		ZVAL_ZVAL(paramsList[3],line,1,0);

		CHooks_callHooks("HOOKS_ERROR_HAPPEN",paramsList,4 TSRMLS_CC);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);

	MODULE_END

	zval_ptr_dtor(&errorInstance);
}

PHP_METHOD(CException,getTopErrors)
{
	
	zval	*code = NULL,
			*content = NULL,
			*file = NULL,
			*line = NULL,
			*other = NULL;


	//发生错误	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzzzz",&code,&content,&file,&line,&other) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CException_getTopErrors(code,content,file,line TSRMLS_CC);
}

PHP_METHOD(CException,__destruct)
{
	zend_update_property_null(CExceptionCe,getThis(), ZEND_STRL("errorList") TSRMLS_CC);
}

PHP_METHOD(CException,getTopException)
{
	zval	*exception,
			*getMessageZval,
			*getFileZval,
			*getLineZval,
			*appPath;

	char	*filePath,
			*tipsPath;

	zend_class_entry	*exceptionCe,
						**webClassCe;

	zend_hash_find(EG(class_table),"cwebapp",strlen("cwebapp")+1,(void**)&webClassCe);
	appPath = zend_read_static_property(*webClassCe,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//将所有未被捕获的异常 转成错误
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&exception) == FAILURE){
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(exception)){
		return;
	}

	//获取异常类
	exceptionCe = Z_OBJCE_P(exception);

	//异常消息
	MODULE_BEGIN
		zval	constructReturn,
				constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "getMessage", 0);
		call_user_function(NULL, &exception, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		getMessageZval = &constructReturn;
	MODULE_END

	//文件
	MODULE_BEGIN
		zval	constructReturn,
				constructVal;

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "getFile", 0);
		call_user_function(NULL, &exception, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		getFileZval = &constructReturn;
		str_replace("\\","/",Z_STRVAL_P(getFileZval),&filePath);
		str_replace(Z_STRVAL_P(appPath),"APP_PATH",filePath,&tipsPath);
		efree(filePath);
	MODULE_END

	//行号
	MODULE_BEGIN
		zval	constructReturn,
				constructVal;

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "getLine", 0);
		call_user_function(NULL, &exception, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		getLineZval = &constructReturn;
	MODULE_END

	php_error_docref(NULL TSRMLS_CC,E_ERROR,"[UncatchException] Uncatched %s : %s - File:%s - Line:%d",exceptionCe->name,Z_STRVAL_P(getMessageZval),tipsPath,Z_LVAL_P(getLineZval));

}


PHP_METHOD(CException,closeErrorShow)
{
	zval *errorInstance;
	CException_getInstance(&errorInstance TSRMLS_CC);
	zend_update_property_long(CExceptionCe,errorInstance, ZEND_STRL("errorOutput"), 0 TSRMLS_CC);
	zval_ptr_dtor(&errorInstance);
	RETVAL_TRUE;
}

PHP_METHOD(CException,getErrorShow)
{
	RETVAL_FALSE;
}

int CException_hasFatalErrors(TSRMLS_D){
	zval *lastError;

	//检测是否发生致命性错误
	error_get_last(&lastError);
	if(IS_ARRAY == Z_TYPE_P(lastError)){
		zval **errorType;
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(lastError),"type",strlen("type")+1,(void**)&errorType)){
			if(IS_LONG == Z_TYPE_PP(errorType) && (Z_LVAL_PP(errorType) == E_USER_ERROR || Z_LVAL_PP(errorType) == E_PARSE || Z_LVAL_PP(errorType) == E_CORE_ERROR || Z_LVAL_PP(errorType) == E_COMPILE_ERROR || Z_LVAL_PP(errorType) == E_ERROR ) ){
				zval_ptr_dtor(&lastError);
				return 1;
			}
		}
	}

	zval_ptr_dtor(&lastError);
	return 0;
}

PHP_METHOD(CException,hasFatalErrors)
{
	int status; 
	status = CException_hasFatalErrors(TSRMLS_C);

	if(status){
		RETURN_TRUE;
	}else{
		RETURN_FALSE;
	}
}

void CException_filterFileTruePath(zval *filePath,char **endPath)
{
	char	*fileLikePath,
			*fileReturn;

	zval *appPath;

	TSRMLS_FETCH();

	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);


	str_replace(Z_STRVAL_P(appPath),"APP_PATH",Z_STRVAL_P(filePath),&fileLikePath);
	str_replace("\\","/",fileLikePath,&*endPath);
	efree(fileLikePath);
}

PHP_METHOD(CException,filterFileTruePath)
{
	zval	*filePath;
	char	*fileLikePath,
			*fileReturn;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&filePath) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	if(IS_STRING != Z_TYPE_P(filePath)){
		RETVAL_ZVAL(filePath,1,0);
		return;
	}

	CException_filterFileTruePath(filePath,&fileReturn);
	RETVAL_STRING(fileReturn,0);
}
