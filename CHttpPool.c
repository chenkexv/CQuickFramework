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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CQuickFramework.h"
#include "php_CApplication.h"
#include "php_CException.h"
#include "php_CHttpPool.h"
#include "php_CHttp.h"
#include "php_CWebApp.h"
#include "php_CHttpServer.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <pthread.h>
#include <hiredis.h>
#include <curl/curl.h>
#include <sys/sysinfo.h>
#include <sys/prctl.h>


//zend类方法
zend_function_entry CHttpPool_functions[] = {
	PHP_ME(CHttpPool,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CHttpPool,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CHttpPool,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CHttpPool,setThreadMaxNum,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,add,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,registerCallback,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,setResultToRedis,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,setResultToFile,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,setRejectedPolicy,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,getPoolStatus,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CHttpPool,setRejectedTaskNum,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};


//http Pool __destroy function
static void free_httpPool(zend_rsrc_list_entry *rsrc TSRMLS_DC){

	threadpool_t *pool = (threadpool_t*)rsrc->ptr;
	threadpool_destory(pool);
	efree(pool);
}

//模块被加载时
static int le_resource_httpPool;
CMYFRAME_REGISTER_CLASS_RUN(CHttpPool)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHttpPool",CHttpPool_functions);
	CHttpPoolCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CHttpPoolCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CHttpPoolCe, ZEND_STRL("maxThreadNum"),50,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CHttpPoolCe, ZEND_STRL("poolResourceId"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	//callback functions
	zend_declare_property_string(CHttpPoolCe, ZEND_STRL("redisCallbackKey"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CHttpPoolCe, ZEND_STRL("fileCallbackKey"),"",ZEND_ACC_PRIVATE TSRMLS_CC); 
	zend_declare_property_string(CHttpPoolCe, ZEND_STRL("fileTrueName"),"",ZEND_ACC_PRIVATE TSRMLS_CC); 
	

	//max task num , more than this num will reject add task
	zend_declare_property_long(CHttpPoolCe, ZEND_STRL("maxTask"),100000,ZEND_ACC_PRIVATE TSRMLS_CC); 
	zend_declare_property_long(CHttpPoolCe, ZEND_STRL("rejectType"),1,ZEND_ACC_PRIVATE TSRMLS_CC); 

	zend_declare_property_string(CHttpPoolCe, ZEND_STRL("callback"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	le_resource_httpPool = zend_register_list_destructors_ex(free_httpPool, NULL, "httpPool", module_number);

	return SUCCESS;
}

PHP_METHOD(CHttpPool,setRejectedPolicy){
	long num;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&num) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setRejectedPolicy] params error,must give long in (1,2,3)", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_long(CHttpPoolCe,getThis(),ZEND_STRL("rejectType"),num TSRMLS_CC);
}

PHP_METHOD(CHttpPool,setRejectedTaskNum){
	long num;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&num) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setRejectedTaskNum] params error,must give long", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_long(CHttpPoolCe,getThis(),ZEND_STRL("maxTask"),num TSRMLS_CC);
}

PHP_METHOD(CHttpPool,setResultToRedis){

	char	*keyName;

	int		keyNameLen = 0,
			redisStatus = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&keyName,&keyNameLen) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setResultToRedis] params error,must give string", 7001 TSRMLS_CC);
		return;
	}

	redisStatus = CRedis_checkWriteRead("main" TSRMLS_CC);
	if(!redisStatus){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->registerCallback] Test redis fail , check redis config [main->REDIS_HOST] is right. ", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_string(CHttpPoolCe,getThis(),ZEND_STRL("redisCallbackKey"),keyName TSRMLS_CC);
}

//check cpu core num and mem to set how many process to create
int checkProcessNum(){
	
	int canCpuNum = get_nprocs();

	return canCpuNum > 0 ? canCpuNum*2 : 1;
}

PHP_METHOD(CHttpPool,registerCallback){
	
	char	*callFunction;
	int		callFunctionLen = 0,
			redisStatus = 0,
			i;
	zval	*callArr,
			**className,
			**funcName;
	zend_class_entry	**classEntryPP,
						*classEntryCe;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&callFunction,&callFunctionLen) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->registerCallback] params error,must give string like \"[base::callback]\" ", 7001 TSRMLS_CC);
		return;
	}

	//check function name
	php_explode("::",callFunction,&callArr TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(callArr) && zend_hash_num_elements(Z_ARRVAL_P(callArr)) == 2){
	}else{
		zval_ptr_dtor(&callArr);
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->registerCallback] params error,must give string like \"[base::callback]\" ", 7001 TSRMLS_CC);
		return;
	}

	//check class exists
	zend_hash_index_find(Z_ARRVAL_P(callArr),0,(void**)&className);
	zend_hash_index_find(Z_ARRVAL_P(callArr),1,(void**)&funcName);
	php_strtolower(Z_STRVAL_PP(className),strlen(Z_STRVAL_PP(className))+1);
	if(zend_hash_find(EG(class_table),Z_STRVAL_PP(className),strlen(Z_STRVAL_PP(className))+1,(void**)&classEntryPP) == FAILURE){
		char errMessage[1024];
		sprintf(errMessage,"[CPoolException] call [CHttpPool->registerCallback] params error,the class[%s] is not exists",Z_STRVAL_PP(className));
		zval_ptr_dtor(&callArr);
		zend_throw_exception(CPoolExceptionCe, errMessage, 7001 TSRMLS_CC);
		return;
	}
	classEntryCe = *classEntryPP;
	if(!zend_hash_exists(&classEntryCe->function_table,Z_STRVAL_PP(funcName),strlen(Z_STRVAL_PP(funcName))+1)){
		char errMessage[1024];
		sprintf(errMessage,"[CPoolException] call [CHttpPool->registerCallback] params error,the function[%s] is not exists",Z_STRVAL_PP(funcName));
		zval_ptr_dtor(&callArr);
		zend_throw_exception(CPoolExceptionCe, errMessage, 7001 TSRMLS_CC);
		return;
	}
	zval_ptr_dtor(&callArr);


	redisStatus = CRedis_checkWriteRead("main" TSRMLS_CC);
	if(!redisStatus){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->registerCallback] In NTS environment, callback trigger relies on multi-process, and Redis is chosen as process communication carrier. Make sure that Redis is configured correctly. ", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_string(CHttpPoolCe,getThis(),ZEND_STRL("callback"),callFunction TSRMLS_CC);

	//check create how many process
	int processNum = checkProcessNum();

	//create process
	for(i = 0 ; i < processNum;i++){
	
		//fork a child process
		int childPid=fork();
		if(childPid==-1){
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->registerCallback] create child process failed", 7001 TSRMLS_CC);
			return;
		}else if(childPid == 0){

			//child process action exec a php request
			zval	**SERVER,
					*appPath,
					**phpPath;
			char	*phpTruePath,
					*runCommand,
					*runParams;
			(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
			if(zend_hash_find(Z_ARRVAL_PP(SERVER),"_",strlen("_")+1,(void**)&phpPath) == SUCCESS && IS_STRING == Z_TYPE_PP(phpPath)){
				phpTruePath = estrdup(Z_STRVAL_PP(phpPath));
			}else{
				phpTruePath = estrdup("php");
			}
			appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
			
			//run command
			spprintf(&runCommand,0,"%s/index.php",Z_STRVAL_P(appPath));
			spprintf(&runParams,0,"CCrontabController/recHttpPool/callback/%s",callFunction);

			int status = execl(phpTruePath, "php", runCommand,runParams, NULL); 

			efree(runCommand);
			efree(phpTruePath);
			efree(runParams);
		}else{


		}
	}
}

PHP_METHOD(CHttpPool,setResultToFile){

	char	*keyName;

	int		keyNameLen = 0;

	char	*logsPath,
			*logsFileName,
			*thisMothDate;

	zval	*appPath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&keyName,&keyNameLen) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setResultToFile] params error,must give string", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_string(CHttpPoolCe,getThis(),ZEND_STRL("fileCallbackKey"),keyName TSRMLS_CC);

	//read logs Path
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	php_date("Ym",&thisMothDate);
	spprintf(&logsPath,0,"%s%s%s",Z_STRVAL_P(appPath),"/logs/userlog/",thisMothDate);
	efree(thisMothDate);

	//创建目录
	if(FAILURE == fileExist(logsPath)){
		//尝试创建文件夹
		php_mkdir(logsPath);

		//创建失败
		if(FAILURE == fileExist(logsPath)){
			efree(logsPath);
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setResultToFile] error, cant not create logs save path", 7001 TSRMLS_CC);
			return;
		}

		//如果是cli 将目录所有设为apache:apche
		MODULE_BEGIN
			char    *command,
					*returnString;
			spprintf(&command, 0,"chown apache:apache -R %s", logsPath);
			exec_shell_return(command,&returnString);
			efree(command);
			efree(returnString);
		MODULE_END
	}

	spprintf(&logsFileName,0,"%s/%s%s",logsPath,keyName,".log");
	zend_update_property_string(CHttpPoolCe,getThis(),ZEND_STRL("fileTrueName"),logsFileName TSRMLS_CC);
	efree(logsPath);
	
	//check file is exists
	if(FAILURE == fileExist(logsFileName)){
		//create this file
		FILE *fd = fopen(logsFileName,"w+"); 
		if(fd == NULL){
			efree(logsFileName);
			zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setResultToFile] error, cant not create logs save path", 7001 TSRMLS_CC);
			return;
		}
		fclose(fd);
	}

	efree(logsFileName);
}

PHP_METHOD(CHttpPool,__construct)
{
	int		rsid = 0;
	zval	*maxThreadNum;

	maxThreadNum = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("maxThreadNum"),0 TSRMLS_CC);

	curl_global_init(CURL_GLOBAL_ALL);

	//create a thread Pool
	threadpool_t *pool;
	pool = emalloc(sizeof(threadpool_t));
    threadpool_init(pool,Z_LVAL_P(maxThreadNum));

	rsid = zend_list_insert(pool, le_resource_httpPool);
	zend_update_property_long(CHttpPoolCe,getThis(),ZEND_STRL("poolResourceId"),rsid TSRMLS_CC);
	RETURN_LONG(rsid);
}

static pthread_mutex_t fileMut = PTHREAD_MUTEX_INITIALIZER;
PHP_METHOD(CHttpPool,__destruct)
{
	curl_global_cleanup();
	pthread_mutex_destroy(&fileMut);
}


struct string {
  char *ptr;
  size_t len;
};
 
void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;
 
  return size*nmemb;
}

int saveDataToRedis(httpRequest_t *t,char *content){

	redisContext *conn = redisConnect(t->redisHost, t->redisPort);
    if(conn->err) {
        redisFree(conn); 
        return -1;
    } 

	if(t->redisPassword != NULL){
		char	authName[strlen(t->redisPassword) + 10];
		sprintf(authName,"auth %s",t->redisPassword);
		redisReply *authReply = redisCommand(conn, authName);  
		freeReplyObject(authReply);  
	}


	//rpush
	if(strlen(content) > 0){
		char *command = malloc(strlen(content)+10+strlen(t->redisKey));
		sprintf(command,"rpush %s %s",t->redisKey,content);
		redisReply *rpushReply = redisCommand(conn, command); 
		if(rpushReply != NULL){
			freeReplyObject(rpushReply);
		}
		free(command);
	}

	//callfun
	if(strlen(content) > 0 && t->callbackFun == 1){
		char *command = malloc(strlen(content)+10+strlen("CQuickFrameHttpPool"));
		sprintf(command,"rpush %s %s","CQuickFrameHttpPool",content);
		redisReply *rpushReply = redisCommand(conn, command); 
		if(rpushReply != NULL){
			freeReplyObject(rpushReply);
		}
		free(command);
	}


	redisFree(conn); 
	return 0;
}

int saveDataToFile(char *fileName,char *fileContent){

	 FILE *fp = fopen(fileName, "a+");
	 if(fp == NULL){
		 return -1;
	 }

	 
	 time_t ptime;
     struct tm *p;
	 char	showTime[50],
			*saveContent;
     time(&ptime);
     p = gmtime(&ptime);

     sprintf(showTime,"#LogTime:%d/%02d/%02d %02d:%02d:%02d => %ld %s",(1900 + p->tm_year),(1 + p->tm_mon),p->tm_mday,(p->tm_hour+8)%24,p->tm_min,p->tm_sec,ptime,"\r\n");
	 saveContent = malloc(strlen(showTime) + strlen(fileContent) + 20);
	 sprintf(saveContent,"%s%s%s%s",showTime,"LogContent:",fileContent,"\r\n\r\n");
	
	 //lock file
	 pthread_mutex_lock(&fileMut);
	 fwrite(saveContent, strlen(saveContent), 1, fp);
	 free(saveContent);

	 fclose(fp);
     pthread_mutex_unlock(&fileMut);

}

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char padding_char = '=';
int base64_encode(const unsigned char * sourcedata, char * base64)
{
    int i=0, j=0;
    unsigned char trans_index=0;    // 索引是8位，但是高两位都为0
    const int datalength = strlen((const char*)sourcedata);
    for (; i < datalength; i += 3){
        // 每三个一组，进行编码
        // 要编码的数字的第一个
        trans_index = ((sourcedata[i] >> 2) & 0x3f);
        base64[j++] = base64char[(int)trans_index];
        // 第二个
        trans_index = ((sourcedata[i] << 4) & 0x30);
        if (i + 1 < datalength){
            trans_index |= ((sourcedata[i + 1] >> 4) & 0x0f);
            base64[j++] = base64char[(int)trans_index];
        }else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            base64[j++] = padding_char;

            break;   // 超出总长度，可以直接break
        }
        // 第三个
        trans_index = ((sourcedata[i + 1] << 2) & 0x3c);
        if (i + 2 < datalength){ // 有的话需要编码2个
            trans_index |= ((sourcedata[i + 2] >> 6) & 0x03);
            base64[j++] = base64char[(int)trans_index];

            trans_index = sourcedata[i + 2] & 0x3f;
            base64[j++] = base64char[(int)trans_index];
        }
        else{
            base64[j++] = base64char[(int)trans_index];

            base64[j++] = padding_char;

            break;
        }
    }

    base64[j] = '\0'; 

    return 0;
}
inline int num_strchr(const char *str, char c) // 
{
    const char *pindex = strchr(str, c);
    if (NULL == pindex){
        return -1;
    }
    return pindex - str;
}
/* 解码
* const char * base64 码字
* unsigned char * dedata， 解码恢复的数据
*/
int base64_decode(const char * base64, unsigned char * dedata)
{
    int i = 0, j=0;
    int trans[4] = {0,0,0,0};
    for (;base64[i]!='\0';i+=4){
        // 每四个一组，译码成三个字符
        trans[0] = num_strchr(base64char, base64[i]);
        trans[1] = num_strchr(base64char, base64[i+1]);
        // 1/3
        dedata[j++] = ((trans[0] << 2) & 0xfc) | ((trans[1]>>4) & 0x03);

        if (base64[i+2] == '='){
            continue;
        }
        else{
            trans[2] = num_strchr(base64char, base64[i + 2]);
        }
        // 2/3
        dedata[j++] = ((trans[1] << 4) & 0xf0) | ((trans[2] >> 2) & 0x0f);

        if (base64[i + 3] == '='){
            continue;
        }
        else{
            trans[3] = num_strchr(base64char, base64[i + 3]);
        }

        // 3/3
        dedata[j++] = ((trans[2] << 6) & 0xc0) | (trans[3] & 0x3f);
    }

    dedata[j] = '\0';

    return 0;
}

void *doHttpRequest(void *args){


	CURL *curl;
    int i = 100;
    struct curl_slist *headers = NULL;
	char	*cutHeader,
			*nextHeader,
			*inner_ptr = NULL,
			*errorString;
	CURLcode res;
	long	l_code;
	double  d_code;
	char errbuf[CURL_ERROR_SIZE];

	curl = curl_easy_init();

	//set url
	curl_easy_setopt(curl, CURLOPT_URL, ((httpRequest_t*)args)->hosts);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL,(long)1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,0); 
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,0);


	//split header
	cutHeader = strdup(((httpRequest_t*)args)->header);
	nextHeader = strtok_r(cutHeader,"\n",&inner_ptr);
	while(nextHeader != NULL) {
		headers = curl_slist_append(headers,nextHeader);
		nextHeader = strtok_r(NULL,"\n",&inner_ptr);
	}
	free(cutHeader);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	//set get result type
	struct string s;
    init_string(&s);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writefunc); 
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&s);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

	//request
	errbuf[0] = 0;
	res = curl_easy_perform(curl);

	

	//create a response object
	int http_code = 0;
	double total_time = 0.0;
	double namelookup_time = 0.0;
	double connect_time = 0.0;
	double pretransfer_time = 0.0;


	if(CURLE_OK == res) {
		if (curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &l_code) == CURLE_OK) {
			http_code = l_code;
		}
		if (curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &d_code) == CURLE_OK) {
			total_time = d_code;
		}
		if (curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &d_code) == CURLE_OK) {
			namelookup_time = d_code;
		}
		if (curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &d_code) == CURLE_OK) {
			connect_time = d_code;
		}
		if (curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &d_code) == CURLE_OK) {
			pretransfer_time = d_code;
		}
		errorString = strdup("");
	}else{
		errorString = malloc(strlen(errbuf)*2);
		base64_encode(errbuf,errorString);
	}


	//create a json
	char *contebtBase = (char*)malloc(s.len*2);
	base64_encode(s.ptr,contebtBase);
	char *sendParams = malloc(strlen(((httpRequest_t*)args)->taskName)*2);
	base64_encode(((httpRequest_t*)args)->taskName,sendParams);
	char *jsonString = (char*)malloc(strlen(contebtBase)+strlen(((httpRequest_t*)args)->taskName)+200+(CURLE_OK != res ? strlen(errbuf) : 0));
	sprintf(jsonString,"{\"error\":\"%s\",\"params\":\"%s\",\"content\":\"%s\",\"http_code\":%ld,\"total_time\":%lf,\"namelookup_time\":%lf,\"connect_time\":%lf,\"pretransfer_time\":%lf}",(CURLE_OK != res ? errorString : ""),sendParams,contebtBase,http_code,total_time,namelookup_time,connect_time,pretransfer_time);
	free(errorString);

	if( ((httpRequest_t*)args)->redisHost != NULL && ((httpRequest_t*)args)->redisPort != 0){
		saveDataToRedis((httpRequest_t*)args,jsonString);
	}

	if(((httpRequest_t*)args)->fileName != NULL){
		saveDataToFile(((httpRequest_t*)args)->fileName,jsonString);
	}

	//destroy
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	free(contebtBase);
	free(jsonString);
    free(s.ptr);
	free(sendParams);

	if(((httpRequest_t*)args)->taskName != NULL) free(((httpRequest_t*)args)->taskName);
	if(((httpRequest_t*)args)->hosts != NULL) free(((httpRequest_t*)args)->hosts);
	if(((httpRequest_t*)args)->params != NULL) free(((httpRequest_t*)args)->params);
	if(((httpRequest_t*)args)->method != NULL) free(((httpRequest_t*)args)->method);
	if(((httpRequest_t*)args)->header != NULL) free(((httpRequest_t*)args)->header);
	if(((httpRequest_t*)args)->fileName != NULL) free(((httpRequest_t*)args)->fileName);
	if(((httpRequest_t*)args)->redisHost != NULL) free(((httpRequest_t*)args)->redisHost);
	if(((httpRequest_t*)args)->redisPassword != NULL) free(((httpRequest_t*)args)->redisPassword);
	if(((httpRequest_t*)args)->redisKey != NULL) free(((httpRequest_t*)args)->redisKey);
	free((httpRequest_t*)args);


}

httpRequest_t *CHttpPool_parseCHttpToString(zval *object TSRMLS_DC){

	zval			*url,
					*params,
					*timeout,
					*method,
					*header,
					**nowHeader;
	
	int				i,h,a;
	smart_str		headerString = {0};

	httpRequest_t	*rt = malloc(sizeof(httpRequest_t));

	url = zend_read_property(CHttpCe,object,ZEND_STRL("url"),0 TSRMLS_CC);
	params = zend_read_property(CHttpCe,object,ZEND_STRL("params"),0 TSRMLS_CC);
	timeout = zend_read_property(CHttpCe,object,ZEND_STRL("timeout"),0 TSRMLS_CC);
	method = zend_read_property(CHttpCe,object,ZEND_STRL("method"),0 TSRMLS_CC);
	header = zend_read_property(CHttpCe,object,ZEND_STRL("header"),0 TSRMLS_CC);

	//save method
	php_strtoupper(Z_STRVAL_P(method),strlen(Z_STRVAL_P(method))+1);
	rt->method = strdup(Z_STRVAL_P(method));
	
	//save header
	if(IS_ARRAY == Z_TYPE_P(header) && zend_hash_num_elements(Z_ARRVAL_P(header)) > 0 ){
		h = zend_hash_num_elements(Z_ARRVAL_P(header));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(header));
		for(i = 0 ; i < h;i++){
			zend_hash_get_current_data(Z_ARRVAL_P(header),(void**)&nowHeader);
			smart_str_appends(&headerString,Z_STRVAL_PP(nowHeader));
			smart_str_appends(&headerString,"\n");
			zend_hash_move_forward(Z_ARRVAL_P(header));
		}
		smart_str_0(&headerString);
		rt->header = strdup(headerString.c);
		smart_str_free(&headerString);
	}else{
		rt->header = NULL;
	}

	//save  params
	if(strcmp(rt->method,"get") == 0 && (IS_ARRAY == Z_TYPE_P(params) || IS_STRING == Z_TYPE_P(params))){
		char	*paramsQuery,
				*thisTrueHost;
		if(IS_ARRAY == Z_TYPE_P(params)){
			http_build_query(params,&paramsQuery);
		}else if(IS_STRING == Z_TYPE_P(params)){
			paramsQuery = estrdup(Z_STRVAL_P(params));
		}else{
			paramsQuery = estrdup("");
		}

		if(strstr(Z_STRVAL_P(url),"?") != NULL){
			spprintf(&thisTrueHost,0,"%s%s%s",Z_STRVAL_P(url),"&",paramsQuery);
		}else{
			spprintf(&thisTrueHost,0,"%s%s%s",Z_STRVAL_P(url),"?",paramsQuery);
		}
		rt->params = NULL;
		rt->hosts = strdup(thisTrueHost);
		efree(paramsQuery);
		efree(thisTrueHost);
	}else{
		if(IS_ARRAY == Z_TYPE_P(params) || IS_STRING == Z_TYPE_P(params)){
			char	*paramsQuery;
			if(IS_ARRAY == Z_TYPE_P(params)){
				http_build_query(params,&paramsQuery);
			}else if(IS_STRING == Z_TYPE_P(params)){
				paramsQuery = estrdup(Z_STRVAL_P(params));
			}else{
				paramsQuery = estrdup("");
			}
			rt->params = strdup(paramsQuery);
			efree(paramsQuery);
		}else{
			rt->params = NULL;
		}
		rt->hosts = strdup(Z_STRVAL_P(url));
	}

	return rt;
}

PHP_METHOD(CHttpPool,getPoolStatus){

	zval	*poolResourceId,
			*callParams,
			*redisBack;
	int		type;
	threadpool_t *pool;

	//get pool resource
	poolResourceId = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("poolResourceId"),0 TSRMLS_CC);
	pool = zend_list_find(Z_LVAL_P(poolResourceId), &type);
	if(type != le_resource_httpPool){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->add] get thread pool resource fail", 7001 TSRMLS_CC);
		return;
	}

	//get redis callback
	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,"CQuickFrameHttpPool",1);
	CRedis_callFunction("main","llen",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);

	array_init(return_value);
	add_assoc_long(return_value,"taskNum",pool->taskNum);
	add_assoc_long(return_value,"ideThread",pool->idle);
	add_assoc_long(return_value,"maxThread",pool->max_threads);
	add_assoc_long(return_value,"nowThread",pool->counter);
	add_assoc_long(return_value,"callbackEvent",0);

	if(IS_LONG == Z_TYPE_P(redisBack)){
		add_assoc_long(return_value,"callbackEvent",Z_LVAL_P(redisBack));
	}

	zval_ptr_dtor(&redisBack);
}

PHP_METHOD(CHttpPool,add){
	
	zval	*poolResourceId,
			*requestObject,
			*redisCallbackKey,
			*fileCallbackKey,
			*taskNum,
			*rejectType,
			*fileTrueName,
			*callback,
			*callParams,
			*redisBack;

	char	*taskName = NULL;
	int		type,
			taskNameLen = 0,
			waitCallbackEvent = 0;
	threadpool_t *pool;
	httpRequest_t *requestHttp;

	//get params
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|s",&requestObject,&taskName,&taskNameLen) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->add] must give string, object(CHttp)", 7001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	if(IS_OBJECT != Z_TYPE_P(requestObject)){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->add] must give a CHttp Object", 7001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	//chec is object from CHttp
	if(strcmp(Z_OBJCE_P(requestObject)->name,"CHttp") != 0){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->add] must give a CHttp Object", 7001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	//get pool resource
	poolResourceId = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("poolResourceId"),0 TSRMLS_CC);
	pool = zend_list_find(Z_LVAL_P(poolResourceId), &type);
	if(type != le_resource_httpPool){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->add] get thread pool resource fail", 7001 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	//wait to do callbackevent
	callback = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("callback"),0 TSRMLS_CC);
	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,"CQuickFrameHttpPool",1);
	CRedis_callFunction("main","llen",callParams,&redisBack TSRMLS_CC);
	zval_ptr_dtor(&callParams);
	if(IS_LONG == Z_TYPE_P(redisBack)){
		waitCallbackEvent = Z_LVAL_P(redisBack);
	}

	if(waitCallbackEvent > 0){
		if(IS_STRING == Z_TYPE_P(callback) && strlen(Z_STRVAL_P(callback)) > 0 ){
		}else{
			//clear redis
			zval *params1;
			MAKE_STD_ZVAL(params1);
			array_init(params1);
			add_next_index_string(params1,"CQuickFrameHttpPool",1);
			CRedis_callFunction("main","del",callParams,&redisBack TSRMLS_CC);
			zval_ptr_dtor(&callParams);
			zval_ptr_dtor(&redisBack);
			waitCallbackEvent = 0;
		}
	}

	//check task Num
	taskNum = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("maxTask"),0 TSRMLS_CC);
	rejectType = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("rejectType"),0 TSRMLS_CC);
	if(pool->taskNum > Z_LVAL_P(taskNum) || waitCallbackEvent > Z_LVAL_P(taskNum)){

		if(Z_LVAL_P(rejectType) != 1 && Z_LVAL_P(rejectType) != 2 && Z_LVAL_P(rejectType) != 3){
			Z_LVAL_P(rejectType) = 1;
		}

		//wait for use
		if(1 == Z_LVAL_P(rejectType)){

			while(1){
				//wait for now task less max limit 
				if(pool->taskNum <= Z_LVAL_P(taskNum) && waitCallbackEvent <= Z_LVAL_P(taskNum) ){
					break;
				}
				usleep(10000);

				//recheck redis
				if(pool->taskNum <= Z_LVAL_P(taskNum)){
					MAKE_STD_ZVAL(callParams);
					array_init(callParams);
					add_next_index_string(callParams,"CQuickFrameHttpPool",1);
					CRedis_callFunction("main","llen",callParams,&redisBack TSRMLS_CC);
					zval_ptr_dtor(&callParams);
					if(IS_LONG == Z_TYPE_P(redisBack)){
						waitCallbackEvent = Z_LVAL_P(redisBack);
					}else{
						waitCallbackEvent = 0;
					}
				}

			}
		}else if(2 == Z_LVAL_P(rejectType)){

			//throw a exception 
			zend_throw_exception(CRejectExceptionCe, "[CRejectException] ThreadPool is too busy, call [setRejectedTaskNum] to increase task limit or call [setRejectedPolicy] to set reject proxy", 7001 TSRMLS_CC);
			RETVAL_FALSE;
			return;

		}else if(3 == Z_LVAL_P(rejectType)){
			//ignore
			RETVAL_FALSE;
			return;
		}	
	}


	//parse CHttpObject to a string
	requestHttp = CHttpPool_parseCHttpToString(requestObject TSRMLS_C);

	//check use redis redisCallbackKey
	redisCallbackKey = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("redisCallbackKey"),0 TSRMLS_CC);
	fileCallbackKey = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("fileCallbackKey"),0 TSRMLS_CC);
	fileTrueName = zend_read_property(CHttpPoolCe,getThis(),ZEND_STRL("fileTrueName"),0 TSRMLS_CC);

	if(taskNameLen > 0){
		requestHttp->taskName = strdup(taskName);
	}else{
		requestHttp->taskName = strdup("");
	}
	requestHttp->fileName = NULL;
	requestHttp->redisHost = NULL;
	requestHttp->redisPort = 0;
	requestHttp->redisKey = NULL;
	requestHttp->redisPassword = NULL;
	requestHttp->callbackFun = 0;

	if(IS_STRING == Z_TYPE_P(callback) && strlen(Z_STRVAL_P(callback)) > 0 ){
		requestHttp->callbackFun = 1;
	}

	if(IS_STRING == Z_TYPE_P(fileTrueName) && strlen(Z_STRVAL_P(fileTrueName)) > 0){
		requestHttp->fileName = strdup(Z_STRVAL_P(fileTrueName));
	}

	//init main thread logsstring

	if((IS_STRING == Z_TYPE_P(redisCallbackKey) && strlen(Z_STRVAL_P(redisCallbackKey)) > 0) || (requestHttp->callbackFun == 1)){
		zval	*cconfigInstanceZval,
				*redisHost,
				*redisPort,
				*redisPassword;

		if(IS_STRING == Z_TYPE_P(redisCallbackKey)){
			requestHttp->redisKey = strdup(Z_STRVAL_P(redisCallbackKey));
		}

		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("REDIS_HOST",cconfigInstanceZval,&redisHost TSRMLS_CC);
		CConfig_load("REDIS_PORT",cconfigInstanceZval,&redisPort TSRMLS_CC);
		CConfig_load("REDIS_PASSWORD",cconfigInstanceZval,&redisPassword TSRMLS_CC);

		if(IS_STRING == Z_TYPE_P(redisHost) && strlen(Z_STRVAL_P(redisHost)) > 0){
			requestHttp->redisHost = strdup(Z_STRVAL_P(redisHost));
		}
		if(IS_STRING == Z_TYPE_P(redisPassword) && strlen(Z_STRVAL_P(redisPassword)) > 0){
			requestHttp->redisPassword = strdup(Z_STRVAL_P(redisPassword));
		}
		if(IS_LONG == Z_TYPE_P(redisPort)){
			requestHttp->redisPort = Z_LVAL_P(redisPort);
		}
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&redisHost);
		zval_ptr_dtor(&redisPort);
		zval_ptr_dtor(&redisPassword);
	}

	threadpool_add_task(pool,doHttpRequest,(void*)requestHttp);
	RETVAL_TRUE;
}

int CHttpPool_getInstance(zval **returnZval TSRMLS_DC){

	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CHttpPoolCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zval			*object;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CHttpPoolCe);

		//执行构造器
		if (CHttpPoolCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CHttpPoolCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CHttpPoolCe,ZEND_STRL("instance"),object TSRMLS_CC);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

PHP_METHOD(CHttpPool,getInstance){

	zval *object;

	CHttpPool_getInstance(&object TSRMLS_CC);

	RETVAL_ZVAL(object,1,1);
}

PHP_METHOD(CHttpPool,setThreadMaxNum){

	long	num;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&num) == FAILURE){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setThreadMaxNum] the params type error,the params is a int between 1 to 100", 7001 TSRMLS_CC);
		return;
	}

	if(num < 1 || num > 100){
		zend_throw_exception(CPoolExceptionCe, "[CPoolException] call [CHttpPool->setThreadMaxNum] the params type error,the params is a int between 1 to 100", 7001 TSRMLS_CC);
		return;
	}

	RETVAL_ZVAL(getThis(),1,0);
}


#endif