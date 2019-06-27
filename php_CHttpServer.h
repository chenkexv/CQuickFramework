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
zend_class_entry	*CMicroServerCe,
					*CMicroResponseCe,
					*CMicroRequestCe;

//类方法:创建应用
PHP_METHOD(CMicroServer,__construct);
PHP_METHOD(CMicroServer,getInstance);
PHP_METHOD(CMicroServer,bind);
PHP_METHOD(CMicroServer,listen);
PHP_METHOD(CMicroServer,onRequest);
PHP_METHOD(CMicroServer,onRoute);
PHP_METHOD(CMicroResponse,setHeader);
PHP_METHOD(CMicroResponse,setBody);
PHP_METHOD(CMicroResponse,send);
PHP_METHOD(CMicroRequest,getHeader);
PHP_METHOD(CMicroRequest,getRawHeader);
PHP_METHOD(CMicroRequest,getBody);
PHP_METHOD(CMicroRequest,getUrl);
PHP_METHOD(CMicroRequest,getUri);
PHP_METHOD(CMicroRequest,getAgent);
PHP_METHOD(CMicroRequest,getHost);
PHP_METHOD(CMicroRequest,getIp);
PHP_METHOD(CMicroRequest,isWap);
PHP_METHOD(CMicroRequest,isCli);
PHP_METHOD(CMicroRequest,getPath);
PHP_METHOD(CMicroRequest,getQueryString);
