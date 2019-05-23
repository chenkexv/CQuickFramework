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
zend_class_entry	*CHashTableCe;


//类方法:创建应用
PHP_METHOD(CHashTable,__construct);
PHP_METHOD(CHashTable,getInstance);
PHP_METHOD(CHashTable,setData);
PHP_METHOD(CHashTable,get);
PHP_METHOD(CHashTable,set);
PHP_METHOD(CHashTable,remove);
PHP_METHOD(CHashTable,clear);
PHP_METHOD(CHashTable,toString);
PHP_METHOD(CHashTable,toJson);
PHP_METHOD(CHashTable,toSerialize);
PHP_METHOD(CHashTable,toArray);
PHP_METHOD(CHashTable,contains);
PHP_METHOD(CHashTable,count);
PHP_METHOD(CHashTable,isEmpty);
PHP_METHOD(CHashTable,keys);
PHP_METHOD(CHashTable,values);
PHP_METHOD(CHashTable,sortByField);
PHP_METHOD(CHashTable,sort);
PHP_METHOD(CHashTable,ksort);
PHP_METHOD(CHashTable,exist);

