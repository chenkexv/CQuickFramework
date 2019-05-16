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
zend_class_entry	*CConsumerCe;


PHP_METHOD(CConsumer,setEmptySleepTime);
PHP_METHOD(CConsumer,setProcessMaxNum);
PHP_METHOD(CConsumer,setMemoryLimit);
PHP_METHOD(CConsumer,setTimeLimit);
PHP_METHOD(CConsumer,registerHeartbeatCallback);
PHP_METHOD(CConsumer,registerMessageCallback);
PHP_METHOD(CConsumer,setProducer);
PHP_METHOD(CConsumer,setLogName);
PHP_METHOD(CConsumer,ack);
PHP_METHOD(CConsumer,run);
PHP_METHOD(CConsumer,setMQId);
PHP_METHOD(CConsumer,setMQType);
PHP_METHOD(CConsumer,setListKey);