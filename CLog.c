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


#include "php_CQuickFramework.h"
#include "php_CLog.h"
#include "php_CException.h"
#include "php_CWebApp.h"



//zend类方法
zend_function_entry CLog_functions[] = {
	PHP_ME(CLog,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CLog,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CLog,save,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CLog,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CLog,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(CLog,write,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CLog,parse,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CLog,writeFile,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CLog)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CLog",CLog_functions);
	CLogCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//全局变量
	zend_declare_property_null(CLogCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CLogCe, ZEND_STRL("params"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CLogCe, ZEND_STRL("logList"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

int CLog_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CLogCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//查询自身类对象
		zend_hash_find(EG(class_table),"clog",strlen("clog")+1,(void**)&classCePP);
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
		zend_update_static_property(CLogCe,ZEND_STRL("instance"),object TSRMLS_CC);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

PHP_METHOD(CLog,getInstance)
{
	zval *instanceZval;

	CLog_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,1);
}

PHP_METHOD(CLog,parse)
{
	//读入一个日志文件 将其拆为数组
	char	*filePath,
			*truePath,
			*fileContent;
	int		filePathLen = 0;
	zval	*appPath,
			*matchResult,
			*returnArray;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&filePath,&filePathLen) == FAILURE){
		array_init(return_value);
		return;
	}

	if(filePathLen == 0){
		zend_throw_exception(CClassNotFoundExceptionCe, "[CClassNotFoundException] Call [CLog->parse] Parameter error , the first params must be a string meaning log name", 5001 TSRMLS_CC);
		return;
	}

	//主目录
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(appPath)){
		array_init(return_value);
		return;
	}

	if( strstr(filePath,Z_STRVAL_P(appPath)) == NULL){
		char tempPath[10240];
		sprintf(tempPath,"%s%s%s",Z_STRVAL_P(appPath),"/",filePath);
		truePath = estrdup(tempPath);
	}else{
		truePath = estrdup(filePath);
	}

	//判断文件存在
	if(FAILURE == fileExist(truePath)){
		char tempMessage[10240];
		zend_throw_exception(CClassNotFoundExceptionCe, "[CClassNotFoundException] Call [CLog->parse] the log file not exists", 5001 TSRMLS_CC);
		return;
	}

	//读取文件
	file_get_contents(truePath,&fileContent);

	MAKE_STD_ZVAL(returnArray);
	array_init(returnArray);

	//拆解
	if(preg_match_all("/#LogTime:(.*)\\nLogContent:(.*)\\n/",fileContent,&matchResult)){

		zval **result1,
			 **result2,
			 **thisKeyString,
			 **thisValString,
			 *thisValArray,
			 *thisSaveArray,
			 **explodeZval,
			 *jsonDecodeVal,
			 *saveHashTable,
			 *saveParamsTable;

		char	*trimKey;

		unsigned long i,j,x,n;
		zend_hash_index_find(Z_ARRVAL_P(matchResult),1,(void**)&result1);
		zend_hash_index_find(Z_ARRVAL_P(matchResult),2,(void**)&result2);


		//遍历日期
		j = zend_hash_num_elements(Z_ARRVAL_PP(result1));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(result1));
		for(i = 0 ; i < j ; i++){

			//键
			zend_hash_index_find(Z_ARRVAL_PP(result1),i,(void**)&thisKeyString);

			//值
			zend_hash_index_find(Z_ARRVAL_PP(result2),i,(void**)&thisValString);

			//explode分割
			php_explode("|LogSplit|",Z_STRVAL_PP(thisValString),&thisValArray);

			//保存值
			MAKE_STD_ZVAL(saveHashTable);
			array_init(saveHashTable);

			MAKE_STD_ZVAL(saveParamsTable);
			array_init(saveParamsTable);


			//插入数组
			x = zend_hash_num_elements(Z_ARRVAL_P(thisValArray));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisValArray));
			for(n = 0 ; n < x; n++){
				//取出值进行json_decode
				zend_hash_get_current_data(Z_ARRVAL_P(thisValArray),(void**)&explodeZval);
				json_decode(Z_STRVAL_PP(explodeZval),&jsonDecodeVal);
				if(zend_hash_num_elements(Z_ARRVAL_P(jsonDecodeVal)) == 0){
					zval_ptr_dtor(&jsonDecodeVal);
					MAKE_STD_ZVAL(jsonDecodeVal);
					ZVAL_STRING(jsonDecodeVal,Z_STRVAL_PP(explodeZval),1);
				}
				add_index_zval(saveParamsTable,n,jsonDecodeVal);
				zend_hash_move_forward(Z_ARRVAL_P(thisValArray));
			}

			php_trim(Z_STRVAL_PP(thisKeyString),"\r\n",&trimKey);
			add_assoc_string(saveHashTable,"date",trimKey,1);
			add_assoc_zval(saveHashTable,"content",saveParamsTable);
			add_next_index_zval(returnArray,saveHashTable);
			efree(trimKey);
			zval_ptr_dtor(&thisValArray);
			zend_hash_move_forward(Z_ARRVAL_PP(result1));
		}
	}

	zval_ptr_dtor(&matchResult);
	efree(truePath);
	efree(fileContent);

	RETVAL_ZVAL(returnArray,1,1);
}

PHP_METHOD(CLog,set)
{
	zval	*logList,
			*content,
			**thisArray,
			*saveContent;

	char *name;
	int nameLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&name,&nameLen,&content) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	logList = zend_read_property(CLogCe,getThis(),ZEND_STRL("logList"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(logList)){
		MAKE_STD_ZVAL(logList);
		array_init(logList);
		zend_update_property(CLogCe,getThis(),ZEND_STRL("logList"),logList TSRMLS_CC);
	}

	//判断存在key
	if(!zend_hash_exists(Z_ARRVAL_P(logList),name,strlen(name) + 1)){
		zval *saveModelList;
		HashTable *saveModelTable;
		ALLOC_HASHTABLE(saveModelTable);
		zend_hash_init(saveModelTable,4,NULL,NULL,0);
		MAKE_STD_ZVAL(saveModelList);
		Z_TYPE_P(saveModelList) = IS_ARRAY;
		Z_ARRVAL_P(saveModelList) = saveModelTable;
		zend_hash_add(Z_ARRVAL_P(logList),name,strlen(name)+1,&saveModelList,sizeof(zval*),NULL);
	}

	if(zend_hash_find(Z_ARRVAL_P(logList),name,strlen(name)+1,(void**)&thisArray) == FAILURE){
		RETVAL_FALSE;
		return;
	}
	
	//向数组末尾追加值
	MAKE_STD_ZVAL(saveContent);
	ZVAL_ZVAL(saveContent,content,1,0);
	zend_hash_next_index_insert(Z_ARRVAL_PP(thisArray),&saveContent,sizeof(zval*),0);
	RETVAL_TRUE;
}

PHP_METHOD(CLog,save)
{
	zval	*logList,
			*appPath;

	char logPath[10240],
		 logFile[10240],
		 *logText,
		 *thisMothDate,
		 *thisMothTime;

	int i,j,m,n;

	char	*logName;
	ulong	iLoginName;
	zval	**logContentList,
			**logContent;

	zval	*fopenHandle;

	logList = zend_read_property(CLogCe,getThis(),ZEND_STRL("logList"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(logList)){
		RETVAL_FALSE;
		return;
	}

	//日志路径
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(appPath)){
		RETVAL_FALSE;
		return;
	}
	php_date("Ym",&thisMothDate);
	sprintf(logPath,"%s%s%s%s",Z_STRVAL_P(appPath),"/logs/userlog/",thisMothDate,"/");
	efree(thisMothDate);

	//创建目录
	if(FAILURE == fileExist(logPath)){
		//尝试创建文件夹
		php_mkdir(logPath);

		//创建失败
		if(FAILURE == fileExist(logPath)){
			RETVAL_FALSE;
			return;
		}
	}

	//遍历日志
	m = zend_hash_num_elements(Z_ARRVAL_P(logList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(logList));
	for( i = 0 ; i < m ; i++){

		zend_hash_get_current_key(Z_ARRVAL_P(logList),&logName,&iLoginName,0);
		zend_hash_get_current_data(Z_ARRVAL_P(logList),(void**)&logContentList);

		//文件路径
		sprintf(logFile,"%s%s%s%s",logPath,"/",logName,".log");

		//打开文件
		php_fopen(logFile,"a+",&fopenHandle);

		//锁定文件
		if (php_flock(fopenHandle,2)){
			n = zend_hash_num_elements(Z_ARRVAL_PP(logContentList));
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(logContentList));
			for(j = 0 ; j < n ; j++){

				smart_str smart_logContentStr = {0};

				zend_hash_get_current_data(Z_ARRVAL_PP(logContentList),(void**)&logContent);

				if(IS_STRING != Z_TYPE_PP(logContent)){
					zend_hash_move_forward(Z_ARRVAL_PP(logContentList));
					continue;
				}

				//写入一行
				php_date("Y-m-d H:i:s",&thisMothTime);
				smart_str_appends(&smart_logContentStr,"#LogTime:");
				smart_str_appends(&smart_logContentStr,thisMothTime);
				smart_str_appends(&smart_logContentStr,PHP_EOL);
				smart_str_appends(&smart_logContentStr,"LogContent:");
				smart_str_appends(&smart_logContentStr,Z_STRVAL_PP(logContent));
				smart_str_appends(&smart_logContentStr,PHP_EOL);
				smart_str_appends(&smart_logContentStr,PHP_EOL);
				smart_str_0(&smart_logContentStr);
				efree(thisMothTime);

				//写入文件
				php_fwrite(fopenHandle,smart_logContentStr.c);

				smart_str_free(&smart_logContentStr);

				zend_hash_move_forward(Z_ARRVAL_PP(logContentList));
			}

			//释放文件锁
			php_flock(fopenHandle,3);
		}

		//关闭文件
		php_fclose(fopenHandle);
		zval_ptr_dtor(&fopenHandle);
	
		zend_hash_move_forward(Z_ARRVAL_P(logList));
	}

	RETVAL_TRUE;
}

PHP_METHOD(CLog,__construct)
{
	zval	*params = NULL,
			*useParams = NULL,
			*logList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|z",&params) == FAILURE){
		return;
	}

	MAKE_STD_ZVAL(useParams);
	if(NULL == params || IS_NULL == Z_TYPE_P(params)){
		array_init(useParams);
	}else{
		ZVAL_ZVAL(useParams,params,1,0);
	}

	zend_update_property(CLogCe,getThis(),ZEND_STRL("params"),useParams TSRMLS_CC);
	zval_ptr_dtor(&useParams);
}

PHP_METHOD(CLog,__destruct)
{
	zval errAction,
		errorMessage;
	INIT_ZVAL(errAction);
	ZVAL_STRING(&errAction,"save",0);
	call_user_function(NULL, &getThis(), &errAction, &errorMessage, 0, NULL TSRMLS_CC);
	zval_dtor(&errorMessage);
}

PHP_METHOD(CLog,write)
{
	char	*name,
			*content,
			logPath[10240],
			*thisMothTime,
			*thisMothDate,
			*logText,
			filePath[10240],
			*paramsString;

	int		nameLen,
			contentLen,
			argc = ZEND_NUM_ARGS(),
			i = 0;

	zval	***args;

	zval	*appPath;

	smart_str	smart_logContentStr = {0},
				smart_tempContent = {0};

	//获取不定长度参数
	args = (zval***)safe_emalloc(argc,sizeof(zval**),0);
	if(ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc,args) == FAILURE){
		zend_throw_exception(CClassNotFoundExceptionCe, "[CClassNotFoundException] Call [CLog->write] Parameter error , the first params must be a string meaning log name", 5001 TSRMLS_CC);
		return;
	}

	for(i = 0 ; i < argc ; i++){

		if(i == 0){
			if( IS_STRING != Z_TYPE_PP(args[i]) ){
				zend_throw_exception(CClassNotFoundExceptionCe, "[CClassNotFoundException] Call [CLog->write] Parameter error , the first params must be a string meaning log name", 5001 TSRMLS_CC);
				efree(args);
				return;
			}
			name = estrdup(Z_STRVAL_PP(args[i]));
			continue;
		}

		//将值加入到参数列表
		json_encode(*args[i],&paramsString);
		smart_str_appends(&smart_tempContent,paramsString);
		if(i < argc - 1){
			smart_str_appends(&smart_tempContent,"|LogSplit|");
		}
		efree(paramsString);
	}

	efree(args);
	smart_str_0(&smart_tempContent);

	if(strlen(name) == 0){
		name = estrdup("userLog");
	}

	//日志路径
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(appPath)){
		RETVAL_FALSE;
		return;
	}


	php_date("Ym",&thisMothDate);
	sprintf(logPath,"%s%s%s%s",Z_STRVAL_P(appPath),"/logs/userlog/",thisMothDate,"/");
	efree(thisMothDate);
	
	
	//创建目录
	if(FAILURE == fileExist(logPath)){

		zval *sapiZval;

		//尝试创建文件夹
		php_mkdir(logPath);

		//创建失败
		if(FAILURE == fileExist(logPath)){
			RETVAL_FALSE;
			return;
		}

		//如果是cli 将目录所有设为apache:apche
		if(zend_hash_find(EG(zend_constants),"PHP_SAPI",strlen("PHP_SAPI")+1,(void**)&sapiZval) == SUCCESS && strcmp(Z_STRVAL_P(sapiZval),"cli") == 0){
			char    *command,
					*returnString;
			spprintf(&command, 0,"chown apache:apache -R %s", logPath);
			exec_shell_return(command,&returnString);
			efree(command);
			efree(returnString);
		}
	}

	//日志内容
	php_date("Y-m-d H:i:s",&thisMothTime);

	smart_str_appends(&smart_logContentStr,"#LogTime:");
	smart_str_appends(&smart_logContentStr,thisMothTime);
	smart_str_appends(&smart_logContentStr,PHP_EOL);
	smart_str_appends(&smart_logContentStr,"LogContent:");
	smart_str_appends(&smart_logContentStr,smart_tempContent.c);
	smart_str_appends(&smart_logContentStr,PHP_EOL);
	smart_str_appends(&smart_logContentStr,PHP_EOL);
	smart_str_0(&smart_logContentStr);
	efree(thisMothTime);


	//调用writeFile函数
	sprintf(filePath,"%s%s%s",logPath,name,".log");
	CLog_writeFile(filePath,smart_logContentStr.c);

	smart_str_free(&smart_logContentStr);
	smart_str_free(&smart_tempContent);
	efree(name);
}

int CLog_writeFile(char *filePath,char *content)
{
	zval *fopenHandle;

	//打开文件
	php_fopen(filePath,"a+",&fopenHandle);
	
	//锁定文件
	if (php_flock(fopenHandle,2)){
		//日志内容
		php_fwrite(fopenHandle,content);
		php_flock(fopenHandle,3);
	}
		
	//关闭流
	php_fclose(fopenHandle);
	zval_ptr_dtor(&fopenHandle);
}

int CLog_writeSystemFile(char *content TSRMLS_DC){

	zval *appPath;

	char logPath[10240],
		 filePath[10240],
		 *thisMothTime;

	//日志路径
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(appPath)){
		return FAILURE;
	}


	sprintf(logPath,"%s%s",Z_STRVAL_P(appPath),"/logs/systemlog/");
	
	
	//创建目录
	if(FAILURE == fileExist(logPath)){
		//尝试创建文件夹
		php_mkdir(logPath);

		//创建失败
		if(FAILURE == fileExist(logPath)){
			return FAILURE;
		}
	}

	//日期
	php_date("Y-m-d",&thisMothTime);

	//调用writeFile函数
	sprintf(filePath,"%s%s%s",logPath,thisMothTime,".log");
	efree(thisMothTime);

	CLog_writeFile(filePath,content);


	return SUCCESS;
}

PHP_METHOD(CLog,writeFile)
{
	char	*name,
			*content;

	int		nameLen,
			contentLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&name,&nameLen,&content,&contentLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	CLog_writeFile(name,content);
}
