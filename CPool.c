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
#include "php_CQuickFramework.h"
#include "php_CPool.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

//zend类方法
zend_function_entry CPool_functions[] = {
	PHP_ME(CPool,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CPool,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CPool,add,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPool,destory,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPool,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CPool)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CPool",CPool_functions);
	CPoolCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CPoolCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CPoolCe, ZEND_STRL("maxThreadNum"),4,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPoolCe, ZEND_STRL("runTaskCount"),100,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPoolCe, ZEND_STRL("processList"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

//zend类方法
zend_function_entry CPoolRunable_functions[] = {
	ZEND_ABSTRACT_ME(CPoolRunable, run, NULL)
	{NULL, NULL, NULL}
};

CMYFRAME_REGISTER_CLASS_RUN(CPoolRunable)
{
	//注册CWebApp类
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CPoolRunable",CPoolRunable_functions);
	CPoolRunableCe = zend_register_internal_interface(&funCe TSRMLS_CC);

	return SUCCESS;
}

int CPool_getInstance(zval **returnZval,int maxThreadNum TSRMLS_DC){

	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CPoolCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zval			*object;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CPoolCe);

		//执行构造器
		if (CPoolCe->constructor) {
			zval constructReturn;
			zval constructVal;
			zval *params[1];
			MAKE_STD_ZVAL(params[0]);
			ZVAL_LONG(params[0],maxThreadNum);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CPoolCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, params TSRMLS_CC);
			zval_ptr_dtor(&params[0]);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CPoolCe,ZEND_STRL("instance"),object TSRMLS_CC);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

int CPool_createPorcessPool(int workerNum,zval *object TSRMLS_DC){

	int	i;

	for(i = 0 ; i < workerNum ;i++){
		int masterPid=fork();
		if(masterPid==-1){
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->createPool] create master process failed", 7001 TSRMLS_CC);
			return;
		}else if(masterPid == 0){

			//child process action exec a php request
			zval	**SERVER,
					*appPath,
					**phpPath;
			char	*phpTruePath,
					*runCommand,
					*runParams;
			int		status = 0;
			(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
			if(zend_hash_find(Z_ARRVAL_PP(SERVER),"_",strlen("_")+1,(void**)&phpPath) == SUCCESS && IS_STRING == Z_TYPE_PP(phpPath)){
				phpTruePath = estrdup(Z_STRVAL_PP(phpPath));
			}else{
				phpTruePath = estrdup("php");
			}
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			
			//run command
			spprintf(&runCommand,0,"%s/index.php",Z_STRVAL_P(appPath));
			spprintf(&runParams,0,"CCrontabController/poolWoker");

			status = execl(phpTruePath, "php", runCommand,runParams, NULL); 

			efree(runCommand);
			efree(phpTruePath);
			efree(runParams);
		}else{
			//set sign to watch child quit
			signal(SIGCHLD, SIG_IGN);
		}
	}
}


PHP_METHOD(CPool,__construct)
{
	long maxThreadNum = 0;

	int	redisStatus = 0;

	zval	*processList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&maxThreadNum) == FAILURE){
		return;
	}

	redisStatus = CRedis_checkWriteRead("main" TSRMLS_CC);
	if(!redisStatus){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->__consturct] Test redis fail , check redis config [main->REDIS_HOST] is right. ", 7001 TSRMLS_CC);
		return;
	}

	if(maxThreadNum == 0){
		maxThreadNum = get_nprocs();
		maxThreadNum = maxThreadNum > 0 ? maxThreadNum*2 : 1;
	}

	zend_update_property_long(CPoolCe,getThis(),ZEND_STRL("maxThreadNum"),maxThreadNum TSRMLS_CC);


	//create a process pool
	CPool_createPorcessPool(maxThreadNum,getThis() TSRMLS_CC);

}

PHP_METHOD(CPool,getInstance)
{
	zval *object;

	long maxThreadNum = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&maxThreadNum) == FAILURE){
		return;
	}

	CPool_getInstance(&object,maxThreadNum TSRMLS_CC);
	RETVAL_ZVAL(object,1,1);
}

void checkPoolStatus(zval *object TSRMLS_DC){

	zval	**thisVal,
			*childArr,
			*maxThreadNum;
	int		i,h,childNum = 0;
	char	getChildCommand[150],
			*childListString;

	//maxThreadNum
	maxThreadNum = zend_read_property(CPoolCe,object,ZEND_STRL("maxThreadNum"), 0 TSRMLS_CC);

	//ps --ppid 14132
	pid_t nowPid = getpid();
	sprintf(getChildCommand,"ps --ppid %d",nowPid);
	exec_shell_return(getChildCommand,&childListString);
	php_explode("\n",childListString,&childArr);
	if(IS_ARRAY != Z_TYPE_P(childArr)){
		efree(childListString);
		zval_ptr_dtor(&childArr);
		return;
	}
	h = zend_hash_num_elements(Z_ARRVAL_P(childArr));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(childArr),(void**)&thisVal);
		if(strstr(Z_STRVAL_PP(thisVal),"CQuickPoolWoker") != NULL || strstr(Z_STRVAL_PP(thisVal),"php") != NULL){
			childNum++;
		}
		zend_hash_move_forward(Z_ARRVAL_P(childArr));
	}

	if(childNum < Z_LVAL_P(maxThreadNum)){
		//try to fork new
		int needFork = Z_LVAL_P(maxThreadNum) - childNum;

		CPool_createPorcessPool(needFork,object TSRMLS_CC);
	}
}

PHP_METHOD(CPool,add)
{
	zval	*runObject,
			*callParams,
			*objectString,
			*redisBack,
			*encodeJsonZval,
			*runTaskCount;
	int		runCount;
	char	*encodeString;

	zend_class_entry *classCe;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&runObject) == FAILURE){
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(runObject)){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->add] must give a object and implements CPoolRunable", 7001 TSRMLS_CC);
		return;
	}

	classCe = Z_OBJCE_P(runObject);

	//check has run function
	if(!zend_hash_exists(&classCe->function_table,"run",4)){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->add] must give a object and implements CPoolRunable", 7001 TSRMLS_CC);
		return;
	}

	runTaskCount = zend_read_property(CPoolCe,getThis(),ZEND_STRL("runTaskCount"),0 TSRMLS_CC);
	if(runCount % 100  == 0){
		//check pool status
		checkPoolStatus(getThis() TSRMLS_CC);

	}
	runCount = Z_LVAL_P(runTaskCount) + 1;
	zend_read_property(CPoolCe,getThis(),ZEND_STRL("runTaskCount"),runCount TSRMLS_CC);

	//serialize
	serialize(runObject,&objectString);
	
	MAKE_STD_ZVAL(encodeJsonZval);
	array_init(encodeJsonZval);
	add_assoc_string(encodeJsonZval,"class",estrdup(classCe->name),0);
	add_assoc_string(encodeJsonZval,"object",Z_STRVAL_P(objectString),1);
	json_encode(encodeJsonZval,&encodeString);
	zval_ptr_dtor(&encodeJsonZval);

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,"CQuickFramePool",1);
	add_next_index_string(callParams,encodeString,1);
	CRedis_callFunction("main","rpush",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);
	zval_ptr_dtor(&redisBack);
	zval_ptr_dtor(&objectString);
	efree(encodeString);
}

PHP_METHOD(CPool,destory)
{
	char	getChildCommand[150],
			*childListString;
	zval	*childArr,
			**thisVal;
	int		i,h;

	pid_t nowPid = getpid();
	sprintf(getChildCommand,"ps --ppid %d",nowPid);
	exec_shell_return(getChildCommand,&childListString);
	php_explode("\n",childListString,&childArr);

	if(IS_ARRAY != Z_TYPE_P(childArr)){
		efree(childListString);
		zval_ptr_dtor(&childArr);
		return;
	}
	h = zend_hash_num_elements(Z_ARRVAL_P(childArr));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(childArr),(void**)&thisVal);
		if(strstr(Z_STRVAL_PP(thisVal),"CQuickPoolWoker") != NULL || strstr(Z_STRVAL_PP(thisVal),"php") != NULL){
			
			//cut for " "
			zval	*cutInfo,
					**childPid;
			php_explode(" ",Z_STRVAL_PP(thisVal),&cutInfo);
			if(zend_hash_index_find(Z_ARRVAL_P(cutInfo),1,(void**)&childPid) == SUCCESS && IS_STRING == Z_TYPE_PP(childPid)){
				if(isdigitstr(Z_STRVAL_PP(childPid))){
					pid_t cpid = toInt(Z_STRVAL_PP(childPid));
					kill(cpid,SIGHUP);
				}
			}

			zval_ptr_dtor(&cutInfo);
		}
		zend_hash_move_forward(Z_ARRVAL_P(childArr));
	}


	efree(childListString);
	zval_ptr_dtor(&childArr);
}

PHP_METHOD(CPool,setThreadMaxNum)
{
}

PHP_METHOD(CPool,__destruct)
{
	
}

PHP_METHOD(CPoolRunable,run)
{
}
#endif