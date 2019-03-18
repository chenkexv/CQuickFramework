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
zend_class_entry	*CRabbitHelperCe;


//类方法:创建应用
PHP_METHOD(CRabbitHelper,getInstance);
PHP_METHOD(CRabbitHelper,__construct);
PHP_METHOD(CRabbitHelper,getOverview);
PHP_METHOD(CRabbitHelper,getQueues);
PHP_METHOD(CRabbitHelper,getAckRate);
PHP_METHOD(CRabbitHelper,getGetRate);
PHP_METHOD(CRabbitHelper,getPublishRate);
PHP_METHOD(CRabbitHelper,getDeliverGetRate);
PHP_METHOD(CRabbitHelper,getUnAckRate);
PHP_METHOD(CRabbitHelper,getUnAckNum);
PHP_METHOD(CRabbitHelper,getQueueInfo);
PHP_METHOD(CRabbitHelper,getQueueAckRate);
PHP_METHOD(CRabbitHelper,getQueueGetRate);
PHP_METHOD(CRabbitHelper,getQueuePublishRate);
PHP_METHOD(CRabbitHelper,getQueueDeliverGetRate);
PHP_METHOD(CRabbitHelper,getQueueUnAckRate);
PHP_METHOD(CRabbitHelper,getQueueUnAckNum);
PHP_METHOD(CRabbitHelper,getQueueIdleSince);
PHP_METHOD(CRabbitHelper,getQueueState);
PHP_METHOD(CRabbitHelper,getQueueConsumerNum);
PHP_METHOD(CRabbitHelper,getNodes);
PHP_METHOD(CRabbitHelper,getNodeInfo);
PHP_METHOD(CRabbitHelper,getNodeMemoryUsedRate);
PHP_METHOD(CRabbitHelper,getNodeFdUsedRate);
PHP_METHOD(CRabbitHelper,getNodeSocketUsedRate);
PHP_METHOD(CRabbitHelper,getNodeErlangProcessUsedRate);
