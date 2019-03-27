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
#include "php_CHooks.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include "php_CDebug.h"


//zend类方法
zend_function_entry CHooks_functions[] = {
	PHP_ME(CHooks,loadPlugin,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,_getPathFile,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getHooksRegisterList,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getPluginLoadSuccess,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getPluginLoadFail,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,callHooks,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,getHooksList,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,_setHooksFunctionLevel,NULL,ZEND_ACC_PRIVATE | ZEND_ACC_STATIC)
	PHP_ME(CHooks,registerHook,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CHooks,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CHooks)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHooks",CHooks_functions);
	CHooksCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量_pluginList _hooks _failLoadPluginList
	zend_declare_property_null(CHooksCe, ZEND_STRL("_pluginList"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_hooks"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_failLoadPluginList"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CHooksCe, ZEND_STRL("_allHooks"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}

zend_function_entry CDataObject_functions[] = {
	PHP_ME(CDataObject,asArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDataObject,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CDataObject,get,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//CDataObject
CMYFRAME_REGISTER_CLASS_RUN(CDataObject)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CDataObject",CDataObject_functions);
	CDataObjectCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量_pluginList _hooks _failLoadPluginList
	zend_declare_property_null(CDataObjectCe, ZEND_STRL("data"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//获取目录下的文件
int CHooks_getPathFile(char *path,zval **returnZval TSRMLS_DC)
{
	zval	*dirFile;

	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);

	//判断目录存在
	if(SUCCESS != fileExist(path)){
		return FAILURE;
	}

	//读取目录
	php_scandir(path,&dirFile);

	//遍历目录 去掉特殊符号等
	if(IS_ARRAY == Z_TYPE_P(dirFile)){

		int		i,
				dirNum;

		zval	**nData,
				*filterArray,
				*fileParams;


		MAKE_STD_ZVAL(filterArray);
		array_init(filterArray);

		

		//加入需要过滤目录
		add_next_index_string(filterArray,".",1);
		add_next_index_string(filterArray,"..",1);
		add_next_index_string(filterArray,".svn",1);

		//遍历取出的目录
		dirNum = zend_hash_num_elements(Z_ARRVAL_P(dirFile));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(dirFile));
		for(i = 0 ; i < dirNum ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(dirFile),(void**)&nData);

			if(0 == in_array(Z_STRVAL_PP(nData),filterArray)){
				add_next_index_string(*returnZval,Z_STRVAL_PP(nData),1);
			}

			zend_hash_move_forward(Z_ARRVAL_P(dirFile));
		}

		//清理
		zval_ptr_dtor(&dirFile);
		zval_ptr_dtor(&filterArray);
		return SUCCESS;
	}

	zval_ptr_dtor(&dirFile);
	return FAILURE;
}

void CHooks_loadSystemPlugin(TSRMLS_D){

	zval	*pluginObject;

	MAKE_STD_ZVAL(pluginObject);
	object_init_ex(pluginObject,CDebugCe);

	//调用其构造函数
	if (CDebugCe->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, CDebugCe->constructor->common.function_name, 0);
		call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//调用setHooks注册函数
	MODULE_BEGIN
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, "setHooks", 0);
		call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	MODULE_END

	//save to instance static
	zend_update_static_property(CDebugCe,ZEND_STRL("instance"),pluginObject TSRMLS_CC);

	zval_ptr_dtor(&pluginObject);
}

//载入插件
void CHooks_loadPlugin(TSRMLS_D){
	
	zval	*returnZval,
			*cconfigInstanceZval,
			*loadPluginZval,
			*loadPluginListZval,
			*loadPluginPathConfZval,
			*codePath,
			*appPath,
			*pluginPathZval;

	char	*pluginPath;

	zend_class_entry **cwebClassEntry;

	zend_hash_find(EG(class_table),"cwebapp",strlen("cwebapp")+1,(void**)&cwebClassEntry);
	codePath = zend_read_static_property(*cwebClassEntry,ZEND_STRL("code_path"),0 TSRMLS_CC);
	appPath = zend_read_static_property(*cwebClassEntry,ZEND_STRL("app_path"),0 TSRMLS_CC);

	//获取LOAD_PLUGIN和LOAD_LIST两项配置
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("LOAD_PLUGIN",cconfigInstanceZval,&loadPluginZval TSRMLS_CC);

	CConfig_load("LOAD_LIST",cconfigInstanceZval,&loadPluginListZval TSRMLS_CC);
	CConfig_load("PLUGIN_PATH",cconfigInstanceZval,&loadPluginPathConfZval TSRMLS_CC);

	if(IS_BOOL != Z_TYPE_P(loadPluginZval)){
		zval_ptr_dtor(&loadPluginZval);
		zval_ptr_dtor(&loadPluginListZval);
		zval_ptr_dtor(&loadPluginPathConfZval);
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}

	//不加载插件
	if(IS_BOOL == Z_TYPE_P(loadPluginZval) && Z_LVAL_P(loadPluginZval) == 0){
		zval_ptr_dtor(&loadPluginZval);
		zval_ptr_dtor(&loadPluginListZval);
		zval_ptr_dtor(&loadPluginPathConfZval);
		zval_ptr_dtor(&cconfigInstanceZval);
		return;
	}


	//插件路径
	if(IS_STRING == Z_TYPE_P(loadPluginPathConfZval) && Z_STRLEN_P(loadPluginPathConfZval) > 0 ){
		strcat2(&pluginPath,Z_STRVAL_P(appPath),"/",Z_STRVAL_P(loadPluginPathConfZval),"/",NULL);
	}else{
		strcat2(&pluginPath,Z_STRVAL_P(appPath),"/plugins/",NULL);
	}
	zval_ptr_dtor(&loadPluginPathConfZval);


	//保存插件路径变量
	zend_update_static_property_string(CWebAppCe,ZEND_STRL("cplugin_path"), pluginPath TSRMLS_CC);
	pluginPathZval = zend_read_static_property(CWebAppCe,ZEND_STRL("cplugin_path"),0 TSRMLS_CC);


	//加载全部插件
	if(IS_NULL == Z_TYPE_P(loadPluginListZval)){
		
		zval	*pluginDirList,
				**pluginNameZval,
				*loadFailList,
				*loadSuccessList;

		int		pluginNum,
				i,
				loadFileStatus;

		char	pluginConfigFileName[1024];


		MAKE_STD_ZVAL(loadSuccessList);
		array_init(loadSuccessList);

		MAKE_STD_ZVAL(loadFailList);
		array_init(loadFailList);

		//获取目录下的所有插件文件夹
		CHooks_getPathFile(pluginPath,&pluginDirList TSRMLS_CC);

		if(IS_ARRAY != Z_TYPE_P(pluginDirList)){
			zval_ptr_dtor(&loadFailList);
			zval_ptr_dtor(&loadSuccessList);
			return;
		}

		//依次读取插件
		pluginNum = zend_hash_num_elements(Z_ARRVAL_P(pluginDirList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(pluginDirList));
		for(i = 0 ; i < pluginNum ; i++)
		{
			zend_hash_get_current_data(Z_ARRVAL_P(pluginDirList),(void**)&pluginNameZval);

			//获取插件的主配置文件
			sprintf(pluginConfigFileName,"%s%s%s%s%s",Z_STRVAL_P(pluginPathZval),Z_STRVAL_PP(pluginNameZval),"/",Z_STRVAL_PP(pluginNameZval),".php");

			//配置文件不存在
			if(SUCCESS != fileExist(pluginConfigFileName)){
				//将此计入加载失败列表
				zval	*thisFailData;
				add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
				zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
				continue;
			}

			//尝试加载插件主配置文件 
			loadFileStatus = CLoader_loadFile(pluginConfigFileName);

			//尝试实例化该插件文件
			if(SUCCESS == loadFileStatus){
				zend_class_entry	**thisClass,
									*thisClassP;

				char *className;

				zval *pluginObject;

				//类名
				className = estrdup(Z_STRVAL_PP(pluginNameZval));
				php_strtolower(className,strlen(className)+1);

				//查找类
				if(zend_hash_find(EG(class_table),className,strlen(className)+1,(void**)&thisClass ) == FAILURE){
					//如果找不到类则发出一条警告
					php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] lose base define Class",className);
					zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
					efree(className);
					continue;
				}

				//取地址
				thisClassP = *thisClass;
				
				//检查是否继承CPlugin
				if(thisClassP->parent){
					char *parentName;
					parentName = estrdup(thisClassP->parent->name);
					php_strtolower(parentName,strlen(parentName)+1);
					if(strcmp(parentName,"cplugin") != 0){
						php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] must extends the class CPlugin",Z_STRVAL_PP(pluginNameZval));
						zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
						//计入加载失败列表
						add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
						efree(className);
						continue;
					}
					efree(parentName);
				}else{
					php_error_docref(NULL TSRMLS_CC,E_NOTICE,"Plugin[%s] must extends the class CPlugin",Z_STRVAL_PP(pluginNameZval));
					zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
					//计入加载失败列表
					add_next_index_string(loadFailList,Z_STRVAL_PP(pluginNameZval),1);
					efree(className);
					continue;
				}

				//实例化该插件
				MAKE_STD_ZVAL(pluginObject);
				object_init_ex(pluginObject,thisClassP);

				//调用其构造函数
				if (thisClassP->constructor) {
					zval constructReturn;
					zval constructVal;
					INIT_ZVAL(constructVal);
					ZVAL_STRING(&constructVal, thisClassP->constructor->common.function_name, 0);
					call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
					zval_dtor(&constructReturn);
				}

				//调用setHooks注册函数
				MODULE_BEGIN
					zval constructReturn;
					zval constructVal;
					INIT_ZVAL(constructVal);
					ZVAL_STRING(&constructVal, "setHooks", 0);
					call_user_function(NULL, &pluginObject, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
					zval_dtor(&constructReturn);
				MODULE_END

				zval_ptr_dtor(&pluginObject);

				//将函数记入插件列表
				add_next_index_string(loadSuccessList,Z_STRVAL_PP(pluginNameZval),1);


				efree(className);
			}

			zend_hash_move_forward(Z_ARRVAL_P(pluginDirList));
		}

		//将成功和失败的更新至类变量
		zend_update_static_property(CHooksCe,ZEND_STRL("_pluginList"),loadSuccessList TSRMLS_CC);
		zend_update_static_property(CHooksCe,ZEND_STRL("_failLoadPluginList"),loadFailList TSRMLS_CC);

		zval_ptr_dtor(&pluginDirList);
		zval_ptr_dtor(&loadFailList);
		zval_ptr_dtor(&loadSuccessList);
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&loadPluginZval);
	zval_ptr_dtor(&loadPluginListZval);

	efree(pluginPath);

	//全无的情况返回失败
	return;
}

//析构函数
PHP_METHOD(CHooks,__destruct){

}


//类方法:创建应用对象
PHP_METHOD(CHooks,loadPlugin)
{
	CHooks_loadPlugin(TSRMLS_C);
	RETVAL_LONG(1);
}


//读取目录下的文件
PHP_METHOD(CHooks,_getPathFile)
{
	char	*path;
	int		pathLen;

	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&path,&pathLen) == FAILURE){
		return;
	}

	CHooks_getPathFile(path,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

//获取已注册的钩子列表
PHP_METHOD(CHooks,getHooksRegisterList)
{
	//返回_hooks变量
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//获取已注册的插件列表
PHP_METHOD(CHooks,getPluginLoadSuccess)
{
	//返回_pluginList变量
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_pluginList"), 0 TSRMLS_CC);

	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//调用Hooks的调用等级
void CHooks_setHooksFunctionLevel(zval *functionList,zval **levelList TSRMLS_DC){

	zval **table1Zval,
		 **table2Zval,
		 *addKey,
		 *saveSortZval;
	HashTable *sortArr;
	int	i,
		j,
		num1,
		num2;
	char *key1,
		 *key2,
		 *addKeyStr;
	ulong ukey1,
		  ukey2;

	ALLOC_HASHTABLE(sortArr);
	zend_hash_init(sortArr,8,NULL,NULL,0);

	num1 = zend_hash_num_elements(Z_ARRVAL_P(functionList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(functionList));
	for(i = 0 ; i < num1 ;i++){
		zend_hash_get_current_key(Z_ARRVAL_P(functionList),&key1,&ukey1,0);
		zend_hash_get_current_data(Z_ARRVAL_P(functionList),(void**)&table1Zval);
		
		num2 = zend_hash_num_elements(Z_ARRVAL_PP(table1Zval));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(table1Zval));
		for(j = 0 ; j < num2 ; j++){	
			zend_hash_get_current_key(Z_ARRVAL_PP(table1Zval),&key2,&ukey2,0);
			zend_hash_get_current_data(Z_ARRVAL_PP(table1Zval),(void**)&table2Zval);
			
			//新增一个key值
			strcat2(&addKeyStr,key1,"|",key2,NULL);
			add_assoc_string(*table2Zval,"key",addKeyStr,1);
			efree(addKeyStr);

			//向sortArr填充值
			zend_hash_next_index_insert(sortArr,&*table2Zval,sizeof(zval*),NULL);

			zend_hash_move_forward(Z_ARRVAL_PP(table1Zval));
		}


		zend_hash_move_forward(Z_ARRVAL_P(functionList));
	}

	MAKE_STD_ZVAL(saveSortZval);
	Z_TYPE_P(saveSortZval) = IS_ARRAY;
	Z_ARRVAL_P(saveSortZval) = sortArr;
	CArraySort_sortArrayDesc(saveSortZval,"callLevel",&*levelList TSRMLS_CC);
	zval_ptr_dtor(&saveSortZval);
}

//获取注册失败的 
PHP_METHOD(CHooks,getPluginLoadFail)
{
	//返回_failLoadPluginList变量
	zval *hooksZval;

	hooksZval = zend_read_static_property(CHooksCe,ZEND_STRL("_failLoadPluginList"), 0 TSRMLS_CC);
	ZVAL_ZVAL(return_value,hooksZval,1,0);
}

//调用注册到Hooks的函数
void CHooks_callHooks(char *hookName,zval* paramList[],zend_uint paramNum TSRMLS_DC){

	int n,
		funNum;

	zval *hooksList,
		 *hooksNameZval,
		 **thisHooksFunctionList,
		 *callFunctionSort,
		 **callFunctionData,
		 **keyData,
		 **callObject,
		 **callNum,
		 hookCallAction,
		 hookCallRetrun;

	char *hooksKeyName = NULL,
		 *pluginName = NULL,
		 *functionName = NULL;

	zend_function *callFunc;

	//读取自身Hooks变量
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(hooksList)){
		return;
	}

	//获取该Hooks下的函数
	if(zend_hash_find(Z_ARRVAL_P(hooksList),hookName,strlen(hookName)+1,(void**)&thisHooksFunctionList) == FAILURE){
		return;
	}

	//按照call_level重新对待调用函数队列排序
	CHooks_setHooksFunctionLevel(*thisHooksFunctionList,&callFunctionSort TSRMLS_CC);

	//遍历待调用函数列表
	funNum = zend_hash_num_elements(Z_ARRVAL_P(callFunctionSort));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(callFunctionSort));
	for(n = 0 ; n < funNum; n++){
		zend_hash_get_current_data(Z_ARRVAL_P(callFunctionSort),(void**)&callFunctionData);
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"key",strlen("key")+1,(void**)&keyData);

		hooksKeyName = estrdup(Z_STRVAL_PP(keyData));
		pluginName = strtok(hooksKeyName,"|");
		functionName = strtok(NULL,"|");
		if(pluginName == NULL || functionName == NULL){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//判断提供调用的对象正确 callObject
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"callObject",strlen("callObject")+1,(void**)&callObject);
		if(IS_OBJECT != Z_TYPE_PP(callObject)){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//调用次数
		zend_hash_find(Z_ARRVAL_PP(callFunctionData),"callNum",strlen("callNum")+1,(void**)&callNum);

		//判断该对象中是否存在该方法
		php_strtolower(functionName,strlen(functionName)+1);

		if( ! zend_hash_exists(& (Z_OBJCE_PP(callObject)->function_table),functionName,strlen(functionName)+1) ){
			zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
			efree(hooksKeyName);
			continue;
		}

		//调用该函数
		INIT_ZVAL(hookCallAction);
		ZVAL_STRING(&hookCallAction,functionName,0);

		//调用钩子函数
		if(SUCCESS == call_user_function(NULL,callObject,&hookCallAction,&hookCallRetrun, paramNum ,paramList TSRMLS_CC)){
			int updateCallNum = 1;
			zval *updateCallNumZval;
			if(IS_LONG == Z_TYPE_PP(callNum)){
				Z_LVAL_PP(callNum) = Z_LVAL_PP(callNum) + 1;
			}
		}

		zval_dtor(&hookCallRetrun);
		zend_hash_move_forward(Z_ARRVAL_P(callFunctionSort));
		efree(hooksKeyName);
	}

	zval_ptr_dtor(&callFunctionSort);
}

//调用注册到Hooks的函数
PHP_METHOD(CHooks,callHooks)
{
	int argc = ZEND_NUM_ARGS(),
		i,
		n,
		funNum;

	zval ***args,
		 *paramList[32],
		 thisVal;

	char *hookName = NULL;

	args = (zval***)safe_emalloc(argc,sizeof(zval**),0);

	if(ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc,args) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[HooksException] Call the Hooks function when you need to specify the name of the hook", 7001 TSRMLS_CC);
		efree(args);
		return;
	}else{
		//处理任意数目参数
		for(i = 0 ; i < argc ; i++){
			if(i == 0){
				//布置的Hooks名字
				if(IS_STRING != Z_TYPE_PP(args[i])){
					continue;
				}
				hookName = Z_STRVAL_PP(args[i]);
			}else{
				//依次获取需要传递的参数
				MAKE_STD_ZVAL(paramList[i-1]);
				ZVAL_ZVAL(paramList[i-1],*args[i],1,0);
			}
		}
	}

	CHooks_callHooks(hookName,paramList,i-1 TSRMLS_CC);
	efree(args);
	for(n = 0 ; n < i-1;n++){
		zval_ptr_dtor(&paramList[n]);
	}
}

PHP_METHOD(CHooks,getHooksList)
{
	zval *allHookList;
	allHookList = zend_read_static_property(CHooksCe,ZEND_STRL("_allHooks"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(allHookList)){
		RETURN_ZVAL(allHookList,1,0);
	}


	//生成插件列表
	MAKE_STD_ZVAL(allHookList);
	array_init(allHookList);

	//记录Hooks
	add_assoc_string(allHookList,"HOOKS_ROUTE_START","Framework parse URL request before triggering this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ROUTE_END","Framework parse URL request is completed to trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ROUTE_ERROR","Frameword unable to route request will triggered this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_CONTROLLER_INIT","Framework trigger this Hooks after init controller",0);
	add_assoc_string(allHookList,"HOOKS_ACTION_INIT","Framework triggering this Hooks after action has run",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_BEFORE","Framework query database before triggering this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_END","Framework trigger this Hooks after query database",0);
	add_assoc_string(allHookList,"HOOKS_EXECUTE_ERROR","Framework catched CDbException will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_ERROR_HAPPEN","Framework catched Fatal error will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_EXCEPTION_HAPPEN","Framework catched Exception will trigger this Hooks",0);
	add_assoc_string(allHookList,"HOOKS_SYSTEM_SHUTDOWN","Framework trigger this Hooks before system destory the request",0);
	add_assoc_string(allHookList,"HOOKS_CACHE_SET","Framework trigger this Hooks before write cache",0);
	add_assoc_string(allHookList,"HOOKS_CACHE_GET","Framework trigger this Hooks before read cache",0);
	add_assoc_string(allHookList,"HOOKS_LOADER_START","Framework trigger this Hooks before system init ClassLoader",0);
	add_assoc_string(allHookList,"HOOKS_VIEW_GET","Framework trigger this Hooks before system init ViewTemplate",0);
	add_assoc_string(allHookList,"HOOKS_VIEW_SHOW","Framework trigger this Hooks before display a webPage",0);
	add_assoc_string(allHookList,"HOOKS_URL_CREATE","Framework trigger this Hooks before system create a url",0);


	zend_update_static_property(CHooksCe,ZEND_STRL("_allHooks"),allHookList TSRMLS_CC);
	RETURN_ZVAL(allHookList,1,0);
}

PHP_METHOD(CHooks,_setHooksFunctionLevel)
{
	
}


//销毁变量
void CHooks_destruct(TSRMLS_D){

	

}


void CHooks_registerHooks(char *hooksName,char *runFunctionName,zval *runObject,int callLevel TSRMLS_DC){


	char				*callClassName;
	zval				*hooksList,
						**hooksKeyList,
						**classNameKeyList,
						**runFunctionKeyList;

	zend_class_entry	*callObjectCe;

	//获取类名
	callObjectCe = Z_OBJCE_P(runObject);
	callClassName = estrdup(callObjectCe->name);

	//设置钩子
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(hooksList)){
		array_init(hooksList);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),hooksList TSRMLS_CC);
	}

	//判断有无Hooks_name的hooksName-className-runFunction的key
	if(!zend_hash_exists(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(hooksList,hooksName,strlen(hooksName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1,(void**)&hooksKeyList);
	

	//判断有无className的key
	if(!zend_hash_exists(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*hooksKeyList,callClassName,strlen(callClassName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1,(void**)&classNameKeyList);

	//判断有无runFunction的key
	if(!zend_hash_exists(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*classNameKeyList,runFunctionName,strlen(runFunctionName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&runFunctionKeyList);



	//尝试对runFunctionKeyList更新hooks记录
	MODULE_BEGIN
		zval	*thisAddVal,
				*callFunctionKeyData,
				**callStatus;
		

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&callStatus)){
			
			//调用次数
			add_assoc_long(*callStatus,"callNum",0);

			//调用对象
			MAKE_STD_ZVAL(thisAddVal);
			ZVAL_ZVAL(thisAddVal,runObject,1,0);
			add_assoc_zval_ex(*callStatus,"callObject",strlen("callObject")+1,thisAddVal);

			//调用等级
			add_assoc_long(*callStatus,"callLevel",callLevel);
	
		}

	MODULE_END

	//将该数据保存在_hooks中
	MODULE_BEGIN
		zval *saveHooks;
		MAKE_STD_ZVAL(saveHooks);
		ZVAL_ZVAL(saveHooks,hooksList,1,0);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),saveHooks TSRMLS_CC);
		zval_ptr_dtor(&saveHooks);
	MODULE_END

	efree(callClassName);

}


//注册Hooks
PHP_METHOD(CHooks,registerHook)
{

	char	*hooksName,
			*runFunctionName,
			*callClassName;

	int		hooksNameLen,
			runFunctionNameLen;
	long	callLevel = 0;

	zval	*runObject,
			*hooksList,
			**hooksKeyList,
			**classNameKeyList,
			**runFunctionKeyList;

	zend_class_entry	*callObjectCe;


	//接受4个参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssz|l",&hooksName,&hooksNameLen,&runFunctionName,&runFunctionNameLen,&runObject,&callLevel) == FAILURE){
		RETVAL_FALSE;
		return;
	}


	//检测runObject是否是一个对象 不是则抛出异常
	if(IS_OBJECT != Z_TYPE_P(runObject)){
		zend_throw_exception(CPluginExceptionCe, "[HooksException] Registered hook function transfer the call object error [param 3]", "" TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	//获取类名
	callObjectCe = Z_OBJCE_P(runObject);
	callClassName = estrdup(callObjectCe->name);

	//设置钩子
	hooksList = zend_read_static_property(CHooksCe,ZEND_STRL("_hooks"), 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(hooksList)){
		array_init(hooksList);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),hooksList TSRMLS_CC);
	}

	//判断有无Hooks_name的hooksName-className-runFunction的key
	if(!zend_hash_exists(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(hooksList,hooksName,strlen(hooksName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_P(hooksList),hooksName,strlen(hooksName)+1,(void**)&hooksKeyList);
	

	//判断有无className的key
	if(!zend_hash_exists(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*hooksKeyList,callClassName,strlen(callClassName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(hooksKeyList),callClassName,strlen(callClassName)+1,(void**)&classNameKeyList);

	//判断有无runFunction的key
	if(!zend_hash_exists(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1)){
		//添加一个HashTable
		zval *hooksNameList;
		MAKE_STD_ZVAL(hooksNameList);
		array_init(hooksNameList);
		add_assoc_zval_ex(*classNameKeyList,runFunctionName,strlen(runFunctionName)+1,hooksNameList);
	}
	zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&runFunctionKeyList);



	//尝试对runFunctionKeyList更新hooks记录
	MODULE_BEGIN
		zval	*thisAddVal,
				*callFunctionKeyData,
				**callStatus;
		

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(classNameKeyList),runFunctionName,strlen(runFunctionName)+1,(void**)&callStatus)){
			
			//调用次数
			add_assoc_long(*callStatus,"callNum",0);

			//调用对象
			MAKE_STD_ZVAL(thisAddVal);
			ZVAL_ZVAL(thisAddVal,runObject,1,0);
			add_assoc_zval_ex(*callStatus,"callObject",strlen("callObject")+1,thisAddVal);

			//调用等级
			add_assoc_long(*callStatus,"callLevel",callLevel);
	
		}

	MODULE_END

	//将该数据保存在_hooks中
	MODULE_BEGIN
		zval *saveHooks;
		MAKE_STD_ZVAL(saveHooks);
		ZVAL_ZVAL(saveHooks,hooksList,1,0);
		zend_update_static_property(CHooksCe,ZEND_STRL("_hooks"),saveHooks TSRMLS_CC);
		zval_ptr_dtor(&saveHooks);
	MODULE_END

	efree(callClassName);

	RETVAL_TRUE;
}

PHP_METHOD(CDataObject,asArray)
{
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	RETVAL_ZVAL(thisVal,1,0);
}

CHooks_setDataObject(zval *object,zval *data TSRMLS_DC){
	zend_update_property(CDataObjectCe,object,ZEND_STRL("data"),data TSRMLS_CC);
}

CHooks_getDataObject(zval *object,zval **returnData TSRMLS_DC){
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,object,ZEND_STRL("data"),0 TSRMLS_CC);
	MAKE_STD_ZVAL(*returnData);
	ZVAL_ZVAL(*returnData,thisVal,1,0);
}

PHP_METHOD(CDataObject,set)
{
	zval	*data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	//setter
	CHooks_setDataObject(getThis(),data TSRMLS_CC);
	RETVAL_TRUE;
}

PHP_METHOD(CDataObject,get)
{
	zval	*thisVal;
	thisVal = zend_read_property(CDataObjectCe,getThis(),ZEND_STRL("data"),0 TSRMLS_CC);
	RETVAL_ZVAL(thisVal,1,0);
}