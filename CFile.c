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
#include "php_CException.h"
#include "php_CFile.h"

#ifndef PHP_WIN32
#include <sys/types.h>
#include <fcntl.h>
#include <aio.h>
#endif


//zend类方法
zend_function_entry CFile_functions[] = {
	PHP_ME(CFile,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CFile,getPath,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,getFileName,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,getFileType,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,append,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,readAll,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,setContent,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,save,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,copy,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,rename,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,getLastError,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,delete,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,exist,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,writeAll,NULL,ZEND_ACC_PUBLIC)

#ifndef PHP_WIN32
	PHP_ME(CFile,asyncAppend,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,asyncReadAll,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFile,asyncWriteAll,NULL,ZEND_ACC_PUBLIC)
#endif

	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CFile)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CFile",CFile_functions);
	CFileCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CFileCe, ZEND_STRL("filePath"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CFileCe, ZEND_STRL("pathInfo"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CFileCe, ZEND_STRL("content"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CFileCe, ZEND_STRL("lastError"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	return SUCCESS;
}

#ifndef PHP_WIN32
PHP_METHOD(CFile,asyncAppend)
{
	char	*content;
	int		contentLen = 0;

	zval	*sapiZval,
			*filePath;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		zend_throw_exception(CIOExceptionCe, "[CIOException] call [CFile->asyncAppend] is only used in cli", 10001 TSRMLS_CC);
		return;
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&content,&contentLen) == FAILURE){
		return;
	}

	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(SUCCESS != fileExist(Z_STRVAL_P(filePath))){
		char errMessage[1024];
		sprintf(errMessage,"[CIOException] call [CFile->asyncAppend] , the file not exists : %s",Z_STRVAL_P(filePath));
		zend_throw_exception(CIOExceptionCe, errMessage, 10001 TSRMLS_CC);
		return;
	}

	//


}

PHP_METHOD(CFile,asyncReadAll)
{
	zval	*sapiZval;

	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		zend_throw_exception(CIOExceptionCe, "[CIOException] call [CFile->asyncAppend] is only used in cli", 10001 TSRMLS_CC);
		return;
	}




}

PHP_METHOD(CFile,asyncWriteAll){

	zval *sapiZval;
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
	}else{
		zend_throw_exception(CIOExceptionCe, "[CIOException] call [CFile->asyncAppend] is only used in cli", 10001 TSRMLS_CC);
		return;
	}
}
#endif

PHP_METHOD(CFile,__construct){

	char	*filePath;

	int		filePathLen = 0;

	zval	functionName,
			functionReturn,
			*params[1],
			*saveInfo,
			*saveToClass;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&filePath,&filePathLen) == FAILURE){
		return;
	}

	zend_update_property_string(CFileCe,getThis(),ZEND_STRL("filePath"),filePath TSRMLS_CC);

	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"pathinfo",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],filePath,1);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	saveInfo = &functionReturn;
	MAKE_STD_ZVAL(saveToClass);
	ZVAL_ZVAL(saveToClass,saveInfo,1,0);
	zend_update_property(CFileCe,getThis(),ZEND_STRL("pathInfo"),saveToClass TSRMLS_CC);
	zval_dtor(&functionReturn);
	zval_ptr_dtor(&saveToClass);
}

PHP_METHOD(CFile,getPath){
	zval	*pathInfo,
			**thisVal;
	pathInfo = zend_read_property(CFileCe,getThis(),ZEND_STRL("pathInfo"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(pathInfo) && SUCCESS == zend_hash_find(Z_ARRVAL_P(pathInfo),"dirname",strlen("dirname")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) ){
		RETVAL_STRING(Z_STRVAL_PP(thisVal),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CFile,getFileName){
	zval	*pathInfo,
			**thisVal;
	pathInfo = zend_read_property(CFileCe,getThis(),ZEND_STRL("pathInfo"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(pathInfo) && SUCCESS == zend_hash_find(Z_ARRVAL_P(pathInfo),"basename",strlen("basename")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) ){
		RETVAL_STRING(Z_STRVAL_PP(thisVal),1);
	}else{
		RETVAL_STRING("",1);
	}
}


PHP_METHOD(CFile,getFileType){
	zval	*pathInfo,
			**thisVal;
	pathInfo = zend_read_property(CFileCe,getThis(),ZEND_STRL("pathInfo"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(pathInfo) && SUCCESS == zend_hash_find(Z_ARRVAL_P(pathInfo),"extension",strlen("extension")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) ){
		RETVAL_STRING(Z_STRVAL_PP(thisVal),1);
	}else{
		RETVAL_STRING("",1);
	}
}

PHP_METHOD(CFile,readAll){

	zval	*filePath;
	char	*fileContent;
	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(SUCCESS != fileExist(Z_STRVAL_P(filePath))){
		zend_throw_exception(CIOExceptionCe, "[CIOException] call [CFile->readAll] , the file not exists", 10001 TSRMLS_CC);
		return;
	}
	file_get_contents(Z_STRVAL_P(filePath),&fileContent);
	RETVAL_STRING(fileContent,0);
}

PHP_METHOD(CFile,append){

	char	*string;
	int		stringLen = 0;
	zval	functionName,
			functionReturn,
			*params[3],
			*filePath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(SUCCESS != fileExist(Z_STRVAL_P(filePath))){
		zend_throw_exception(CIOExceptionCe, "[CIOException] call [CFile->readAll] , the file not exists", 10001 TSRMLS_CC);
		return;
	}

	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"file_put_contents",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],Z_STRVAL_P(filePath),1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],string,1);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_LONG(params[2],8);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 3, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	if(IS_BOOL == Z_TYPE(functionReturn) && 0 == Z_LVAL(functionReturn)){
		RETVAL_FALSE;
	}
	RETVAL_TRUE;
	zval_dtor(&functionReturn);
}

PHP_METHOD(CFile,setContent)
{
	char	*string;
	int		stringLen = 0;
	zval	*filePath;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}
	zend_update_property_string(CFileCe,getThis(),ZEND_STRL("content"),string TSRMLS_CC);
}

PHP_METHOD(CFile,save){
	zval	*filePath,
			*content;
	int		isover = 1;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&isover) == FAILURE){
		return;
	}
	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	content = zend_read_property(CFileCe,getThis(),ZEND_STRL("content"),0 TSRMLS_CC);

	//not overwrite
	if(isover == 0 && SUCCESS == fileExist(Z_STRVAL_P(filePath))){
		RETURN_FALSE;
	}

	if(0 != file_put_contents(Z_STRVAL_P(filePath),Z_STRVAL_P(content))){
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

int CFile_delete(char *filePath TSRMLS_DC){
	zval	functionName,
			functionReturn,
			*params[1];

	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"unlink",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],filePath,1);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	if(IS_BOOL == Z_TYPE(functionReturn) && 1 == Z_LVAL(functionReturn)){
		zval_dtor(&functionReturn);
		return 1;
	}
	zval_dtor(&functionReturn);
	return 0;
}

int CFile_copy(char *filePath,char *toPath TSRMLS_DC){
	zval	functionName,
			functionReturn,
			*saveReturn,
			*params[2];

	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"copy",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],filePath,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],toPath,1);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	if(IS_BOOL == Z_TYPE(functionReturn) && 1 == Z_LVAL(functionReturn)){
		zval_dtor(&functionReturn);
		return 1;
	}
	zval_dtor(&functionReturn);
	return 0;
}

int CFile_rename(char *filePath,char *toPath TSRMLS_DC){
	zval	functionName,
			functionReturn,
			*params[2];

	INIT_ZVAL(functionName);
	ZVAL_STRING(&functionName,"rename",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],filePath,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],toPath,1);
	call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	if(IS_BOOL == Z_TYPE(functionReturn) && 1 == Z_LVAL(functionReturn)){
		zval_dtor(&functionReturn);
		return 1;
	}
	zval_dtor(&functionReturn);
	return 0;
}

PHP_METHOD(CFile,copy){

	char	*string;
	int		stringLen = 0,
			isover = 1,
			copyStatus;

	zval	*filePath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|b",&string,&stringLen,&isover) == FAILURE){
		return;
	}

	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);

	if(isover && SUCCESS == fileExist(string)){
		CFile_delete(string TSRMLS_CC);
	}

	copyStatus = CFile_copy(Z_STRVAL_P(filePath),string TSRMLS_CC);
	if(copyStatus){
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}

PHP_METHOD(CFile,rename){

	char	*string;
	int		stringLen = 0,
			isover = 1,
			copyStatus;

	zval	*filePath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|b",&string,&stringLen,&isover) == FAILURE){
		return;
	}

	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);

	if(isover && SUCCESS == fileExist(string)){
		CFile_delete(string TSRMLS_CC);
	}

	copyStatus = CFile_rename(Z_STRVAL_P(filePath),string TSRMLS_CC);
	if(copyStatus){
		zend_update_property_string(CFileCe,getThis(),ZEND_STRL("filePath"),string TSRMLS_CC);
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}

PHP_METHOD(CFile,getLastError){
	zval	*lastError;
	lastError = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	RETVAL_STRING(Z_STRVAL_P(lastError),1);
}

PHP_METHOD(CFile,delete){
	zval	*filePath;
	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(SUCCESS != fileExist(Z_STRVAL_P(filePath))){
		RETURN_FALSE;
	}
	if(CFile_delete(Z_STRVAL_P(filePath) TSRMLS_CC)){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

PHP_METHOD(CFile,exist){
	zval	*filePath;
	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(SUCCESS != fileExist(Z_STRVAL_P(filePath))){
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

PHP_METHOD(CFile,writeAll){
	char	*string;
	int		stringLen = 0;
	zval	*filePath;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	filePath = zend_read_property(CFileCe,getThis(),ZEND_STRL("filePath"),0 TSRMLS_CC);
	if(file_put_contents(Z_STRVAL_P(filePath),string)){
		RETURN_TRUE;
	}
	RETVAL_FALSE;
}
