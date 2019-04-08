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
#include "php_CRequest.h"
#include "php_CRouteParse.h"
#include "php_CRoute.h"
#include "php_CException.h"
#include "php_CWebApp.h"


//zend类方法
zend_function_entry CRequset_functions[] = {
	PHP_ME(CRequest,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,setController,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRequest,setAction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRequest,setModule,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRequest,getController,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getAction,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getModule,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CRequest,run,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRequest,_checkActionPreFix,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRequest,routeDoing,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRequest,createController,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRequest,createAction,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRequest,execAction,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CRequest,isSuccess,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CRequest,closeRouter,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,openRouter,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getUseRouterStatus,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,Args,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getUrl,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getUri,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getIp,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getPreUrl,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getAgent,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getHost,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getStartTime,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getRegisterEventTime,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getErrorMessage,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getPath,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,createUrl,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,disablePOST,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,disableGET,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,isWap,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,end,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,removeXSS,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)

	PHP_ME(CRequest,getAllMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getFreeMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getVirtualMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getFreeVirtualMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getMaxUseMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getApacheMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getCurrentProcessMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getMysqlMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getMysqlNTMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getMemcacheMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getRedisMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getFreeDisk,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getAllDisk,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getProcessList,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CRequest,getCPULoad,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CRequset)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CRequest",CRequset_functions);
	CRequestCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义静态变量
	zend_declare_property_null(CRequestCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("controller"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("action"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("module"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("errorMessage"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("_useRouter"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CRequestCe, ZEND_STRL("controllerObj"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

//获取CRequest单例对象
int CRequest_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CRequestCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zval			*object,
						*saveObject;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CRequestCe);

		//执行构造器
		if (CRequestCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CRequestCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CRequestCe,ZEND_STRL("instance"),object TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

//类方法:创建应用对象
PHP_METHOD(CRequest,getInstance)
{
	zval *instanceZval;

	CRequest_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,0);
	zval_ptr_dtor(&instanceZval);

}

PHP_METHOD(CRequest,getController)
{
	zval *controller;

	controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
	RETVAL_STRING(Z_STRVAL_P(controller),1);
}

PHP_METHOD(CRequest,setController)
{
	char *val;
	int valLen;
	zval *saveZval;

	//设置控制器
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//更新变量
	MAKE_STD_ZVAL(saveZval);
	ZVAL_STRING(saveZval,val,1);
	zend_update_property(CRequestCe,getThis(),ZEND_STRL("controller"),saveZval TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),val TSRMLS_CC);
	zval_ptr_dtor(&saveZval);
}

PHP_METHOD(CRequest,getAction)
{
	zval *action,
		 *cconfigInstanceZval,
		 *actionPrefix;


	action = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);

	//读取控制器前缀
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("ACTION_PREFIX",cconfigInstanceZval,&actionPrefix TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(actionPrefix)){

		char *replacePrefixAction;
		str_replace(Z_STRVAL_P(actionPrefix),"",Z_STRVAL_P(action),&replacePrefixAction);
		RETVAL_STRING(replacePrefixAction,1);
		efree(replacePrefixAction);

	}else{
		RETVAL_STRING(Z_STRVAL_P(action),1);
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&actionPrefix);
}

PHP_METHOD(CRequest,setAction)
{
	char *val;
	int valLen;
	zval *saveZval;

	//设置控制器
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//更新变量
	MAKE_STD_ZVAL(saveZval);
	ZVAL_STRING(saveZval,val,1);
	zend_update_property(CRequestCe,getThis(),ZEND_STRL("action"),saveZval TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),val TSRMLS_CC);
	zval_ptr_dtor(&saveZval);
}

PHP_METHOD(CRequest,setModule)
{
	char *val;
	int valLen;
	zval *saveZval;

	//设置控制器
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//更新变量
	MAKE_STD_ZVAL(saveZval);
	ZVAL_STRING(saveZval,val,1);
	zend_update_property(CRequestCe,getThis(),ZEND_STRL("module"),saveZval TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisModule"),val TSRMLS_CC);
	zval_ptr_dtor(&saveZval);
}

PHP_METHOD(CRequest,getModule)
{
	zval *module;

	module = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
	RETVAL_STRING(Z_STRVAL_P(module),1);
}

PHP_METHOD(CRequest,__construct)
{
	zval	*cconfigInstanceZval,
			*thisConf,
			*saveZval;

	char	*defaultController,
			*defaultAction,
			*defaultModule;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//控制器
	CConfig_load("DEFAULT_CONTROLLER",cconfigInstanceZval,&thisConf TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(thisConf)){
		defaultController = estrdup(Z_STRVAL_P(thisConf));
	}else{
		defaultController = estrdup("base");
	}

	zend_update_property_string(CRequestCe,getThis(),ZEND_STRL("controller"),defaultController TSRMLS_CC);
	zval_ptr_dtor(&thisConf);

	//方法
	CConfig_load("DEFAULT_ACTION",cconfigInstanceZval,&thisConf TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(thisConf)){
		defaultAction = estrdup(Z_STRVAL_P(thisConf));
	}else{
		defaultAction = estrdup("index");
	}
	zend_update_property_string(CRequestCe,getThis(),ZEND_STRL("action"),defaultAction TSRMLS_CC);
	zval_ptr_dtor(&thisConf);

	//模块
	CConfig_load("DEFAULT_MODLUE",cconfigInstanceZval,&thisConf TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(thisConf)){
		defaultModule = estrdup(Z_STRVAL_P(thisConf));
	}else{
		defaultModule = estrdup("www");
	}

	zend_update_property_string(CRequestCe,getThis(),ZEND_STRL("module"),defaultModule TSRMLS_CC);
	zval_ptr_dtor(&thisConf);

	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),defaultController TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),defaultAction TSRMLS_CC);
	zend_update_static_property_string(CRouteCe,ZEND_STRL("thisModule"),defaultModule TSRMLS_CC);

	efree(defaultController);
	efree(defaultAction);
	efree(defaultModule);

	zval_ptr_dtor(&cconfigInstanceZval);
}

int checkChildClass(zend_class_entry *controllerEntryP)
{
	if(!controllerEntryP->parent){
		return FAILURE;
	}else if(controllerEntryP->parent && strcmp("CController",controllerEntryP->parent->name) != 0){
		return checkChildClass(controllerEntryP->parent);
	}else if(controllerEntryP->parent && strcmp("CController",controllerEntryP->parent->name) == 0){
		return SUCCESS;
	}
}


//创建控制器
void CRequest_createController(char *path,char *name,zval **returnZval TSRMLS_DC)
{
	zval	*controllerObject,
			*saveController;

	zend_class_entry	**controllerCePP,
						*controllerCe;

	char *tureClassName;

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	tureClassName = estrdup(name);
	php_strtolower(tureClassName,strlen(tureClassName)+1);


	//编译该文件
	if(zend_hash_find(EG(class_table),tureClassName,strlen(tureClassName)+1,(void**)&controllerCePP ) == FAILURE){
		if(CLoader_loadFile(path) != SUCCESS){
			char *errMessage;
			strcat2(&errMessage,"[CClassNotFoundException] Cannot load controller files[",name,"]",NULL);
			zend_throw_exception(CClassNotFoundExceptionCe, errMessage, 100001 TSRMLS_CC);
			efree(errMessage);
			return;
		}
	}

	//查询该类结构体
	if(zend_hash_find(EG(class_table),tureClassName,strlen(tureClassName)+1,(void**)&controllerCePP ) == FAILURE){
		char *errMessage;
		strcat2(&errMessage,"[RouteException] Can't find the controller files[",name,"],Confirm the identity of the controller file name and the name of the class",NULL);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,"%s",errMessage);
		return;
	}
	efree(tureClassName);

	//取地址
	controllerCe = *controllerCePP;
	
	//是否为抽象类
	if(controllerCe->ce_flags & ZEND_ACC_EXPLICIT_ABSTRACT_CLASS){
		char *errMessage;
		strcat2(&errMessage,"[RouteException] The controller",name,"] Cannot be designed into an abstract class",NULL);
		zend_throw_exception(CRouteExceptionCe, errMessage, 100002 TSRMLS_CC);
		efree(errMessage);
		return;
	}

	//是否为接口
	if(controllerCe->ce_flags & ZEND_ACC_INTERFACE){
		char *errMessage;
		strcat2(&errMessage,"[RouteException] The controller",name,"] Can't be designed interface",NULL);
		zend_throw_exception(CRouteExceptionCe, errMessage, 100002 TSRMLS_CC);
		efree(errMessage);
		return;
	}

	//检查是否继承CController
	if(FAILURE ==  checkChildClass(controllerCe)){
		char *errMessage;
		strcat2(&errMessage,"[RouteException] The controller[",name,"] Should inherit CController main controller",NULL);
		zend_throw_exception(CRouteExceptionCe, errMessage, 100002 TSRMLS_CC);
		efree(errMessage);
		return;
	}

	//实例化
	MAKE_STD_ZVAL(controllerObject);
    object_init_ex(controllerObject,controllerCe);

	//调用其构造函数
	if (controllerCe->constructor) {
		zval constructReturn;
		zval constructVal,
			 *diInstanceZval,
			 *paramsList[1],
			 param1;
		INIT_ZVAL(constructVal);
		CDiContainer_getInstance(&diInstanceZval TSRMLS_CC);
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],diInstanceZval,1,1);
		ZVAL_STRING(&constructVal, controllerCe->constructor->common.function_name, 0);
		call_user_function(NULL, &controllerObject, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&constructReturn);
	}

	//将控制器对象存入静态变量
	zend_update_static_property(CRouteCe,ZEND_STRL("controllerObject"),controllerObject TSRMLS_CC);

	//返回控制器对象
	ZVAL_ZVAL(*returnZval,controllerObject,1,1);
	return;
}

//创建控制器
PHP_METHOD(CRequest,createController)
{
	char	*path,
			*name;

	int		pathLen,
			nameLen;

	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&path,&pathLen,&name,&nameLen) == FAILURE){
		RETVAL_NULL();
		return;
	}

	//调用创建控制器函数
	CRequest_createController(path,name,&returnZval TSRMLS_CC);

	//更新类属性
	zend_update_property(CRequestCe,getThis(),ZEND_STRL("controllerObj")+1,returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,0);
	zval_ptr_dtor(&returnZval);
}


//创建路由对象
void CRequest_checkActionPreFix(zval *route,zval **returnZval TSRMLS_DC)
{
	zval	*cconfigInstanceZval,
			*actionPreZval,
			*routeInstance,
			*thisController,
			*thisAction,
			*thisModule,
			*setStatus;

	char	*controller,
			*action,
			*module;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//方法前缀
	CConfig_load("ACTION_PREFIX",cconfigInstanceZval,&actionPreZval TSRMLS_CC);

	zval_ptr_dtor(&cconfigInstanceZval);
	
	thisController = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
	thisAction = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);
	thisModule = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
	
	//将值从结构体中取出
	controller = estrdup(Z_STRVAL_P(thisController));
	action = estrdup(Z_STRVAL_P(thisAction));
	module = estrdup(Z_STRVAL_P(thisModule));

	//检查方法名前缀
	if(IS_STRING == Z_TYPE_P(actionPreZval) && strlen(Z_STRVAL_P(actionPreZval)) >0 ){
		efree(action);
		strcat2(&action,Z_STRVAL_P(actionPreZval),Z_STRVAL_P(thisAction),NULL);
	}

	//获取路由单例对象
	CRoute_getInstance(&routeInstance TSRMLS_CC);

	//设置路由结果
	CRoute_setController((controller),routeInstance,&setStatus TSRMLS_CC);
	CRoute_setAction((action),routeInstance,&setStatus TSRMLS_CC);
	CRoute_setModule((module),routeInstance,&setStatus TSRMLS_CC);

	zval_ptr_dtor(&actionPreZval);

	efree(controller);
	efree(action);
	efree(module);

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,routeInstance,1,1);
	return;
}

//调用请求方法
void CRequest_createAction(zval *controllerObject,char *requsetAction TSRMLS_DC)
{
	zval	fnReturn,
			fnName;

	//执行方法
	INIT_ZVAL(fnName);
	ZVAL_STRING(&fnName,requsetAction,0);
	call_user_function(NULL, &controllerObject, &fnName, &fnReturn, 0, NULL TSRMLS_CC);
	zval_dtor(&fnReturn);
}

//调用请求方法
PHP_METHOD(CRequest,createAction)
{
	zval	*controllerObject;

	char	*actionName;
	int		actionNameLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&controllerObject,&actionName,&actionNameLen) == FAILURE){
		RETVAL_NULL();
		return;
	}

	CRequest_createAction(controllerObject,actionName TSRMLS_CC);

}

//创建路由对象
PHP_METHOD(CRequest,_checkActionPreFix)
{
	zval	*returnZval,
			*routeZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&routeZval) == FAILURE){
		RETVAL_NULL();
		return;
	}

	CRequest_checkActionPreFix(routeZval,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

//执行路由
void CRequest_routeDoing(zval *routeObject TSRMLS_DC)
{
	zval	*returnZval,
			*cconfigInstanceZval,
			*defaultModelZval,
			*useModule,
			*codePath,
			*appPath,
			*module,
			*controller;

	char	*requsetModule,
			*requestController;

	//配置单例
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

	//检查默认模块
	CConfig_load("DEFAULT_MODLUE",cconfigInstanceZval,&defaultModelZval TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(defaultModelZval)){
		ZVAL_STRING(defaultModelZval,"www",1);
	}

	//CODEPAHT
	codePath = zend_read_static_property(CWebAppCe,ZEND_STRL("code_path"),0 TSRMLS_CC);
	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);
	module = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);
	controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);

	//当前模块
	requsetModule = estrdup(Z_STRVAL_P(module));
	requestController = estrdup(Z_STRVAL_P(controller));


	//实例化控制器
	if(NULL == requestController){
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&defaultModelZval);
		php_error_docref(NULL TSRMLS_CC, E_ERROR,"[RouteException]The request of the path, the file does not exist or access");
		return;
	}

	//确定是否使用模块
	CConfig_load("USE_MODULE",cconfigInstanceZval,&useModule TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(useModule)){
		ZVAL_BOOL(useModule,0);
	}

	//主模块
	if( (IS_BOOL == Z_TYPE_P(useModule) && Z_LVAL_P(useModule) == 0) || strcmp(Z_STRVAL_P(defaultModelZval),requsetModule) == 0 ){

		char	*controllerPath,
				*controllLessName;

		zval *controllerMapConfItem;

		zend_class_entry	**controllerCePP;

		int hasExistController = FAILURE;

		//控制器文件
		strcat2(&controllerPath,Z_STRVAL_P(codePath),"/controllers/",requestController,".php",NULL);

		//check class exists
		controllLessName = estrdup(requestController);
		php_strtolower(controllLessName,strlen(controllLessName)+1);
		hasExistController = zend_hash_find(EG(class_table),controllLessName,strlen(controllLessName)+1,(void**)&controllerCePP);
		efree(controllLessName);


		if(SUCCESS == fileExist(controllerPath) || SUCCESS == hasExistController){
			//创建控制器对象
			zval *getReturn;
			CRequest_createController(controllerPath,(requestController),&getReturn TSRMLS_CC);
			zval_ptr_dtor(&getReturn);
			efree(controllerPath);
		}else{
			char *errMessage;
			strcat2(&errMessage,"[RouteException] Has not been defined controller:",requestController,NULL);
			efree(controllerPath);
			efree(requsetModule);
			efree(requestController);
			zval_ptr_dtor(&useModule);
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&defaultModelZval);
			CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);
			zend_throw_exception(CRouteExceptionCe, errMessage, 100000 TSRMLS_CC);	
			efree(errMessage);
			return;
		}
	}else{

		//独立模块
		char	*moduleName,
				*modulePath;

		moduleName = estrdup(Z_STRVAL_P(module));
		strcat2(&modulePath,Z_STRVAL_P(appPath),"/modules/",moduleName,NULL);

		//判断是否存在该模块目录
		if(SUCCESS == fileExist(modulePath)){
		
			//检查控制器文件
			char *moduleControllerPath;
			strcat2(&moduleControllerPath,Z_STRVAL_P(appPath),"/modules/",moduleName,"/controllers/",requestController,".php",NULL);
			if(SUCCESS == fileExist(moduleControllerPath)){
				//创建控制器对象
				zval *getReturn;
				CRequest_createController(moduleControllerPath,requestController,&getReturn TSRMLS_CC);
				zval_ptr_dtor(&getReturn);
				efree(moduleName);
				efree(modulePath);
				efree(moduleControllerPath);
			}else{
				char *errMessage;
				strcat2(&errMessage,"[RouteException] Module[",moduleName,"] Has not been defined controller:",requestController,NULL);
				efree(moduleName);
				efree(modulePath);
				efree(requsetModule);
				efree(requestController);
				efree(moduleControllerPath);
				zval_ptr_dtor(&cconfigInstanceZval);
				CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);	
				zend_throw_exception(CRouteExceptionCe, errMessage, 100000 TSRMLS_CC);
				efree(errMessage);
				return;
			}
		}else{
			//发出错误
			char *errMessage;
			strcat2(&errMessage,"[RouteException]The module requested does not exist:",moduleName,NULL);
			efree(moduleName);
			efree(requsetModule);
			efree(requestController);
			zval_ptr_dtor(&cconfigInstanceZval);
			php_error_docref(NULL TSRMLS_CC, E_ERROR,"%s",errMessage);
			return;
		}
	}

	zval_ptr_dtor(&useModule);
	zval_ptr_dtor(&defaultModelZval);
	efree(requsetModule);
	efree(requestController);
	zval_ptr_dtor(&cconfigInstanceZval);
}

//执行路由
PHP_METHOD(CRequest,routeDoing)
{
	zval	*routeObject;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&routeObject) == FAILURE){
		RETVAL_NULL();
		return;
	}
	CRequest_routeDoing(routeObject TSRMLS_CC);
	RETVAL_NULL();

}

//执行方法
void CRequest_execAction(zval *routeObject,zval *object TSRMLS_DC)
{
	zval	*controllerObject,
			*trueAction,
			*actionPreZval,
			*thisController,
			*thisAction;

	char	*requsetController,
			*requsetAction;


	zend_class_entry	**controllerCePP,
						*controllerCe;

	zend_function		*requsetActionEntry;

	thisController = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
	thisAction = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);
	controllerObject = zend_read_static_property(CRouteCe,ZEND_STRL("controllerObject"),0 TSRMLS_CC);


	//请求参数
	requsetController = estrdup(Z_STRVAL_P(thisController));
	requsetAction = estrdup(Z_STRVAL_P(thisAction));

	//控制器对象丢失
	if(IS_OBJECT != Z_TYPE_P(controllerObject)){
		efree(requsetController);
		efree(requsetAction);
		zend_throw_exception(CExceptionCe, "[CMyFrameFatal]CMyFrame fatal error:CMyRoute structure reference object is missing", 100000 TSRMLS_CC);	
		return;
	}

	//请求方式不存在
	if(strlen(requsetAction) <= 0){
		efree(requsetController);
		efree(requsetAction);
		zend_throw_exception(CRouteExceptionCe, "[RouteException]The system can't find the request method", 100002 TSRMLS_CC);	
		return;
	}

	//获取类结构体
	php_strtolower(requsetController,strlen(requsetController)+1);
	zend_hash_find(EG(class_table),requsetController,strlen(requsetController)+1,(void**)&controllerCePP);
	controllerCe = *controllerCePP;

	//检查存在__before魔术函数
	if(zend_hash_exists(&controllerCe->function_table,"__before",strlen("__before")+1)){

		zend_function    *beforeEntry;
		//判断函数是否可调
		zend_hash_find(&controllerCe->function_table,"__before",strlen("__before")+1,(void **)&beforeEntry);
		if(beforeEntry->common.fn_flags & ZEND_ACC_PUBLIC){
			//调用before方法
			zval fnReturn;
			zval fnName;
			INIT_ZVAL(fnName);
			ZVAL_STRING(&fnName,"__before", 0);
			call_user_function(NULL, &controllerObject, &fnName, &fnReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&fnReturn);
		}else{
			php_error_docref(NULL TSRMLS_CC, E_WARNING ,"[RuntimeWarnning]The controller[%s] Define a magic methods __before but access is not enough to provide CMyFrame calls",requsetController);
		}
	}

	//检查类存在请求方法
	php_strtolower(requsetAction,strlen(requsetAction)+1);
	if(!zend_hash_exists(&controllerCe->function_table,requsetAction,strlen(requsetAction)+1)){
	
		//若不存在请求方法时 尝试检查__error魔术函数
		if(zend_hash_exists(&controllerCe->function_table,"__error",strlen("__error")+1)){
			zend_function    *errorEntry;
			//判断函数是否可调
			zend_hash_find(&controllerCe->function_table,"__error",strlen("__error")+1,(void **)&errorEntry);
			if(errorEntry->common.fn_flags & ZEND_ACC_PUBLIC){
				//调用error方法
				zval fnReturn;
				zval fnName;
				INIT_ZVAL(fnName);
				ZVAL_STRING(&fnName,"__error", 0);
				call_user_function(NULL, &controllerObject, &fnName, &fnReturn, 0, NULL TSRMLS_CC);
				zval_dtor(&fnReturn);
				efree(requsetController);
				efree(requsetAction);

				return;
			}else{
				php_error_docref(NULL TSRMLS_CC, E_WARNING ,"[RuntimeWarnning]The controller[%s] Define a magic methods __error but access is not enough to provide CMyFrame calls",estrdup(requsetController));
				return;
			}
		}else{
			char *errMessage;
			strcat2(&errMessage,"[RouteException]The requested method does not exist:",requsetAction,NULL);
			CHooks_callHooks("HOOKS_ROUTE_ERROR",NULL,0 TSRMLS_CC);
			zend_throw_exception(CRouteExceptionCe, errMessage, 100002 TSRMLS_CC);
			efree(errMessage);
			efree(requsetController);
			efree(requsetAction);
			return;
		}
	}

	//判断请求方法的访问权限
	zend_hash_find(&controllerCe->function_table,requsetAction,strlen(requsetAction)+1,(void **)&requsetActionEntry);
	if(!requsetActionEntry->common.fn_flags & ZEND_ACC_PUBLIC){
		char *errMessage;
		strcat2(&errMessage,"[RouteException] Action [",requsetAction,"] Don't have access",NULL);
		zend_throw_exception(CRouteExceptionCe, errMessage, 100002 TSRMLS_CC);
		efree(errMessage);
		efree(requsetController);
		efree(requsetAction);
		return;
	}

	//执行请求方法
	CRequest_createAction(controllerObject,requsetAction TSRMLS_CC);

	efree(requsetController);
	efree(requsetAction);
}

//执行方法
PHP_METHOD(CRequest,execAction)
{
	zval	*routeObject;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&routeObject) == FAILURE){
		RETVAL_NULL();
		return;
	}
	CRequest_execAction(routeObject,getThis() TSRMLS_CC);


	RETVAL_NULL();
}

PHP_METHOD(CRequest,isSuccess)
{
	RETVAL_TRUE;
}

PHP_METHOD(CRequest,closeRouter)
{
	//更新关闭路由
	zend_update_static_property_bool(CRequestCe,ZEND_STRL("_useRouter"),0 TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CRequest,openRouter)
{
	//启用路由
	zend_update_static_property_bool(CRequestCe,ZEND_STRL("_useRouter"),1 TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CRequest,getUseRouterStatus)
{
	zval *useRouter;
	useRouter = zend_read_static_property(CRequestCe,ZEND_STRL("_useRouter"),0 TSRMLS_CC);

	RETVAL_BOOL(Z_LVAL_P(useRouter));
}

void CRequest_filterHTML(zval *array,zval **newArray TSRMLS_DC)
{
	int		i,h;
	zval	**thisVal;
	char	*filterString,
			*otherKey;
	ulong	thisIntKey;

	if(IS_ARRAY != Z_TYPE_P(array)){
		return;
	}

	MAKE_STD_ZVAL(*newArray);
	array_init(*newArray);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	h = zend_hash_num_elements(Z_ARRVAL_P(array));
	for(i = 0 ; i < h;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(array),(void**)&thisVal);
		convert_to_string(*thisVal);
		strip_tags(Z_STRVAL_PP(thisVal),&filterString);

		if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_P(array))){
			zend_hash_get_current_key(Z_ARRVAL_P(array), &otherKey, &thisIntKey, 0);
			add_index_string(*newArray,thisIntKey,filterString,1);
		}else if(HASH_KEY_IS_STRING == zend_hash_get_current_key_type(Z_ARRVAL_P(array))){
			zend_hash_get_current_key(Z_ARRVAL_P(array), &otherKey, &thisIntKey, 0);
			add_assoc_string(*newArray,otherKey,filterString,1);
		}
		efree(filterString);
		zend_hash_move_forward(Z_ARRVAL_P(array));
	}
}

void CRequest_string_htmlspecialchars(char *replaceTempString1,char **replaceTempString2 TSRMLS_DC)
{
	char	*tempString1,
			*tempString2;

	zval	*find,
			*replace;

	MAKE_STD_ZVAL(find);
	MAKE_STD_ZVAL(replace);
	array_init(find);
	array_init(replace);
	
	add_next_index_string(find,"&",1);
	add_next_index_string(find,"\"",1);
	add_next_index_string(find,"<",1);
	add_next_index_string(find,">",1);

	add_next_index_string(replace,"&amp;",1);
	add_next_index_string(replace,"&quot;",1);
	add_next_index_string(replace,"&lt;",1);
	add_next_index_string(replace,"&gt;",1);

	str_replaceArray(find,replace,replaceTempString1,&tempString1);

	if(strstr(tempString1,"&amp;#") != NULL){
		preg_replace("/&amp;((#(\\d{3,5}|x[a-fA-F0-9]{4}));)/", "&\\1", tempString1,&tempString2);
		*replaceTempString2 = estrdup(tempString2);
		efree(tempString2);
	}else{
		*replaceTempString2 = estrdup(replaceTempString1);
	}

	zval_ptr_dtor(&replace);
	zval_ptr_dtor(&find);
	efree(tempString1);

}

void CRequest_xssRemove(char *string,char **result TSRMLS_DC){

	zval	*match,
			**match1,
			*findTags,
			**thisVal,
			*tempZval1,
			*searchs,
			*replaces;

	char	*allowtags = "img|a|font|div|table|tbody|caption|tr|td|th|br|p|b|strong|i|u|em|span|ol|ul|li|blockquote",
			*skipkeysString = "/(onabort|onactivate|onafterprint|onafterupdate|onbeforeactivate|onbeforecopy|onbeforecut|onbeforedeactivate|onbeforeeditfocus|onbeforepaste|onbeforeprint|onbeforeunload|onbeforeupdate|onblur|onbounce|oncellchange|onchange|onclick|oncontextmenu|oncontrolselect|oncopy|oncut|ondataavailable|ondatasetchanged|ondatasetcomplete|ondblclick|ondeactivate|ondrag|ondragend|ondragenter|ondragleave|ondragover|ondragstart|ondrop|onerror|onerrorupdate|onfilterchange|onfinish|onfocus|onfocusin|onfocusout|onhelp|onkeydown|onkeypress|onkeyup|onlayoutcomplete|onload|onlosecapture|onmousedown|onmouseenter|onmouseleave|onmousemove|onmouseout|onmouseover|onmouseup|onmousewheel|onmove|onmoveend|onmovestart|onpaste|onpropertychange|onreadystatechange|onreset|onresize|onresizeend|onresizestart|onrowenter|onrowexit|onrowsdelete|onrowsinserted|onscroll|onselect|onselectionchange|onselectstart|onstart|onstop|onsubmit|onunload|javascript|script|eval|behaviour|expression|style|class)/i",
			*replaceTempString1,
			*replaceTempString2,
			*replaceTempString3,
			*replaceTempString4,
			*replaceTempString5,
			*replaceTempString6,
			*replaceTempString9,
			*replaceTempString10,
			*allowTags = "/^[\\/|\\s]?(img|a|font|div|table|tbody|caption|tr|td|th|br|p|b|strong|i|u|em|span|ol|ul|li|blockquote)(\\s+|$)/is",
			*searchTagsTemp;

	int		i,h;

	if(!preg_match_all("/\\<([^\\<]+)\\>/is", string, &match)){
		*result = estrdup(string);
		zval_ptr_dtor(&match);
		return;
	}

	if(IS_ARRAY != Z_TYPE_P(match)){
		*result = estrdup(string);
		zval_ptr_dtor(&match);
		return;
	}

	//read1
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(match),1,(void**)&match1) && IS_ARRAY == Z_TYPE_PP(match1)){
	}else{
		*result = estrdup(string);
		zval_ptr_dtor(&match);
		return;
	}

	MAKE_STD_ZVAL(searchs);
	MAKE_STD_ZVAL(replaces);
	array_init(searchs);
	array_init(replaces);

	add_next_index_string(searchs,"<",1);
	add_next_index_string(searchs,">",1);
	add_next_index_string(replaces,"&lt;",1);
	add_next_index_string(replaces,"&gt;",1);
		
	//qu chong
	array_unique(*match1,&findTags);

	//foreach
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(findTags));
	h = zend_hash_num_elements(Z_ARRVAL_P(findTags));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(findTags),(void**)&thisVal);

		spprintf(&searchTagsTemp,0,"%s%s%s","&lt;",Z_STRVAL_PP(thisVal),"&gt;");
		add_next_index_string(searchs,searchTagsTemp,1);
		efree(searchTagsTemp);

		str_replace("'&amp;","_uch_tmp_str_",Z_STRVAL_PP(thisVal),&replaceTempString1);
		CRequest_string_htmlspecialchars(replaceTempString1,&replaceTempString2 TSRMLS_CC);
		str_replace("_uch_tmp_str_","'&amp;",replaceTempString2,&replaceTempString3);
		str_replace("\\",".",replaceTempString3,&replaceTempString4);
		str_replace("/*","/.",replaceTempString4,&replaceTempString5);
		preg_replace(skipkeysString,".",replaceTempString5,&replaceTempString6);
		
		if(!preg_match(allowTags,replaceTempString6,&tempZval1)){
			efree(replaceTempString6);
			replaceTempString6 = estrdup("");
		}

		//relace and append
		if(strlen(replaceTempString6) <= 0){
			replaceTempString9 = estrdup("");
		}else{
			char	*replaceTempString7;
			str_replace("&quot;","\"",replaceTempString6,&replaceTempString7);
			spprintf(&replaceTempString9,0,"%s%s%s","<",replaceTempString7,">");
			efree(replaceTempString7);
		}

		add_next_index_string(replaces,replaceTempString9,1);

		efree(replaceTempString1);
		efree(replaceTempString2);
		efree(replaceTempString3);
		efree(replaceTempString4);
		efree(replaceTempString5);
		efree(replaceTempString6);
		efree(replaceTempString9);
		zval_ptr_dtor(&tempZval1);

		zend_hash_move_forward(Z_ARRVAL_P(findTags));
	}

	str_replaceArray(searchs,replaces,string,&replaceTempString10);

	*result = estrdup(replaceTempString10);

	//destoy
	zval_ptr_dtor(&findTags);
	zval_ptr_dtor(&match);
	efree(replaceTempString10);
	zval_ptr_dtor(&replaces);
	zval_ptr_dtor(&searchs);
}

PHP_METHOD(CRequest,removeXSS)
{
	char	*string,
			*returnString;
	int		stringLen = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CRequest_xssRemove(string,&returnString TSRMLS_CC);
	RETVAL_STRING(returnString,1);
	efree(returnString);
}


void CRequest_Args(char *key,char *type,char *from,int noFilter,zval **returnString TSRMLS_DC){

	char *getReturn,
		 *lowFrom,
		 *lowType;
	MAKE_STD_ZVAL(*returnString);
	if(key == "" || key == NULL){
		ZVAL_NULL(*returnString);
		return;
	}

	lowType = estrdup(type);
	php_strtolower(lowType,strlen(lowType)+1);


	//直接取POST中的array
	if(strcmp(lowType,"array") == 0){
		zval *postArray = NULL;
		getPostParamsZval(key,&postArray);
		if(IS_NULL != Z_TYPE_P(postArray)){
			zval	*filterArray;
			//fiter html
			CRequest_filterHTML(postArray,&filterArray TSRMLS_CC);
			ZVAL_ZVAL(*returnString,filterArray,1,1);
			zval_ptr_dtor(&postArray);
			efree(lowType);
			return;
		}else{
			zval_ptr_dtor(&postArray);
			array_init(*returnString);
			efree(lowType);
			return;
		}
	}

	//将from统一成小写
	lowFrom = estrdup(from);
	php_strtolower(lowFrom,strlen(lowFrom)+1);
	
	if(strcmp(lowFrom,"get") == 0){

		getGetParams(key,&getReturn);
		if(getReturn == NULL){
			getReturn = estrdup("");
		}

	}else if(strcmp(lowFrom,"post") == 0){

		getPostParams(key,&getReturn);
		if(getReturn == NULL){
			getReturn = estrdup("");
		}

	}else{

		//未指定 优先取POST 若不存在取GET
		getPostParams(key,&getReturn);
		if(getReturn == NULL){
			getGetParams(key,&getReturn);
			if(getReturn == NULL){
				getReturn = estrdup("");
			}
		}
	}
	efree(lowFrom);


	//判断类型
	if(strcmp(lowType,"int") == 0){
		int thisVal = toInt(getReturn);
		ZVAL_LONG(*returnString,thisVal);
	}else if(strcmp(lowType,"string") == 0){

		if(noFilter == 1){
			ZVAL_STRING(*returnString,getReturn,1);
		}else{
			char *thisVal;
			strip_tags(getReturn,&thisVal);
			ZVAL_STRING(*returnString,thisVal,1);
			efree(thisVal);
		}
	}else if(strcmp(lowType,"html") == 0){
		//filter xss
		char	*xssFilterString;
		CRequest_xssRemove(getReturn,&xssFilterString TSRMLS_CC);
		ZVAL_STRING(*returnString,xssFilterString,1);
		efree(xssFilterString);
	}else{
		if(noFilter == 1){
			ZVAL_STRING(*returnString,getReturn,1);
		}else{
			char *thisVal;
			CRequest_xssRemove(getReturn,&thisVal TSRMLS_CC);
			ZVAL_STRING(*returnString,thisVal,1);
			efree(thisVal);
		}
	}

	efree(lowType);
	efree(getReturn);
}

PHP_METHOD(CRequest,Args)
{
	
	char *key = "",
		 *type = "string",
		 *from = "REQUEST";

	zval *returnString;

	int keyLen = 0,
		typeLen = 0,
		fromLen = 0,
		noFilter = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|ssb",&key,&keyLen,&type,&typeLen,&from,&fromLen,&noFilter) == FAILURE){
		RETURN_NULL();
	}

	CRequest_Args(key,type,from,noFilter,&returnString TSRMLS_CC);
	RETVAL_ZVAL(returnString,1,1);
}

PHP_METHOD(CRequest,getUrl)
{
	zval *requsetUri;

	char	*host,
			*url;
	getServerParam("HTTP_HOST",&host TSRMLS_CC);
	if(host == NULL){
		RETVAL_NULL();
		return;
	}

	requsetUri = zend_read_static_property(CRouteCe,ZEND_STRL("requsetUri"),0 TSRMLS_CC);
	strcat2(&url,"http://",host,Z_STRVAL_P(requsetUri),NULL);
	RETVAL_STRING(url,1);
	efree(url);
}

PHP_METHOD(CRequest,getUri)
{
	zval *requsetUri;

	requsetUri = zend_read_static_property(CRouteCe,ZEND_STRL("requsetUri"),0 TSRMLS_CC);

	RETVAL_STRING(Z_STRVAL_P(requsetUri),1);
}

PHP_METHOD(CRequest,getIp)
{
	char *clientIp,
		 *remoteAdder;

	getServerParam("HTTP_CLIENT_IP",&clientIp TSRMLS_CC);

	if(clientIp != NULL){
		ZVAL_STRING(return_value,clientIp,1);
		efree(clientIp);
		return; 
	}

	getServerParam("REMOTE_ADDR",&remoteAdder TSRMLS_CC);
	if(remoteAdder != NULL){
		ZVAL_STRING(return_value,remoteAdder,1);
		efree(remoteAdder);
	}else{
		ZVAL_STRING(return_value,"0.0.0.0",1);
	}
}

PHP_METHOD(CRequest,getPreUrl)
{
	char *perUrl;
	getServerParam("HTTP_REFERER",&perUrl TSRMLS_CC);
	if(perUrl == NULL){
		RETVAL_NULL();
		return;
	}
	ZVAL_STRING(return_value,perUrl,1);
	efree(perUrl);
}

PHP_METHOD(CRequest,getAgent)
{
	char *perUrl;
	getServerParam("HTTP_USER_AGENT",&perUrl TSRMLS_CC);
	if(perUrl == NULL){
		RETVAL_NULL();
		return;
	}
	ZVAL_STRING(return_value,perUrl,1);
	efree(perUrl);
}

PHP_METHOD(CRequest,getHost)
{
	char	*key,
			*host,
			*port = "",
			hostRetrun[10240];
	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_NULL();
	}

	if(keyLen == 0){
		key = "http";
	}


	getServerParam("HTTP_X_FORWARDED_HOST",&host TSRMLS_CC);
	if(host == NULL){
		getServerParam("HTTP_HOST",&host TSRMLS_CC);
	}

	if(host == NULL){
		RETVAL_NULL();
		return;
	}

	sprintf(hostRetrun,"%s%s%s",key,"://",host);
	RETVAL_STRING(hostRetrun,1);
	efree(host);
}

PHP_METHOD(CRequest,getStartTime)
{
	zval	**startTimeZval,
			**startTime;

	if(zend_hash_find(&EG(symbol_table),"SYSTEM_INIT",strlen("SYSTEM_INIT")+1,(void**)&startTimeZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(startTimeZval)){

		if(zend_hash_find(Z_ARRVAL_PP(startTimeZval),"frameBegin",strlen("frameBegin")+1,(void**)&startTime) == SUCCESS && IS_DOUBLE == Z_TYPE_PP(startTime)){
			RETVAL_DOUBLE(Z_DVAL_PP(startTime));
			return;
		}
	}
	
	RETVAL_NULL();
}

PHP_METHOD(CRequest,getRegisterEventTime)
{
	zval	**startTimeZval,
			**startTime;

	if(zend_hash_find(&EG(symbol_table),"SYSTEM_INIT",strlen("SYSTEM_INIT")+1,(void**)&startTimeZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(startTimeZval)){

		if(zend_hash_find(Z_ARRVAL_PP(startTimeZval),"registerEvent",strlen("registerEvent")+1,(void**)&startTime) == SUCCESS && IS_DOUBLE == Z_TYPE_PP(startTime)){
			RETVAL_DOUBLE(Z_DVAL_PP(startTime));
			return;
		}
	}
	
	RETVAL_NULL();
}

PHP_METHOD(CRequest,getErrorMessage)
{

}

PHP_METHOD(CRequest,getPath)
{
	RETVAL_STRING("",1);
}

PHP_METHOD(CRequest,createUrl)
{
	zval *paramsList,
		 *otherParams,
		 *otherParamsCopy;

	char *url = "";

	//获取参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a|z",&paramsList,&otherParams) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CRouteParse_url(paramsList,&url TSRMLS_CC);
	RETVAL_STRING(url,1);
	efree(url);
}

PHP_METHOD(CRequest,disablePOST)
{
}

PHP_METHOD(CRequest,disableGET)
{
}

//判断是否为wap方式的请求
PHP_METHOD(CRequest,isWap)
{

	//如果有HTTP_X_WAP_PROFILE则一定是移动设备 
	MODULE_BEGIN 
		char *httpWapProfile = NULL;
		getServerParam("HTTP_X_WAP_PROFILE",&httpWapProfile TSRMLS_CC);
		if(httpWapProfile != NULL){
			if(strlen(httpWapProfile) > 0){
				RETVAL_TRUE;
				efree(httpWapProfile);
				return;
			}
			efree(httpWapProfile);
		}

	MODULE_END


	// 如果via信息含有wap则一定是移动设备,部分服务商会屏蔽该信息  
	MODULE_BEGIN
		char *httpVia = NULL;
		getServerParam("HTTP_VIA",&httpVia TSRMLS_CC);
		if(httpVia != NULL){

			//包含wap
			php_strtolower(httpVia,strlen(httpVia)+1);
			if(strstr(httpVia,"wap") != NULL){
				RETVAL_TRUE;
				efree(httpVia);
				return;
			}
			efree(httpVia);
		}
	MODULE_END


	//判断UserAgent中包含手机品牌信息
	MODULE_BEGIN
		char *userAgent = NULL;
		getServerParam("HTTP_USER_AGENT",&userAgent TSRMLS_CC);
		if(userAgent != NULL){
			char *clientKey = "/(nokia|sony|ericsson|mot|samsung|htc|sgh|lg|sharp|sie-|philips|panasonic|alcatel|lenovo|iphone|ipod|blackberry|meizu|android|netfront|symbian|ucweb|windowsce|palm|operamini|operamobi|openwave|nexusone|cldc|midp|wap|mobile)/i";
			zval *matchArr;

			php_strtolower(userAgent,strlen(userAgent)+1);
			if(1 == preg_match(clientKey,userAgent,&matchArr)){
				zval_ptr_dtor(&matchArr);
				efree(userAgent);
				RETVAL_TRUE;
				return;
			}
			zval_ptr_dtor(&matchArr);
			efree(userAgent);
		}
	MODULE_END


	//均不符合 返回FLASE
	RETVAL_FALSE;
}

//修正web服务器URL重写时附加的GET参数
void CRquest_fixWebServerRewriteParams(TSRMLS_D){

	zval **getDataZval;
	if(zend_hash_find(&EG(symbol_table), ZEND_STRS("_GET"), (void **)&getDataZval) == SUCCESS){
		if(IS_ARRAY == Z_TYPE_PP(getDataZval)){
			int		i = 0,
					paramsNum = 0;
			char	*key;
			ulong	ikey;
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(getDataZval));
			paramsNum = zend_hash_num_elements(Z_ARRVAL_PP(getDataZval));
			for(i = 0 ; i < paramsNum ; i++){
				zend_hash_get_current_key(Z_ARRVAL_PP(getDataZval),&key,&ikey,0);
				zend_hash_del(Z_ARRVAL_PP(getDataZval),key,strlen(key)+1);
			}
		}
	}
}

//解析执行请求
void CRequest_run(zval *object,zval **getRouteZval TSRMLS_DC)
{
	zval	*routeZval,
			*giveRouteZval,
			*routeInstance,
			*reReadRouteZval,
			*controllerObject,
			*getRouteReturn;

	//路由对象
	CRoute_getInstance(&routeInstance TSRMLS_CC);

	//修正应URL重写多出来的get参数
	CRquest_fixWebServerRewriteParams(TSRMLS_C);

	//触发路由之前的HOOKS_ROUTE_START钩子
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],routeInstance,1,0);
		CHooks_callHooks("HOOKS_ROUTE_START",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&routeInstance);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	//解析路由
	CRouteParse_getRoute(&getRouteReturn TSRMLS_CC);
	MAKE_STD_ZVAL(*getRouteZval);
	ZVAL_ZVAL(*getRouteZval,getRouteReturn,1,0);

	//获取路由对象
	CRequest_checkActionPreFix(getRouteReturn,&routeZval TSRMLS_CC);

	//触发路由后的HOOKS_ROUTE_END钩子
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],routeZval,1,0);
		CHooks_callHooks("HOOKS_ROUTE_END",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	//重新设置路由 重新读取routeZval的值设置到内核CMyRoute结构体
	reReadRouteZval = zend_read_property(CRouteCe,routeZval,ZEND_STRL("controller"),1 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(reReadRouteZval) && strlen(Z_STRVAL_P(reReadRouteZval)) > 0 ){
		zend_update_static_property_string(CRouteCe,ZEND_STRL("thisController"),Z_STRVAL_P(reReadRouteZval) TSRMLS_CC);
	}else{
		//报告异常
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to[HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setController(string controllerName) method when the parameter type errors", 12001 TSRMLS_CC);
	}

	reReadRouteZval = zend_read_property(CRouteCe,routeZval,ZEND_STRL("action"),1 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(reReadRouteZval) && strlen(Z_STRVAL_P(reReadRouteZval)) > 0 ){
		zend_update_static_property_string(CRouteCe,ZEND_STRL("thisAction"),Z_STRVAL_P(reReadRouteZval) TSRMLS_CC);
	}else{
		//报告异常
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to[HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setAction(string actionName) method when the parameter type errors", 12001 TSRMLS_CC);
	}

	reReadRouteZval = zend_read_property(CRouteCe,routeZval,ZEND_STRL("module"),1 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(reReadRouteZval) && strlen(Z_STRVAL_P(reReadRouteZval)) > 0 ){
		zend_update_static_property_string(CRouteCe,ZEND_STRL("thisModule"),Z_STRVAL_P(reReadRouteZval) TSRMLS_CC);
	}else{
		//报告异常
		zend_throw_exception(CRouteExceptionCe, "[PluginException] Registered to[HOOKS_ROUTE_END] function in the call CRoute::getInstance()->setModule(string moduleName) method when the parameter type errors", 12001 TSRMLS_CC);
	}

	//执行路由
	MAKE_STD_ZVAL(giveRouteZval);
	ZVAL_ZVAL(giveRouteZval,routeZval,1,0);

	CRequest_routeDoing(giveRouteZval TSRMLS_CC);

	controllerObject = zend_read_static_property(CRouteCe,ZEND_STRL("controllerObject"),1 TSRMLS_CC);

	//触发控制器初始化HOOKS_CONTROLLER_INIT钩子
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],controllerObject,1,0);
		CHooks_callHooks("HOOKS_CONTROLLER_INIT",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	//执行方法
	CRequest_execAction(giveRouteZval,object TSRMLS_CC);

	//触发执行方法后HOOKS_ACTION_INIT钩子
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],controllerObject,1,0);
		CHooks_callHooks("HOOKS_ACTION_INIT",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	zval_ptr_dtor(&routeZval);
	zval_ptr_dtor(&getRouteReturn);
	zval_ptr_dtor(&giveRouteZval);
}

//解析执行请求
PHP_METHOD(CRequest,run)
{
	zval	*routeDataZval;
	
	//解析路由
	CRequest_run(getThis(),&routeDataZval TSRMLS_CC);
	zval_ptr_dtor(&routeDataZval);	
}


//结束请求
PHP_METHOD(CRequest,end)
{
	
}








PHP_METHOD(CRequest,getCPULoad){

#ifdef PHP_WIN32

#else
	unsigned int total;

    float user;
    float nice;
    float system;
    float idle;

    char cpu[21];
    char text[201];

    HashTable *returnData;

    zval *userVal,
         *niceVal,
         *systemVal,
         *idleVal;

    FILE *fp;

        fp = fopen("/proc/stat", "r");
         while (fgets(text, 200, fp))
        {
                if (strstr(text, "cpu"))
                {
                sscanf(text, "%s %f %f %f %f", cpu, &user, &nice, &system, &idle);
                break;
                }
        }
        fclose(fp);



        ALLOC_HASHTABLE(returnData);
        zend_hash_init(returnData,64,NULL,NULL,0);

        MAKE_STD_ZVAL(userVal);
        ZVAL_DOUBLE(userVal,user);
        zend_hash_add(returnData,"user",strlen("user")+1,&userVal,sizeof(zval*),NULL);
		//zval_ptr_dtor(&userVal);

        MAKE_STD_ZVAL(niceVal);
        ZVAL_DOUBLE(niceVal,nice);
        zend_hash_add(returnData,"nice",strlen("nice")+1,&niceVal,sizeof(zval*),NULL);
		//zval_ptr_dtor(&userVal);

        MAKE_STD_ZVAL(systemVal);
        ZVAL_DOUBLE(systemVal,system);
        zend_hash_add(returnData,"system",strlen("system")+1,&systemVal,sizeof(zval*),NULL);
		//zval_ptr_dtor(&userVal);

        MAKE_STD_ZVAL(idleVal);
        ZVAL_DOUBLE(idleVal,idle);
        zend_hash_add(returnData,"idle",strlen("idle")+1,&idleVal,sizeof(zval*),NULL);
		//zval_ptr_dtor(&userVal);

		Z_TYPE_P(return_value) = IS_ARRAY;
        Z_ARRVAL_P(return_value) = returnData;

#endif
}



PHP_METHOD(CRequest,getAllMemory)
{
	RETVAL_LONG(getMemory());
}

PHP_METHOD(CRequest,getFreeMemory)
{
	RETVAL_LONG(getFreeMemory());
}

PHP_METHOD(CRequest,getVirtualMemory)
{
	RETVAL_LONG(getVirtualMemory());
}

PHP_METHOD(CRequest,getFreeVirtualMemory)
{
	RETVAL_LONG(getFreeVirtualMemory());
}

#define BUF_SIZE 1024
PHP_METHOD(CRequest,getMaxUseMemory)
{
#ifdef PHP_WIN32
	zval *processList,
		 **processData,
		 *addZval;
	int i,
		maxUsed = 0;
	ulong ikey;

	char *maxProcess,
		 *key;

	HashTable *returnData;

	processList = getProcessList();

	
	if(IS_ARRAY == Z_TYPE_P(processList)){
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(processList));
		for(i = 0 ; i < zend_hash_num_elements(Z_ARRVAL_P(processList)); i++){
			zend_hash_get_current_data(Z_ARRVAL_P(processList),(void**)&processData);
			zend_hash_get_current_key(Z_ARRVAL_P(processList),&key,&ikey,0);

			if(Z_LVAL_PP(processData) > maxUsed){
				maxProcess = key;
				maxUsed = Z_LVAL_PP(processData);
			}
			zend_hash_move_forward(Z_ARRVAL_P(processList));
		}

		ALLOC_HASHTABLE(returnData);
		zend_hash_init(returnData,2,NULL,NULL,0);

		MAKE_STD_ZVAL(addZval);
		ZVAL_LONG(addZval,maxUsed);
		zend_hash_add(returnData,"use",strlen("use")+1,&addZval,sizeof(zval*),NULL);
		//zval_ptr_dtor(&addZval);

		MAKE_STD_ZVAL(addZval);
		ZVAL_STRING(addZval,maxProcess,1);
		zend_hash_add(returnData,"name",strlen("name")+1,&addZval,sizeof(zval*),NULL);
		//zval_ptr_dtor(&addZval);

		Z_TYPE_P(return_value) = IS_ARRAY;
		Z_ARRVAL_P(return_value) = returnData;
	}
#else
		DIR *dir;
        struct dirent *ptr;
        FILE *fp;
        char filepath[50];
        char cur_task_name[50];
        char buf[BUF_SIZE];
        dir = opendir("/proc");
        HashTable *returnData,
					*backData;
		char *maxName;

		int maxUsed = 0;

        zval *pidZval,
			 *saveBack,
			 *returnBackZval;
        ALLOC_HASHTABLE(returnData);
        zend_hash_init(returnData,512,NULL,NULL,0);

        if (NULL != dir)
        {

                while ((ptr = readdir(dir)) != NULL) //循环读取路径下的每一个文件/文件夹
                {
                        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)){
                                continue;
                        }
                        if (DT_DIR != ptr->d_type){
                                continue;
                        }

                        sprintf(filepath, "/proc/%s/status", ptr->d_name);//生成要读取的文件的路径i

						fp = fopen(filepath, "r");//打开文件
                        if (NULL != fp)
                        {
                                HashTable *processData;
                                zval *processZval,
                                     *processIdZval,
                                     *zvalPidData,
                                     *statusZval;

                                char *status;
                                int i = 0,
                                    hasMemory = 0;
                                ALLOC_HASHTABLE(processData);
                                zend_hash_init(processData,8,NULL,NULL,0);

                                if( fgets(buf, BUF_SIZE-1, fp) == NULL ){
                                        fclose(fp);
                                        continue;
                                }
                                
								sscanf(buf, "%*s %s", cur_task_name);

                                MAKE_STD_ZVAL(processZval);
                                ZVAL_STRING(processZval,cur_task_name,1);
                                zend_hash_add(processData,"name",strlen("name")+1,&processZval,sizeof(zval*),NULL);
								//zval_ptr_dtor(&processZval);

                                MAKE_STD_ZVAL(processIdZval);
                                ZVAL_STRING(processIdZval,ptr->d_name,1);
                                zend_hash_add(processData,"pid",strlen("pid")+1,&processIdZval,sizeof(zval*),NULL);
								//zval_ptr_dtor(&processIdZval);

                                MAKE_STD_ZVAL(zvalPidData);
								Z_TYPE_P(zvalPidData) = IS_ARRAY;
                                Z_ARRVAL_P(zvalPidData) = processData;

                                fgets(buf, BUF_SIZE-1, fp);
                                str_replace("State:","",&buf,&status);
                                MAKE_STD_ZVAL(statusZval);
                                ZVAL_STRING(statusZval,status,1);
                                zend_hash_add(processData,"status",strlen("status")+1,&statusZval,sizeof(zval*),NULL);
								//zval_ptr_dtor(&statusZval);


                                while(fgets(buf, BUF_SIZE-1, fp) != NULL){
                                        if(strstr(buf,"VmSize") != NULL){
                                                char *memChar,
													 *memCharTemp,
													 *memCharTemp1;
												int thisMemSize = 0;
                                                str_replace("VmSize:","",&buf,&memCharTemp);
												str_replace("kB","",memCharTemp,&memCharTemp1);
												php_trim(memCharTemp1,"",&memChar);
												thisMemSize = toInt(memChar);
												if(thisMemSize >= maxUsed){
													maxUsed = thisMemSize;
													maxName = estrdup(cur_task_name);
												}

                                                zval *memZval;
                                                MAKE_STD_ZVAL(memZval);
                                                ZVAL_STRING(memZval,memChar,1);
                                                zend_hash_add(processData,"memory",strlen("memory")+1,&memZval,sizeof(zval*),NULL);
												//zval_ptr_dtor(&memZval);
                                        }
                                        if(strstr(buf,"PPid") != NULL){
                                                char *ppidChar;
                                                str_replace("PPid:","",&buf,&ppidChar);
                                                zval *ppidZval;
                                                MAKE_STD_ZVAL(ppidZval);
                                                ZVAL_STRING(ppidZval,ppidChar,1);
                                                zend_hash_add(processData,"ppid",strlen("ppid")+1,&ppidZval,sizeof(zval*),NULL);
												//zval_ptr_dtor(&ppidZval);
                                        }
                                }

                                zend_hash_update(returnData,ptr->d_name,strlen(ptr->d_name)+1,&zvalPidData,sizeof(zval*),NULL);
								//zval_ptr_dtor(&zvalPidData);
                                fclose(fp);
                        }
                }
                closedir(dir);//关闭路径
        }

		ALLOC_HASHTABLE(backData);
        zend_hash_init(backData,2,NULL,NULL,0);

		MAKE_STD_ZVAL(saveBack);
		ZVAL_STRING(saveBack,maxName,1);
		zend_hash_add(backData,"name",strlen("name")+1,&saveBack,sizeof(zval*),NULL);
		//zval_ptr_dtor(&saveBack);

		MAKE_STD_ZVAL(saveBack);
		ZVAL_LONG(saveBack,maxUsed);
		zend_hash_add(backData,"use",strlen("use")+1,&saveBack,sizeof(zval*),NULL);
		//zval_ptr_dtor(&saveBack);

		MAKE_STD_ZVAL(returnBackZval);
		Z_TYPE_P(returnBackZval) = IS_ARRAY;
		Z_ARRVAL_P(returnBackZval) = backData;

		ZVAL_ZVAL(return_value,returnBackZval,1,0);
		//zval_ptr_dtor(&returnBackZval);
#endif
}

PHP_METHOD(CRequest,getApacheMemory)
{
	RETVAL_LONG(getProcessInfo("httpd"));
}

PHP_METHOD(CRequest,getCurrentProcessMemory)
{
	RETVAL_LONG(getCurrentProcessInfo());
}

PHP_METHOD(CRequest,getMysqlMemory)
{
	RETVAL_LONG(getProcessInfo("mysqld.exe"));
}

PHP_METHOD(CRequest,getMysqlNTMemory)
{
	RETVAL_LONG(getProcessInfo("mysqld-nt.exe"));
}

PHP_METHOD(CRequest,getMemcacheMemory)
{
	RETVAL_LONG(getProcessInfo("memcached.exe"));
}

PHP_METHOD(CRequest,getRedisMemory)
{
	RETVAL_LONG(getProcessInfo("redis-server.exe"));
}

PHP_METHOD(CRequest,getFreeDisk)
{
#ifdef PHP_WIN32
	char *disk;
	int diskLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&disk,&diskLen) == FAILURE){
		return;
	}
	if(diskLen > 0){
		RETVAL_LONG(getDiskAll(disk,1));
	}
#endif
}

PHP_METHOD(CRequest,getAllDisk)
{
#ifdef PHP_WIN32
	char *disk;
	int diskLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&disk,&diskLen) == FAILURE){
		return;
	}
	if(diskLen > 0){
		RETVAL_LONG(getDiskAll(disk,2));
	}
#endif
}


PHP_METHOD(CRequest,getProcessList)
{
#ifdef PHP_WIN32
	zval *processList;
	getProcessList(&processList);
	
	Z_TYPE_P(return_value) = IS_ARRAY;
	Z_ARRVAL_P(return_value) = Z_ARRVAL_P(processList);
#endif
}
