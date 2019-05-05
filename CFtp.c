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
#include "php_CException.h"
#include "php_CFtp.h"


//zend类方法
zend_function_entry CFtp_functions[] = {
	PHP_ME(CFtp,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CFtp,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CFtp,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CFtp,chdir,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,mkdir,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,upload,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,download,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,getConnection,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,deleteFile,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,deleteDir,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,chmod,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,list,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CFtp,close,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CFtp)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CFtp",CFtp_functions);
	CFtpCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CFtpCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CFtpCe, ZEND_STRL("hostname"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CFtpCe, ZEND_STRL("username"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CFtpCe, ZEND_STRL("password"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CFtpCe, ZEND_STRL("port"),21,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CFtpCe, ZEND_STRL("passive"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_bool(CFtpCe, ZEND_STRL("debug"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CFtpCe, ZEND_STRL("conn_id"),ZEND_ACC_PRIVATE TSRMLS_CC);
}

//创建单例对象
int CFtp_getInstance(char *configFileName,zval **returnZval TSRMLS_DC)
{
	zval	*selfInstace = NULL,
			*funResultZval,
			**instaceSaveZval;

	int		configFileNameLen,
			hasExistConfig = 0;


	//读取类单列对象
	selfInstace = zend_read_static_property(CFtpCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CFtpCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CFtpCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(selfInstace), configFileName, strlen(configFileName)+1);

	//不存在尝试实例化
	if(0 == hasExistConfig){
		
		zval	*object = NULL,
				constructReturn,
				constructVal,
				*instaceZval = NULL,
				*toReturnObject = NULL;


		MAKE_STD_ZVAL(object);
		object_init_ex(object,CFtpCe);

		//执行其构造器 并传入参数
		if (CFtpCe->constructor) {
			zval *params[1],
				 param1;

			params[0] = &param1;
			INIT_ZVAL(constructVal);
			MAKE_STD_ZVAL(params[0]);
			ZVAL_STRING(params[0],configFileName,1);
			ZVAL_STRING(&constructVal, CFtpCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, params TSRMLS_CC);
			zval_ptr_dtor(&params[0]);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,configFileName,object);
		zend_update_static_property(CFtpCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		return;
	}

	//直接取instace静态变量中的返回值
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),configFileName,strlen(configFileName)+1,(void**)&instaceSaveZval) ){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,*instaceSaveZval,1,0);
	}
}

PHP_METHOD(CFtp,getInstance)
{
	char	*configFileName = NULL;

	int		configFileNameLen = 0;

	zval	*resultZval;

	//获取配置参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&configFileName,&configFileNameLen) == FAILURE){
		return;
	}

	if(configFileName == NULL){
		configFileName = "main";
	}

	//调用C API
	CFtp_getInstance(configFileName,&resultZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,resultZval,1,1);
}

void CFtp_getConnect(zval *ftpConfig,zval *object TSRMLS_DC)
{
	zval	**hostnameZval,
			**portZval,
			**usernameZval,
			**passwordZval,
			**passiveZval,
			*conn,
			*saveConn;

	int		port = 21,
			passive = 1;

	if(SUCCESS != zend_hash_find(Z_ARRVAL_P(ftpConfig),"hostname",strlen("hostname")+1,(void**)&hostnameZval) || IS_STRING != Z_TYPE_PP(hostnameZval)){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] call ftp object ,but the config lost hostname", 10001 TSRMLS_CC);
		return;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(ftpConfig),"port",strlen("port")+1,(void**)&portZval) && IS_LONG == Z_TYPE_PP(portZval)){
		port = Z_LVAL_PP(portZval);
	}

	if(SUCCESS != zend_hash_find(Z_ARRVAL_P(ftpConfig),"username",strlen("username")+1,(void**)&usernameZval) || IS_STRING != Z_TYPE_PP(usernameZval)){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] call ftp object ,but the config lost username", 10001 TSRMLS_CC);
		return;
	}

	if(SUCCESS != zend_hash_find(Z_ARRVAL_P(ftpConfig),"password",strlen("password")+1,(void**)&passwordZval) || IS_STRING != Z_TYPE_PP(passwordZval)){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] call ftp object ,but the config lost password", 10001 TSRMLS_CC);
		return;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(ftpConfig),"passive",strlen("passive")+1,(void**)&passiveZval) && IS_BOOL == Z_TYPE_PP(passiveZval)){
		passive = Z_LVAL_PP(passiveZval);
	}

	//call ftp_connect
	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];

		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_PP(hostnameZval),1);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[1],port);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_connect", 0);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		conn = &callReturn;

		if(IS_RESOURCE != Z_TYPE_P(conn)){
			char	errMessage[1024];
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			zval_dtor(&callReturn);
			sprintf(errMessage,"%s%s%s%d%s","[CFtpException] connect to ftp[",Z_STRVAL_PP(hostnameZval),":",port,"] failed");
			zend_throw_exception(CFtpExceptionCe, errMessage, 10001 TSRMLS_CC);
			return;
		}
		MAKE_STD_ZVAL(saveConn);
		ZVAL_ZVAL(saveConn,conn,1,0);
		zend_update_property(CFtpCe,object,ZEND_STRL("conn_id"),saveConn TSRMLS_CC);
		zval_dtor(&callReturn);
		zval_ptr_dtor(&saveConn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	MODULE_END

	//login
	conn = zend_read_property(CFtpCe,object,ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		char	errMessage[1024];
		sprintf(errMessage,"%s%s%s%d%s","[CFtpException] connect to ftp[",Z_STRVAL_PP(hostnameZval),":",port,"] failed");
		zend_throw_exception(CFtpExceptionCe, errMessage, 10001 TSRMLS_CC);
		return;
	}

	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[3];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_login", 0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],conn,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],Z_STRVAL_PP(usernameZval),1);
		MAKE_STD_ZVAL(params[2]);
		ZVAL_STRING(params[2],Z_STRVAL_PP(passwordZval),1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 3, params TSRMLS_CC);

		if(IS_BOOL == Z_TYPE(callReturn) && 1 == Z_LVAL(callReturn)){
		}else{
			char	errMessage[1024];
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			zval_ptr_dtor(&params[2]);
			zval_dtor(&callReturn);
			sprintf(errMessage,"%s%s%s%s%s","[CFtpException] login to ftp[",Z_STRVAL_PP(usernameZval),":",Z_STRVAL_PP(hostnameZval),"] failed");
			zend_throw_exception(CFtpExceptionCe, errMessage, 10001 TSRMLS_CC);
			return;
		}
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
	MODULE_END

	//use passive
	if(passive == 1){
		zval	callFunction,
				callReturn,
				*params[2];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_pasv", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_BOOL(params[1],1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	}

	zend_update_property(CFtpCe,object,ZEND_STRL("conn_id"),conn TSRMLS_CC);
}

PHP_METHOD(CFtp,__construct)
{
	char	*configName,
			nowConfigName[128];

	int		configNameLen = 0;

	zval	*cconfigInstanceZval,
			*ftpConfig;

	//check extension exists
	if (!zend_hash_exists(&module_registry, "ftp", strlen("ftp")+1)) {
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] Call [CFtp->__construct] need install the [ftp] extension ", 10001 TSRMLS_CC);
		return;
	}


	//获取配置参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&configName,&configNameLen) == FAILURE){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] Call [CFtp->__construct] params error ", 10001 TSRMLS_CC);
		return;
	}

	sprintf(nowConfigName,"%s%s","ftp.",configName);

	//read config
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load(nowConfigName,cconfigInstanceZval,&ftpConfig TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(ftpConfig)){
		char	errorMessage[1024];
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&ftpConfig);
		sprintf(errorMessage,"%s%s","[CFtpException] call ftp object ,but the config not exists : ",nowConfigName);
		zend_throw_exception(CFtpExceptionCe, errorMessage, 10001 TSRMLS_CC);
		return;
	}

	//check this configs is right
	if(zend_hash_exists(Z_ARRVAL_P(ftpConfig),"hostname",strlen("hostname")) && zend_hash_exists(Z_ARRVAL_P(ftpConfig),"username",strlen("username"))){
		char	errorMessage[1024];
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&ftpConfig);
		sprintf(errorMessage,"%s%s","[CFtpException] call ftp object ,but the config lost hostname or username ",nowConfigName);
		zend_throw_exception(CFtpExceptionCe, errorMessage, 10001 TSRMLS_CC);
		return;
	}

	//connect
	CFtp_getConnect(ftpConfig,getThis() TSRMLS_CC);

	//destroy
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&ftpConfig);
}

PHP_METHOD(CFtp,__destruct)
{

}

PHP_METHOD(CFtp,chdir)
{
	char	*path;
	zval	*conn;

	int		pathLen = 0,
			mode = 755;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_chdir", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_STRING(params[1],path,1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		
		if(Z_TYPE(callReturn) == IS_BOOL && 0 == Z_LVAL(callReturn)){
			RETVAL_FALSE;
		}else{
			RETVAL_TRUE;
		}
	
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	MODULE_END
}

PHP_METHOD(CFtp,mkdir)
{
	char	*path;
	zval	*conn;

	int		pathLen = 0,
			mode = 755;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_mkdir", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_STRING(params[1],path,1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		
		if(Z_TYPE(callReturn) == IS_BOOL && 0 == Z_LVAL(callReturn)){
			RETVAL_FALSE;
		}else{
			RETVAL_TRUE;
		}
	
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
	MODULE_END
}

//bin=2   ascill=1
int CFtp_getSendType(char *filetype TSRMLS_DC){

	if(
		strcmp(filetype,"txt") == 0 || 
		strcmp(filetype,"text") == 0 || 
		strcmp(filetype,"php") == 0 || 
		strcmp(filetype,"phps") == 0 || 
		strcmp(filetype,"php4") == 0 || 
		strcmp(filetype,"js") == 0 || 
		strcmp(filetype,"css") == 0 || 
		strcmp(filetype,"htm") == 0 || 
		strcmp(filetype,"html") == 0 || 
		strcmp(filetype,"phtml") == 0 || 
		strcmp(filetype,"phtml") == 0 || 
		strcmp(filetype,"shtml") == 0 || 
		strcmp(filetype,"log") == 0 || 
		strcmp(filetype,"xml") == 0 || 
		strcmp(filetype,"data") == 0 || 
		strcmp(filetype,"txt") == 0
	){
		return 1;
	}

	return 2;
}

void CFtp_list(zval *conn,char *truePath,zval **list TSRMLS_DC){

	zval	callFunction,
			callReturn,
			*returnData,
			*params[2];
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction, "ftp_nlist", 0);
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_ZVAL(params[0],conn,1,0);
	ZVAL_STRING(params[1],truePath,1);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
	returnData = &callReturn;
	MAKE_STD_ZVAL(*list);
	ZVAL_ZVAL(*list,returnData,1,0);
	zval_dtor(&callReturn);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
}

PHP_METHOD(CFtp,upload)
{
	char	*remotePath,
			*localPath;

	int		remotePathLen = 0,
			localPathLen = 0,
			mode = 0;

	zval	*conn;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|l",&remotePath,&remotePathLen,&localPath,&localPathLen,&mode) == FAILURE){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] call upload params array,need give upload(string removePath,string localPath,int mode)", 10001 TSRMLS_CC);
		return;
	}

	//get conn
	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] get connection failed", 10001 TSRMLS_CC);
		return;
	}

	//check file exists
	if(SUCCESS != fileExist(localPath)){
		zend_throw_exception(CFtpExceptionCe, "[CFtpException] call upload but the local file is not exist", 10001 TSRMLS_CC);
		return;
	}

	if(mode == 0){
		//shi pei
		char	*fileType;
		get_file_extname(localPath,&fileType TSRMLS_CC);
		mode = CFtp_getSendType(fileType TSRMLS_CC);
		efree(fileType);
	}
	
	//ftp_put
	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[4];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_put", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		MAKE_STD_ZVAL(params[3]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_STRING(params[1],remotePath,1);
		ZVAL_STRING(params[2],localPath,1);
		ZVAL_LONG(params[3],mode);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 4, params TSRMLS_CC);

		if(Z_TYPE(callReturn) == IS_BOOL && 1 == Z_LVAL(callReturn)){
			RETVAL_TRUE;
		}else{
			RETVAL_FALSE;
		}
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
	MODULE_END

}

PHP_METHOD(CFtp,download)
{}

PHP_METHOD(CFtp,getConnection)
{
	zval	*conn;

	//get conn
	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	RETVAL_ZVAL(conn,1,0);
}

int CFtp_deleteFile(zval *conn,char *file TSRMLS_DC){

	zval	callFunction,
			callReturn,
			*params[2];
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction, "ftp_delete", 0);
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_ZVAL(params[0],conn,1,0);
	ZVAL_STRING(params[1],file,1);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
	if(Z_TYPE(callReturn) == IS_BOOL && 1 == Z_LVAL(callReturn)){
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		return 1;
	}else{
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		return 0;
	}
}

PHP_METHOD(CFtp,deleteFile)
{
	char	*path;
	zval	*conn;

	int		pathLen = 0,
			status = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	status = CFtp_deleteFile(conn,path TSRMLS_CC);
	RETVAL_BOOL(status);
}

int CFtp_deleteDir(zval *conn,char *path TSRMLS_DC){

	zval	*fileList;

	//get files list
	CFtp_list(conn,path,&fileList TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(fileList)){
		int		i,h;
		zval	**thisData;
		h = zend_hash_num_elements(Z_ARRVAL_P(fileList));
		for(i = 0 ; i < h ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(fileList),(void**)&thisData);
			//delete file
			if(!CFtp_deleteFile(conn,Z_STRVAL_PP(thisData) TSRMLS_CC)){
				CFtp_deleteDir(conn,Z_STRVAL_PP(thisData) TSRMLS_CC);
			}
			zend_hash_move_forward(Z_ARRVAL_P(fileList));
		}
	}
	zval_ptr_dtor(&fileList);

	//call rmdir
	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[2];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_rmdir", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_STRING(params[1],path,1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		if(Z_TYPE(callReturn) == IS_BOOL && 1 == Z_LVAL(callReturn)){
			zval_dtor(&callReturn);
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			return 1;
		}else{
			zval_dtor(&callReturn);
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			return 0;
		}
	MODULE_END
}

PHP_METHOD(CFtp,deleteDir)
{
	zval	*conn;

	char	*path;
	int		pathLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}


	CFtp_deleteDir(conn,path TSRMLS_CC);
	
}

PHP_METHOD(CFtp,chmod)
{
	char	*path,
			*mode;
	zval	*conn;

	int		pathLen = 0,
			modeLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&path,&pathLen,&mode,&modeLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	MODULE_BEGIN
		zval	callFunction,
				callReturn,
				*params[3];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction, "ftp_chmod", 0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		ZVAL_ZVAL(params[0],conn,1,0);
		ZVAL_STRING(params[1],mode,1);
		ZVAL_STRING(params[2],path,1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 3, params TSRMLS_CC);
		
		if(Z_TYPE(callReturn) == IS_BOOL && 0 == Z_LVAL(callReturn)){
			RETVAL_FALSE;
		}else{
			RETVAL_TRUE;
		}
	
		zval_dtor(&callReturn);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
	MODULE_END
}


PHP_METHOD(CFtp,list)
{
	zval	*conn,
			*returnData;

	char	*path,
			*truePath;
	int		pathLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&path,&pathLen) == FAILURE){
		RETURN_FALSE;
	}

	conn = zend_read_property(CFtpCe,getThis(),ZEND_STRL("conn_id"),1 TSRMLS_CC);

	if(IS_RESOURCE != Z_TYPE_P(conn)){
		RETURN_FALSE;
	}

	if(pathLen == 0){
		truePath = estrdup(".");
	}else{
		truePath = estrdup(path);
	}

	CFtp_list(conn,truePath,&returnData TSRMLS_CC);
	RETVAL_ZVAL(returnData,1,1);
	efree(truePath);
}

PHP_METHOD(CFtp,close)
{}