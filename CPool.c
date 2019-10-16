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
	PHP_ME(CPool,setIPCType,NULL,ZEND_ACC_PUBLIC)
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
	zend_declare_property_long(CPoolCe, ZEND_STRL("runTaskCount"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPoolCe, ZEND_STRL("ipcType"),1,ZEND_ACC_PRIVATE TSRMLS_CC);   //IPC type 1:pipe 2:redis
	zend_declare_property_long(CPoolCe, ZEND_STRL("selfPipeFd"),-1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CPoolCe, ZEND_STRL("lastCheckStatusTime"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPoolCe, ZEND_STRL("processList"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPoolCe, ZEND_STRL("pipeList"),ZEND_ACC_PRIVATE TSRMLS_CC);

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


int CPool_getInstance(zval **returnZval,int maxThreadNum,int ipcType TSRMLS_DC){

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
			zval *params[2];
			MAKE_STD_ZVAL(params[0]);
			ZVAL_LONG(params[0],maxThreadNum);
			MAKE_STD_ZVAL(params[1]);
			ZVAL_LONG(params[1],ipcType);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CPoolCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 2, params TSRMLS_CC);
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
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

void checkPoolStatus(zval *object TSRMLS_DC){

	zval	**thisVal,
			*childArr,
			*maxThreadNum,
			*ipcType,
			*pipeList,
			**thisPipe,
			*lastCheckTime;
	int		i,h,childNum = 0,processStatus = -1,needForkNum = 0,liveProcess = 0;
	char	*key;
	ulong	ikey = 1;

	//maxThreadNum
	maxThreadNum = zend_read_property(CPoolCe,object,ZEND_STRL("maxThreadNum"), 0 TSRMLS_CC);
	pipeList = zend_read_property(CPoolCe,object,ZEND_STRL("pipeList"), 0 TSRMLS_CC);
	ipcType = zend_read_property(CPoolCe,object,ZEND_STRL("ipcType"), 0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(pipeList)){
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(pipeList));
		childNum = zend_hash_num_elements(Z_ARRVAL_P(pipeList));
		for(i = 0 ; i < childNum ; i++){
			zend_hash_get_current_key(Z_ARRVAL_P(pipeList),&key,&ikey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(pipeList),(void**)&thisPipe);

			//不为0时 代表子进程已意外退出
			processStatus = kill(ikey,0);
			if(processStatus != 0){
				php_printf("[CPoolRuntime] child-process [%d] exited unexpectedly, unable to detect survival, need start a new process\n",ikey);
				close(Z_LVAL_PP(thisPipe));
				zend_hash_index_del(Z_ARRVAL_P(pipeList),ikey);
				CPool_createPorcessPool(1,Z_LVAL_P(ipcType),object TSRMLS_CC);
				break;
			}
			zend_hash_move_forward(Z_ARRVAL_P(pipeList));
		}
		
	}

	//记录最后检查的时间
	microtime(&lastCheckTime);
	zend_update_property_long(CPoolCe,object,ZEND_STRL("lastCheckStatusTime"),Z_DVAL_P(lastCheckTime) TSRMLS_CC);
	zval_ptr_dtor(&lastCheckTime);
}


//在接受到子进程退出信号后 清理现场 并重新检查进程池保持足够的worker进程
static zval *thisObject = NULL;
void catchSig(int sig){
	php_printf("[CPoolRuntime] [%d-%d] Receive a child process exit signal [%d]\n",getppid(),getpid(),sig);
	int endPid = wait(NULL);
	php_printf("[CPoolRuntime] The process for determining the unexpected termination is [%d]\n",endPid);
	checkPoolStatus(thisObject TSRMLS_CC);
}

int CPool_createPorcessPool(int workerNum,int ipcType,zval *object TSRMLS_DC){

	int		i;
	zval	*pipeList,
			*lastCheckTime;

	thisObject = object;

	pipeList = zend_read_property(CPoolCe,object,ZEND_STRL("pipeList"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(pipeList)){
		zval *saveData;
		MAKE_STD_ZVAL(saveData);
		array_init(saveData);
		zend_update_property(CPoolCe,object,ZEND_STRL("pipeList"),saveData TSRMLS_CC);
		zval_ptr_dtor(&saveData);
		pipeList = zend_read_property(CPoolCe,object,ZEND_STRL("pipeList"), 0 TSRMLS_CC);
	}


	for(i = 0 ; i < workerNum ;i++){

		int forkPid = -1;
		int fd[2];

		//create this child's pipe
		if(pipe(fd) < 0){
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->createPool] create master process failed", 7001 TSRMLS_CC);
			return;
		}


		forkPid=fork();
		if(forkPid==-1){
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->createPool] create master process failed", 7001 TSRMLS_CC);
			return;
		}else if(forkPid == 0){

			//child process action exec a php request
			zval	**SERVER,
					*appPath,
					**phpPath;
			char	*phpTruePath,
					*runCommand,
					*runParams;
			int		status = 0;

			//child process will close write,and open read 
			close(fd[1]);

			(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
			if(zend_hash_find(Z_ARRVAL_PP(SERVER),"_",strlen("_")+1,(void**)&phpPath) == SUCCESS && IS_STRING == Z_TYPE_PP(phpPath)){
				phpTruePath = estrdup(Z_STRVAL_PP(phpPath));
			}else{
				phpTruePath = estrdup("php");
			}
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			
			//run command
			spprintf(&runCommand,0,"%s/index.php",Z_STRVAL_P(appPath));
			spprintf(&runParams,0,"CCrontabController/poolWoker/ipc/%d/pipe/%d",ipcType,fd[0]);
			zend_update_property_long(CPoolCe,object,ZEND_STRL("selfPipeFd"),fd[0] TSRMLS_CC);

			status = execl(phpTruePath, "php", runCommand,runParams, NULL); 

			efree(runCommand);
			efree(phpTruePath);
			efree(runParams);

		}else{

			//set sign to ignore child quit
			//signal(SIGCHLD, SIG_IGN);
			signal(SIGCHLD, catchSig);

			//parent only write and close read
			close(fd[0]);
			add_index_long(pipeList,forkPid,fd[1]);
		}
	}

	//set to object
	zend_update_property(CPoolCe,object,ZEND_STRL("pipeList"),pipeList TSRMLS_CC);

	//记录首次创建Pool的时间
	microtime(&lastCheckTime);
	zend_update_property_long(CPoolCe,object,ZEND_STRL("lastCheckStatusTime"),Z_DVAL_P(lastCheckTime) TSRMLS_CC);
	zval_ptr_dtor(&lastCheckTime);
}


PHP_METHOD(CPool,__construct)
{
	long	maxThreadNum = 0,
			ipcType = 1;

	int	redisStatus = 0;

	zval	*processList;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|ll",&maxThreadNum,&ipcType) == FAILURE){
		return;
	}

	//ipc type is 2, check redis status
	if(ipcType == 2){
		redisStatus = CRedis_checkWriteRead("main" TSRMLS_CC);
		if(!redisStatus){
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->__consturct] Test redis fail , check redis config [main->REDIS_HOST] is right. ", 7001 TSRMLS_CC);
			return;
		}
	}

	if(maxThreadNum == 0){
		maxThreadNum = get_nprocs();
		maxThreadNum = maxThreadNum > 0 ? maxThreadNum*2 : 1;
	}

	zend_update_property_long(CPoolCe,getThis(),ZEND_STRL("maxThreadNum"),maxThreadNum TSRMLS_CC);
	zend_update_property_long(CPoolCe,getThis(),ZEND_STRL("ipcType"),ipcType TSRMLS_CC);


	//create a process pool
	CPool_createPorcessPool(maxThreadNum,ipcType,getThis() TSRMLS_CC);

}

PHP_METHOD(CPool,setIPCType){

}

PHP_METHOD(CPool,getInstance)
{
	zval *object;

	long maxThreadNum = 0,
		 ipcType = 1;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|ll",&maxThreadNum,&ipcType) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CPool->getInstance] params error ", 7001 TSRMLS_CC);
		return;
	}


	CPool_getInstance(&object,maxThreadNum,ipcType TSRMLS_CC);
	RETVAL_ZVAL(object,1,1);
}


PHP_METHOD(CPool,add)
{
	zval	*runObject,
			*callParams,
			*objectString,
			*redisBack,
			*encodeJsonZval,
			*runTaskCount,
			*ipcType,
			*maxThreadNum,
			*lastCheckStatusTime;
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

	//每执行10万次请求或距离上次检查超过600秒 主进程主动检查子进程池健康情况
	runTaskCount = zend_read_property(CPoolCe,getThis(),ZEND_STRL("runTaskCount"),0 TSRMLS_CC);
	lastCheckStatusTime = zend_read_property(CPoolCe,getThis(),ZEND_STRL("lastCheckStatusTime"),0 TSRMLS_CC);
	runCount = Z_LVAL_P(runTaskCount) > 100000000 ? 0 : Z_LVAL_P(runTaskCount) + 1;
	if(runCount % 100000  == 0){
		//check pool status
		checkPoolStatus(getThis() TSRMLS_CC);
	}

	zend_update_property_long(CPoolCe,getThis(),ZEND_STRL("runTaskCount"),runCount TSRMLS_CC);

	//serialize
	serialize(runObject,&objectString);

	//ipc type
	ipcType = zend_read_property(CPoolCe,getThis(),ZEND_STRL("ipcType"),0 TSRMLS_CC);

	//only ipc type is 2 , save to redis
	if(IS_LONG == Z_TYPE_P(ipcType) && Z_LVAL_P(ipcType) == 2){
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
		efree(encodeString);
	}else{

		//Rotation training is written into different child processes
		maxThreadNum = zend_read_property(CPoolCe,getThis(),ZEND_STRL("maxThreadNum"), 0 TSRMLS_CC);
		int thisMessageOffset = runCount % Z_LVAL_P(maxThreadNum);
		zval *pipeList = zend_read_property(CPoolCe,getThis(),ZEND_STRL("pipeList"), 0 TSRMLS_CC);
		
		

		//如没有可用的子进程 则阻塞开始等待 并不断检查子线程状态
		zval **nowNeedAssign;
		while(1){
			if(IS_ARRAY == Z_TYPE_P(pipeList)){

				//根据取模来按偏移指派任务
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(pipeList));
				int i;
				for(i = 0 ; i < thisMessageOffset;i++){
					zend_hash_move_forward(Z_ARRVAL_P(pipeList));
				}

				if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(pipeList),(void**)&nowNeedAssign)){
					break;
				}
			}else{
				usleep(10000);
				checkPoolStatus(getThis() TSRMLS_CC);
			}
		}

		MAKE_STD_ZVAL(encodeJsonZval);
		array_init(encodeJsonZval);
		add_assoc_string(encodeJsonZval,"class",estrdup(classCe->name),0);
		add_assoc_string(encodeJsonZval,"object",Z_STRVAL_P(objectString),1);
		json_encode(encodeJsonZval,&encodeString);
		zval_ptr_dtor(&encodeJsonZval);

		char	*putData;
		spprintf(&putData,0,"%s\n",encodeString);
		efree(encodeString);


		//写入管道  当目标进程来不及处理时  阻塞当前进程
		write((int)Z_LVAL_PP(nowNeedAssign),putData, strlen(putData)+1);
		//php_printf("[CPoolRuntime] push to target pipeId[%d] message : %s\n",Z_LVAL_PP(nowNeedAssign),putData);
		efree(putData);
	}


	zval_ptr_dtor(&objectString);
}

void CPool_destroy(zval *object TSRMLS_DC){

	int		n,m;
	char	buf[1],
			*key;
	ulong	ikey;
	zval	*pipeList = zend_read_property(CPoolCe,object,ZEND_STRL("pipeList"), 0 TSRMLS_CC);
	zval	*selfPipeFd = zend_read_property(CPoolCe,object,ZEND_STRL("selfPipeFd"), 0 TSRMLS_CC);
	zval	**thisPipeIndex;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pipeList));
	m = zend_hash_num_elements(Z_ARRVAL_P(pipeList));
	for(n = 0 ; n < m ; n++){
		zend_hash_get_current_key(Z_ARRVAL_P(pipeList),&key,&ikey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(pipeList),(void**)&thisPipeIndex);
		//发送子进程结束信号 
		kill(ikey,SIGHUP);
		close(Z_LVAL_PP(thisPipeIndex));
		zend_hash_move_forward(Z_ARRVAL_P(pipeList));
	}

	//关闭写管道
	if(IS_LONG == Z_TYPE_P(selfPipeFd) && Z_LVAL_P(selfPipeFd) > 0){
		close(Z_LVAL_P(selfPipeFd));
	}

	signal(SIGCHLD, SIG_IGN);
}

PHP_METHOD(CPool,destory)
{
	CPool_destroy(getThis() TSRMLS_CC);
}

PHP_METHOD(CPool,setThreadMaxNum)
{
}

PHP_METHOD(CPool,__destruct)
{
	CPool_destroy(getThis() TSRMLS_CC);
}

PHP_METHOD(CPoolRunable,run)
{
}
#endif