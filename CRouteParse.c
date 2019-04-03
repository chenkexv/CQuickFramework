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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


#include "php_CMyFrameExtension.h"
#include "php_CRouteParse.h"
#include "php_CRoute.h"


//zend类方法
zend_function_entry CRouteParseParse_functions[] = {
	PHP_ME(CRouteParse,url,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,NoWriteUrl,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRouteParse,URLrewriteType_1,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,URLrewriteType_2,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,urlWriteResult,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,getOtherParams,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRouteParse,getSubdomain,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,getCacheRoute,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,getRoute,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,getSubdomainUrl,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,_setCacheRoute,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,getRequestUri,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRouteParse,requestURI,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRouteParseParse)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CRouteParse",CRouteParseParse_functions);
	CRouteParseCe = zend_register_internal_class(&funCe TSRMLS_CC);

	return SUCCESS;
}

//组织动态参数
void getOtherParams(zval *paramsList,char **getStr TSRMLS_DC){

	zval **otherValZval;
	int otherNum,otherRun;
	char *otherKey,
		 *otherVal,
		 *otherParams = "";
	ulong otheriKey;
	int hasValue = 0;

	smart_str smart_otherParams = {0};

	otherNum = zend_hash_num_elements(Z_ARRVAL_P(paramsList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(paramsList));

	for(otherRun = 0 ; otherRun < otherNum; otherRun++){

		zend_hash_get_current_key(Z_ARRVAL_P(paramsList),&otherKey,&otheriKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(paramsList),(void**)&otherValZval);

		if(strcmp(otherKey,"c") != 0 && strcmp(otherKey,"a") != 0 && strcmp(otherKey,"m") != 0){


			if(hasValue == 1){
				smart_str_appends(&smart_otherParams,"&");
			}

			if(IS_LONG == Z_TYPE_PP(otherValZval)){


				smart_str_appends(&smart_otherParams,otherKey);
				smart_str_appends(&smart_otherParams,"=");
				smart_str_append_long(&smart_otherParams,Z_LVAL_PP(otherValZval));
				hasValue = 1;

			}else if(IS_STRING == Z_TYPE_PP(otherValZval)){

				smart_str_appends(&smart_otherParams,otherKey);
				smart_str_appends(&smart_otherParams,"=");
				smart_str_appends(&smart_otherParams,Z_STRVAL_PP(otherValZval));
				hasValue = 1;

			}else{

				otherVal = NULL;
				php_error_docref(NULL TSRMLS_CC, E_WARNING ,"Route[url] Parameters [%s] type errors",otherKey);

			}

		}
		zend_hash_move_forward(Z_ARRVAL_P(paramsList));
	}

	if(hasValue == 0){
		*getStr = NULL;
		smart_str_0(&smart_otherParams);
		smart_str_free(&smart_otherParams);
		return;
	}

	smart_str_0(&smart_otherParams);
	*getStr = estrdup(smart_otherParams.c);
	smart_str_free(&smart_otherParams);
}

//依照子域名和host组装URL
void getResultUrl(char *module,char *domain,char *url,char **getStr){
	if(module == NULL){
		char tempUrl[10240];
		sprintf(tempUrl,"%s%s","/",url);
		*getStr = estrdup(tempUrl);
        return;
	}else{
		char tempUrl[10240];
		sprintf(tempUrl,"%s%s%s%s%s","://",module,domain,"/",url);
		*getStr = estrdup(tempUrl);
		return;
	}
}


//以原生方式返回
void urlNoWriteResult(zval *paramsList,zval *reWriteList,int type,char **getStr TSRMLS_DC)
{
	//Type为1时 按照/id/123/tid/456/cid/543方式重写

	zval **controllerZval,
		 **actionZval,
		 **moduleZval;

	char *controller,
		 *action,
		 *url = "",
		 tempUrl[10240];

	smart_str smart_url = {0};

	int i,j;
	char *key,
		 *thisVal;
	ulong ikey;
	zval **val;

	if(zend_hash_find(Z_ARRVAL_P(paramsList),"c",strlen("c")+1,(void**)&controllerZval) == SUCCESS  && IS_STRING == Z_TYPE_PP(controllerZval)){
		controller = estrdup(Z_STRVAL_PP(controllerZval));
		zend_hash_del(Z_ARRVAL_P(paramsList),"c",strlen("c")+1);
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"Route[url] not specify Controller");
	}

	if(zend_hash_find(Z_ARRVAL_P(paramsList),"a",strlen("a")+1,(void**)&actionZval) == SUCCESS && IS_STRING == Z_TYPE_PP(actionZval) ){
		action = estrdup(Z_STRVAL_PP(actionZval));
		zend_hash_del(Z_ARRVAL_P(paramsList),"a",strlen("a")+1);
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"Route[url] not specify Action");
	}


	//控制器方法
	smart_str_appends(&smart_url,controller);
	smart_str_appends(&smart_url,"/");
	smart_str_appends(&smart_url,action);

	efree(controller);
	efree(action);

	j = zend_hash_num_elements(Z_ARRVAL_P(paramsList));
	for(i = 0 ; i < j ; i++){

		if(zend_hash_get_current_key_type(Z_ARRVAL_P(paramsList)) == HASH_KEY_IS_LONG){
			zend_hash_move_forward(Z_ARRVAL_P(paramsList));
			continue;
		}

		zend_hash_get_current_key(Z_ARRVAL_P(paramsList),&key,&ikey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(paramsList),(void**)&val);

		if(strlen(key) <= 0){
			zend_hash_move_forward(Z_ARRVAL_P(paramsList));
			continue;
		}

		//m参数不呈现
		if(strcmp(key,"m") == 0){
			zend_hash_move_forward(Z_ARRVAL_P(paramsList));
			continue;
		}

		//拼接URL
		if(IS_STRING == Z_TYPE_PP(val) && Z_STRLEN_PP(val) > 0 ){
		
			smart_str_appends(&smart_url,"/");
			smart_str_appends(&smart_url,key);
			smart_str_appends(&smart_url,"/");
			smart_str_appends(&smart_url,Z_STRVAL_PP(val));

		}else if(IS_LONG == Z_TYPE_PP(val) ){

			
			smart_str_appends(&smart_url,"/");
			smart_str_appends(&smart_url,key);
			smart_str_appends(&smart_url,"/");
			smart_str_append_long(&smart_url,Z_LVAL_PP(val));
			
		}else{
			zend_hash_move_forward(Z_ARRVAL_P(paramsList));
			continue;
		}
		zend_hash_move_forward(Z_ARRVAL_P(paramsList));
	}

	smart_str_0(&smart_url);

	*getStr = estrdup(smart_url.c);
	
	smart_str_free(&smart_url);
}

//以重新方式返回URL
void urlWriteResult(zval *paramsList,zval *reWriteList,int type,char **getStr TSRMLS_DC)
{
	zval **controllerZval,
		 **actionZval,
		 **moduleZval;

	char *routeKey,
		  routKeyByte[1024],
		 *subName = NULL,
		 *routeRules = NULL,
		 *url;

	int i,j;
	char *key;
	ulong ikey;
	zval **foreachVal;

	//匹配参数
	HashTable *paramsResult;

	zval *paramsResultZval;

	zval *paramsKeyReplaceZval,
		 *paramsValReplaceZval;


	//确定含有指定重写规则
	if(zend_hash_find(Z_ARRVAL_P(paramsList),"c",strlen("c")+1,(void**)&controllerZval) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"Route[url] not specify Controller");
	}
	if(zend_hash_find(Z_ARRVAL_P(paramsList),"a",strlen("a")+1,(void**)&actionZval) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"Route[url] not specify Action");
	}

	//确定是否包含子域名
	if(zend_hash_find(Z_ARRVAL_P(paramsList),"m",strlen("m")+1,(void**)&moduleZval) == SUCCESS){
		subName = Z_STRVAL_PP(moduleZval);
	}

	//组装重写规则Key
	sprintf(routKeyByte,"%s%s%s",Z_STRVAL_PP(controllerZval),"@",Z_STRVAL_PP(actionZval));
	routeKey = estrdup(routKeyByte);

	//遍历规则表获取重写规则
	j = zend_hash_num_elements(Z_ARRVAL_P(reWriteList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(reWriteList));
	for(i = 0 ; i < j ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(reWriteList),&key,&ikey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(reWriteList),(void**)&foreachVal);
		if(IS_STRING == Z_TYPE_PP(foreachVal) && strcmp(routeKey,Z_STRVAL_PP(foreachVal)) == 0 ){
			routeRules = key;
			break;
		}
		zend_hash_move_forward(Z_ARRVAL_P(reWriteList));
	}

	efree(routeKey);

	//若无按重写规则返回
	if(routeRules == NULL){
		char *returnChar;
		urlNoWriteResult(paramsList,reWriteList,type,&returnChar TSRMLS_CC);		
		*getStr = estrdup(returnChar);
		efree(returnChar);
		return;
	}


	//解析规则
	if(preg_match_all("%<\\w+?:.*?>%",routeRules,&paramsResultZval)){
		//获取匹配结果集
		zval **matchReults,
			 **paramsMatchZval,
			 *paramsRulesZval;
		char *paramsMatch,
			 *paramsKey,
			 *paramsRules = NULL;

		//匹配集
		zval **rulesValZval,
			 **rulesParamsVal,
			 *replaceValZval;
		char *rules,
			 *rulesMatch,
			 *rulesKey,
			 *rulesVal,
			 *charToInt;
		ulong rulesiKey;

		zval *paramsKeyValZval;

		MAKE_STD_ZVAL(paramsKeyValZval);
		array_init(paramsKeyValZval);

		
		MAKE_STD_ZVAL(paramsKeyReplaceZval);
		MAKE_STD_ZVAL(paramsValReplaceZval);
		array_init(paramsKeyReplaceZval);
		array_init(paramsValReplaceZval);


		paramsResult = Z_ARRVAL_P(paramsResultZval);

		j = 0;
		if(zend_hash_num_elements(paramsResult) > 0){
			zend_hash_get_current_data(paramsResult,(void**)&matchReults);
			j = zend_hash_num_elements(Z_ARRVAL_PP(matchReults));
		}

		//遍历结果集
		for(i = 0 ; i < j ; i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(matchReults),(void**)&paramsMatchZval);
			php_trim(Z_STRVAL_PP(paramsMatchZval),"<>",&paramsMatch);

			//将规则记录
			add_next_index_string(paramsKeyReplaceZval,Z_STRVAL_PP(paramsMatchZval),1);

			//形式错误的直接忽略
			if(strstr(paramsMatch,":") == NULL){
				zend_hash_move_forward(Z_ARRVAL_PP(matchReults));
				php_error_docref(NULL TSRMLS_CC, E_WARNING ,"Route[url] Configs RewriteList is flawed");
				continue;
			}

			//切割参数形式
			paramsKey = strtok((paramsMatch),":");
			paramsRules = strtok(NULL,":");
			if(paramsRules == NULL){
				zend_hash_move_forward(Z_ARRVAL_PP(matchReults));
				php_error_docref(NULL TSRMLS_CC, E_WARNING ,"Route[url] Configs RewriteList is flawed");
				continue;
			}

			add_assoc_string(paramsKeyValZval,paramsKey,paramsRules,1);
			
			efree(paramsMatch);
			zend_hash_move_forward(Z_ARRVAL_PP(matchReults));
		}



		//尝试匹配表达式参数
		j = zend_hash_num_elements(Z_ARRVAL_P(paramsKeyValZval));
		for(i = 0 ; i < j ; i++){

			zend_hash_get_current_data(Z_ARRVAL_P(paramsKeyValZval),(void**)&rulesValZval);
			zend_hash_get_current_key(Z_ARRVAL_P(paramsKeyValZval),&rulesKey,&rulesiKey,0);

			str_replace("%","\%",Z_STRVAL_PP(rulesValZval),&rules);
			strcat2(&rulesMatch,"%",rules,"%",NULL);
			efree(rules);

			//判断给定的参数集是否存在重写参数 且符合要求类型
			if(zend_hash_find(Z_ARRVAL_P(paramsList),rulesKey,strlen(rulesKey)+1,(void**)&rulesParamsVal) == SUCCESS){

				//判断给定的参数是否符合类型
				if(IS_STRING == Z_TYPE_PP(rulesParamsVal)){
					zval *waitMatchTable;
					if(preg_match(rulesMatch,Z_STRVAL_PP(rulesParamsVal),&waitMatchTable)){
						rulesVal = estrdup(Z_STRVAL_PP(rulesParamsVal));
					}else{
						rulesVal = estrdup("0");
					}
					zval_ptr_dtor(&waitMatchTable);
					
				}else if(IS_LONG == Z_TYPE_PP(rulesParamsVal)){
					toChar(Z_LVAL_PP(rulesParamsVal),&rulesVal);
				}else{
					rulesVal = estrdup("0");
				}


				add_next_index_string(paramsValReplaceZval,rulesVal,1);
				efree(rulesVal);

				//移除paramsList中本次已匹配的项
				zend_hash_del(Z_ARRVAL_P(paramsList),rulesKey,strlen(rulesKey)+1);

			}else{
				add_next_index_string(paramsValReplaceZval,"0",1);
			}

			zend_hash_move_forward(Z_ARRVAL_P(paramsKeyValZval));

			efree(rulesMatch);
		}

		if(zend_hash_num_elements(Z_ARRVAL_P(paramsKeyValZval)) > 0){
			//生成URL
			str_replaceArray(paramsKeyReplaceZval,paramsValReplaceZval,routeRules,&url);
		}else{
			url = estrdup(routeRules);
		}

		zval_ptr_dtor(&paramsKeyValZval);
		zval_ptr_dtor(&paramsKeyReplaceZval);
		zval_ptr_dtor(&paramsValReplaceZval);

	}else{
		url = estrdup(routeRules);
	}

	zval_ptr_dtor(&paramsResultZval);

	//组织其他未指定的URL参数
	MODULE_BEGIN
		char *otherParams,
			 *endUrl;
		getOtherParams(paramsList,&otherParams TSRMLS_CC);
		if(otherParams != NULL){
			strcat2(&endUrl,url,"?",otherParams,NULL);
			efree(otherParams);
			*getStr = estrdup(endUrl);
			efree(endUrl);
			efree(url);
			return;
		}
		*getStr = estrdup(url);
		efree(url);
		return;
	MODULE_END
}

//获取子域名
void getSubdomain(char **getStr TSRMLS_DC)
{
	//当前请求的URL
	char *requestNow;
	char *serverType = "HTTP_HOST";
	char *subdomain;

	//确定请求的URL(cli)
	getServerParam(serverType,&requestNow TSRMLS_CC);
	if(requestNow == NULL) {
		*getStr = "www";
		return;
	};

	//切割第一个子域名
	subdomain = strtok( (requestNow), ".");
	*getStr = estrdup(subdomain);
	efree(requestNow);
}

//生成URL
void CRouteParse_url(zval *paramsList,char **returnUrl TSRMLS_DC)
{
	char *c,
		 *a,
		 *module = NULL,
		 *defaultController,
		 *defaultAction,
		 *defaultModule,
		 *useRewrite,
		 *resultUrl = "",
		 *domain;

	zval **thisVal,
		 *cconfigInstanceZval,
		 *configVal,
		 *reWriteList;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//重写路由表
	CConfig_load("URLRewrite.LIST",cconfigInstanceZval,&configVal TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(configVal)){
		MAKE_STD_ZVAL(reWriteList);
		array_init(reWriteList);
	}else{
		MAKE_STD_ZVAL(reWriteList);
		ZVAL_ZVAL(reWriteList,configVal,1,0);
	}
	zval_ptr_dtor(&configVal);

	//确定根域名
	CConfig_load("COOKIE_DOMAIN",cconfigInstanceZval,&configVal TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configVal) && strlen(Z_STRVAL_P(configVal)) > 0 ){
		domain = estrdup(Z_STRVAL_P(configVal));
	}else{
		//从当前请求中提取根域名
		char *requestNow;
		char *subDomain;

		getSubdomain(&subDomain TSRMLS_CC);
		getServerParam("HTTP_HOST",&requestNow TSRMLS_CC);
		if(requestNow != NULL){
			str_replace(subDomain,"",requestNow,&domain);
			efree(subDomain);
			efree(requestNow);
		}else{
			domain = estrdup("");
		}
	}
	zval_ptr_dtor(&configVal);

	//获取参数中的c和a参数
	if(zend_hash_exists(Z_ARRVAL_P(paramsList),"c",strlen("c")+1) && SUCCESS == zend_hash_find(Z_ARRVAL_P(paramsList),"c",strlen("c")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) && strlen(Z_STRVAL_PP(thisVal)) > 0 ){
	}else{
		zval_ptr_dtor(&reWriteList);
		efree(domain);
		*returnUrl = estrdup("/");
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}

	if(zend_hash_exists(Z_ARRVAL_P(paramsList),"a",strlen("a")+1) && SUCCESS == zend_hash_find(Z_ARRVAL_P(paramsList),"a",strlen("a")+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) && strlen(Z_STRVAL_PP(thisVal)) > 0  ){
	}else{
		zval_ptr_dtor(&reWriteList);
		efree(domain);
		*returnUrl = estrdup("/");
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}

	if(zend_hash_exists(Z_ARRVAL_P(paramsList),"m",strlen("m")+1) && SUCCESS ==  zend_hash_find(Z_ARRVAL_P(paramsList),"m",strlen("m")+1,(void**)&thisVal)  && IS_STRING == Z_TYPE_PP(thisVal) && strlen(Z_STRVAL_PP(thisVal)) > 0   ){
	}else{
		zend_hash_del(Z_ARRVAL_P(paramsList),"m",strlen("m")+1);
	}


	//使用重写
	CConfig_load("URLRewrite.OPEN",cconfigInstanceZval,&configVal TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configVal)){
		useRewrite = estrdup(Z_STRVAL_P(configVal));
	}else{
		useRewrite = estrdup("on");
	}
	php_strtolower(useRewrite,strlen(useRewrite)+1);
	zval_ptr_dtor(&configVal);


	if(strcmp(useRewrite,"on") == 0){
		char *tempUrl;
		//调用重写函数
		urlWriteResult(paramsList,reWriteList,1,&tempUrl TSRMLS_CC);
		getResultUrl(module,domain,tempUrl,&resultUrl);
		*returnUrl = estrdup(resultUrl);
		efree(tempUrl);
		efree(resultUrl);
		efree(useRewrite);
		efree(domain);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&reWriteList);
		return;
	}else{
		//调用非重写函数
		char *reNoWriteUrl;
		urlNoWriteResult(paramsList,reWriteList,1,&reNoWriteUrl TSRMLS_CC);
		getResultUrl(module,domain,reNoWriteUrl,&resultUrl);
		*returnUrl = estrdup(resultUrl);
		efree(reNoWriteUrl);
		efree(resultUrl);
		efree(domain);
		efree(useRewrite);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&reWriteList);
		return;
	}

	efree(useRewrite);
	*returnUrl = estrdup(resultUrl);
	return;
}

PHP_METHOD(CRouteParse,url)
{}

PHP_METHOD(CRouteParse,NoWriteUrl)
{}

PHP_METHOD(CRouteParse,URLrewriteType_1)
{}

PHP_METHOD(CRouteParse,URLrewriteType_2)
{}

PHP_METHOD(CRouteParse,urlWriteResult)
{}

PHP_METHOD(CRouteParse,getOtherParams)
{}

//获取当前请求子域名
void CRouteParse_getSubdomain(zval **returnZval TSRMLS_DC)
{
	char	*hostAddress = NULL,
			*httpXHost,
			*httpHost,
			*subName;

	MAKE_STD_ZVAL(*returnZval);

	//获取HTTP_X_FORWARDED_HOST变量
	getServerParam("HTTP_X_FORWARDED_HOST",&httpXHost TSRMLS_CC);
	if(NULL != httpXHost){
		hostAddress = estrdup(httpXHost);
		efree(httpXHost);
	}else
	{
		//判断有无HTTP_HOST变量
		getServerParam("HTTP_HOST",&httpHost TSRMLS_CC);
		if(NULL != httpHost){
			hostAddress = estrdup(httpHost);
			efree(httpHost);
		}
	}

	//无法获取到子域名时返回空
	if(NULL == hostAddress){
		ZVAL_NULL(*returnZval);
		return;
	}

	//按第一个.符号分割
	subName = strtok(hostAddress,".");
	ZVAL_STRING(*returnZval,subName,1);
	efree(hostAddress);
	return;
}

//获取当前请求子域名
PHP_METHOD(CRouteParse,getSubdomain)
{
	zval *returnZval,
		 *resultZval;

	HashTable *returnTable;

	CRouteParse_getSubdomain(&returnZval TSRMLS_CC);
	
	//组装数据返回
	ALLOC_HASHTABLE(returnTable);
	zend_hash_init(returnTable,4,NULL,NULL,0);
	zend_hash_add(returnTable,"m",strlen("m")+1,&returnZval,sizeof(zval*),NULL);

	MAKE_STD_ZVAL(resultZval);
	Z_TYPE_P(resultZval) = IS_ARRAY;
	Z_ARRVAL_P(resultZval) = returnTable;
	ZVAL_ZVAL(return_value,resultZval,1,0);
}

PHP_METHOD(CRouteParse,getCacheRoute)
{}


PHP_METHOD(CRouteParse,getSubdomainUrl)
{}

PHP_METHOD(CRouteParse,_setCacheRoute)
{}

PHP_METHOD(CRouteParse,getRequestUri)
{}

//获取请求URI
void CRouteParse_requestURI(zval **returnZval TSRMLS_DC)
{
	zval	*sapiZval;
	
	char	*thisUri;


	//获取PHP_SAPI宏以判断请求方式
	if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) != SUCCESS){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_STRING(*returnZval,"",1);
		return;
	}

	if(IS_STRING != Z_TYPE_P(sapiZval)){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_STRING(*returnZval,"",1);
		return;
	}

	if( strcmp(Z_STRVAL_P(sapiZval),"cli") == 0 ){

		//cli命令行运行
		zval **SERVER;
		(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);

		//判断是否存在[args][1]
		if(zend_hash_exists(Z_ARRVAL_PP(SERVER),"argv",strlen("argv")+1)){
                        zval    **argsZval,
                                        **argsUri;
                        zend_hash_find(Z_ARRVAL_PP(SERVER),"argv",strlen("argv")+1, (void **)&argsZval);

                        //判断有无[1]
                        if(SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(argsZval),1,(void**)&argsUri) && IS_STRING == Z_TYPE_PP(argsUri) ){
                                //移除右边的/符号
                                char *thisCliUri;
                                php_trim(Z_STRVAL_PP(argsUri),"/",&thisCliUri);
                                MAKE_STD_ZVAL(*returnZval);
                                ZVAL_STRING(*returnZval,thisCliUri,1);
								efree(thisCliUri);
                                return;
                        }
                }
		
		//无法确定返回NULL
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_STRING(*returnZval," ",1);
		return;
	}else{

		//获取SERVER确定最终URI
		char	*requestUri,
				*phpSelf,
				*redirectUrl;


		getServerParam("REQUEST_URI",&requestUri TSRMLS_CC);
		getServerParam("PHP_SELF",&phpSelf TSRMLS_CC);
		getServerParam("REDIRECT_URL",&redirectUrl TSRMLS_CC);

		if(NULL != requestUri){
			thisUri = requestUri;
		}else if(NULL != phpSelf){
			thisUri = phpSelf;
		}else if(NULL != redirectUrl){
			thisUri = redirectUrl;
		}else{	
			//触发路由之前的HOOKS_ROUTE_START钩子
			CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[RouteException] The server is unable to resolve the time request");
			return;
		}

		//过滤分层目录
		MODULE_BEGIN
			char	*pathFileTemp,
					*pathFile,
					*uri;

			zval		*findTable,
						*replaceTable;

			str_replace("/index.php","",phpSelf,&pathFileTemp);
			php_trim(pathFileTemp," ",&pathFile);

			MAKE_STD_ZVAL(findTable);
			MAKE_STD_ZVAL(replaceTable);
			array_init(findTable);
			array_init(replaceTable);

			add_next_index_string(findTable,pathFile,0);
			add_next_index_string(replaceTable,"",0);
			str_replaceArray(findTable,replaceTable,thisUri,&uri);

			MAKE_STD_ZVAL(*returnZval);
			ZVAL_STRING(*returnZval,uri,1);

			efree(uri);
			efree(pathFileTemp);
			efree(pathFile);

			if(NULL != requestUri){
				efree(requestUri);
			}
			if(NULL != phpSelf){
				efree(phpSelf);
			}
			if(NULL != redirectUrl){
				efree(redirectUrl);
			}

		MODULE_END
	}

	return;
}

//获取请求URI
PHP_METHOD(CRouteParse,requestURI)
{
	zval *returnZval;
	CRouteParse_requestURI(&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,0); 
	zval_ptr_dtor(&returnZval);
}

//移除魔术引号
void stripslashes(zval *val TSRMLS_DC)
{
	int i,num;
	zval	**thisZval,
			*saveVal;
	char	*key;
	ulong	ukey;

	if(IS_ARRAY != Z_TYPE_P(val)){
		return;
	}

	num = zend_hash_num_elements(Z_ARRVAL_P(val));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
	for(i = 0 ; i < num ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(val),(void**)&thisZval);
		zend_hash_get_current_key(Z_ARRVAL_P(val),&key,&ukey,0);

		MAKE_STD_ZVAL(saveVal);
		ZVAL_STRING(saveVal,"32132132132321321321321321321",1);
		zend_hash_del(Z_ARRVAL_P(val),key,strlen(key)+1);

		//zend_hash_update(Z_ARRVAL_P(val),key,strlen(key)+1,&saveVal,sizeof(zval*),NULL);

		zend_hash_move_forward(Z_ARRVAL_P(val));
	}



}

//解析路由
void CRouteParse_getRoute(zval **returnZval TSRMLS_DC)
{
	zval	*requestUrlZval,
			*cconfigInstanceZval;

	char	*requestUri,
			*host = NULL,
			*query = NULL,
			*requsetM,
			*hostTemp;

	zval	*params;


	//返回的数据
	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//获取M参数
	MODULE_BEGIN
		char	*getM,
				*mVal = "www";
		zval	*subdomainZval;
		getGetParam("m",&getM TSRMLS_CC);
		if(NULL == getM){
			CRouteParse_getSubdomain(&subdomainZval TSRMLS_CC);
			if(IS_NULL == Z_TYPE_P(subdomainZval)){
				//获取配置
				zval	*thisItemZval;

				CConfig_load("DEFAULT_MODLUE",cconfigInstanceZval,&thisItemZval TSRMLS_CC);
				if(IS_STRING == Z_TYPE_P(thisItemZval) && strlen(Z_STRVAL_P(thisItemZval)) > 0 ){
					mVal = estrdup(Z_STRVAL_P(thisItemZval));
				}else{
					mVal = estrdup("www");
				}
				zval_ptr_dtor(&thisItemZval);
			}else{
				mVal = estrdup(Z_STRVAL_P(subdomainZval));
			}
			zval_ptr_dtor(&subdomainZval);
		}else{
			mVal = estrdup("www");
			efree(getM);
		}

		requsetM = estrdup(mVal);

		zend_update_static_property_string(CRouteCe,"thisModule",strlen("thisModule"),requsetM TSRMLS_CC);
		efree(mVal);

	MODULE_END


	//直接获取c、a参数
	MODULE_BEGIN
		char	*getC,
				*getA;

		getGetParam("c",&getC TSRMLS_CC);
		getGetParam("a",&getA TSRMLS_CC);

		if(NULL != getC && NULL != getA){
			//给结构体复制
			zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),getC TSRMLS_CC);
			zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),getA TSRMLS_CC);
			efree(getA);
			efree(getC);
			//返回GET参数集合
			MODULE_BEGIN
				zval **getDataZval;
				(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&getDataZval);
				ZVAL_ZVAL(*returnZval,*getDataZval,1,0);
				efree(requsetM);
				zval_ptr_dtor(&cconfigInstanceZval);
				return;
			MODULE_END
		}
	MODULE_END

	//获取请求URI以解析参数
	CRouteParse_requestURI(&requestUrlZval TSRMLS_CC);

	if(IS_STRING == Z_TYPE_P(requestUrlZval)){
		zend_update_static_property_string(CRouteCe,ZEND_STRL("requsetUri"),Z_STRVAL_P(requestUrlZval) TSRMLS_CC);
	}else{
		zend_update_static_property_string(CRouteCe,ZEND_STRL("requsetUri"),"" TSRMLS_CC);
	}

	//解析uri
	host = strtok(estrdup(Z_STRVAL_P(requestUrlZval)),"?");
	query = strtok(NULL,"?");

	//分割动态参数
	MAKE_STD_ZVAL(params);
	array_init(params);

	if(NULL != query){
		int paramsNum = 0,
			i = 0;

		char *paramKey = NULL,
			 *paramValue,
			 *skeyHead,
			 *cutResult = NULL;

		ulong ikeyHead;

		zval * paramValueZval;

		zval *saveGetParams,
			 *paramsTemp,
			 **keyTempZval;

		zval *dyParams;

		//调用parse_str解析query部分参数
		parse_str(query,&paramsTemp);

		//遍历变量params生成key->value键值对
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(paramsTemp));
		paramsNum = zend_hash_num_elements(Z_ARRVAL_P(paramsTemp));
		for(i = 0;i<paramsNum;i++){

			if(HASH_KEY_IS_STRING == zend_hash_get_current_key_type(Z_ARRVAL_P(paramsTemp))){
				//切割
				zend_hash_get_current_key(Z_ARRVAL_P(paramsTemp),&skeyHead,&ikeyHead,0);
				zend_hash_get_current_data(Z_ARRVAL_P(paramsTemp),(void**)&keyTempZval);
				add_assoc_string(params,skeyHead,Z_STRVAL_PP(keyTempZval),1);
				zend_hash_move_forward(Z_ARRVAL_P(paramsTemp));

			}else if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_P(paramsTemp))){
				int thisIntKey;
				char thisKeyString[64];
				zend_hash_get_current_key(Z_ARRVAL_P(paramsTemp), &skeyHead, &thisIntKey, 0);
				zend_hash_get_current_data(Z_ARRVAL_P(paramsTemp),(void**)&keyTempZval);
				sprintf(thisKeyString,"%d",thisIntKey);
				add_assoc_string(params,thisKeyString,Z_STRVAL_PP(keyTempZval),1);
				zend_hash_move_forward(Z_ARRVAL_P(paramsTemp));
			}
		}

		//销毁中间变量
		zval_ptr_dtor(&paramsTemp);
	}

	//确定urlPath 去掉左右的/符号
	php_trim(host,"/",&hostTemp);
	efree(host);
	host = estrdup(hostTemp);
	efree(hostTemp);

	//判断是否允许进入的首页
	MODULE_BEGIN
		char	*urlKey;
		zval	*thisItemZval;

		urlKey = host;
		if(strcmp(urlKey,"") == 0 || NULL == urlKey ){
			urlKey = "index";
		}

		//读取ALLOW_INDEX配置
		CConfig_load("ALLOW_INDEX",cconfigInstanceZval,&thisItemZval TSRMLS_CC);
		if(IS_NULL == Z_TYPE_P(thisItemZval) || IS_ARRAY != Z_TYPE_P(thisItemZval)){
			array_init(thisItemZval);
			add_next_index_string(thisItemZval,"index",1);
		}

		//判断允许的首页
		if(strcmp((urlKey),"") == 0 || strcmp((urlKey),"/") == 0 || in_array(urlKey,thisItemZval) == 1 ){

			//默认控制器
			zval	*defaultController,
					*defaultAction;

			CConfig_load("DEFAULT_CONTROLLER",cconfigInstanceZval,&defaultController TSRMLS_CC);
			if(IS_NULL == Z_TYPE_P(defaultController)){
				ZVAL_STRING(defaultController,"base",1);
			}

			CConfig_load("DEFAULT_ACTION",cconfigInstanceZval,&defaultAction TSRMLS_CC);
			if(IS_NULL == Z_TYPE_P(defaultAction)){
				ZVAL_STRING(defaultAction,"index",1);
			}

			//给路由结构体赋值
			zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),Z_STRVAL_P(defaultController) TSRMLS_CC);
			zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),Z_STRVAL_P(defaultAction) TSRMLS_CC);

			//将动态参数合并到GET中
			setGetParamArray(Z_ARRVAL_P(params) TSRMLS_CC);

			//返回GET参数集合
			zval_ptr_dtor(&requestUrlZval);
			zval_ptr_dtor(&defaultController);
			zval_ptr_dtor(&defaultAction);

			efree(host);
			efree(requsetM);
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&params);
			return;
		}
		zval_ptr_dtor(&thisItemZval);
	MODULE_END

	//分析路由表 寻找最相似的重写规则
	MODULE_BEGIN
		zval	*routeTable,
				**data,
				**matchAllZval,
				*controllerZval,
				*actionZval,
				**getDataZval,
				**paramsData,
				*setGetDataZval;

		char	*skey,
				*thisRouteController,
				*thisRouteAction,
				*regPatternReplace,
				*getParamsReg = "%<\\w+?:(.*?)>%",
				*getUrlParamsReg = "%<\\w+?:.*?>%",
				*matchAll = NULL,
				*controllerAction = NULL,
				*getKey,
				*regPatternReplaceMatch,
				*regPatternReplaceStr;

		ulong	ikey,
				getIkey;

		HashTable	*regAllResult,
					*paramsResult;

		zval	*regAllResultZval;

		int r,
			rNum,
			paramsListTableNum = 0,
			x = 0;

		//路由规则表
		CConfig_load("URLRewrite.LIST",cconfigInstanceZval,&routeTable TSRMLS_CC);

		if(IS_ARRAY != Z_TYPE_P(routeTable)){
			array_init(routeTable);
		}

		//遍历路由表
		rNum = zend_hash_num_elements(Z_ARRVAL_P(routeTable));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(routeTable));
		for(r = 0 ; r < rNum; r++){

			//获取路由表的键、值
			zend_hash_get_current_data(Z_ARRVAL_P(routeTable),(void**)&data);
			zend_hash_get_current_key(Z_ARRVAL_P(routeTable),&skey,&ikey,0);

			preg_repalce(getParamsReg,"($1)",skey,&regPatternReplaceMatch);
			str_replace("%","\%",regPatternReplaceMatch,&regPatternReplaceStr);
			efree(regPatternReplaceMatch);
			strcat2(&regPatternReplace,"%",regPatternReplaceStr,"%",NULL);
			efree(regPatternReplaceStr);

			if(preg_match(regPatternReplace,host,&regAllResultZval))
			{
				regAllResult = Z_ARRVAL_P(regAllResultZval);
				if(zend_hash_index_find(regAllResult, 0, (void**)&matchAllZval) == FAILURE){
					zend_hash_move_forward(Z_ARRVAL_P(routeTable));
					continue;
				}

				matchAll = Z_STRVAL_PP(matchAllZval);
				php_trim(Z_STRVAL_PP(matchAllZval),"",&matchAll);

				if(strcmp(matchAll,(host)) != 0){
					zend_hash_move_forward(Z_ARRVAL_P(routeTable));
					continue;
				}

				//解析动态参数
				if(matchAll != NULL && strlen(matchAll) > 0)
				{
					int j							= 0, 
						queryPramsNum				= 0;
					ulong queryIKey;
					char * requestController		= NULL,
						 * requestAction			= NULL,
						 * cutParamsKey				= NULL,
						 * cutParamsVal				= NULL,
						 * queryKey					= NULL,
						 * queryParamsValStr        = NULL;
						 
					zval * curValZval				= NULL,
						 ** cutParamsValZval		= NULL,
						 ** queryParamsVal			= NULL,
						 ** queryParamsValTable		= NULL,
						 *paramsResultZval;

					char *cutControllerAction = NULL;
					if(preg_match_all(getUrlParamsReg,skey,&paramsResultZval)){

						zend_hash_get_current_data(Z_ARRVAL_P(paramsResultZval), (void**)&queryParamsValTable);

						paramsResult = Z_ARRVAL_PP(queryParamsValTable);

						queryPramsNum = zend_hash_num_elements(paramsResult);
						zend_hash_internal_pointer_reset(paramsResult);

						for(j = 0 ; j < queryPramsNum ; j++){
							zend_hash_get_current_data(paramsResult, (void**)&queryParamsVal);
							zend_hash_get_current_key(paramsResult, &queryKey, &queryIKey, 0);
							queryParamsValStr = Z_STRVAL_PP(queryParamsVal);
							php_trim(queryParamsValStr,"<>",&queryParamsValStr);

							if(strstr(queryParamsValStr,":") != NULL){
								//将参数按：分割并加入一个参数的哈希表
								cutParamsKey = strtok(queryParamsValStr,":");

								//判断检索中的参数值表是否存在参数值
								if(zend_hash_index_find(regAllResult, j + 1, (void**)&cutParamsValZval) != FAILURE){
									add_assoc_string(params,cutParamsKey,Z_STRVAL_PP(cutParamsValZval),1);
								}
							}

							//移动指针
							zend_hash_move_forward(paramsResult);
						}
					}
					efree(matchAll);
					zval_ptr_dtor(&paramsResultZval);

					//检测Controller和Action有效性

					//对隐藏变量赋值


					//将路由数据组合
					controllerAction = Z_STRVAL_PP(data);
					cutControllerAction = estrdup(controllerAction);
					requestController = strtok(cutControllerAction,"@");
					requestAction = strtok(NULL,"@");

					if(requestController != NULL && requestController != ""){ 
						zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),(requestController) TSRMLS_CC);
					}
					if(requestAction != NULL && requestController != ""){ 
						zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),(requestAction) TSRMLS_CC);
					}

					//给路由结构体赋值
					zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),(requestController) TSRMLS_CC);
					zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),(requestAction) TSRMLS_CC);

					//将动态参数合并到GET中
					MODULE_BEGIN
						zval **getDataZval;
						setGetParamArray(Z_ARRVAL_P(params) TSRMLS_CC);

						//返回GET参数集合
						(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&getDataZval);
						ZVAL_ZVAL(*returnZval,*getDataZval,1,0);
						efree(requsetM);
						efree(host);
						zval_ptr_dtor(&requestUrlZval);
						zval_ptr_dtor(&params);
						zval_ptr_dtor(&cconfigInstanceZval);
						return ;
					MODULE_END
				}	
			}
			zval_ptr_dtor(&regAllResultZval);
			efree(regPatternReplace);
			zend_hash_move_forward(Z_ARRVAL_P(routeTable));
		}
		zval_ptr_dtor(&routeTable);
		zval_ptr_dtor(&cconfigInstanceZval);

	MODULE_END


	//未发现路由规则
	MODULE_BEGIN
	
			int noRewiteParamsNum = 0,
				paramsListTableNum;

			char	*urlNoExtendsName = NULL,
					*noRewriteController = NULL,
					*noRewriteResult = NULL,
					*noRewriteKey = NULL,
					*noRewriteAction = NULL;

			zval	*controllerZval,
					*actionZval,
					**getDataZval;
		
			zval *noRewiteList;

			int x=0;

			ulong getIkey;

			if(strstr(host,".html") != NULL){
				urlNoExtendsName = strtok(host,".");
			}else{
				urlNoExtendsName = host;
			}

			//使用默认重写方式匹配
			if(strstr(urlNoExtendsName,"/") != NULL)
			{
				int n = 0;
				int nowRewriteListNum = 0;
				int paramsInt;
				char *nkey = NULL,
					 *nSetKey = NULL,
					 *nSetKey1 = NULL,
					 *nNextKey = NULL;
				ulong nIkey;
				zval **nData		= NULL,
					 *noWriteToParamZval = NULL,
					 **nNextData	= NULL;

				MAKE_STD_ZVAL(noRewiteList);
				array_init(noRewiteList);

				noRewriteResult = strtok( urlNoExtendsName,"/");
				while( noRewriteResult != NULL ) {
					add_next_index_string(noRewiteList,noRewriteResult,1);
					noRewriteResult = strtok( NULL, "/" );
				}

				nowRewriteListNum = zend_hash_num_elements(Z_ARRVAL_P(noRewiteList));
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(noRewiteList));
				for( n = 0 ; n < nowRewriteListNum ; n++){
					zend_hash_get_current_data(Z_ARRVAL_P(noRewiteList),(void**)&nData);
					if(n == 0){
						noRewriteController = Z_STRVAL_PP(nData);
					}else if(n == 1){
						noRewriteAction = Z_STRVAL_PP(nData);
					}else{
						//按顺序组织参数值
						paramsInt = n;
						if(paramsInt % 2 == 0 ){
							nSetKey = Z_STRVAL_PP(nData);
							if(zend_hash_index_exists(Z_ARRVAL_P(noRewiteList),paramsInt+1)){
								//存入参数列表
								zend_hash_index_find(Z_ARRVAL_P(noRewiteList),paramsInt+1,(void**)&nNextData);

								//判断是否存在 存在则不覆盖以动态附加为准
								if(!zend_hash_exists(Z_ARRVAL_P(params),nSetKey,strlen(nSetKey)+1)){
									add_assoc_string(params,nSetKey,Z_STRVAL_PP(nNextData),1);
								}
							}
						}
					}
					zend_hash_move_forward(Z_ARRVAL_P(noRewiteList));
				}

				//对控制器方法赋值
				add_assoc_string(params,"0",noRewriteController,1);
				add_assoc_string(params,"1",noRewriteAction,1);


				//给路由结构体赋值
				zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),noRewriteController TSRMLS_CC);
				zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),noRewriteAction TSRMLS_CC);

				//将动态参数合并到GET中
				MODULE_BEGIN
					zval **getDataZval;
					zend_hash_index_del(Z_ARRVAL_P(params),0);
					zend_hash_index_del(Z_ARRVAL_P(params),1);
					setGetParamArray(Z_ARRVAL_P(params) TSRMLS_CC);

					//返回GET参数集合
					(void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&getDataZval);
					ZVAL_ZVAL(*returnZval,*getDataZval,1,0);

					zval_ptr_dtor(&params);
					zval_ptr_dtor(&noRewiteList);
					zval_ptr_dtor(&requestUrlZval);
					efree(host);
					efree(requsetM);
					return;
				MODULE_END
			}else{

				//触发路由之前的HOOKS_ROUTE_START钩子
				CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);	

				php_error_docref(NULL TSRMLS_CC,E_ERROR,"[RouteException] The requested address does not exist");
				return;
			}
	MODULE_END

	//触发路由之前的HOOKS_ROUTE_START钩子
	CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);	
	php_error_docref(NULL TSRMLS_CC,E_ERROR,"[RouteException] The requested address does not exist");
	return;
}

//解析路由
PHP_METHOD(CRouteParse,getRoute)
{
	zval *returnZval;

	CRouteParse_getRoute(&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1); 
}
