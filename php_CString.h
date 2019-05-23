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
zend_class_entry	*CStringCe;


//类方法:创建应用
PHP_METHOD(CString,__construct);
PHP_METHOD(CString,getInstance);
PHP_METHOD(CString,getFirstChar);
PHP_METHOD(CString,setData);
PHP_METHOD(CString,getCharst);
PHP_METHOD(CString,toAscii);
PHP_METHOD(CString,toChar);
PHP_METHOD(CString,at);
PHP_METHOD(CString,substr);
PHP_METHOD(CString,length);
PHP_METHOD(CString,toUTF8);
PHP_METHOD(CString,toGBK);
PHP_METHOD(CString,toArray);
PHP_METHOD(CString,isUTF8);
PHP_METHOD(CString,isGBK);
PHP_METHOD(CString,isSimpleString);
PHP_METHOD(CString,isComplexString);
PHP_METHOD(CString,isIp);
PHP_METHOD(CString,isUrl);
PHP_METHOD(CString,isEmail);
PHP_METHOD(CString,isDate);
PHP_METHOD(CString,isJson);
PHP_METHOD(CString,isPhone);
PHP_METHOD(CString,isIDCard);
PHP_METHOD(CString,isNumber);
