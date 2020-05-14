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
zend_class_entry	*CTcpServerCe,
					*CTcpClientCe,
					*CSocketCe;

//类方法:创建应用
PHP_METHOD(CTcpServer,__construct);
PHP_METHOD(CTcpServer,__destruct);
PHP_METHOD(CTcpServer,getInstance);
PHP_METHOD(CTcpServer,bind);
PHP_METHOD(CTcpServer,listen);
PHP_METHOD(CTcpServer,on);
PHP_METHOD(CTcpServer,onData);
PHP_METHOD(CTcpServer,Action_worker);

PHP_METHOD(CTcpClient,__construct);
PHP_METHOD(CTcpClient,__destruct);
PHP_METHOD(CTcpClient,getInstance);
PHP_METHOD(CTcpClient,connect);
PHP_METHOD(CTcpClient,onConnect);
PHP_METHOD(CTcpClient,on);
PHP_METHOD(CTcpClient,onDisconnect);
PHP_METHOD(CTcpClient,emit);
PHP_METHOD(CTcpClient,close);


PHP_METHOD(CSocket,close);
PHP_METHOD(CSocket,client);
PHP_METHOD(CSocket,read);
PHP_METHOD(CSocket,emit);
PHP_METHOD(CSocket,getSocketId);
PHP_METHOD(CSocket,getRemoteIp);
PHP_METHOD(CSocket,getConnectTime);
PHP_METHOD(CSocket,getSessionId);
PHP_METHOD(CSocket,getProcessId);
PHP_METHOD(CSocket,getLastActiveTime);
