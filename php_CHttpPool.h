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

//zend类对象
zend_class_entry	*CHttpPoolCe;

//类方法:创建应用
PHP_METHOD(CHttpPool,__construct);
PHP_METHOD(CHttpPool,__destruct);
PHP_METHOD(CHttpPool,getInstance);
PHP_METHOD(CHttpPool,setThreadMaxNum);
PHP_METHOD(CHttpPool,add);
PHP_METHOD(CHttpPool,setResultToRedis);
PHP_METHOD(CHttpPool,setResultToFile);
PHP_METHOD(CHttpPool,registerCallback);
PHP_METHOD(CHttpPool,getPoolStatus);
PHP_METHOD(CHttpPool,setRejectedPolicy);
PHP_METHOD(CHttpPool,setRejectedTaskNum);

typedef struct httpRequest 
{
	char *taskName;
	char *header;
	char *hosts;
	char *method;
	char *params;
	char *redisHost;
	char *redisPassword;
	char *redisKey;
	int	 redisPort;
	char *fileName;
	char *logs;
	int	 callbackFun;

}httpRequest_t;

typedef struct httpResponse 
{
	char *content;
	char *taskName;
	long http_code;
	double total_time;
	double namelookup_time;
	double connect_time;
	double pretransfer_time;
	
}httpResponse_t;