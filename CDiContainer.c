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
#include "php_CDiContainer.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CDiContainer_functions[] = {
	PHP_ME(CDiContainer,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDiContainer,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDiContainer,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDiContainer,del,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDiContainer,has,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDiContainer,singleton,NULL,ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDiContainer)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CDiContainer",CDiContainer_functions);
	CDiContainerCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//全局变量
	zend_declare_property_null(CDiContainerCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDiContainerCe, ZEND_STRL("serviceList"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

int CDiContainer_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CDiContainerCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_OBJECT != Z_TYPE_P(instanceZval) ){
		
		zval			*object,
						*saveObject;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CDiContainerCe);

		//将类对象保存在instance静态变量
		zend_update_static_property(CDiContainerCe,ZEND_STRL("instance"),object TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

PHP_METHOD(CDiContainer,getInstance)
{
	zval *instanceZval;

	CDiContainer_getInstance(&instanceZval TSRMLS_CC);


	ZVAL_ZVAL(return_value,instanceZval,1,1);
}

void CDiContainer_set(char *name,zval *defined,zval *params,zval *object TSRMLS_DC)
{
	zval *serviceList,
		 *addZval,
		 *setParams,
		 *setObject,
		 *setDefined,
		 *newList;


	serviceList = zend_read_property(CDiContainerCe,object,ZEND_STRL("serviceList"),0 TSRMLS_CC);

	if(IS_NULL == Z_TYPE_P(serviceList)){
		zval *saveList;
		MAKE_STD_ZVAL(saveList);
		array_init(saveList);
		zend_update_property(CDiContainerCe,object,ZEND_STRL("serviceList"),saveList TSRMLS_CC);
		serviceList = zend_read_property(CDiContainerCe,object,ZEND_STRL("serviceList"),0 TSRMLS_CC);
		zval_ptr_dtor(&saveList);
	}

	MAKE_STD_ZVAL(addZval);
	array_init(addZval);
	add_assoc_zval(addZval,"defined",defined);
	add_assoc_zval(addZval,"params",params);
	add_assoc_zval(serviceList,name,addZval);

	zend_update_property(CDiContainerCe,object,ZEND_STRL("serviceList"),serviceList TSRMLS_CC);
}

PHP_METHOD(CDiContainer,set)
{
	char *name;
	int nameLen;

	zval *defined,
		 *params = NULL,
		 *useParams = NULL,
		 *useDefined = NULL;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|z",&name,&nameLen,&defined,&params) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	if(params == NULL || IS_NULL == Z_TYPE_P(params)){
		MAKE_STD_ZVAL(useParams);
		array_init(useParams);
	}else{
		MAKE_STD_ZVAL(useParams);
		ZVAL_ZVAL(useParams,params,1,0);
	}

	MAKE_STD_ZVAL(useDefined);
	ZVAL_ZVAL(useDefined,defined,1,0);

	//注册服务
	CDiContainer_set(name,useDefined,useParams,getThis() TSRMLS_CC);
	RETVAL_TRUE;
}

void CDiContainer_get(char *name,zval *object,zval *params,zval **returnData TSRMLS_DC)
{
	zval	*serviceList,
			**thisData,
			*backData;

	MAKE_STD_ZVAL(*returnData);
	ZVAL_NULL(*returnData);

	serviceList = zend_read_property(CDiContainerCe,object,ZEND_STRL("serviceList"),0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(serviceList)){
		char errorMessage[10240];
		sprintf(errorMessage,"%s","[CDiException] DiContainer is Error");
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}

	if( ! zend_hash_exists(Z_ARRVAL_P(serviceList),name,strlen(name)+1) ){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}


	if( zend_hash_find(Z_ARRVAL_P(serviceList),name,strlen(name)+1,(void**)&thisData) == SUCCESS ){

		zval **definedZval,
			 **pathZval;

		//检查是否存在defined
		if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_OBJECT == Z_TYPE_PP(definedZval) ){

			ZVAL_ZVAL(*returnData,*definedZval,1,0);
			return;

		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_STRING == Z_TYPE_PP(definedZval) ){
	
			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			if(strstr(Z_STRVAL_PP(definedZval),"::") != NULL){
				zval	*thisObject,
						*thisRetrunData;

				char	*className = "",
						*actionName;

				zend_class_entry **classEnCePP,
								 *classEnCe;

				zval	constructReturn,
							constructVal,
							*saveParams,
							sendParams;

				HashTable *callParams;

				//拆分类名和方法名
				className = strtok(Z_STRVAL_PP(definedZval), "::"); 
				actionName = strtok(NULL, "::");
				
				//查询类对象
				php_strtolower(className,strlen(className)+1);
				if(FAILURE == zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&classEnCePP)){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}

				classEnCe = *classEnCePP;

				//调用静态方法
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal,actionName, 1);
				ALLOC_HASHTABLE(callParams);
				zend_hash_init(callParams,2,NULL,NULL,0);


				MAKE_STD_ZVAL(saveParams);
				ZVAL_STRING(saveParams,className,1);
				zend_hash_next_index_insert(callParams,&saveParams,sizeof(zval*),0);


				MAKE_STD_ZVAL(saveParams);
				ZVAL_STRING(saveParams,actionName,1);
				zend_hash_next_index_insert(callParams,&saveParams,sizeof(zval*),0);
	


				INIT_ZVAL(sendParams);
				Z_TYPE(sendParams) = IS_ARRAY;
				Z_ARRVAL(sendParams) = callParams;

				call_user_function(NULL, NULL, &sendParams, &constructReturn, 0, NULL TSRMLS_CC);

				zend_hash_destroy(callParams);
				
				thisRetrunData = &constructReturn;
				ZVAL_ZVAL(*returnData,thisRetrunData,1,0);
				zval_dtor(&constructReturn);

				//寻找serverList->key[object]
				zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&thisRetrunData,sizeof(zval*),NULL);

				zval_ptr_dtor(&thisParams);

				return;
			}


			//尝试加载文件
			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				if(CLoader_loadFile(Z_STRVAL_PP(definedZval)) == FAILURE){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}

			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}

			ZVAL_ZVAL(*returnData,getObject,1,0);
			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);


			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(definedZval) && zend_hash_find(Z_ARRVAL_PP(definedZval),"path",strlen("path")+1,(void**)&pathZval) == SUCCESS ){
			
			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				if(CLoader_loadFile(Z_STRVAL_PP(pathZval)) == FAILURE){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}
			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}
			ZVAL_ZVAL(*returnData,getObject,1,0);
			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);


			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_NULL == Z_TYPE_PP(definedZval)){

			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				//找不到则调用CLoader方法加载文件
				zval *canLoad;
				CLoader_load(thisGetClassName,&canLoad TSRMLS_CC);
				if(IS_BOOL == Z_TYPE_P(canLoad) && Z_LVAL_P(canLoad) == 1){
				}else{
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
				zval_ptr_dtor(&canLoad);
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}
			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}
			ZVAL_ZVAL(*returnData,getObject,1,0);
			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);


			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}

	}
}

PHP_METHOD(CDiContainer,get)
{
	char *name;
	int nameLen;

	zval	*returnData,
			*params = NULL,
			*useParams = NULL;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&name,&nameLen,&params) == FAILURE){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}

	MAKE_STD_ZVAL(useParams);
	if(params == NULL || IS_NULL == Z_TYPE_P(params)){
		array_init(useParams);
	}else{
		ZVAL_ZVAL(useParams,params,1,0);
	}

	CDiContainer_get(name,getThis(),useParams,&returnData TSRMLS_CC);
	zval_ptr_dtor(&useParams);
	RETVAL_ZVAL(returnData,1,1);
}

PHP_METHOD(CDiContainer,del)
{
	char *name;
	int nameLen;
	zval	*serviceList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&nameLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	serviceList = zend_read_property(CDiContainerCe,getThis(),ZEND_STRL("serviceList"),0 TSRMLS_CC);
	if(zend_hash_exists(Z_ARRVAL_P(serviceList),name,strlen(name)+1)){

		zend_hash_del(Z_ARRVAL_P(serviceList),name,strlen(name)+1);

		RETVAL_TRUE;
		return;
	}

	RETVAL_FALSE;
}

PHP_METHOD(CDiContainer,has)
{
	char *name;
	int nameLen;
	zval	*serviceList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&name,&nameLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	serviceList = zend_read_property(CDiContainerCe,getThis(),ZEND_STRL("serviceList"),0 TSRMLS_CC);
	if(zend_hash_exists(Z_ARRVAL_P(serviceList),name,strlen(name)+1)){
		RETVAL_TRUE;
		return;
	}

	RETVAL_FALSE;
}


void CDiContainer_singleton(char *name,zval *object,zval *params,zval **returnData TSRMLS_DC)
{
	zval	*serviceList,
			**thisData,
			*backData;

	MAKE_STD_ZVAL(*returnData);
	ZVAL_NULL(*returnData);

	serviceList = zend_read_property(CDiContainerCe,object,ZEND_STRL("serviceList"),0 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(serviceList)){
		char errorMessage[10240];
		sprintf(errorMessage,"%s","[CDiException] DiContainer is Error");
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}

	if( ! zend_hash_exists(Z_ARRVAL_P(serviceList),name,strlen(name)+1) ){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}

	serviceList = zend_read_property(CDiContainerCe,object,ZEND_STRL("serviceList"),0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(serviceList) && zend_hash_find(Z_ARRVAL_P(serviceList),name,strlen(name)+1,(void**)&thisData) == SUCCESS ){

		zval **definedZval,
			 **pathZval;

		//检查是否存在defined
		if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_OBJECT == Z_TYPE_PP(definedZval) ){
			ZVAL_ZVAL(*returnData,*definedZval,1,0);
			return;
		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_STRING == Z_TYPE_PP(definedZval) ){
	
			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			zval **hasObject;
			if(zend_hash_find(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,(void**)&hasObject) == SUCCESS && IS_OBJECT == Z_TYPE_PP(hasObject)){
				ZVAL_ZVAL(*returnData,*hasObject,1,0);
				return;
			}


			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			if(strstr(Z_STRVAL_PP(definedZval),"::") != NULL){
				zval	*thisObject,
						*thisRetrunData;

				char	*className = "",
						*actionName;

				zend_class_entry **classEnCePP,
								 *classEnCe;

				zval	constructReturn,
							constructVal;

				//拆分类名和方法名
				className = strtok(Z_STRVAL_PP(definedZval), "::"); 
				actionName = strtok(NULL, "::");
				
				//查询类对象
				php_strtolower(className,strlen(className)+1);
				if(FAILURE == zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&classEnCePP)){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}

				classEnCe = *classEnCePP;

				//调用静态方法
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal,actionName, 1);
				call_user_function(&classEnCe->function_table, NULL, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);

				thisRetrunData = &constructReturn;
				ZVAL_ZVAL(*returnData,thisRetrunData,1,0);

				//寻找serverList->key[object]
				zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&thisRetrunData,sizeof(zval*),NULL);

				zval_ptr_dtor(&thisParams);
				zval_dtor(&constructReturn);
	
				return;
			}

			//尝试加载文件
			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				if(CLoader_loadFile(Z_STRVAL_PP(definedZval)) == FAILURE){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}
			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}
			ZVAL_ZVAL(*returnData,getObject,1,0);

			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);

			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_ARRAY == Z_TYPE_PP(definedZval) && zend_hash_find(Z_ARRVAL_PP(definedZval),"path",strlen("path")+1,(void**)&pathZval) == SUCCESS ){
			
			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			zval **hasObject;
			if(zend_hash_find(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,(void**)&hasObject) == SUCCESS && IS_OBJECT == Z_TYPE_PP(hasObject)){
				ZVAL_ZVAL(*returnData,*hasObject,1,0);
				return;
			}

			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				if(CLoader_loadFile(Z_STRVAL_PP(pathZval)) == FAILURE){
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}
			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}
			ZVAL_ZVAL(*returnData,getObject,1,0);
			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);

			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}else if(zend_hash_find(Z_ARRVAL_PP(thisData),"defined",strlen("defined")+1,(void**)&definedZval) == SUCCESS && IS_NULL == Z_TYPE_PP(definedZval)){

			zend_class_entry	**getClassEncePP,
								 *getClassCe;

			char *thisGetClassName;

			zval *getObject,
				 *setParams,
				 *thisParams;

			zval **hasObject;
			if(zend_hash_find(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,(void**)&hasObject) == SUCCESS && IS_OBJECT == Z_TYPE_PP(hasObject)){
				ZVAL_ZVAL(*returnData,*hasObject,1,0);
				return;
			}

			MAKE_STD_ZVAL(thisParams);
			Z_TYPE_P(thisParams) = IS_NULL;
			if(IS_NULL != Z_TYPE_P(params)){
				ZVAL_ZVAL(thisParams,params,1,0);
			}else{
				zval **findParams;
				if(zend_hash_find(Z_ARRVAL_PP(thisData),"params",strlen("params")+1,(void**)&findParams) == SUCCESS){
					ZVAL_ZVAL(thisParams,*findParams,1,0);
				}
			}

			thisGetClassName = estrdup(name);
			php_strtolower(thisGetClassName,strlen(thisGetClassName)+1);
			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				//找不到则调用CLoader方法加载文件
				zval *canLoad;
				CLoader_load(thisGetClassName,&canLoad TSRMLS_CC);
				if(IS_BOOL == Z_TYPE_P(canLoad) && Z_LVAL_P(canLoad) == 1){
				}else{
					char errorMessage[10240];
					sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
					zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
					return;
				}
				zval_ptr_dtor(&canLoad);
			}

			if(FAILURE == zend_hash_find(EG(class_table),thisGetClassName,strlen(thisGetClassName)+1,(void**)&getClassEncePP)){
				char errorMessage[10240];
				sprintf(errorMessage,"%s%s%s","[",thisGetClassName,"] cannot load path component registration");
				zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
				return;
			}
			getClassCe = *getClassEncePP;
			
			MAKE_STD_ZVAL(getObject);
			object_init_ex(getObject,getClassCe);

			//执行构造器
			if (getClassCe->constructor) {
				zval constructReturn;
				zval constructVal,
					 param1,
					 *paramList[1];
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_ZVAL(paramList[0],thisParams,1,0);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, getClassCe->constructor->common.function_name, 1);
				if(IS_NULL != Z_TYPE_P(thisParams)){
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 1, paramList TSRMLS_CC);
					zval_ptr_dtor(&paramList[0]);
				}else{
					call_user_function(NULL, &getObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
				}
				zval_dtor(&constructReturn);
			}
			ZVAL_ZVAL(*returnData,getObject,1,0);
			zend_hash_update(Z_ARRVAL_PP(thisData),"object",strlen("object")+1,&getObject,sizeof(zval*),NULL);

			zval_ptr_dtor(&thisParams);
			efree(thisGetClassName);

			return;
		}

	}else{
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}
}

PHP_METHOD(CDiContainer,singleton)
{
	char *name;
	int nameLen;

	zval	*returnData,
			*params = NULL,
			*useParams = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&name,&nameLen,&params) == FAILURE){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ClassNotFound] Try to obtain the component is not registered : " ,name);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 1400 TSRMLS_CC);
		return;
	}

	MAKE_STD_ZVAL(useParams);
	if(params == NULL || IS_NULL == Z_TYPE_P(params)){
		array_init(useParams);
	}else{
		ZVAL_ZVAL(params,useParams,1,0);
	}


	CDiContainer_singleton(name,getThis(),useParams,&returnData TSRMLS_CC);
	zval_ptr_dtor(&useParams);
	RETVAL_ZVAL(returnData,1,1);
}
