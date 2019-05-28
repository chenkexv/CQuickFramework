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
#include "php_CLoader.h"
#include "php_CWebApp.h"
#include "php_CRoute.h"


//zend类方法
zend_function_entry CLoader_functions[] = {
	PHP_ME(CLoader,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CLoader,importFile,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CLoader,load,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CLoader,registerClassMap,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CLoader)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CLoader",CLoader_functions);
	CLoaderCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CLoaderCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CLoaderCe, ZEND_STRL("_loadMapp"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	return SUCCESS;
}

//引入执行文件
int CLoader_loadFile(char *path){

	zend_file_handle file_handle;
    zend_op_array         *op_array;
    char realpath[MAXPATHLEN];

	TSRMLS_FETCH();

	if (!VCWD_REALPATH(path, realpath)) {
		return -1;
    }

	//判断文件存在
	if( fileExist(realpath) == FAILURE ){
		php_error_docref(NULL TSRMLS_CC,E_WARNING,"Try to introduce a non-existent file");
		return -1;
	}

	file_handle.filename = realpath;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE_ONCE TSRMLS_CC);

	if (op_array && file_handle.handle.stream.handle) {
		int dummy = 1;
        if (!file_handle.opened_path) {
			file_handle.opened_path = realpath;
        }

        zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1, (void *)&dummy, sizeof(int), NULL);
	}

    zend_destroy_file_handle(&file_handle TSRMLS_CC);

    if (op_array) {
		zval *result = NULL;
		
		CMYFRAME_STORE_EG_ENVIRON();

        EG(return_value_ptr_ptr) = &result;
        EG(active_op_array) = op_array;

#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)) || (PHP_MAJOR_VERSION > 5)
                if (!EG(active_symbol_table)) {
                        zend_rebuild_symbol_table(TSRMLS_C);
                }
#endif

        zend_execute(op_array TSRMLS_CC);
        destroy_op_array(op_array TSRMLS_CC);
        efree(op_array);
        if (!EG(exception)) {
			if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
				zval_ptr_dtor(EG(return_value_ptr_ptr));
            }
        }          
	
		CMYFRAME_RESTORE_EG_ENVIRON();
        return 0;
	}

	return -1;
}

//加载文件返回内容
void CLoader_getFile(char *path,zval **returnData TSRMLS_DC){

	zval	*retval;
	zend_file_handle zfd;

	zfd.type = ZEND_HANDLE_FILENAME;
	zfd.filename = path;
	zfd.free_filename = 0;
	zfd.opened_path = NULL;

	zend_execute_scripts(ZEND_INCLUDE TSRMLS_CC,&retval,1,&zfd); 

	MAKE_STD_ZVAL(*returnData);
	ZVAL_ZVAL(*returnData,retval,1,1);
}



//引入类
void CLoader_import(char *className,char *classPath,zval **returnZval TSRMLS_DC)
{

	//检查文件存在
	if(SUCCESS != fileExist(classPath)){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_BOOL(*returnZval,0);
		return;
	}


	//尝试引入文件
	CLoader_getFile(classPath,&*returnZval TSRMLS_CC);
	return;
}

PHP_METHOD(CLoader,importFile)
{
	char *filePath;
	int filePathLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&filePath,&filePathLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CMyFrameException] CMyFrame call CLoader::import() method, parameter error");
		return;
	}

	CLoader_loadFile(filePath);
	RETVAL_TRUE;
}

//引入类
PHP_METHOD(CLoader,import)
{
	char	*className,
			*classPath;

	int		classNameLen,
			classPathLen;

	zval	*returnZval;

	//获取配置参数
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&className,&classNameLen,&classPath,&classPathLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CMyFrameException] CMyFrame call CLoader::import() method, parameter error");
		return;
	}
}

void CLoader_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CLoaderCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//查询自身类对象
		zend_hash_find(EG(class_table),"cloader",strlen("cloader")+1,(void**)&classCePP);
		classCe = *classCePP;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);

		//执行构造器
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_update_static_property(CLoaderCe,ZEND_STRL("instance"),saveObject TSRMLS_CC);
		zval_ptr_dtor(&saveObject);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);

		//触发完成CLoader对象时Hooks函数HOOKS_LOADER_START
		MODULE_BEGIN
			zval	*paramsList[1],
					param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],object,1,0);
			CHooks_callHooks("HOOKS_LOADER_START",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		MODULE_END

		zval_ptr_dtor(&object);

		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
}

//类方法:创建应用对象
PHP_METHOD(CLoader,getInstance)
{
	zval *instanceZval;
	CLoader_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,1);
}

//搜寻文件
void CLoader_load(char *className,zval **returnZval TSRMLS_DC)
{
	zval	*classMapZval,
			*cconfigInstanceZval,
			*importConfigItem,
			*codePath,
			*appPath,
			*moduleZval;

	int		existClassMap = 0;

	char	moduleControllerPath[1024],
			moduleClassPath[1024],
			moduleModelPath[1024],
			mainControllerPath[1024],
			mainClassPath[1024],
			mainModelPath[1024],
			cmyFrameCom[1024];

	MAKE_STD_ZVAL(*returnZval);

	//appPath
	codePath = zend_read_static_property(CWebAppCe,ZEND_STRL("code_path"),0 TSRMLS_CC);
	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);
	moduleZval = zend_read_static_property(CRouteCe,ZEND_STRL("thisModule"),0 TSRMLS_CC);

	//检查类名中是否有_下划线
	MODULE_BEGIN
		char	*copyClassName,
				*tempClassName;
		copyClassName = estrdup(className);
		if(strstr(copyClassName,"_") != NULL){
			str_replace("_","/",copyClassName,&className);
		}
		efree(copyClassName);
	MODULE_END

	//策略一:寻找映射表的地址
	classMapZval = zend_read_static_property(CLoaderCe,ZEND_STRL("_loadMapp"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(classMapZval) ){
		//判断是否存在类映射表
		if(zend_hash_exists(Z_ARRVAL_P(classMapZval),className,strlen(className)+1)){
			existClassMap = 1;
		}
	}

	//策略二:所处模块下控制器
	sprintf(moduleControllerPath,"%s%s%s%s%s%s",Z_STRVAL_P(appPath),"/modules/",Z_STRVAL_P(moduleZval),"/controllers/",className,".php");

	//策略三:所处模块下类目
	sprintf(moduleClassPath,"%s%s%s%s%s%s",Z_STRVAL_P(appPath),"/modules/",Z_STRVAL_P(moduleZval),"/classes/",className,".php");

	//策略四:所处模块下模型
	sprintf(moduleModelPath,"%s%s%s%s%s%s",Z_STRVAL_P(appPath),"/modules/",Z_STRVAL_P(moduleZval),"/models/",className,".php");

	//策略五:主控制器
	sprintf(mainControllerPath,"%s%s%s%s",Z_STRVAL_P(codePath),"/controllers/",className,".php");

	//策略六:主类目
	sprintf(mainClassPath,"%s%s%s%s",Z_STRVAL_P(codePath),"/classes/",className,".php");

	//策略七:CMyFrame PHP版本
	sprintf(cmyFrameCom,"%s%s%s%s",Z_STRVAL_P(appPath),"/Framework/components/",className,".php");

	//策略八:主模型
	sprintf(mainModelPath,"%s%s%s%s",Z_STRVAL_P(codePath),"/models/",className,".php");


	//存在映射路径
	if(1 == existClassMap){
		//取出映射中的地址
		zval **classPathSaved;
		zend_hash_find(Z_ARRVAL_P(classMapZval),className,strlen(className)+1,(void**)&classPathSaved);
		if(IS_STRING != Z_TYPE_PP(classPathSaved)){
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CMyFrameFatal]CMyFrame try to Call CLoader::load() method, loss of HashTable references");
			ZVAL_BOOL(*returnZval,0);
			return;
		}
		//判断文件是否存在
		if(SUCCESS != fileExist(Z_STRVAL_PP(classPathSaved))){
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[ClassNotFound]ClassPathMap in Class [%s] specified path [%s] does not exist",className,Z_STRVAL_PP(classPathSaved));
			ZVAL_BOOL(*returnZval,0);
			return;
		}
		//引入文件
		CLoader_loadFile(Z_STRVAL_PP(classPathSaved));
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(moduleControllerPath)){
		//模块控制器
		CLoader_loadFile(moduleControllerPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(moduleClassPath)){
		//模块类目
		CLoader_loadFile(moduleClassPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(moduleModelPath)){
		//模块模型
		CLoader_loadFile(moduleModelPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(mainControllerPath)){
		//主控制器
		CLoader_loadFile(mainControllerPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(mainClassPath)){
		//主类目
		CLoader_loadFile(mainClassPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(cmyFrameCom)){
		//PHP版本
		CLoader_loadFile(cmyFrameCom);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else if(SUCCESS == fileExist(mainModelPath)){
		//主模型
		CLoader_loadFile(mainModelPath);
		ZVAL_BOOL(*returnZval,1);
		return;
	}else{
		//配置文件中的其他import路径
		zval	*needTayList;
		int		importNum,
				i;
		zval	**importVal,
				**importCheckPath;

		char	*importPath,
				checkClassPath[10240];

		//策略九:配置文件中加载的路径
		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("IMPORT",cconfigInstanceZval,&importConfigItem TSRMLS_CC);
		zval_ptr_dtor(&cconfigInstanceZval);

		if(IS_ARRAY != Z_TYPE_P(importConfigItem) ){
			ZVAL_BOOL(*returnZval,0);
			zval_ptr_dtor(&importConfigItem);
			return;
		}

		MAKE_STD_ZVAL(needTayList);
		array_init(needTayList);

		//将import路径中的值换成目录
		importNum = zend_hash_num_elements(Z_ARRVAL_P(importConfigItem));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(importConfigItem));
		for(i = 0 ; i < importNum ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(importConfigItem),(void**)&importVal);
			
			if(IS_STRING == Z_TYPE_PP(importVal)){
				char	*importTemp;
				zval	*thisAddZval;
				char *importPathStr;
				str_replace(".","/",Z_STRVAL_PP(importVal),&importPathStr);
				str_replace("*","",importPathStr,&importPath);
				strcat2(&importTemp,Z_STRVAL_P(appPath),"/",importPath,NULL);
				efree(importPathStr);
				efree(importPath);
				add_next_index_string(needTayList,importTemp,1);
				efree(importTemp);
			}
			zend_hash_move_forward(Z_ARRVAL_P(importConfigItem));
		}

		//遍历所有待加载目录
		importNum = zend_hash_num_elements(Z_ARRVAL_P(needTayList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(needTayList));
		for(i = 0 ; i < importNum ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(needTayList),(void**)&importCheckPath);
			sprintf(checkClassPath,"%s%s%s",Z_STRVAL_PP(importCheckPath),className,".php");

			//判断文件是否存在
			if(SUCCESS == fileExist(checkClassPath)){
				CLoader_loadFile(checkClassPath);
				ZVAL_BOOL(*returnZval,1);
				zval_ptr_dtor(&needTayList);
				zval_ptr_dtor(&importConfigItem);
				return;
			}
			zend_hash_move_forward(Z_ARRVAL_P(needTayList));
		}

		zval_ptr_dtor(&needTayList);

	}


	//返回false
	zval_ptr_dtor(&importConfigItem);
	ZVAL_BOOL(*returnZval,0);
	return;
}

//搜寻文件
PHP_METHOD(CLoader,load)
{
	char	*className;
	int		classNameLen;
	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&className,&classNameLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CLoader_load(className,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

void CLoader_registerClassMap(char *className,char *classPath TSRMLS_DC)
{
	zval	*classMap,
			*savePathZval;

	//获取类映射数组

	classMap = zend_read_static_property(CLoaderCe,ZEND_STRL("_loadMapp"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(classMap)){
		zval *saveClassMap;
		MAKE_STD_ZVAL(saveClassMap);
		array_init(saveClassMap);
		zend_update_static_property(CLoaderCe,ZEND_STRL("_loadMapp"),saveClassMap TSRMLS_CC);
		classMap = zend_read_static_property(CLoaderCe,ZEND_STRL("_loadMapp"),0 TSRMLS_CC);
		zval_ptr_dtor(&saveClassMap);
	}

	//判断有无此类的映射
	add_assoc_string(classMap,className,classPath,1);
}

//注册映射表
PHP_METHOD(CLoader,registerClassMap)
{
	int paramsNum;

	//参数个数
	paramsNum = ZEND_NUM_ARGS();

	//参数数目为2
	if(2 == paramsNum){
		char	*className,
				*classPath;
		int		classNameLen,
				classPathLen;
		zval	*savePathZval;

		zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&className,&classNameLen,&classPath,&classPathLen);
		CLoader_registerClassMap(className,classPath TSRMLS_CC);
		RETVAL_TRUE;

	}else if(1 == paramsNum){
		zval	*regMap,
				**classPathZval;
		int		regNum,
				i;
		char	*className;
		ulong	classUName;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&regMap) == FAILURE){
			RETVAL_FALSE;
			return;
		}

		//不为数组
		if(IS_ARRAY != Z_TYPE_P(regMap)){
			RETVAL_FALSE;
			return;
		}

		//遍历数组
		regNum = zend_hash_num_elements(Z_ARRVAL_P(regMap));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(regMap));
		for(i = 0 ; i < regNum ; i++){

			zend_hash_get_current_data(Z_ARRVAL_P(regMap),(void**)&classPathZval);
			zend_hash_get_current_key(Z_ARRVAL_P(regMap),&className,&classUName,0);

			if(IS_STRING == Z_TYPE_PP(classPathZval)){
				CLoader_registerClassMap(className,Z_STRVAL_PP(classPathZval) TSRMLS_CC);
			}
			zend_hash_move_forward(Z_ARRVAL_P(regMap));
		}
		
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}