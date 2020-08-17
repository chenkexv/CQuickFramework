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
					*CSocketCe,
					*CSocketClientCe,
					*CTcpGatewayCe;

//类方法:创建应用
PHP_METHOD(CTcpServer,__construct);
PHP_METHOD(CTcpServer,__destruct);
PHP_METHOD(CTcpServer,getInstance);
PHP_METHOD(CTcpServer,bind);
PHP_METHOD(CTcpServer,listen);
PHP_METHOD(CTcpServer,on);
PHP_METHOD(CTcpServer,onData);
PHP_METHOD(CTcpServer,setWorker);
PHP_METHOD(CTcpServer,onError);
PHP_METHOD(CTcpServer,gateway);


PHP_METHOD(CTcpServer,sendToSessionId);
PHP_METHOD(CTcpServer,sendToUid);
PHP_METHOD(CTcpServer,getGroup);
PHP_METHOD(CTcpServer,broadcastToGroup);
PHP_METHOD(CTcpServer,broadcast);
PHP_METHOD(CTcpServer,getAllConnection);




PHP_METHOD(CTcpClient,__construct);
PHP_METHOD(CTcpClient,__destruct);
PHP_METHOD(CTcpClient,getInstance);
PHP_METHOD(CTcpClient,connect);
PHP_METHOD(CTcpClient,on);
PHP_METHOD(CTcpClient,send);
PHP_METHOD(CTcpClient,close);
PHP_METHOD(CTcpClient,onError);
PHP_METHOD(CTcpClient,sleep);
PHP_METHOD(CTcpClient,setHeartbeatInterval);


PHP_METHOD(CSocket,close);
PHP_METHOD(CSocket,getClientInfo);
PHP_METHOD(CSocket,read);
PHP_METHOD(CSocket,send);
PHP_METHOD(CSocket,getGroup);
PHP_METHOD(CSocket,getSocketId);
PHP_METHOD(CSocket,getRemoteIp);
PHP_METHOD(CSocket,getConnectTime);
PHP_METHOD(CSocket,getSessionId);
PHP_METHOD(CSocket,getProcessId);
PHP_METHOD(CSocket,getLastActiveTime);
PHP_METHOD(CSocket,joinGroup);
PHP_METHOD(CSocket,leaveGroup);
PHP_METHOD(CSocket,bindUid);
PHP_METHOD(CSocket,unBindUid);
PHP_METHOD(CSocket,setSession);
PHP_METHOD(CSocket,getSession);
PHP_METHOD(CSocket,delSession);
PHP_METHOD(CSocket,clearSession);


PHP_METHOD(CSocketClient,read);
PHP_METHOD(CSocketClient,send);


PHP_METHOD(CTcpGateway,__construct);
PHP_METHOD(CTcpGateway,__destruct);
PHP_METHOD(CTcpGateway,getInstance);
PHP_METHOD(CTcpGateway,bind);
PHP_METHOD(CTcpGateway,listen);
PHP_METHOD(CTcpGateway,on);
PHP_METHOD(CTcpGateway,onData);
PHP_METHOD(CTcpGateway,onError);
