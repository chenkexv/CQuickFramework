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

#ifndef PHP_WIN32

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CQuickFramework.h"
#include "php_CApplication.h"
#include "php_CException.h"
#include "php_CHttpServer.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <pthread.h>


//zend类方法
zend_function_entry CMicroServer_functions[] = {
	PHP_ME(CMicroServer,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CMicroServer,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CMicroServer,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CMicroServer,bind,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroServer,listen,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroServer,onRequest,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroServer,onRoute,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CMicroServer)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CMicroServer",CMicroServer_functions);
	CMicroServerCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CMicroServerCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CMicroServerCe, ZEND_STRL("host"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMicroServerCe, ZEND_STRL("port"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMicroServerCe, ZEND_STRL("launchFramework"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CMicroServerCe, ZEND_STRL("callbackObject"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CMicroServerCe, ZEND_STRL("routeObject"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

zend_function_entry CMicroRequest_functions[] = {
	PHP_ME(CMicroRequest,getHeader,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroRequest,getRawHeader,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroRequest,getBody,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroRequest,getUrl,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getUri,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getAgent,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getHost,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getIp,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,isWap,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,isCli,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getPath,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CMicroRequest,getQueryString,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CMicroRequest)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CMicroRequest",CMicroRequest_functions);
	CMicroRequestCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestRaw"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestHeaderRaw"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestBodyRaw"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CMicroRequestCe, ZEND_STRL("requestHeader"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestUri"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestUrl"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("host"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("userAgent"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("ip"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CMicroRequestCe, ZEND_STRL("requestQueryString"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

zend_function_entry CMicroResponse_functions[] = {
	PHP_ME(CMicroResponse,setHeader,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroResponse,setBody,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMicroResponse,send,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CMicroResponse)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CMicroResponse",CMicroResponse_functions);
	CMicroResponseCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_long(CMicroResponseCe, ZEND_STRL("fd"),-1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMicroResponseCe, ZEND_STRL("httpCode"),200,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CMicroResponseCe, ZEND_STRL("header"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMicroResponseCe, ZEND_STRL("hasSendHeader"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}



void getHttpDesc(int useHttpCode,char **statusDesc){

	switch(useHttpCode){
		case 200:
			*statusDesc = estrdup("OK");
			break;

		case 301:
			*statusDesc = estrdup("Moved Permanently");
			break;

		case 302:
			*statusDesc = estrdup("Move temporarily");
			break;

		case 304:
			*statusDesc = estrdup("Not Modified");
			break;

		case 401:
			*statusDesc = estrdup("Unauthorized");
			break;

		case 403:
			*statusDesc = estrdup("Forbidden");
			break;

		case 404:
			*statusDesc = estrdup("Not Found");
			break;

		case 405:
			*statusDesc = estrdup("Method Not Allowed");
			break;

		case 500:
			*statusDesc = estrdup("Internal Server Error");
			break;

		case 502:
			*statusDesc = estrdup("Bad Gateway");
			break;

		case 503:
			*statusDesc = estrdup("Service Unavailable");
			break;

		case 504:
			*statusDesc = estrdup("Gateway Timeout");
			break;

		default:
			*statusDesc = estrdup("Unknow");
			break;
	}
}

void createResponseContent(zval *object,char *body,int thisCode,char **resultContent TSRMLS_DC){

	zval	*httpCode,
			*httpHeader,
			*hasSendHeader;

	int		useHttpCode = 200;

	char	*statusDesc;

	smart_str	content = {0};


	hasSendHeader = zend_read_property(CMicroResponseCe,object,ZEND_STRL("hasSendHeader"), 0 TSRMLS_CC);

	if(Z_LVAL_P(hasSendHeader) == 0){
		httpCode = zend_read_property(CMicroResponseCe,object,ZEND_STRL("httpCode"), 0 TSRMLS_CC);
		httpHeader = zend_read_property(CMicroResponseCe,object,ZEND_STRL("header"), 0 TSRMLS_CC);

		if(thisCode != 0){
			useHttpCode = thisCode;
		}else{
			useHttpCode = Z_LVAL_P(httpCode);
		}

		getHttpDesc(useHttpCode,&statusDesc);
		smart_str_appends(&content,"HTTP/1.1 ");
		smart_str_append_long(&content,useHttpCode);
		smart_str_appends(&content," ");
		smart_str_appends(&content,statusDesc);
		efree(statusDesc);
		smart_str_appends(&content,"\r\nServer:CQuickFrameExtension\r\n");

		//add user header
		smart_str_appends(&content,"\r\n");
		zend_update_property_long(CMicroResponseCe,object,ZEND_STRL("hasSendHeader"),1 TSRMLS_CC);
	}


	smart_str_appends(&content,body);

	smart_str_0(&content);
	*resultContent = estrdup(content.c);
	smart_str_free(&content);
	
}

PHP_METHOD(CMicroResponse,setHeader){
}
PHP_METHOD(CMicroResponse,setBody){
}

PHP_METHOD(CMicroResponse,send){

	char	*content,
			*resultContent;
	int		contentLen = 0,
			epfd,
			fd;
	long	code = 0;

	zval	*fdZval;

	struct epoll_event ev;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&content,&contentLen,&code) == FAILURE){
		RETURN_FALSE;
	}

	fdZval = zend_read_property(CMicroResponseCe,getThis(),ZEND_STRL("fd"), 0 TSRMLS_CC);
	fd = Z_LVAL_P(fdZval);
	createResponseContent(getThis(),content,code,&resultContent TSRMLS_CC);
	write(fd,resultContent,strlen(resultContent));
	efree(resultContent);

}

PHP_METHOD(CMicroRequest,getHeader){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestHeader"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}
PHP_METHOD(CMicroRequest,getRawHeader){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestHeaderRaw"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}
PHP_METHOD(CMicroRequest,getBody){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestBodyRaw"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}
PHP_METHOD(CMicroRequest,getUrl){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestUrl"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getUri){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestUri"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getAgent){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("userAgent"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getHost){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("host"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getIp){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("ip"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getPath){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestPath"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,getQueryString){
	zval	*data;
	data = zend_read_property(CMicroRequestCe,getThis(),ZEND_STRL("requestQueryString"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CMicroRequest,isWap){
	
}

PHP_METHOD(CMicroRequest,isCli){
	RETVAL_FALSE;
}



int doCallbackFunction(zval *object,zval *params1,zval *params2 TSRMLS_DC){

	zval	*callback;

	char	*callback_name;

	int		callStatus;


	callback = zend_read_property(CMicroServerCe,object,ZEND_STRL("callbackObject"), 0 TSRMLS_CC);
	if(IS_OBJECT != Z_TYPE_P(callback)){
		return 0;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		efree(callback_name);
        return 0;
    }
	
	MODULE_BEGIN
		zval	constructVal,
				contruReturn,
				*paramsList[2];
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callback_name,0);
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_ZVAL(paramsList[0],params1,1,0);
		ZVAL_ZVAL(paramsList[1],params2,1,0);
		callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,2, paramsList TSRMLS_CC);
		zval_dtor(&contruReturn);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
	MODULE_END

	efree(callback_name);
	return callStatus;
}

int doRouteCallbackFunction(zval *object,zval *params1,zval *params2 TSRMLS_DC){

	zval	*routeObject,
			**thisRouteObject,
			*nowRequestPath,
			*callback;

	int		callStatus = 0;

	char	*callback_name;

	routeObject = zend_read_property(CMicroServerCe,object,ZEND_STRL("routeObject"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(routeObject)){
		return 0;
	}

	//request
	nowRequestPath = zend_read_property(CMicroRequestCe,params1,ZEND_STRL("requestPath"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(nowRequestPath) && strlen(Z_STRVAL_P(nowRequestPath)) > 0){
	}else{
		return;
	}

	//find this route
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(routeObject),Z_STRVAL_P(nowRequestPath),strlen(Z_STRVAL_P(nowRequestPath))+1,(void**)&thisRouteObject) && IS_OBJECT == Z_TYPE_PP(thisRouteObject) ){
	}else{
		return;
	}

	callback = *thisRouteObject;
	if(IS_OBJECT != Z_TYPE_P(callback)){
		return 0;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		efree(callback_name);
        return 0;
    }
	
	MODULE_BEGIN
		zval	constructVal,
				contruReturn,
				*paramsList[2];
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callback_name,0);
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_ZVAL(paramsList[0],params1,1,0);
		ZVAL_ZVAL(paramsList[1],params2,1,0);
		callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,2, paramsList TSRMLS_CC);
		zval_dtor(&contruReturn);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
	MODULE_END

	efree(callback_name);
	return callStatus;
	
}

void setLogs(char *format, ...){
	
	va_list args;
    int ret;
    char *buffer;
    int size;

    va_start(args, format);
    size = vspprintf(&buffer, 0, format, args);
   
	php_printf("%s",buffer);
	
    efree(buffer);
    va_end(args);
}

//begin listen
int startListen(char *host,int port){

	int sock = socket(AF_INET,SOCK_STREAM,0);
    if( sock < 0 ){
		return -2;
    }

    //if TIME_WAIT server will restart
    int opt = 1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);// 地址为任意类型
    local.sin_port = htons(port);

    if( bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0 ){
		return -3;
    }


    if( listen(sock,5) < 0 ){
		return -4;
    }
    return sock;
}

void parseQueryStringToGet(char *queryString TSRMLS_DC){
	zval	*queryArray;
	parse_str(queryString,&queryArray);
	setGetParamArray(Z_ARRVAL_P(queryArray) TSRMLS_CC);
	zval_ptr_dtor(&queryArray);
}

void parseRequestHeader(zval *microRequestObject,char *raw,zval **headerArray TSRMLS_CC){

	zval	*rows,
			**thisVal,
			*keyVal = NULL,
			**requestHost,
			*matchRequestType,
			**requestMethod;
	
	int		i,h;

	MAKE_STD_ZVAL(*headerArray);
	array_init(*headerArray);

	php_explode("\r\n",raw,&rows);
	if(IS_ARRAY != Z_TYPE_P(rows)){
		zval_ptr_dtor(&rows);
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(rows));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(rows));
	zend_hash_get_current_data(Z_ARRVAL_P(rows),(void**)&requestHost);

	//check request Type
	if(!preg_match("/([A-Z]*)\\s(.*)\\sHTTP\\/([0-9\\.]+)/",Z_STRVAL_PP(requestHost),&matchRequestType)){
		zval_ptr_dtor(&rows);
		zval_ptr_dtor(&matchRequestType);
		return;
	}

	zend_hash_move_forward(Z_ARRVAL_P(rows));
	for( i = 1 ; i < h;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(rows),(void**)&thisVal);
		if(IS_STRING != Z_TYPE_PP(thisVal)){
			zend_hash_move_forward(Z_ARRVAL_P(rows));
			continue;
		}


		php_explode(":",Z_STRVAL_PP(thisVal),&keyVal);
		if(IS_ARRAY == Z_TYPE_P(keyVal) && 2 <= zend_hash_num_elements(Z_ARRVAL_P(keyVal))){
			zval	**key,
					*saveToServer;
			char	*replaceKey,
					*thisHeaderVal,
					*trimThisVal,
					*serverSaveKey,
					*httpServerSaveKey;
			zend_hash_index_find(Z_ARRVAL_P(keyVal),0,(void**)&key);
			spprintf(&replaceKey,0,"%s:",Z_STRVAL_PP(key));
			str_replace(replaceKey,"",Z_STRVAL_PP(thisVal),&thisHeaderVal);
			php_trim(thisHeaderVal," ",&trimThisVal);
			add_assoc_string(*headerArray,Z_STRVAL_PP(key),trimThisVal,1);
			efree(replaceKey);
			efree(thisHeaderVal);

			//save to server
			str_replace("-","_",Z_STRVAL_PP(key),&serverSaveKey);
			php_strtoupper(serverSaveKey,strlen(serverSaveKey)+1);
			spprintf(&httpServerSaveKey,0,"HTTP_%s",serverSaveKey);
			MAKE_STD_ZVAL(saveToServer);
			ZVAL_STRING(saveToServer,trimThisVal,1);
			setServerParam(httpServerSaveKey,saveToServer);
			zval_ptr_dtor(&saveToServer);
			efree(serverSaveKey);
			efree(trimThisVal);
			efree(httpServerSaveKey);
		}
		zval_ptr_dtor(&keyVal);

		zend_hash_move_forward(Z_ARRVAL_P(rows));
	}

	//set params to $_SERVER
	MODULE_BEGIN

		zval	**requestProtocol,
				*saveProtocol,
				**requestUri,
				*parseUrl;

		char	*protocolFull,
				*copyUri,
				*requestPath,
				*requestQueryString;

		//requestMethod
		zend_hash_index_find(Z_ARRVAL_P(matchRequestType),1,(void**)&requestMethod);
		setServerParam("REQUEST_METHOD",*requestMethod);

		//SERVER_PROTOCOL
		zend_hash_index_find(Z_ARRVAL_P(matchRequestType),3,(void**)&requestProtocol);
		spprintf(&protocolFull,0,"HTTP/%s",Z_STRVAL_PP(requestProtocol));
		MAKE_STD_ZVAL(saveProtocol);
		ZVAL_STRING(saveProtocol,protocolFull,1);
		setServerParam("SERVER_PROTOCOL",saveProtocol);
		efree(protocolFull);
		zval_ptr_dtor(&saveProtocol);

		//requestURI
		zend_hash_index_find(Z_ARRVAL_P(matchRequestType),2,(void**)&requestUri);
		setServerParam("REQUEST_URI",*requestUri);
	
		//set requestUri
		if(strstr(Z_STRVAL_PP(requestUri),"?") != NULL){
			zval	*savePath,
					*saveQuery;
			copyUri = estrdup(Z_STRVAL_PP(requestUri));
			requestPath = strtok(copyUri,"?");
			requestQueryString = strtok(NULL,"?");
			zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestPath"),requestPath TSRMLS_CC);
			zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestQueryString"),requestQueryString TSRMLS_CC);
			MAKE_STD_ZVAL(savePath);
			ZVAL_STRING(savePath,requestPath,1);
			MAKE_STD_ZVAL(saveQuery);
			ZVAL_STRING(saveQuery,requestQueryString,1);
			setServerParam("REQUEST_PATH",savePath);
			setServerParam("QUERY_STRING",saveQuery);

			parseQueryStringToGet(requestQueryString TSRMLS_CC);

			zval_ptr_dtor(&saveQuery);
			zval_ptr_dtor(&savePath);
			efree(copyUri);
		}else{
			zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestPath"),Z_STRVAL_PP(requestUri) TSRMLS_CC);
			setServerParam("REQUEST_PATH",*requestUri);
		}

	MODULE_END

	zval_ptr_dtor(&matchRequestType);
	zval_ptr_dtor(&rows);
}


void parseSocketAddrInfo(zval *microRequestObject,int fd TSRMLS_DC){
	
	zval	*saveIp,
			*savePort;
	struct sockaddr_in socketInfo;
	int		socketInfoLen = sizeof(socketInfo);
	getpeername(fd, (struct sockaddr*)&socketInfo, &socketInfoLen);

	zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("ip"),inet_ntoa(socketInfo.sin_addr) TSRMLS_CC);
	
	//REMOTE_ADDR
	MAKE_STD_ZVAL(saveIp);
	ZVAL_STRING(saveIp,inet_ntoa(socketInfo.sin_addr),1);
	setServerParam("REMOTE_ADDR",saveIp);
	zval_ptr_dtor(&saveIp);

	//REMOTE_PORT
	MAKE_STD_ZVAL(savePort);
	ZVAL_LONG(savePort,ntohs(socketInfo.sin_port));
	setServerParam("REMOTE_PORT",saveIp);
	zval_ptr_dtor(&savePort);

}

void errorOutPut(int fd,int code){
}

void *createRequestObjectForThread(void *args){

	free(((request_t*)args)->requestContent);
	free(args);
}


int createRequestObject(int fd,zval *object,char *requestContent TSRMLS_DC){

	zval	*microRequestObject,
			*microResponseObject,
			*callbackObject,
			*cutContent,
			**header,
			**body,
			*headerArray;

	char	*outPutString,
			*resultContent;


	MAKE_STD_ZVAL(microRequestObject);
	object_init_ex(microRequestObject,CMicroRequestCe);
	MAKE_STD_ZVAL(microResponseObject);
	object_init_ex(microResponseObject,CMicroResponseCe);

	//set request info to request Object
	zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestRaw"),requestContent TSRMLS_CC);
	php_explode("\r\n\r\n",requestContent,&cutContent);
	if(IS_ARRAY == Z_TYPE_P(cutContent) && 2 == zend_hash_num_elements(Z_ARRVAL_P(cutContent))){
	}else{
		zval_ptr_dtor(&microRequestObject);
		zval_ptr_dtor(&microResponseObject);
		zval_ptr_dtor(&cutContent);
		errorOutPut(fd,400);
		close(fd);
		return 0;
	}

	zend_hash_index_find(Z_ARRVAL_P(cutContent),0,(void**)&header);
	zend_hash_index_find(Z_ARRVAL_P(cutContent),1,(void**)&body);
	zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestHeaderRaw"),Z_STRVAL_PP(header) TSRMLS_CC);
	zend_update_property_string(CMicroRequestCe,microRequestObject,ZEND_STRL("requestBodyRaw"),Z_STRVAL_PP(body) TSRMLS_CC);
	

	//parse request header
	parseRequestHeader(microRequestObject,Z_STRVAL_PP(header),&headerArray TSRMLS_DC);
	zend_update_property(CMicroRequestCe,microRequestObject,ZEND_STRL("requestHeader"),headerArray TSRMLS_CC);
	zval_ptr_dtor(&headerArray);

	//get socket addr info
	parseSocketAddrInfo(microRequestObject,fd TSRMLS_CC);

	//write socket info
	zend_update_property_long(CMicroResponseCe,microResponseObject,ZEND_STRL("fd"),fd TSRMLS_CC);

	//ob_start
	ob_start();

	//call user callbackfunction
	doCallbackFunction(object,microRequestObject,microResponseObject);
	doRouteCallbackFunction(object,microRequestObject,microResponseObject);

	//call end check to response
	ob_get_contents(&outPutString);
	ob_end_clean();
	createResponseContent(microResponseObject,outPutString,0,&resultContent TSRMLS_CC);
	write(fd,resultContent,strlen(resultContent));
	efree(resultContent);
	efree(outPutString);
	close(fd);

	zval_ptr_dtor(&cutContent);
	zval_ptr_dtor(&microRequestObject);
	zval_ptr_dtor(&microResponseObject);


}

int condition_init(condition_t *cond){
    int status;
    if((status = pthread_mutex_init(&cond->pmutex,NULL))){
        return status;
	}
    if((status = pthread_cond_init(&cond->pcond,NULL))){
        return status;
	}
    return 0;
}

int condition_lock(condition_t *cond){
    return pthread_mutex_lock(&cond -> pmutex);
}

int condition_unlock(condition_t *cond){
    return pthread_mutex_unlock(&cond -> pmutex);
}

int condition_wait(condition_t *cond){
    return pthread_cond_wait(&cond -> pcond,&cond -> pmutex);
}

int condition_timewait(condition_t *cond,const struct timespec *abstime){
    return pthread_cond_timedwait(&cond->pcond,&cond->pmutex,abstime);
}
int condition_signal(condition_t *cond){
    return pthread_cond_signal(&cond->pcond);
}

int condition_broadcast(condition_t *cond){
    return pthread_cond_broadcast(&cond -> pcond);
}

int condition_destory(condition_t *cond){
    int status;
    if((status = pthread_mutex_destroy(&cond -> pmutex))){
        return status;
	}
    if((status = pthread_cond_destroy(&cond -> pcond))){
        return status;
	}
    return 0;
}


void *thread_routine(void *arg){
    struct timespec abstime;
    int timeout;

	
    threadpool_t *pool = (threadpool_t *)arg;
    while(1) {
        timeout = 0;
        condition_lock(&pool -> ready);
        pool -> idle++;
		if(pool -> taskNum >0) pool -> taskNum --;
        while(pool -> first == NULL && !pool -> quit){
            clock_gettime(CLOCK_REALTIME,&abstime);
            abstime.tv_sec += 2;
            int status=condition_timewait(&pool -> ready,&abstime);
            if(status == ETIMEDOUT){
                timeout = 1;
                break;
            }

        }
    
        pool -> idle--;

        if(pool -> first != NULL){
            task_t *t = pool -> first;
            pool -> first = t -> next;
        
            condition_unlock(&pool -> ready);
            t -> run(t->arg);
            free(t);
            condition_lock(&pool -> ready);
        }
   
        if(pool -> quit && pool ->first == NULL){
            pool -> counter--;
            if(pool->counter == 0){
                condition_signal(&pool -> ready);
            }
            condition_unlock(&pool->ready);

            break;
        }

        if(timeout &&pool -> first ==NULL){
            pool -> counter--;
            condition_unlock(&pool->ready);
 
            break;
        }
        condition_unlock(&pool -> ready);
    }

	pthread_detach(pthread_self());
    return NULL;
}


void threadpool_init(threadpool_t *pool, int threads)
{
    condition_init(&pool -> ready);
    pool -> first = NULL;
    pool -> last = NULL;
    pool -> counter = 0;
    pool -> idle = 0;
    pool -> max_threads = threads;
    pool -> quit = 0;
	pool -> taskNum = 0;
}


void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg),void *arg)
{
    task_t *newstask = (task_t *)malloc(sizeof(task_t));
    newstask->run = run;
    newstask->arg = arg;
    newstask -> next = NULL;
    
    condition_lock(&pool -> ready);

    if(pool -> first ==NULL){
        pool -> first = newstask;
    }else{
        pool -> last -> next = newstask;
	}
	
    pool -> last = newstask;
	pool -> taskNum ++;

    if(pool -> idle > 0){
        condition_signal(&pool -> ready);
    }
    else if(pool -> counter < pool -> max_threads){
	
        pthread_t tid;
        pthread_create(&tid,NULL,thread_routine,pool);
        pool -> counter++;
    }
    condition_unlock(&pool -> ready);
}

void  threadpool_destory(threadpool_t *pool){

    if(pool -> quit){
        return;
    }
    condition_lock(&pool -> ready);
    pool->quit = 1;
    if(pool -> counter > 0) {

        if(pool -> idle > 0)
            condition_broadcast(&pool->ready);

        while(pool -> counter > 0){
            condition_wait(&pool->ready);
        }
    }
    condition_unlock(&pool->ready);
    condition_destory(&pool -> ready);
}


//porcess event
void handlerEvents(int epfd,struct epoll_event revs[],int num,int listen_sock,zval *object,threadpool_t *pool TSRMLS_CC)
{
    struct epoll_event ev;
    int i = 0;
    for( ; i < num; i++ ){

		int fd = revs[i].data.fd;

		// 如果是监听文件描述符，则调用accept接受新连接
		if( fd == listen_sock && (revs[i].events & EPOLLIN) ){

			struct sockaddr_in client;
			socklen_t len = sizeof(client);
			int new_sock = accept(fd,(struct sockaddr *)&client,&len);

			if( new_sock < 0 ){
				setLogs("accept fail ... \n");
				continue;
			}

			//refarword to message
			ev.events = EPOLLIN;
			ev.data.fd = new_sock;
			epoll_ctl(epfd,EPOLL_CTL_ADD,new_sock,&ev);

			continue;
		}

		// 如果是普通文件描述符，则调用read提供读取数据的服务
		if(revs[i].events & EPOLLIN)	{

			char		buf[10240];
			int			readLen = 0;
			smart_str	allContent = {0};

			while(1){
				readLen = read(fd,buf,sizeof(buf)-1);
				if(readLen <  sizeof(buf) -1){
					buf[readLen] = '\0';
				}
				smart_str_appends(&allContent,buf);
				if(readLen < sizeof(buf) -1){
					break;
				}
			}

			smart_str_0(&allContent);

			if( readLen > 0 ){
			
#ifdef ZTS
				//use thread pool
				request_t *request = (request_t*)malloc(sizeof(request_t));
				request->fd = fd;
				request->requestContent = strdup(allContent.c);
				request->object = object;
				threadpool_add_task(pool,createRequestObjectForThread,request);
#else

				//非线程安全的版本不使用多线程
				if(!createRequestObject(fd,object,allContent.c TSRMLS_DC)){
					close(fd);
				}
#endif

				epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);

			}else if( readLen == 0 ){
				//client close
				close(fd);
				epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
			}
			else{
				setLogs("read fai ...\n");
				close(fd);
				epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
			}
			
			smart_str_free(&allContent);
		}

		//response client
		if( revs[i].events & EPOLLOUT ){
		}
    }
}


//create a epoll http server
int createHttpServer(char *host,int port,zval *object TSRMLS_DC){
	
	int epfd = epoll_create(1024);
	if(epfd < 0){
		//create epoll fail
		return -1;
	}


	int listenSock = startListen(host,port);
	if(listenSock < 0){
		close(epfd);
		return listenSock;
	}

	//init thread pool
	threadpool_t pool;
    threadpool_init(&pool,200);

	struct epoll_event  ev;
    ev.events = EPOLLIN;    //read event
    ev.data.fd = listenSock;

	//regiseter epoll read event
	epoll_ctl(epfd,EPOLL_CTL_ADD,listenSock,&ev);
	struct epoll_event revs[128];
    int n = sizeof(revs)/sizeof(revs[0]);
    int timeout = 3000;
    int num = 0;

	//begin listen
	while(1) {

       //wait for epoll active
       switch( num = epoll_wait(epfd,revs,n,timeout) )
       {
		   case 0:
			   //setLogs("timeout...\n");
			   continue;
		   case -1:
			   setLogs("epoll_wait fail...\n");
			   continue;
		   default:
			   handlerEvents(epfd,revs,num,listenSock,object,&pool TSRMLS_CC);
			   break;
       }


    }

	threadpool_destory(&pool);
    close(epfd);
    close(listenSock);
    return 0;
}

PHP_METHOD(CMicroServer,__construct){


}

PHP_METHOD(CMicroServer,__destruct){

	zend_update_property_null(CMicroServerCe,getThis(),ZEND_STRL("routeObject") TSRMLS_CC);
}

PHP_METHOD(CMicroServer,onRoute){

	char	*route,
			*callback_name;
	int		routeLen = 0;
	zval	*callFunction,
			*routeObject,
			*saveRoute;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&route,&routeLen,&callFunction) == FAILURE){
		return;
	}

	if (!zend_is_callable(callFunction, 0, &callback_name TSRMLS_CC)) {
		efree(callback_name);
		zend_throw_exception(CMicroServerExceptionCe, "[CMicroServerException] call [CMicroServer->onRoute] the params is not a callback function", 7001 TSRMLS_CC);
		return;
	}

	routeObject = zend_read_property(CMicroServerCe,getThis(),ZEND_STRL("routeObject"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(routeObject)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CMicroServerCe,getThis(),ZEND_STRL("routeObject"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		routeObject = zend_read_property(CMicroServerCe,getThis(),ZEND_STRL("routeObject"), 0 TSRMLS_CC);
	}

	MAKE_STD_ZVAL(saveRoute);
	ZVAL_ZVAL(saveRoute,callFunction,1,0);
	add_assoc_zval(routeObject,route,saveRoute);
	zend_update_property(CMicroServerCe,getThis(),ZEND_STRL("routeObject"),routeObject TSRMLS_CC);

	efree(callback_name);
}


PHP_METHOD(CMicroServer,getInstance){

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


	selfInstace = zend_read_static_property(CMicroServerCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CMicroServerCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CMicroServerCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CMicroServerCe);

		//执行其构造器 并传入参数
		if (CMicroServerCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CMicroServerCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CMicroServerCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}


PHP_METHOD(CMicroServer,listen){

	zval	*host,
			*port,
			*object,
			**argv,
			**SERVER;

	int		errorCode = 0,
			isDaemon = 0;

	char	appPath[2024],
			codePath[2024];


	host = zend_read_property(CMicroServerCe,getThis(),ZEND_STRL("host"), 0 TSRMLS_CC);
	port = zend_read_property(CMicroServerCe,getThis(),ZEND_STRL("port"), 0 TSRMLS_CC);

	//init framework
	getcwd(appPath,sizeof(appPath));
	sprintf(codePath,"%s%s",appPath,"/application");
	php_define("APP_PATH",appPath TSRMLS_CC);
	php_define("CODE_PATH",codePath TSRMLS_CC);
	CWebApp_createApp(&object TSRMLS_CC);
	zval_ptr_dtor(&object);


	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
	if(zend_hash_find(Z_ARRVAL_PP(SERVER),"argv",strlen("argv")+1,(void**)&argv) == SUCCESS && IS_ARRAY == Z_TYPE_PP(argv)){
		int	i,h;
		zval **thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_PP(argv));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(argv));
		for(i = 0 ; i < h;i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(argv),(void**)&thisVal);
			if(strstr(Z_STRVAL_PP(thisVal),"--daemon") != NULL){
				isDaemon = 1;
				break;
			}
			zend_hash_move_forward(Z_ARRVAL_PP(argv));
		}
	}

	//daemon
	if(isDaemon){
		php_printf("run as a daemon process..\n");
		int s = daemon(1, 0);
	}

	//create a tcp server
	errorCode = createHttpServer(Z_STRVAL_P(host),Z_LVAL_P(port),getThis() TSRMLS_CC);
	if(errorCode < 0){
		php_printf("createServiceFail,errorCode:%d..\n",errorCode);
	}

	//return
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CMicroServer,onRequest){

	zval	*callback;
	char	*callback_name;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		zend_throw_exception(CMicroServerExceptionCe, "[CMicroServerException] call [CMicroServer->onRequest] the params is not a callback function", 7001 TSRMLS_CC);
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }

	//save to callbackObject
	zend_update_property(CMicroServerCe,getThis(),ZEND_STRL("callbackObject"),callback TSRMLS_CC);
	
    efree(callback_name);
	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CMicroServer,bind){
	
	char	*host;
	int		hostLen = 0;
	long	port = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&host,&hostLen,&port) == FAILURE){
		RETURN_FALSE;
	}

	if(port == 0){
		zend_throw_exception(CMicroServerExceptionCe, "[CMicroServerException] call [CMicroServer->bind] the port must available port", 7001 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CMicroServerCe,getThis(),ZEND_STRL("host"),host TSRMLS_CC);
	zend_update_property_long(CMicroServerCe,getThis(),ZEND_STRL("port"),port TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

#endif