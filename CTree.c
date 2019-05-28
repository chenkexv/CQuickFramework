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
#include "php_CTree.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CTree_functions[] = {
	PHP_ME(CTree,setConfigs,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTree,getLeftPaddingTree,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTree,getTreeArray,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CTree)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CTree",CTree_functions);
	CTreeCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CTreeCe, ZEND_STRL("config"),ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CTreeCe, ZEND_STRL("level"),ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CTreeCe, ZEND_STRL("result"),ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
}

PHP_METHOD(CTree,setConfigs)
{
	zval	*config;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&config) == FAILURE){
		RETURN_FALSE;
	}

	zend_update_static_property(CTreeCe,ZEND_STRL("config"),config TSRMLS_CC);
}

void CTree_getConfigs(zval *config,char *key,char **stringName TSRMLS_DC){

	zval	**thisVal;

	if(IS_ARRAY == Z_TYPE_P(config) && SUCCESS == zend_hash_find(Z_ARRVAL_P(config),key,strlen(key)+1,(void**)&thisVal) && IS_STRING == Z_TYPE_PP(thisVal) ){
		*stringName = estrdup(Z_STRVAL_PP(thisVal));
		return;
	}

	//get default configs
	if(strcmp(key,"primary_key") == 0){
		*stringName = estrdup("id");
		return;
	}

	if(strcmp(key,"parent_key") == 0){
		*stringName = estrdup("parent_id");
		return;
	}

	if(strcmp(key,"expanded_key") == 0){
		*stringName = estrdup("expanded");
		return;
	}

	if(strcmp(key,"leaf_key") == 0){
		*stringName = estrdup("leaf");
		return;
	}

	if(strcmp(key,"children_key") == 0){
		*stringName = estrdup("children");
		return;
	}

	if(strcmp(key,"expanded") == 0){
		*stringName = estrdup("0");
		return;
	}

	if(strcmp(key,"name") == 0){
		*stringName = estrdup("name");
		return;
	}
}


void CTree_buildData(zval *data,zval *configs,zval **returnArray,zval *object TSRMLS_DC)
{
	int		i,h;

	char	*primary_key,
			*parent_key,
			*id;

	zval	**thisVal,
			**idVal,
			**parentVal,
			**parentKeyVal;

	MAKE_STD_ZVAL(*returnArray);
	array_init(*returnArray);

	h = zend_hash_num_elements(Z_ARRVAL_P(data));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));

	CTree_getConfigs(configs,"primary_key",&primary_key TSRMLS_CC);
	CTree_getConfigs(configs,"parent_key",&parent_key TSRMLS_CC);

	for(i = 0;i < h;i++){
		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		
		if(IS_ARRAY != Z_TYPE_PP(thisVal)){
			efree(primary_key);
			efree(parent_key);
			return;
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),primary_key,strlen(primary_key)+1,(void**)&idVal)){
			convert_to_long(*idVal);
		}else{
			efree(primary_key);
			efree(parent_key);
			return;
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),parent_key,strlen(parent_key)+1,(void**)&parentVal)){
			convert_to_long(*parentVal);
		}else{
			efree(primary_key);
			efree(parent_key);
			return;
		}

		//find parent_id key;
		if(SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(returnArray),Z_LVAL_PP(parentVal),(void**)&parentKeyVal) && IS_ARRAY == Z_TYPE_PP(parentKeyVal) ){
			zval	*saveData,
					*saveDataParent;
			MAKE_STD_ZVAL(saveData);
			ZVAL_ZVAL(saveData,*thisVal,1,0);
			add_index_zval(*parentKeyVal,Z_LVAL_PP(idVal),saveData);		
		}else{
			zval	*saveData,
					*saveDataParent;
			MAKE_STD_ZVAL(saveData);
			ZVAL_ZVAL(saveData,*thisVal,1,0);
			MAKE_STD_ZVAL(saveDataParent);
			array_init(saveDataParent);
			add_index_zval(saveDataParent,Z_LVAL_PP(idVal),saveData);
			add_index_zval(*returnArray,Z_LVAL_PP(parentVal),saveDataParent);
		}
		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	efree(primary_key);
	efree(parent_key);
}

void CTree_getConfigDefault(zval *object,zval **returnArray TSRMLS_DC){

	zval *defaultConfig;

	defaultConfig = zend_read_static_property(CTreeCe,ZEND_STRL("config"),1 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(defaultConfig)){

		MAKE_STD_ZVAL(defaultConfig);
		array_init(defaultConfig);
		add_assoc_string(defaultConfig,"primary_key","id",1);
		add_assoc_string(defaultConfig,"parent_key","parent_id",1);
		add_assoc_string(defaultConfig,"expanded_key","expanded",1);
		add_assoc_string(defaultConfig,"leaf_key","leaf",1);
		add_assoc_string(defaultConfig,"children_key","children",1);
		add_assoc_string(defaultConfig,"expanded","0",1);
		add_assoc_string(defaultConfig,"name","name",1);
		zend_update_static_property(CTreeCe,ZEND_STRL("config"),defaultConfig TSRMLS_CC);
		zval_ptr_dtor(&defaultConfig);
		defaultConfig = zend_read_static_property(CTreeCe,ZEND_STRL("config"),1 TSRMLS_CC);
	}

	MAKE_STD_ZVAL(*returnArray);
	ZVAL_ZVAL(*returnArray,defaultConfig,1,0);
}

void CTree_makeTreeCore(zval *configs,int index,zval *data,zval **returnArray,zval *object,int type TSRMLS_DC){

	zval	**beginArray,
			**thisVal,
			**parentIdVal,
			*result,
			*level;
	int		i,h;
	char	*primary_key,
			*parent_key,
			*idString;
	ulong	id;

	MAKE_STD_ZVAL(*returnArray);

	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(data),index,(void**)&beginArray) && IS_ARRAY == Z_TYPE_PP(beginArray) ){
	}else{
		array_init(*returnArray);
		return;
	}

	if(type == 1){
		array_init(*returnArray);
	}

	CTree_getConfigs(configs,"primary_key",&primary_key TSRMLS_CC);
	CTree_getConfigs(configs,"parent_key",&parent_key TSRMLS_CC);

	level = zend_read_static_property(CTreeCe,ZEND_STRL("level"),1 TSRMLS_CC);
	result = zend_read_static_property(CTreeCe,ZEND_STRL("result"),1 TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(level)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CTreeCe,ZEND_STRL("level"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		level = zend_read_static_property(CTreeCe,ZEND_STRL("level"),1 TSRMLS_CC);
	}

	if(IS_ARRAY != Z_TYPE_P(result)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CTreeCe,ZEND_STRL("result"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		result = zend_read_static_property(CTreeCe,ZEND_STRL("result"),1 TSRMLS_CC);
	}


	h = zend_hash_num_elements(Z_ARRVAL_PP(beginArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(beginArray));
	for(i = 0 ; i < h; i++){

		zend_hash_get_current_data(Z_ARRVAL_PP(beginArray),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_PP(beginArray),&idString,&id,0);

		if(IS_ARRAY != Z_TYPE_PP(thisVal)){
			efree(primary_key);
			efree(parent_key);
			array_init(*returnArray);
			return;
		}

		if(type == 1){
			zval	*saveThisVal;
			if(zend_hash_index_exists(Z_ARRVAL_P(data),id)){
				char	*children_key;
				zval	*callReturnArray;

				CTree_getConfigs(configs,"children_key",&children_key TSRMLS_CC);
				CTree_makeTreeCore(configs,id,data,&callReturnArray,object,type TSRMLS_CC);
				add_assoc_zval(*thisVal,children_key,callReturnArray);
				efree(children_key);
			}else{
				char	*leaf_key;
				CTree_getConfigs(configs,"leaf_key",&leaf_key TSRMLS_CC);
				add_assoc_bool(*thisVal,leaf_key,1);
				efree(leaf_key);
			}
			MAKE_STD_ZVAL(saveThisVal);
			ZVAL_ZVAL(saveThisVal,*thisVal,1,0);
			add_next_index_zval(*returnArray,saveThisVal);

		}else if(type == 2){
			int		nowLevel = 0;
			zval	*saveThisVal;
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),parent_key,strlen(parent_key)+1,(void**)&parentIdVal)){
				convert_to_long(*parentIdVal);
			}else{
				efree(primary_key);
				efree(parent_key);
				array_init(*returnArray);
				return;
			}

			if(index == 0){
				nowLevel = 0;
				add_index_long(level,id,0);
			}else{
				zval	**levelParentId;
				int		thisLevelParentId = 1;
				if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(level),Z_LVAL_PP(parentIdVal),(void**)&levelParentId)){
					convert_to_long(*levelParentId);
					thisLevelParentId = Z_LVAL_PP(levelParentId) + 1;
				}
				add_index_long(level,id,thisLevelParentId);
				nowLevel = thisLevelParentId;
			}
			
			add_assoc_long(*thisVal,"level",nowLevel);
			MAKE_STD_ZVAL(saveThisVal);
			ZVAL_ZVAL(saveThisVal,*thisVal,1,0);
			add_next_index_zval(result,saveThisVal);

			if(zend_hash_index_exists(Z_ARRVAL_P(data),id)){
				zval *returnData;
				CTree_makeTreeCore(configs,id,data,&returnData,object,type TSRMLS_CC);
				zval_ptr_dtor(&returnData);
			}
		}
	
		zend_hash_move_forward(Z_ARRVAL_PP(beginArray));
	}	

	if(type == 1){

	}else if(type == 2){
		ZVAL_ZVAL(*returnArray,result,1,0);
	}

	efree(primary_key);
	efree(parent_key);

}

void CTree_appendPreFix(char *prefix,int num,char **returnStrnig){

	int i = 0;
	smart_str appendString = {0};

	if(num == 0){
		*returnStrnig = estrdup("");
		return;
	}

	for(i = 0 ; i < num ; i++){
		smart_str_appends(&appendString,prefix);
	}
	smart_str_0(&appendString);
	*returnStrnig = estrdup(appendString.c);
	smart_str_free(&appendString);
}

void CTree_paddingLeftPrefix(char *name,char *prefix,zval **returnArray TSRMLS_DC){

	int		i,h;
	zval	**thisVal,
			**itemLevel,
			**itemName;



	h = zend_hash_num_elements(Z_ARRVAL_PP(returnArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(returnArray));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_PP(returnArray),(void**)&thisVal);

		if(
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"level",strlen("level")+1,(void**)&itemLevel) && 
			IS_LONG == Z_TYPE_PP(itemLevel) && 
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),name,strlen(name)+1,(void**)&itemName) && 
			IS_STRING == Z_TYPE_PP(itemName)
		){
			char	*appendString,
					*fixName;
			CTree_appendPreFix(prefix,Z_LVAL_PP(itemLevel),&appendString);
			spprintf(&fixName,0,"%s%s",appendString,Z_STRVAL_PP(itemName));
			add_assoc_string(*thisVal,name,fixName,0);
			efree(appendString);
		}


		zend_hash_move_forward(Z_ARRVAL_PP(returnArray));
	}

}

PHP_METHOD(CTree,getLeftPaddingTree)
{
	zval	*array,
			*returnArray,
			*option = NULL,
			*config,
			*endArray,
			**prefix;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a|a",&array,&option) == FAILURE){
		RETURN_FALSE;
	}

	CTree_getConfigDefault(getThis(),&config TSRMLS_CC);
	if(option != NULL && IS_ARRAY == Z_TYPE_P(option)){
		zend_hash_merge(Z_ARRVAL_P(config), Z_ARRVAL_P(option), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*), 1);
	}


	CTree_buildData(array,config,&returnArray,getThis() TSRMLS_CC);
	CTree_makeTreeCore(config,0,returnArray,&endArray,getThis(),2 TSRMLS_CC);

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(config),"prefix",strlen("prefix")+1,(void**)&prefix) && IS_STRING == Z_TYPE_PP(prefix) ){
		char	*name;
		CTree_getConfigs(config,"name",&name TSRMLS_CC);
		CTree_paddingLeftPrefix(name,Z_STRVAL_PP(prefix),&endArray TSRMLS_CC);
		efree(name);
	}

	ZVAL_ZVAL(return_value,endArray,1,0);

	zval_ptr_dtor(&endArray);
	zval_ptr_dtor(&returnArray);
	zval_ptr_dtor(&config);
}


PHP_METHOD(CTree,getTreeArray)
{
	zval	*array,
			*returnArray,
			*option = NULL,
			*config,
			*endArray,
			**prefix;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a|a",&array,&option) == FAILURE){
		RETURN_FALSE;
	}

	CTree_getConfigDefault(getThis(),&config TSRMLS_CC);
	if(option != NULL && IS_ARRAY == Z_TYPE_P(option)){
		zend_hash_merge(Z_ARRVAL_P(config), Z_ARRVAL_P(option), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval*), 1);
	}

	CTree_buildData(array,config,&returnArray,getThis() TSRMLS_CC);
	CTree_makeTreeCore(config,0,returnArray,&endArray,getThis(),1 TSRMLS_CC);


	ZVAL_ZVAL(return_value,endArray,1,0);

	zval_ptr_dtor(&endArray);
	zval_ptr_dtor(&returnArray);
	zval_ptr_dtor(&config);
}