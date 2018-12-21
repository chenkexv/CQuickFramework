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
zend_class_entry	*CResultCe;


PHP_METHOD(CResult,asArray);
PHP_METHOD(CResult,setCache);
PHP_METHOD(CResult,setCastTime);
PHP_METHOD(CResult,getCastTime);
PHP_METHOD(CResult,count);
PHP_METHOD(CResult,offsetGet);
PHP_METHOD(CResult,getKey);
PHP_METHOD(CResult,current);
PHP_METHOD(CResult,setValue);
PHP_METHOD(CResult,setIsMaster);
PHP_METHOD(CResult,getIsMaster);
PHP_METHOD(CResult,setIsCache);
PHP_METHOD(CResult,isCache);
PHP_METHOD(CResult,setSql);
PHP_METHOD(CResult,getSql);
PHP_METHOD(CResult,getAction);
PHP_METHOD(CResult,getWhere);
PHP_METHOD(CResult,setWhereValue);