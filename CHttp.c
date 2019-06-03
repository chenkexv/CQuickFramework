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
#include "php_CHttp.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CHttp_functions[] = {
	PHP_ME(CHttp,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CHttp,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHttp,sendHttpRequest,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHttp,setHeader,NULL,ZEND_ACC_PUBLIC )

	PHP_ME(CHttp,setUesrAgentAndroid,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setUesrAgentiPhone,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setUesrAgentiPad,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setUesrAgentChrome,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setUesrAgentFireFox,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setCookie,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setCertificate,NULL,ZEND_ACC_PUBLIC )

	PHP_ME(CHttp,setMethod,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttp,setTimeout,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttp,setParams,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttp,fllowRedirect,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttp,send,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttp,setUrl,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CHttp)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHttp",CHttp_functions);
	CHttpCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CHttpCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHttpCe, ZEND_STRL("header"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CHttpCe, ZEND_STRL("objectName"),"main",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CHttpCe, ZEND_STRL("url"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CHttpCe, ZEND_STRL("method"),"GET",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CHttpCe, ZEND_STRL("timeout"),30,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CHttpCe, ZEND_STRL("params"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CHttpCe, ZEND_STRL("fllowRedirct"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CHttpCe, ZEND_STRL("certPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

zend_function_entry CHttpResponse_functions[] = {
	PHP_ME(CHttpResponse,getHttpCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpResponse,getCastInfo,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpResponse,getContent,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpResponse,getNamelookTime,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getTotalTime,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,setData,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,asArray,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getRedirect,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getPrimaryIp,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getConnectTime,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getContentType,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getRequestHeader,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getResponseHeader,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,isSuccess,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CHttpResponse,getErrorInfo,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CHttpResponse)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHttpResponse",CHttpResponse_functions);
	CHttpResponseCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CHttpResponseCe, ZEND_STRL("data"),ZEND_ACC_PRIVATE TSRMLS_CC);
	return SUCCESS;
}

void CHttp_setUserAgent(char *ua,zval *object TSRMLS_DC){
	zval	*nowHeader,
			**thisVal;
	int		i,h;
	char	*key;
	ulong	ukey;
	nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(nowHeader)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CHttpCe,object,ZEND_STRL("header"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(nowHeader));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(nowHeader));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(nowHeader),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(nowHeader),&key,&ukey,0);
		if(IS_STRING == Z_TYPE_PP(thisVal) && strstr(Z_STRVAL_PP(thisVal),"User-Agent") != NULL){
			zend_hash_index_del(Z_ARRVAL_P(nowHeader),ukey);
			break;
		}
		zend_hash_move_forward(Z_ARRVAL_P(nowHeader));
	}
	add_next_index_string(nowHeader,ua,1);
}

void CHttp_sendHttpRequest(char *geturl,zval *params,char *getrequsetType,zval *header,int timeout,char *sslPath,zval **returnZval TSRMLS_DC)
{

	char *requsetUrl = "",
			*requsetType,
			*url;

	zval	*curlObject,
			*sendParams,
			*curlExecZval,
			*curlInfo,
			*curlError;

	int		headNum = 0;

	HashTable *resultTable;
	
	url = estrdup(geturl);
	requsetType = estrdup(getrequsetType);
	php_strtolower(requsetType,strlen(requsetType)+1);

	//请求对象
	curl_init(&curlObject);
	
	//GET方式
	if(strcmp(requsetType,"get") == 0){
		char *paramsQuery;
		http_build_query(params,&paramsQuery);
		if(strstr(url,"?") == NULL){
			strcat2(&requsetUrl,url,"?",paramsQuery,NULL);
		}else{
			char tempUrl[10240];
			sprintf(tempUrl,"%s%s%s",url,"&",paramsQuery);
			requsetUrl = estrdup(tempUrl);
		}

		//设置URL
		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,requsetUrl,1);
		curl_setopt(curlObject,10002,sendParams);
		zval_ptr_dtor(&sendParams);

		efree(requsetUrl);
		efree(paramsQuery);
	}else{
		//POST请求
		char *paramsQuery = "";
		if(IS_ARRAY == Z_TYPE_P(params)){
			http_build_query(params,&paramsQuery);
		}else if(IS_STRING == Z_TYPE_P(params)){
			paramsQuery = estrdup(Z_STRVAL_P(params));
		}else{
			paramsQuery = estrdup("");
		}

		//设置URL 使用POST
		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,url,1);
		curl_setopt(curlObject,10002,sendParams);
		zval_ptr_dtor(&sendParams);

		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,paramsQuery,1);
		curl_setopt(curlObject,10015,sendParams);
		zval_ptr_dtor(&sendParams);
		efree(paramsQuery);

		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,1);
		curl_setopt(curlObject,47,sendParams);
		zval_ptr_dtor(&sendParams);
	}

	if(strlen(sslPath) == 0 ){
		//HTTPS不验证证书 CURLOPT_SSL_VERIFYPEER
		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,0);
		curl_setopt(curlObject,64,sendParams);
		zval_ptr_dtor(&sendParams);

		//CURLOPT_SSL_VERIFYHOST
		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,0);
		curl_setopt(curlObject,81,sendParams);
		zval_ptr_dtor(&sendParams);
	}else{
		//set https ca cert check
		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,1);
		curl_setopt(curlObject,64,sendParams);
		zval_ptr_dtor(&sendParams);

		//CURLOPT_SSL_VERIFYHOST  
		MAKE_STD_ZVAL(sendParams);
		ZVAL_LONG(sendParams,2);
		curl_setopt(curlObject,81,sendParams);
		zval_ptr_dtor(&sendParams);

		//CURLOPT_CAINFO
		MAKE_STD_ZVAL(sendParams);
		ZVAL_STRING(sendParams,sslPath,1);
		curl_setopt(curlObject,10065,sendParams);
		zval_ptr_dtor(&sendParams);

	}

	//CURLOPT_RETURNTRANSFER
	MAKE_STD_ZVAL(sendParams);
	ZVAL_BOOL(sendParams,1);
	curl_setopt(curlObject,19913,sendParams);
	zval_ptr_dtor(&sendParams);

	//CURLOPT_SSLVERSION
	MAKE_STD_ZVAL(sendParams);
	ZVAL_LONG(sendParams,1);
	curl_setopt(curlObject,32,sendParams);
	zval_ptr_dtor(&sendParams);


	//header
	if(IS_ARRAY == Z_TYPE_P(header)){
		headNum = zend_hash_num_elements(Z_ARRVAL_P(header));
	}
	if(headNum > 0){

		//CURLOPT_HTTPHEADER
		MAKE_STD_ZVAL(sendParams);
		ZVAL_ZVAL(sendParams,header,1,0);
		curl_setopt(curlObject,10023,sendParams);
		zval_ptr_dtor(&sendParams);

		//CURLINFO_HEADER_OUT
		MAKE_STD_ZVAL(sendParams);
		ZVAL_BOOL(sendParams,1);
		curl_setopt(curlObject,2,sendParams);
		zval_ptr_dtor(&sendParams);

	}

	//CURLOPT_HEADER return responseHeader
	MAKE_STD_ZVAL(sendParams);
	ZVAL_BOOL(sendParams,1);
	curl_setopt(curlObject,42,sendParams);
	zval_ptr_dtor(&sendParams);


	//超时 CURLOPT_TIMEOUT
	MAKE_STD_ZVAL(sendParams);
	ZVAL_LONG(sendParams,timeout);
	curl_setopt(curlObject,13,sendParams);
	zval_ptr_dtor(&sendParams);

	//执行
	curl_exec(curlObject,&curlExecZval);

	//消息
	curl_getinfo(curlObject,&curlInfo);

	//错误
	curl_error(curlObject,&curlError);

	//返回
	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);
	add_assoc_zval(*returnZval,"content",curlExecZval);
	add_assoc_zval(*returnZval,"info",curlInfo);
	add_assoc_zval(*returnZval,"error",curlError);

	efree(url);
	efree(requsetType);
	zval_ptr_dtor(&curlObject);



	return;
}

PHP_METHOD(CHttp,setUesrAgentAndroid){
	RETVAL_ZVAL(getThis(),1,0);
	CHttp_setUserAgent("User-Agent: Mozilla/5.0 (Linux; Android 8.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Mobile Safari/537.36",getThis() TSRMLS_CC);
}

PHP_METHOD(CHttp,setUesrAgentiPhone){
	RETVAL_ZVAL(getThis(),1,0);
	CHttp_setUserAgent("User-Agent: Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1",getThis() TSRMLS_CC);
}

PHP_METHOD(CHttp,setUesrAgentiPad){
	RETVAL_ZVAL(getThis(),1,0);
	CHttp_setUserAgent("User-Agent: Mozilla/5.0 (iPad; CPU OS 11_0 like Mac OS X) AppleWebKit/604.1.34 (KHTML, like Gecko) Version/11.0 Mobile/15A5341f Safari/604.1",getThis() TSRMLS_CC);
}

PHP_METHOD(CHttp,setUesrAgentChrome){
	RETVAL_ZVAL(getThis(),1,0);
	CHttp_setUserAgent("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36",getThis() TSRMLS_CC);
}

PHP_METHOD(CHttp,setUesrAgentFireFox){
	RETVAL_ZVAL(getThis(),1,0);
	CHttp_setUserAgent("User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:60.0) Gecko/20100101 Firefox/60.0",getThis() TSRMLS_CC);
}

void CHttp_setCookie(char *cookie,zval *object TSRMLS_DC){
	zval	*nowHeader,
			**thisVal;
	int		i,h,exist = 0;
	char	*key;
	ulong	ukey;
	nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(nowHeader)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CHttpCe,object,ZEND_STRL("header"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(nowHeader));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(nowHeader));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(nowHeader),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(nowHeader),&key,&ukey,0);

		// > 4k will no langer save;
		if(strlen(Z_STRVAL_PP(thisVal)) > 1024*4){
			return;
		}

		if(strstr(Z_STRVAL_PP(thisVal),"Cookie") != NULL){
			char	*tempCookie;
			spprintf(&tempCookie,0,"%s%s%s",Z_STRVAL_PP(thisVal),";",cookie);
			zend_hash_index_del(Z_ARRVAL_P(nowHeader),ukey);
			add_index_string(nowHeader,ukey,tempCookie,0);
			exist = 1;
		}
		zend_hash_move_forward(Z_ARRVAL_P(nowHeader));
	}


	if(exist == 0){
		char	*tempCookie;
		spprintf(&tempCookie,0,"%s%s","Cookie:",cookie);
		add_next_index_string(nowHeader,tempCookie,0);
	}
}

PHP_METHOD(CHttp,setCookie){
	zval	*params;
	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&params) == FAILURE){
		return;
	}

	if(IS_STRING == Z_TYPE_P(params)){
		CHttp_setCookie(Z_STRVAL_P(params),getThis() TSRMLS_CC);
	}else if(IS_ARRAY == Z_TYPE_P(params)){
		int		i,h;
		zval	**thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_P(params));
		for(i = 0 ; i < h; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(params),(void**)&thisVal);
			CHttp_setCookie(Z_STRVAL_PP(thisVal),getThis() TSRMLS_CC);
			zend_hash_move_forward(Z_ARRVAL_P(params));
		}
	}

}

PHP_METHOD(CHttp,fllowRedirect)
{
	long	fllowRedicrt;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&fllowRedicrt) == FAILURE){
		return;
	}
	zend_update_property_long(CHttpCe,getThis(),ZEND_STRL("fllowRedirct"),fllowRedicrt TSRMLS_CC);
}

PHP_METHOD(CHttp,sendHttpRequest){

	zend_function *curlFe;

	char	*url,
			*requsetTypeChar = "GET";

	zval	*params = NULL,
			*requsetType = NULL,
			*header = NULL,
			*returnZval,
			*useParams,
			*useRequestType,
			*useHeader;
	
	int		urlLen;
	long		timeout = 10;

	if(zend_hash_find(EG(function_table),"curl_init",strlen("curl_init")+1,(void**)&curlFe) == FAILURE){
		zend_throw_exception(CHttpExceptionCe, "[CHttpException] System is not installed the curl extensions, unable to curl sending HTTP requests", 7001 TSRMLS_CC);
		return;
	}


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|zzzl",&url,&urlLen,&params,&requsetType,&header,&timeout) == FAILURE){
		return;
	}


	MAKE_STD_ZVAL(useParams);
	if(params == NULL || IS_NULL == Z_TYPE_P(params)){
		array_init(useParams);
	}else{
		ZVAL_ZVAL(useParams,params,1,0);
	}

	MAKE_STD_ZVAL(useRequestType);
	if(requsetType == NULL || IS_NULL == Z_TYPE_P(requsetType)){
		ZVAL_STRING(useRequestType,"GET",1);
	}else{
		if(IS_STRING == Z_TYPE_P(requsetType)){
			ZVAL_ZVAL(useRequestType,requsetType,1,0);
		}else{
			ZVAL_STRING(useRequestType,"GET",1);
		}
	}

	MAKE_STD_ZVAL(useHeader);
	if(header == NULL || IS_NULL == Z_TYPE_P(header)){
		array_init(useHeader);
	}else{
		ZVAL_ZVAL(useHeader,header,1,0);
	}


	//发送请求
	CResponse_sendHttpRequest(url,useParams,Z_STRVAL_P(useRequestType),useHeader,timeout,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
	zval_ptr_dtor(&useParams);
	zval_ptr_dtor(&useRequestType);
	zval_ptr_dtor(&useHeader);
}

PHP_METHOD(CHttp,getInstance){

	char	*key,
			*saveKey;
	int		keyLen = 0;

	zval	*selfInstace,
			**instaceSaveZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	if(keyLen == 0){
		key = "main";
	}

	selfInstace = zend_read_static_property(CHttpCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CHttpCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CHttpCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CHttpCe);

		//执行其构造器 并传入参数
		if (CHttpCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CHttpCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CHttpCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}

PHP_METHOD(CHttp,__construct){

}

void CHttp_setHeader(char *ua,zval *object TSRMLS_DC){

	zval	*nowHeader,
			**thisVal;
	int		i,h;
	char	*key,
			*copyHeader,
			*headerName,
			*headerValue;
	ulong	ukey;

	copyHeader = estrdup(ua);
	headerName = strtok(copyHeader,":");
	headerValue = strtok(NULL,":");

	nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(nowHeader)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CHttpCe,object,ZEND_STRL("header"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		nowHeader = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(nowHeader));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(nowHeader));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(nowHeader),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(nowHeader),&key,&ukey,0);
		if(IS_STRING == Z_TYPE_PP(thisVal) && strstr(Z_STRVAL_PP(thisVal),headerName) != NULL){
			zend_hash_index_del(Z_ARRVAL_P(nowHeader),ukey);
			break;
		}
		zend_hash_move_forward(Z_ARRVAL_P(nowHeader));
	}
	add_next_index_string(nowHeader,ua,1);
	efree(copyHeader);
}

PHP_METHOD(CHttp,setHeader){

	zval	*nowHeader,
			*addHeader;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&addHeader) == FAILURE){
		zend_throw_exception(CHttpExceptionCe, "[CHttpException] call [CHttp->setHeader] params error,the parmas must be a string or an array", 7001 TSRMLS_CC);
		return;
	}

	nowHeader = zend_read_property(CHttpCe,getThis(),ZEND_STRL("header"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(nowHeader)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CHttpCe,getThis(),ZEND_STRL("header"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		nowHeader = zend_read_property(CHttpCe,getThis(),ZEND_STRL("header"),0 TSRMLS_CC);
	}

	if(IS_STRING == Z_TYPE_P(addHeader)){

		CHttp_setHeader(Z_STRVAL_P(addHeader),getThis() TSRMLS_CC);

	}else if(IS_ARRAY == Z_TYPE_P(addHeader)){
		int		i,h;
		zval	**thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_P(addHeader));
		for(i = 0 ; i < h ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(addHeader),(void**)&thisVal);
			if(IS_STRING == Z_TYPE_PP(thisVal)){

				CHttp_setHeader(Z_STRVAL_PP(thisVal),getThis() TSRMLS_CC);
			}
			zend_hash_move_forward(Z_ARRVAL_P(addHeader));
		}
	}else{
		zend_throw_exception(CHttpExceptionCe, "[CHttpException] call [CHttp->setHeader] params error,the parmas must be a string or an array", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CHttp,setMethod){
	char	*type;
	int		typeLen = 0;

	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&type,&typeLen) == FAILURE){
		return;
	}

	php_strtoupper(type,strlen(type)+1);
	zend_update_property_string(CHttpCe,getThis(),ZEND_STRL("method"),type TSRMLS_CC);
}

PHP_METHOD(CHttp,setParams){
	zval	*params;
	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&params) == FAILURE){
		return;
	}
	zend_update_property(CHttpCe,getThis(),ZEND_STRL("params"),params TSRMLS_CC);
}

PHP_METHOD(CHttp,setUrl){
	char	*type;
	int		typeLen = 0;

	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&type,&typeLen) == FAILURE){
		return;
	}
	zend_update_property_string(CHttpCe,getThis(),ZEND_STRL("url"),type TSRMLS_CC);
}
PHP_METHOD(CHttp,setTimeout){
	long	timeout = 30;
	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&timeout) == FAILURE){
		return;
	}
	zend_update_property_long(CHttpCe,getThis(),ZEND_STRL("timeout"),timeout TSRMLS_CC);
}

PHP_METHOD(CHttp,send){

	zval	*url,
			*params,
			*timeout,
			*method,
			*header,
			*returnZval,
			*callParams,
			*callHeader,
			*responseObject,
			*fllowRedirct,
			*certPath;

	char	*errorString,
			*sslPath;

	url = zend_read_property(CHttpCe,getThis(),ZEND_STRL("url"),0 TSRMLS_CC);
	params = zend_read_property(CHttpCe,getThis(),ZEND_STRL("params"),0 TSRMLS_CC);
	timeout = zend_read_property(CHttpCe,getThis(),ZEND_STRL("timeout"),0 TSRMLS_CC);
	method = zend_read_property(CHttpCe,getThis(),ZEND_STRL("method"),0 TSRMLS_CC);
	header = zend_read_property(CHttpCe,getThis(),ZEND_STRL("header"),0 TSRMLS_CC);
	certPath = zend_read_property(CHttpCe,getThis(),ZEND_STRL("certPath"),0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(params)){
		MAKE_STD_ZVAL(callParams);
		ZVAL_ZVAL(callParams,params,1,0);
	}else{
		MAKE_STD_ZVAL(callParams);
		array_init(callParams);
	}
	
	if(IS_ARRAY == Z_TYPE_P(header)){
		MAKE_STD_ZVAL(callHeader);
		ZVAL_ZVAL(callHeader,header,1,0);
	}else{
		MAKE_STD_ZVAL(callHeader);
		array_init(callHeader);
	}

	//get cert file
	sslPath = estrdup("");
	if(IS_STRING == Z_TYPE_P(certPath) && strlen(Z_STRVAL_P(certPath)) > 0 ){
		efree(sslPath);
		sslPath = estrdup(Z_STRVAL_P(certPath));
	}
 
	CHttp_sendHttpRequest(Z_STRVAL_P(url),callParams,Z_STRVAL_P(method),callHeader,Z_LVAL_P(timeout),sslPath,&returnZval TSRMLS_CC);

	//check fllowRedirct
	fllowRedirct = zend_read_property(CHttpCe,getThis(),ZEND_STRL("fllowRedirct"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(fllowRedirct) && Z_LVAL_P(fllowRedirct) == 1){
		//get redirect_url
		zval	**info,
				**redirect_url;

		if(IS_ARRAY == Z_TYPE_P(returnZval) && SUCCESS == zend_hash_find(Z_ARRVAL_P(returnZval),"info",strlen("info")+1,(void**)&info) &&
			IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"redirect_url",strlen("redirect_url")+1,(void**)&redirect_url) &&
			IS_STRING == Z_TYPE_PP(redirect_url) && strlen(Z_STRVAL_PP(redirect_url)) > 0
		){
			//found redirect_url
			char	*newUrl;
			newUrl = estrdup(Z_STRVAL_PP(redirect_url));
			zval_ptr_dtor(&returnZval);
			CHttp_sendHttpRequest(newUrl,callParams,Z_STRVAL_P(method),callHeader,Z_LVAL_P(timeout),sslPath,&returnZval TSRMLS_CC);
			efree(newUrl);
		}
	}

	MAKE_STD_ZVAL(responseObject);
	object_init_ex(responseObject,CHttpResponseCe);
	zend_update_property(CHttpResponseCe,responseObject,ZEND_STRL("data"),returnZval TSRMLS_CC);

	errorString = estrdup("");
	if(IS_ARRAY == Z_TYPE_P(returnZval)){
		zval **error;
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(returnZval),"error",strlen("error")+1,(void**)&error) && IS_STRING == Z_TYPE_PP(error)){
			efree(errorString);
			errorString = estrdup(Z_STRVAL_PP(error));
		}
	}


	zval_ptr_dtor(&callParams);
	zval_ptr_dtor(&callHeader);
	zval_ptr_dtor(&returnZval);
	efree(sslPath);

	RETVAL_ZVAL(responseObject,1,1);

	//has error
	if(strlen(errorString) > 0){
		char *errorThrow;
		spprintf(&errorThrow,0,"[CHttpException] send http request to : [%s] throw exception information : %s",Z_STRVAL_P(url),errorString);
		efree(errorString);
		zend_throw_exception(CHttpExceptionCe, errorThrow, 1 TSRMLS_CC);
		efree(errorThrow);
	}else{
		efree(errorString);
	}

}

PHP_METHOD(CHttpResponse,getHttpCode){
	zval	*data,
			**info,
			**httpCode;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"http_code",strlen("http_code")+1,(void**)&httpCode) &&
		IS_LONG == Z_TYPE_PP(httpCode)
	){
		RETURN_LONG(Z_LVAL_PP(httpCode));
	}
	RETURN_LONG(0);
}

PHP_METHOD(CHttpResponse,getCastInfo){
	zval	*data,
			**info,
			**httpCode;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info)
	){
		RETURN_ZVAL(*info,1,0);
	}
	RETURN_NULL();
}

int CHttp_getResponseHeaderLen(zval *object TSRMLS_DC){

	zval	*data,
			**info,
			**header_size;
	data = zend_read_property(CHttpResponseCe,object,ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"header_size",strlen("header_size")+1,(void**)&header_size) &&
		IS_LONG == Z_TYPE_PP(header_size)
	){
		return Z_LVAL_PP(header_size);
	}
	return 0;
}

PHP_METHOD(CHttpResponse,getContent){
	zval	*data,
			**content;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"content",strlen("content")+1,(void**)&content)
	){

		//get response Header length
		int		headerLen = 0;
		char	*contentString;
		headerLen = CHttp_getResponseHeaderLen(getThis() TSRMLS_CC);
		substr(Z_STRVAL_PP(content),headerLen,strlen(Z_STRVAL_PP(content)) - headerLen,&contentString);
		RETURN_STRING(contentString,0);
	}
	RETURN_NULL();
}

PHP_METHOD(CHttpResponse,getRedirect){
	zval	*data,
			**info,
			**redirect_url;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"redirect_url",strlen("redirect_url")+1,(void**)&redirect_url) &&
		IS_STRING == Z_TYPE_PP(redirect_url)
	){
		RETURN_STRING(Z_STRVAL_PP(redirect_url),1);
	}
	RETURN_STRING("",1);
}

PHP_METHOD(CHttpResponse,getNamelookTime){
	zval	*data,
			**info,
			**namelookup_time;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"namelookup_time",strlen("namelookup_time")+1,(void**)&namelookup_time) &&
		IS_DOUBLE == Z_TYPE_PP(namelookup_time)
	){
		RETURN_DOUBLE(Z_DVAL_PP(namelookup_time));
	}
	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,getTotalTime){
	zval	*data,
			**info,
			**total_time;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"total_time",strlen("total_time")+1,(void**)&total_time) &&
		IS_DOUBLE == Z_TYPE_PP(total_time)
	){
		RETURN_DOUBLE(Z_DVAL_PP(total_time));
	}
	RETURN_FALSE;
}


PHP_METHOD(CHttpResponse,setData){
	zval	*data;
	RETVAL_ZVAL(getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		return;
	}
	zend_update_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),data TSRMLS_CC);
}

PHP_METHOD(CHttpResponse,asArray){
	zval *data;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CHttpResponse,getPrimaryIp){
	zval	*data,
			**info,
			**primary_ip;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"primary_ip",strlen("primary_ip")+1,(void**)&primary_ip) &&
		IS_STRING == Z_TYPE_PP(primary_ip)
	){
		RETURN_STRING(Z_STRVAL_PP(primary_ip),1);
	}
	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,getConnectTime){
	zval	*data,
			**info,
			**connect_time;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"connect_time",strlen("connect_time")+1,(void**)&connect_time) &&
		IS_DOUBLE == Z_TYPE_PP(connect_time)
	){
		RETURN_DOUBLE(Z_DVAL_PP(connect_time));
	}
	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,getContentType){
	zval	*data,
			**info,
			**content_type;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"content_type",strlen("content_type")+1,(void**)&content_type) &&
		IS_STRING == Z_TYPE_PP(content_type)
	){
		RETURN_STRING(Z_STRVAL_PP(content_type),1);
	}
	RETURN_FALSE;
}

void CHttp_parseRawHeaderToArray(char *string,zval	**returnArray TSRMLS_DC){
	zval	*parseToArray,
			**thisVal,
			*parseKeyVal,
			**key,
			**val;
	int		i,h;
	char	*trimVal;

	MAKE_STD_ZVAL(*returnArray);
	array_init(*returnArray);	
	php_explode("\n",string,&parseToArray);

	h = zend_hash_num_elements(Z_ARRVAL_P(parseToArray));
	for(i = 0 ; i < h; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(parseToArray),(void**)&thisVal);

		//parse :
		php_explode(":",Z_STRVAL_PP(thisVal),&parseKeyVal);

		if(2 != zend_hash_num_elements(Z_ARRVAL_P(parseKeyVal))){
			zval_ptr_dtor(&parseKeyVal);
			zend_hash_move_forward(Z_ARRVAL_P(parseToArray));
			continue;
		}

		//save
		zend_hash_index_find(Z_ARRVAL_P(parseKeyVal),0,(void**)&key);
		zend_hash_index_find(Z_ARRVAL_P(parseKeyVal),1,(void**)&val);
		convert_to_string(*key);
		convert_to_string(*val);
		php_trim(Z_STRVAL_PP(val)," \r\n",&trimVal);
		add_assoc_string(*returnArray,Z_STRVAL_PP(key),trimVal,0);
	
		zval_ptr_dtor(&parseKeyVal);
		zend_hash_move_forward(Z_ARRVAL_P(parseToArray));
	}


	zval_ptr_dtor(&parseToArray);
}

PHP_METHOD(CHttpResponse,getResponseHeader)
{
	zval	*data,
			**content,
			**request_header;
	long	returnRaw = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&returnRaw) == FAILURE){
		return;
	}

	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"content",strlen("content")+1,(void**)&content) && IS_STRING == Z_TYPE_PP(content)){
		
		//get headerLength
		int		headerLen = 0;
		char	*headerString;
		zval	*headerArray;
		headerLen = CHttp_getResponseHeaderLen(getThis() TSRMLS_CC);
		
		substr(Z_STRVAL_PP(content),0,headerLen,&headerString);

		if(1 == returnRaw){
			RETURN_STRING(headerString,0);
		}

		//parse to array
		CHttp_parseRawHeaderToArray(headerString,&headerArray TSRMLS_CC);
		efree(headerString);
		RETURN_ZVAL(headerArray,1,1);
	}

	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,getRequestHeader){
	zval	*data,
			**info,
			**request_header;
	long	returnRaw = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&returnRaw) == FAILURE){
		return;
	}

	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"request_header",strlen("request_header")+1,(void**)&request_header) &&
		IS_STRING == Z_TYPE_PP(request_header)
	){

		if(returnRaw == 1){
			RETURN_STRING(Z_STRVAL_PP(request_header),1);
		}else{
			zval	*headerArray;
			CHttp_parseRawHeaderToArray(Z_STRVAL_PP(request_header),&headerArray TSRMLS_CC);
			RETURN_ZVAL(headerArray,1,1);
		}
	}
	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,isSuccess){
	zval	*data,
			**info,
			**httpCode;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"info",strlen("info")+1,(void**)&info) &&
		IS_ARRAY == Z_TYPE_PP(info) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(info),"http_code",strlen("http_code")+1,(void**)&httpCode) &&
		IS_LONG == Z_TYPE_PP(httpCode) && 200 == Z_LVAL_PP(httpCode)
	){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

PHP_METHOD(CHttpResponse,getErrorInfo){

	zval	*data,
			**error,
			**httpCode;
	data = zend_read_property(CHttpResponseCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	
	if(IS_ARRAY == Z_TYPE_P(data) && SUCCESS == zend_hash_find(Z_ARRVAL_P(data),"error",strlen("error")+1,(void**)&error) && IS_STRING == Z_TYPE_PP(error)){
		RETURN_STRING(Z_STRVAL_PP(error),1);
	}

	RETURN_STRING("",1);
}

PHP_METHOD(CHttp,setCertificate)
{
	char	*path;
	int		pathLen = 0;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		return;
	}

	//check file exist
	if(SUCCESS != fileExist(path)){
		char	errorMessage[1024];
		sprintf(errorMessage,"[CHttpException] Call [CHttp->setCertificate] the certficate file not exists : %s",path);
		zend_throw_exception(CHttpExceptionCe, errorMessage, 7001 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CHttpCe,getThis(),ZEND_STRL("certPath"),path TSRMLS_CC);
}