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

//zend¿‡∂‘œÛ
zend_class_entry	*CDebugCe;


PHP_METHOD(CDebug,debug);
PHP_METHOD(CDebug,dumpDBExecute);
PHP_METHOD(CDebug,dumpErrors);
PHP_METHOD(CDebug,dumpIncludeFiles);
PHP_METHOD(CDebug,dumpMemoryUsed);
PHP_METHOD(CDebug,dumpTimeCast);
PHP_METHOD(CDebug,dumpPluginLoaded);
PHP_METHOD(CDebug,dumpRequestData);
PHP_METHOD(CDebug,setHooks);
PHP_METHOD(CDebug,getDatabaseExecuteEnd);
PHP_METHOD(CDebug,getErrorsData);
PHP_METHOD(CDebug,getRequestShutdown);