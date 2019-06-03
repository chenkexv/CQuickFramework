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
zend_class_entry	*CHttpCe,
					*CHttpResponseCe;

//类方法:创建应用
PHP_METHOD(CHttp,__construct);
PHP_METHOD(CHttp,getInstance);
PHP_METHOD(CHttp,setHeader);
PHP_METHOD(CHttp,setMethod);
PHP_METHOD(CHttp,setTimeout);
PHP_METHOD(CHttp,sendHttpRequest);
PHP_METHOD(CHttp,send);
PHP_METHOD(CHttp,setUrl);
PHP_METHOD(CHttp,setParams);
PHP_METHOD(CHttp,fllowRedirect);
PHP_METHOD(CHttp,setUesrAgentAndroid);
PHP_METHOD(CHttp,setUesrAgentiPhone);
PHP_METHOD(CHttp,setUesrAgentiPad);
PHP_METHOD(CHttp,setUesrAgentChrome);
PHP_METHOD(CHttp,setUesrAgentFireFox);
PHP_METHOD(CHttp,setCookie);
PHP_METHOD(CHttp,setCertificate);

PHP_METHOD(CHttpResponse,getHttpCode);
PHP_METHOD(CHttpResponse,getCastInfo);
PHP_METHOD(CHttpResponse,getContent);
PHP_METHOD(CHttpResponse,getNamelookTime);
PHP_METHOD(CHttpResponse,getTotalTime);
PHP_METHOD(CHttpResponse,setData);
PHP_METHOD(CHttpResponse,isSuccess);
PHP_METHOD(CHttpResponse,asArray);
PHP_METHOD(CHttpResponse,getRedirect);
PHP_METHOD(CHttpResponse,getPrimaryIp);
PHP_METHOD(CHttpResponse,getConnectTime);
PHP_METHOD(CHttpResponse,getContentType);
PHP_METHOD(CHttpResponse,getRequestHeader);
PHP_METHOD(CHttpResponse,getErrorInfo);
PHP_METHOD(CHttpResponse,getResponseHeader);

