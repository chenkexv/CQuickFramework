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
zend_class_entry	*CRequestCe;


//类方法:创建应用

PHP_METHOD(CRequest,getController);
PHP_METHOD(CRequest,setController);
PHP_METHOD(CRequest,getAction);
PHP_METHOD(CRequest,setAction);
PHP_METHOD(CRequest,getModule);
PHP_METHOD(CRequest,setModule);
PHP_METHOD(CRequest,getInstance);
PHP_METHOD(CRequest,__construct);
PHP_METHOD(CRequest,run);
PHP_METHOD(CRequest,_checkActionPreFix);
PHP_METHOD(CRequest,routeDoing);
PHP_METHOD(CRequest,createController);
PHP_METHOD(CRequest,createAction);
PHP_METHOD(CRequest,execAction);
PHP_METHOD(CRequest,isSuccess);
PHP_METHOD(CRequest,closeRouter);
PHP_METHOD(CRequest,openRouter);
PHP_METHOD(CRequest,getUseRouterStatus);
PHP_METHOD(CRequest,Args);
PHP_METHOD(CRequest,getUrl);
PHP_METHOD(CRequest,getUri);
PHP_METHOD(CRequest,getIp);
PHP_METHOD(CRequest,getPreUrl);
PHP_METHOD(CRequest,getAgent);
PHP_METHOD(CRequest,getHost);
PHP_METHOD(CRequest,getStartTime);
PHP_METHOD(CRequest,getRegisterEventTime);
PHP_METHOD(CRequest,getErrorMessage);
PHP_METHOD(CRequest,getPath);
PHP_METHOD(CRequest,createUrl);
PHP_METHOD(CRequest,disablePOST);
PHP_METHOD(CRequest,disableGET);
PHP_METHOD(CRequest,isWap);
PHP_METHOD(CRequest,isCli);
PHP_METHOD(CRequest,end);
PHP_METHOD(CRequest,getRequestMethod);
PHP_METHOD(CRequest,removeXSS);

PHP_METHOD(CRequest,getAllMemory);
PHP_METHOD(CRequest,getFreeMemory);
PHP_METHOD(CRequest,getVirtualMemory);
PHP_METHOD(CRequest,getFreeVirtualMemory);
PHP_METHOD(CRequest,getMaxUseMemory);
PHP_METHOD(CRequest,getApacheMemory);
PHP_METHOD(CRequest,getCurrentProcessMemory);
PHP_METHOD(CRequest,getMysqlMemory);
PHP_METHOD(CRequest,getMysqlNTMemory);
PHP_METHOD(CRequest,getMemcacheMemory);
PHP_METHOD(CRequest,getRedisMemory);
PHP_METHOD(CRequest,getFreeDisk);
PHP_METHOD(CRequest,getAllDisk);
PHP_METHOD(CRequest,getProcessList);
PHP_METHOD(CRequest,getCPULoad);