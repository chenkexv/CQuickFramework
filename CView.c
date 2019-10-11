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
#include "php_CView.h"
#include "php_CRoute.h"
#include "php_CRequest.h"
#include "php_CQuickTemplate.h"
#include "php_CException.h"

//zend类方法
zend_function_entry CView_functions[] = {
	PHP_ME(CView,factory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CView,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CView,getViews,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CView)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CView",CView_functions);
	CViewCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量
	zend_declare_property_null(CViewCe, ZEND_STRL("viewList"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CViewCe, ZEND_STRL("_viewName"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CViewCe, ZEND_STRL("_viewObject"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//向Smarty赋默认值
void smarty_assign(zval *viewObject,char *key,char *val TSRMLS_DC)
{
	zval	callFunction,
			callReturn,
			*paramsList[2],
			param1,
			param2;

	paramsList[0] = &param1;
	paramsList[1] = &param2;

	INIT_ZVAL(callFunction);
	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);

	ZVAL_STRING(&callFunction,"assign",0);
	ZVAL_STRING(paramsList[0],key,1);
	ZVAL_STRING(paramsList[1],val,1);

	call_user_function(NULL, &viewObject, &callFunction, &callReturn, 2, paramsList TSRMLS_CC);

	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	zval_dtor(&callReturn);
}

//复杂形式参数赋值
void smarty_assign_ex(zval *viewObject,char *key,zval *val TSRMLS_DC)
{
	zval	callFunction,
			callReturn,
			*paramsList[2],
			param1,
			param2;

	paramsList[0] = &param1;
	paramsList[1] = &param2;

	INIT_ZVAL(callFunction);
	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);

	ZVAL_STRING(&callFunction,"assign",0);
	ZVAL_STRING(paramsList[0],key,1);
	ZVAL_ZVAL(paramsList[1],val,1,0);
	call_user_function(NULL, &viewObject, &callFunction, &callReturn, 2, paramsList TSRMLS_CC);

	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	zval_dtor(&callReturn);
}


void CView_init_CTemplate(zval *viewObject TSRMLS_DC){

	int		cacheTime = 100,
			allowPhp = 1,
			cached = 0;

	zval	*cconfigInstanceZval,
			*siteconfigInstanceZval,
			*thisConf,
			*saveThis,
			*controller,
			*action,
			*requsetUri,
			*module,
			*conf_coreFile,
			*conf_smarty,
			**conf_cacheDir,
			**conf_compileDir,
			**conf_templateDir,
			**conf_leftTag,
			**conf_rightTag,
			**conf_allowPhp,
			**conf_isCache,
			**conf_cachetime,
			*urlInfo,
			*PageInfo,
			*substrFn,
			*sayTime,
			*checkRightFn,
			*showHtml;

	controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
	action = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);
	module = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
	requsetUri = zend_read_static_property(CRouteCe,ZEND_STRL("requsetUri"),0 TSRMLS_CC);

	//注册块函数
	MAKE_STD_ZVAL(urlInfo);
	array_init(urlInfo);
	add_next_index_string(urlInfo,"CRequest",1);
	add_next_index_string(urlInfo,"createUrl",1);
	CQuickTemplate_register_function(viewObject,"url",urlInfo TSRMLS_CC);
	zval_ptr_dtor(&urlInfo);

	//PageInfo
	MAKE_STD_ZVAL(PageInfo);
	array_init(PageInfo);
	add_next_index_string(PageInfo,"CSmarty",1);
	add_next_index_string(PageInfo,"showPageData",1);
	CQuickTemplate_register_function(viewObject,"PageInfo",PageInfo TSRMLS_CC);
	zval_ptr_dtor(&PageInfo);

	//substr
	MAKE_STD_ZVAL(substrFn);
	array_init(substrFn);
	add_next_index_string(substrFn,"CSmarty",1);
	add_next_index_string(substrFn,"cn_substr",1);
	CQuickTemplate_register_function(viewObject,"substr",substrFn TSRMLS_CC);
	zval_ptr_dtor(&substrFn);

	//sayTime
	MAKE_STD_ZVAL(sayTime);
	array_init(sayTime);
	add_next_index_string(sayTime,"CSmarty",1);
	add_next_index_string(sayTime,"sayTime",1);
	CQuickTemplate_register_function(viewObject,"sayTime",sayTime TSRMLS_CC);
	zval_ptr_dtor(&sayTime);


	//block
	MAKE_STD_ZVAL(checkRightFn);
	array_init(checkRightFn);
	add_next_index_string(checkRightFn,"CSmarty",1);
	add_next_index_string(checkRightFn,"checkRight",1);
	CQuickTemplate_register_block(viewObject,"checkRight",checkRightFn TSRMLS_CC);
	zval_ptr_dtor(&checkRightFn);

	//showHTML
	MAKE_STD_ZVAL(showHtml);
	array_init(showHtml);
	add_next_index_string(showHtml,"CSmarty",1);
	add_next_index_string(showHtml,"showHTML",1);
	CQuickTemplate_register_function(viewObject,"showHTML",showHtml TSRMLS_CC);
	zval_ptr_dtor(&showHtml);


	//读取配置文件
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//其余配置
	CConfig_load("TEMPLATE.CQuickTemplate.CONF_INFO",cconfigInstanceZval,&conf_smarty TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(conf_smarty)){
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO] type Error, not give an array");
		return;
	}

	//缓存目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"cache_dir",strlen("cache_dir")+1,(void**)&conf_cacheDir) && IS_STRING == Z_TYPE_PP(conf_cacheDir)){
	}else{
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO.cache_dir] Error, is not string");
		return;
	}

	//编译目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"compile_dir",strlen("compile_dir")+1,(void**)&conf_compileDir) && IS_STRING == Z_TYPE_PP(conf_compileDir)){
	}else{
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO.compile_dir] Error, is not string");
		return;
	}

	//模板目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"template_dir",strlen("template_dir")+1,(void**)&conf_templateDir) && IS_STRING == Z_TYPE_PP(conf_templateDir)){
	}else{
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO.template_dir] Error, is not string");
		return;
	}

	//左标
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"left_delimiter",strlen("left_delimiter")+1,(void**)&conf_leftTag) && IS_STRING == Z_TYPE_PP(conf_leftTag)){
	}else{
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO.left_delimiter] Error, is not string");
		return;
	}

	//右标
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"right_delimiter",strlen("right_delimiter")+1,(void**)&conf_rightTag) && IS_STRING == Z_TYPE_PP(conf_rightTag)){
	}else{
		zval_ptr_dtor(&conf_smarty);
		zval_ptr_dtor(&cconfigInstanceZval);
		php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->CQuickTemplate->CONF_INFO.right_delimiter] Error, is not string");
		return;
	}

	//重置smarty配置
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"template_dir",strlen("template_dir"),Z_STRVAL_PP(conf_templateDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"compile_dir",strlen("compile_dir"),Z_STRVAL_PP(conf_compileDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"cache_dir",strlen("cache_dir"),Z_STRVAL_PP(conf_cacheDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"left_delimiter",strlen("left_delimiter"),Z_STRVAL_PP(conf_leftTag) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"right_delimiter",strlen("right_delimiter"),Z_STRVAL_PP(conf_rightTag) TSRMLS_CC);

	//检查编译目录
	if(FAILURE == fileExist(Z_STRVAL_PP(conf_compileDir))){
		//尝试创建文件夹
		php_mkdir(Z_STRVAL_PP(conf_compileDir));

		//创建失败
		if(FAILURE == fileExist(Z_STRVAL_PP(conf_compileDir))){
			char *thisMothTime;
			php_date("Y-m-d h:i:s",&thisMothTime);
			php_printf("#LogTime:%s%s[ServerError] CQuickFramework unable to create the __runtime directory, please make sure the server permissions%s-File:%s%s-Line:%s",thisMothTime,"<br>","<br>","CViews.c","<br>","573");
			efree(thisMothTime);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
		}
	}

	//检查缓存目录
	if(FAILURE == fileExist(Z_STRVAL_PP(conf_cacheDir))){
		//尝试创建文件夹
		php_mkdir(Z_STRVAL_PP(conf_cacheDir));

		//创建失败
		if(FAILURE == fileExist(Z_STRVAL_PP(conf_cacheDir))){
			php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
		}
	}

	//设置smarty默认值
	smarty_assign(viewObject,"thisUrl",Z_STRVAL_P(requsetUri) TSRMLS_CC);
	smarty_assign(viewObject,"controller",Z_STRVAL_P(controller) TSRMLS_CC);

	//获取action
	MODULE_BEGIN
		char *actionNoPre;
		zval *actionPreZval;

		CConfig_load("ACTION_PREFIX",cconfigInstanceZval,&actionPreZval TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(actionPreZval) ){
			str_replace(Z_STRVAL_P(actionPreZval),"",Z_STRVAL_P(action),&actionNoPre);
			smarty_assign(viewObject,"action",(actionNoPre) TSRMLS_CC);
			efree(actionNoPre);
		}else{
			smarty_assign(viewObject,"action",Z_STRVAL_P(action) TSRMLS_CC);
		}
		zval_ptr_dtor(&actionPreZval);
	MODULE_END

	smarty_assign(viewObject,"actionPre","Action_" TSRMLS_CC);
	smarty_assign(viewObject,"module",Z_STRVAL_P(module) TSRMLS_CC);

	//获取配置对象
	CConfig_getInstance("site",&siteconfigInstanceZval TSRMLS_CC);
	MODULE_BEGIN
		zval	*staticUrlZval;
		char	*base64Url;

		CConfig_load("staticUrl",siteconfigInstanceZval,&staticUrlZval TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(staticUrlZval)){
			smarty_assign(viewObject,"staticUrl",(Z_STRVAL_P(staticUrlZval)) TSRMLS_CC);
		}
		zval_ptr_dtor(&staticUrlZval);
	MODULE_END

	//设置ip
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"getIp", 0);
		//获取类的静态方法
		call_user_function(&CRequestCe->function_table, NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;
		smarty_assign_ex(viewObject,"ip",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END

	//设置时间time
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"time", 0);
		//获取类的静态方法
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;	
		smarty_assign_ex(viewObject,"time",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END

	//设置sessionID
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"session_id", 0);
		//获取类的静态方法
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;	
		smarty_assign_ex(viewObject,"sessionID",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END

	zval_ptr_dtor(&conf_smarty);
	zval_ptr_dtor(&siteconfigInstanceZval);
	zval_ptr_dtor(&cconfigInstanceZval);
}


//获取视图对象
void CView_factory(char *getviewName,zval **returnZval TSRMLS_DC)
{
	zval *viewListSaved,
		 *returnViews;

	int	existTemplate;

	char *viewName;

	//如果使用自带引擎则直接返回
	if(strcmp(getviewName,"CQuickTemplate") == 0){

		zval	*templateInstance;

		//获取单例
		CQuickTemplate_getInstance(&templateInstance TSRMLS_CC);

		CView_init_CTemplate(templateInstance TSRMLS_CC);


		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,templateInstance,1,1);
		return;
	}


	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	viewName = estrdup(getviewName);

	php_strtolower(viewName,strlen(viewName)+1);

	//读取viewList
	viewListSaved = zend_read_static_property(CViewCe,ZEND_STRL("viewList"),1 TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(viewListSaved)){
		
		zval *saveView;
		MAKE_STD_ZVAL(saveView);
		array_init(saveView);
		zend_update_static_property(CViewCe,ZEND_STRL("viewList"),saveView TSRMLS_CC);
		viewListSaved = zend_read_static_property(CViewCe,ZEND_STRL("viewList"),0 TSRMLS_CC);
		zval_ptr_dtor(&saveView);
	}

	//判断有无该引擎的视图
	existTemplate = zend_hash_exists(Z_ARRVAL_P(viewListSaved),viewName,strlen(viewName)+1);
	if(existTemplate == 0){
		zval *saveObject,
				*thisViewObject;

		//调用自身构造函数
		MAKE_STD_ZVAL(thisViewObject);
		object_init_ex(thisViewObject,CViewCe);

		//调用其构造函数
		if (CViewCe->constructor) {
			zval	constructReturn,
					constructVal,
					*paramsList[1],
					param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],viewName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CViewCe->constructor->common.function_name, 0);
			call_user_function(NULL, &thisViewObject, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
		}


		//将对象存入viewList
		zend_hash_add(Z_ARRVAL_P(viewListSaved),viewName,strlen(viewName)+1,&thisViewObject,sizeof(zval*),NULL);
		zend_update_static_property(CViewCe,ZEND_STRL("viewList"),viewListSaved TSRMLS_CC);
		returnViews = zend_read_property(CViewCe,thisViewObject,ZEND_STRL("_viewObject"),0 TSRMLS_CC);
		ZVAL_ZVAL(*returnZval,returnViews,1,0);
		efree(viewName);
		return;
	}else{
		zval **saveModelZval,
				*saveViewZval,
				*getViewObjectThis;
		//直接从保存表中读取
		zend_hash_find(Z_ARRVAL_P(viewListSaved),viewName,strlen(viewName)+1,(void**)&saveModelZval);

		//从该对象中读取_viewObject变量
		getViewObjectThis = zend_read_property(CViewCe,*saveModelZval,ZEND_STRL("_viewObject"),0 TSRMLS_CC);
		ZVAL_ZVAL(*returnZval,getViewObjectThis,1,0);
		efree(viewName);
		return;
	}

	efree(viewName);
	ZVAL_BOOL(*returnZval,0);
	return;
}

//类方法:创建应用对象
PHP_METHOD(CView,factory)
{
	char	*templateName,
			*templateUsed;

	int		templateNameLen = 0;

	zval	*view,
			*cconfigInstanceZval,
			*useQuickTemplate;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&templateName,&templateNameLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Access is required for a view object set use view engine name");
		return;
	}

	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);


	CView_factory(templateUsed,&view TSRMLS_CC);
	RETVAL_ZVAL(view,1,1);
}

//注册函数
void registerSmartyFunction(zval *viewObject,char *fun,char *rClassName,char *rFunctionName TSRMLS_DC)
{
	zval *registerParamList[2],
			 registerParam0,
			 registerParam1,
			 registerAction,
			 registerReturn,
			 *thisSaveVal;
	HashTable *classFunction;
	registerParamList[0] = &registerParam0;
	registerParamList[1] = &registerParam1;


	MAKE_STD_ZVAL(registerParamList[0]);
	MAKE_STD_ZVAL(registerParamList[1]);

	INIT_ZVAL(registerAction);
	ZVAL_STRING(&registerAction,"register_function",0);

	ZVAL_STRING(registerParamList[0],fun,1);

	array_init(registerParamList[1]);
	add_next_index_string(registerParamList[1],rClassName,1);
	add_next_index_string(registerParamList[1],rFunctionName,1);

	call_user_function(NULL, &viewObject, &registerAction, &registerReturn, 2, registerParamList TSRMLS_CC);

	zval_dtor(&registerReturn);

	zval_ptr_dtor(&registerParamList[0]);
	zval_ptr_dtor(&registerParamList[1]);
}

//判断模板存在
int templateExists(zval *viewObject,char *rClassName TSRMLS_DC)
{
	zval *registerParamList[1],
			 registerParam0,
			 registerAction,
			 registerReturn,
			 *className,
			 *functionName,
			 *thisSaveVal,
			 *resultExist;

	registerParamList[0] = &registerParam0;
	MAKE_STD_ZVAL(registerParamList[0]);

	INIT_ZVAL(registerAction);

	ZVAL_STRING(&registerAction,"template_exists",0);
	ZVAL_STRING(registerParamList[0],rClassName,1);

	call_user_function(NULL, &viewObject, &registerAction, &registerReturn, 1, registerParamList TSRMLS_CC);
	zval_ptr_dtor(&registerParamList[0]);
	zval_dtor(&registerReturn);

	return Z_LVAL(registerReturn);
}

//清楚缓存
void clearSmartyCache(zval *viewObject,char *rClassName,char *rFunctionName TSRMLS_DC)
{
	zval *registerParamList[2],
			 registerParam0,
			 registerParam1,
			 registerAction,
			 registerReturn,
			 *thisSaveVal;

	HashTable *classFunction;
	registerParamList[0] = &registerParam0;
	registerParamList[1] = &registerParam1;


	MAKE_STD_ZVAL(registerParamList[0]);
	MAKE_STD_ZVAL(registerParamList[1]);

	INIT_ZVAL(registerAction);
	ZVAL_STRING(&registerAction,"clear_cache",0);


	ZVAL_STRING(registerParamList[0],rClassName,1);
	ZVAL_STRING(registerParamList[1],rFunctionName,1);

	call_user_function(NULL, &viewObject, &registerAction, &registerReturn, 2, registerParamList TSRMLS_CC);
	zval_ptr_dtor(&registerParamList[0]);
	zval_ptr_dtor(&registerParamList[1]);
	zval_dtor(&registerReturn);
}

//注册块
void registerSmartyBlock(zval *viewObject,char *fun,char *rClassName,char *rFunctionName TSRMLS_DC)
{
	zval *registerParamList[2],
			 registerParam0,
			 registerParam1,
			 registerAction,
			 registerReturn,
			 *thisSaveVal;
	HashTable *classFunction;
	registerParamList[0] = &registerParam0;
	registerParamList[1] = &registerParam1;


	MAKE_STD_ZVAL(registerParamList[0]);
	MAKE_STD_ZVAL(registerParamList[1]);

	INIT_ZVAL(registerAction);
	ZVAL_STRING(&registerAction,"register_block",0);

	ZVAL_STRING(registerParamList[0],fun,1);

	array_init(registerParamList[1]);
	add_next_index_string(registerParamList[1],rClassName,1);
	add_next_index_string(registerParamList[1],rFunctionName,1);

	call_user_function(NULL, &viewObject, &registerAction, &registerReturn, 2, registerParamList TSRMLS_CC);

	zval_ptr_dtor(&registerParamList[0]);
	zval_ptr_dtor(&registerParamList[1]);
	zval_dtor(&registerReturn);
}


void smarty_display(zval *viewObject,char *name,char *num TSRMLS_DC)
{
	zval	callFunction,
			callReturn,
			*paramsList[2],
			param1,
			param2;

	paramsList[0] = &param1;
	paramsList[1] = &param2;

	INIT_ZVAL(callFunction);
	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);

	ZVAL_STRING(&callFunction,"display",0);
	ZVAL_STRING(paramsList[0],name,1);
	ZVAL_STRING(paramsList[1],num,1);

	call_user_function(NULL, &viewObject, &callFunction, &callReturn, 2, paramsList TSRMLS_CC);

	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	zval_dtor(&callReturn);
}

void smarty_fetch(zval *viewObject,char *name,zval **returnZval TSRMLS_DC)
{
	zval	callFunction,
			callReturn,
			*paramsList[1],
			param1,
			*returnData,
			*cpViewObject;

	int callStatus = 0;


	paramsList[0] = &param1;
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"fetch",0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],name,1);

	if(SUCCESS !=call_user_function(NULL, &viewObject, &callFunction, &callReturn, 1, paramsList TSRMLS_CC)){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_NULL(*returnZval);
		return;
	}

	returnData = &callReturn;
	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,returnData,1,0);
	zval_ptr_dtor(&paramsList[0]);
	zval_dtor(&callReturn);
}

//构造函数获取视图
PHP_METHOD(CView,__construct)
{
	char	*templateName,
			*corePath;

	int		templateNameLen,
			cacheTime = 100,
			allowPhp = 1,
			cached = 0,
			useQuickTemplate = 0;

	zval	*cconfigInstanceZval,
			*siteconfigInstanceZval,
			*thisConf,
			*viewObject,
			*saveThis,
			*controller,
			*action,
			*requsetUri,
			*module,
			*conf_coreFile,
			*conf_smarty,
			**conf_cacheDir,
			**conf_compileDir,
			**conf_templateDir,
			**conf_leftTag,
			**conf_rightTag,
			**conf_allowPhp,
			**conf_isCache,
			**conf_cachetime;

	zend_class_entry	**viewEntry,
						*viewEntryP;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&templateName,&templateNameLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Access is required for a view object set use view engine name");
		return;
	}


	//使用C版内置引擎
	if(strcmp(templateName,"ctemplate") == 0){
		return;
	}


	if(strcmp(templateName,"smarty") != 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] CQuickFramework by C Extenstion v1.0 Does not support the view technology");
		return;
	}

	controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
	action = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);
	module = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
	requsetUri = zend_read_static_property(CRouteCe,ZEND_STRL("requsetUri"),0 TSRMLS_CC);


	//读取配置文件
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//核心文件
	CConfig_load("TEMPLATE.smarty.TEMPLATE_PATH",cconfigInstanceZval,&conf_coreFile TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(conf_coreFile) || Z_STRLEN_P(conf_coreFile) <= 0 ){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->TEMPLATE_PATH] Error, do not specify the Smarty engine core file path");
		return;
	}

	//检查主文件
	if(FAILURE == fileExist(Z_STRVAL_P(conf_coreFile))){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Configuration items [TEMPLATE->smarty->TEMPLATE_PATH] Specify the Smarty engine core file address is invalid");
		return;
	}

	//引入文件
	if(SUCCESS != CLoader_loadFile(Z_STRVAL_P(conf_coreFile))){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] CQuickFramework can not init Smarty engine : %s",Z_STRVAL_P(conf_coreFile));
		return;
	}

	//其余配置
	CConfig_load("TEMPLATE.smarty.CONF_INFO",cconfigInstanceZval,&conf_smarty TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(conf_smarty)){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO] type Error, not give an array");
		return;
	}

	//缓存目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"cache_dir",strlen("cache_dir")+1,(void**)&conf_cacheDir) && IS_STRING == Z_TYPE_PP(conf_cacheDir)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.cache_dir] Error, is not string");
		return;
	}

	//编译目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"compile_dir",strlen("compile_dir")+1,(void**)&conf_compileDir) && IS_STRING == Z_TYPE_PP(conf_compileDir)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.compile_dir] Error, is not string");
		return;
	}

	//模板目录
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"template_dir",strlen("template_dir")+1,(void**)&conf_templateDir) && IS_STRING == Z_TYPE_PP(conf_templateDir)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.template_dir] Error, is not string");
		return;
	}

	//左标
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"left_delimiter",strlen("left_delimiter")+1,(void**)&conf_leftTag) && IS_STRING == Z_TYPE_PP(conf_leftTag)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.left_delimiter] Error, is not string");
		return;
	}

	//右标
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"right_delimiter",strlen("right_delimiter")+1,(void**)&conf_rightTag) && IS_STRING == Z_TYPE_PP(conf_rightTag)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.right_delimiter] Error, is not string");
		return;
	}

	//允许php标签
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"allow_php_tag",strlen("allow_php_tag")+1,(void**)&conf_allowPhp) && IS_BOOL == Z_TYPE_PP(conf_allowPhp)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.allow_php_tag] Error, is not boolean");
		return;
	}

	//是否启用缓存
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"caching",strlen("caching")+1,(void**)&conf_isCache) && IS_BOOL == Z_TYPE_PP(conf_isCache)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.caching] Error, is not boolean");
		return;
	}

	//缓存时间
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(conf_smarty),"cache_lifetime",strlen("cache_lifetime")+1,(void**)&conf_cachetime) && IS_LONG == Z_TYPE_PP(conf_cachetime)){
	}else{
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Read the configuration items [TEMPLATE->smarty->CONF_INFO.cache_lifetime] Error, is not long");
		return;
	}

	//实例化视图
	if(zend_hash_find(EG(class_table),templateName,strlen(templateName)+1,(void**)&viewEntry) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ViewException] Configuration items [TEMPLATE->smarty->TEMPLATE_PATH] Specify the Smarty engine core file address is invalid");
		return;
	}

	//取得引擎对象
	viewEntryP = *viewEntry;
	MAKE_STD_ZVAL(viewObject);
	object_init_ex(viewObject,viewEntryP);


	//尝试调用构造器
	if (viewEntryP->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, viewEntryP->constructor->common.function_name, 0);
		call_user_function(NULL, &viewObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//重置smarty配置
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"template_dir",strlen("template_dir"),Z_STRVAL_PP(conf_templateDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"compile_dir",strlen("compile_dir"),Z_STRVAL_PP(conf_compileDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"cache_dir",strlen("cache_dir"),Z_STRVAL_PP(conf_cacheDir) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"left_delimiter",strlen("left_delimiter"),Z_STRVAL_PP(conf_leftTag) TSRMLS_CC);
	zend_update_property_string(Z_OBJCE_P(viewObject),viewObject,"right_delimiter",strlen("right_delimiter"),Z_STRVAL_PP(conf_rightTag) TSRMLS_CC);
	zend_update_property_bool(Z_OBJCE_P(viewObject),viewObject,"allow_php_tag",strlen("allow_php_tag"),Z_BVAL_PP(conf_allowPhp) TSRMLS_CC);
	zend_update_property_bool(Z_OBJCE_P(viewObject),viewObject,"caching",strlen("caching"),Z_BVAL_PP(conf_isCache) TSRMLS_CC);
	zend_update_property_long(Z_OBJCE_P(viewObject),viewObject,"cache_lifetime",strlen("cache_lifetime"),Z_LVAL_PP(conf_cachetime) TSRMLS_CC);


	//检查编译目录
	if(FAILURE == fileExist(Z_STRVAL_PP(conf_compileDir))){
		//尝试创建文件夹
		php_mkdir(Z_STRVAL_PP(conf_compileDir));

		//创建失败
		if(FAILURE == fileExist(Z_STRVAL_PP(conf_compileDir))){
			char *thisMothTime;
			php_date("Y-m-d h:i:s",&thisMothTime);
			php_printf("#LogTime:%s%s[ServerError] CQuickFramework unable to create the __runtime directory, please make sure the server permissions%s-File:%s%s-Line:%s",thisMothTime,"<br>","<br>","CViews.c","<br>","573");
			efree(thisMothTime);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
		}
	}

	//检查缓存目录
	if(FAILURE == fileExist(Z_STRVAL_PP(conf_cacheDir))){
		//尝试创建文件夹
		php_mkdir(Z_STRVAL_PP(conf_cacheDir));

		//创建失败
		if(FAILURE == fileExist(Z_STRVAL_PP(conf_cacheDir))){
			php_printf("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />[ServerError] CQuickFramework unable to create the cache directory, please make sure the server permissions");
		}
	}


	//向smarty注册函数
	registerSmartyFunction(viewObject,"url","CRequest","createUrl" TSRMLS_CC);


	//向smarty注册block
	registerSmartyFunction(viewObject,"PageInfo","CSmarty","showPageData" TSRMLS_CC);
	registerSmartyFunction(viewObject,"substr","CSmarty","cn_substr" TSRMLS_CC);
	registerSmartyFunction(viewObject,"sayTime","CSmarty","sayTime" TSRMLS_CC);
	registerSmartyFunction(viewObject,"showHTML","CSmarty","showHTML" TSRMLS_CC);

	
	//设置smarty默认值
	smarty_assign(viewObject,"thisUrl",Z_STRVAL_P(requsetUri) TSRMLS_CC);
	smarty_assign(viewObject,"controller",Z_STRVAL_P(controller) TSRMLS_CC);

	//获取action
	MODULE_BEGIN
		char *actionNoPre;
		zval *actionPreZval;

		CConfig_load("ACTION_PREFIX",cconfigInstanceZval,&actionPreZval TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(actionPreZval) ){
			str_replace(Z_STRVAL_P(actionPreZval),"",Z_STRVAL_P(action),&actionNoPre);
			smarty_assign(viewObject,"action",(actionNoPre) TSRMLS_CC);
			efree(actionNoPre);
		}else{
			smarty_assign(viewObject,"action",Z_STRVAL_P(action) TSRMLS_CC);
		}
		zval_ptr_dtor(&actionPreZval);
	MODULE_END

	smarty_assign(viewObject,"actionPre","Action_" TSRMLS_CC);
	smarty_assign(viewObject,"module",Z_STRVAL_P(module) TSRMLS_CC);

	//获取配置对象
	CConfig_getInstance("site",&siteconfigInstanceZval TSRMLS_CC);
	MODULE_BEGIN
		zval	*staticUrlZval;
		char	*base64Url;

		CConfig_load("staticUrl",siteconfigInstanceZval,&staticUrlZval TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(staticUrlZval)){
			smarty_assign(viewObject,"staticUrl",(Z_STRVAL_P(staticUrlZval)) TSRMLS_CC);
		}
		zval_ptr_dtor(&staticUrlZval);
	MODULE_END

	//设置ip
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"getIp", 0);
		//获取类的静态方法
		call_user_function(&CRequestCe->function_table, NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;
		smarty_assign_ex(viewObject,"ip",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END

	//设置时间time
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"time", 0);
		//获取类的静态方法
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;	
		smarty_assign_ex(viewObject,"time",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END

	//设置sessionID
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*returnTemp;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"session_id", 0);
		//获取类的静态方法
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 0, NULL TSRMLS_CC);
		returnTemp = &returnObject;	
		smarty_assign_ex(viewObject,"sessionID",returnTemp TSRMLS_CC);
		zval_dtor(&returnObject);
	MODULE_END


	//将试图对象保存至静态变量中
	zend_update_property(CViewCe,getThis(),ZEND_STRL("_viewObject"),viewObject TSRMLS_CC);

	zval_ptr_dtor(&conf_coreFile);
	zval_ptr_dtor(&conf_smarty);
	zval_ptr_dtor(&viewObject);
	zval_ptr_dtor(&siteconfigInstanceZval);
	zval_ptr_dtor(&cconfigInstanceZval);
}

//获取视图对象
PHP_METHOD(CView,getViews)
{
	zval *viewObject;
	viewObject = zend_read_property(CViewCe,getThis(),ZEND_STRL("_viewObject"),0 TSRMLS_CC);
	RETVAL_ZVAL(viewObject,1,0);
}