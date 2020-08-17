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

#ifndef PHP_WIN32

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"
#include "php_CQuickFramework.h"
#include "php_CWebApp.h"
#include "php_CController.h"
#include "php_CApplication.h"
#include "php_CException.h"
#include "php_CTcpServer.h"
#include "php_CHttpServer.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/utsname.h>

#define MSG_USER 1
#define MSG_HEART 2

#define MAX_SENDLIST_LEN  65535

//服务器全局变量
static zval *serverSocketList = NULL;
static zval *serverSessionList = NULL;
static smart_str serverTempBuffer[10240] = {0};
static zval *serverErrorHandler = NULL;
static zval *serverSendList = NULL;
static zval *serverMainObject = NULL;
static int serverListenSocket = 0;
static int serverAccpetLock = 0;
static zval *serverGroup = NULL;
static int serverToGatewaySocket = 0;
static char *serverSessionId = NULL;
static int serverToGatewayStatus = 0; //0 未就绪 1连接成功 2断开连接， 3正在重试 4主动断开不再重试
static int serverGatewayLastActiveTime = 0;
static int serverLastCheckStaus = 0;
static int serverEpollFd = 0;
static zval *serverLocalSessionData = NULL;
static zval *serverUidData = NULL;
static zval *serverAsyncList = NULL;	//服务器异步查询的事件列表

//网关全局变量
static zval *gatewaySocketList = NULL;		//worker的记录
static zval *gatewaySessionList = NULL;		//worker的记录
static zval *gatewayUserSessionList = NULL;
static zval *gatewayUidSessionList = NULL;	//uid和session的对应关系
static zval *gatewaySendList = NULL;
static int gatewayLastCheckTime = 0;
static zval *gatewayGroupList = NULL;	//群组列表
static zval *gatewaySessionData = NULL;	//存放session



//客户端全局变量
static int clientVersion = 1;
static int clientErrNums = 0;
static int clientStatus = 0;	//0 未就绪 1连接成功 2断开连接， 3正在重试 4主动断开不再重试
static int clientSocketId = 0;
static char *clientHost = NULL;
static int clientPort = 0;
static double clientLastMessageTime = 0.0;
static int clientRecProcessId = 0;
static int clientMainEpollFd = 0;
static zval *clientPHPObject = NULL;
static zval *clientErrorHandler = NULL;
static int clientRecProcessWritePipeFd = 0;
static int clientRecProcessReadPipeFd = 0;
static zval *clientSendList = NULL;
static char *clientSessionId = NULL;
static int clientStopMasterWrite = 0;
static int clientLastCheckSocket = 0;
static int clientLastHeartbeat = 0;


//zend类方法
zend_function_entry CTcpGateway_functions[] = {
	PHP_ME(CTcpGateway,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CTcpGateway,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CTcpGateway,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTcpGateway,bind,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpGateway,listen,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpGateway,on,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpGateway,onData,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpGateway,onError,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

zend_function_entry CTcpServer_functions[] = {
	PHP_ME(CTcpServer,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CTcpServer,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CTcpServer,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTcpServer,bind,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,setWorker,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,listen,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpServer,on,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpServer,onData,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpServer,onError,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpServer,gateway,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpServer,sendToSessionId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,sendToUid,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,getGroup,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,broadcastToGroup,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,broadcast,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpServer,getAllConnection,NULL,ZEND_ACC_PUBLIC)
	



	{NULL, NULL, NULL}
};

zend_function_entry CTcpClient_functions[] = {
	PHP_ME(CTcpClient,__construct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CTcpClient,__destruct,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
	PHP_ME(CTcpClient,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTcpClient,connect,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CTcpClient,on,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpClient,onError,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpClient,send,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpClient,close,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpClient,sleep,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CTcpClient,setHeartbeatInterval,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

zend_function_entry CSocket_functions[] = {
	PHP_ME(CSocket,close,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getClientInfo,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,read,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,send,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getGroup,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getSocketId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getRemoteIp,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getConnectTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getSessionId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getProcessId,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getLastActiveTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,joinGroup,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,leaveGroup,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,bindUid,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,unBindUid,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,getSession,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,setSession,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,delSession,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CSocket,clearSession,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

zend_function_entry CSocketClient_functions[] = {
	PHP_ME(CSocketClient,read,NULL,ZEND_ACC_PUBLIC )
	PHP_ME(CSocketClient,send,NULL,ZEND_ACC_PUBLIC )
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CTcpGateway)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CTcpGateway",CTcpGateway_functions);
	CTcpGatewayCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);


	zend_declare_property_null(CTcpGatewayCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CTcpGatewayCe, ZEND_STRL("host"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpGatewayCe, ZEND_STRL("port"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	
	//dataCallbackObject
	zend_declare_property_null(CTcpGatewayCe, ZEND_STRL("eventTable"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CTcpGatewayCe, ZEND_STRL("eventDefault"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}


//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CTcpServer)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CTcpServer",CTcpServer_functions);
	CTcpServerCe = zend_register_internal_class_ex(&funCe,CControllerCe,NULL TSRMLS_CC);


	zend_declare_property_null(CTcpServerCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CTcpServerCe, ZEND_STRL("host"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpServerCe, ZEND_STRL("port"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpServerCe, ZEND_STRL("worker"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CTcpServerCe, ZEND_STRL("pidList"),ZEND_ACC_PRIVATE TSRMLS_CC);

	zend_declare_property_string(CTcpServerCe, ZEND_STRL("gatewayHost"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpServerCe, ZEND_STRL("gatewayPort"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpServerCe, ZEND_STRL("gatewayUse"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	
	//dataCallbackObject
	zend_declare_property_null(CTcpServerCe, ZEND_STRL("eventTable"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CTcpServerCe, ZEND_STRL("eventDefault"),ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CTcpClient)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CTcpClient",CTcpClient_functions);
	CTcpClientCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CTcpClientCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CTcpClientCe, ZEND_STRL("host"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpClientCe, ZEND_STRL("port"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CTcpClientCe, ZEND_STRL("connectTime"),0.00,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CTcpClientCe, ZEND_STRL("eventTable"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CTcpClientCe, ZEND_STRL("heartbeatInterval"),120,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CSocket)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CSocket",CSocket_functions);
	CSocketCe = zend_register_internal_class(&funCe TSRMLS_CC);


	zend_declare_property_string(CSocketCe, ZEND_STRL("message"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CSocketCe, ZEND_STRL("socketId"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CSocketCe, ZEND_STRL("client"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CSocketCe, ZEND_STRL("remoteIp"),"0.0.0.0",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CSocketCe, ZEND_STRL("connectTime"),0.00,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_double(CSocketCe, ZEND_STRL("lastActiveTime"),0.00,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CSocketCe, ZEND_STRL("processId"),0,ZEND_ACC_PRIVATE TSRMLS_CC);


	//socket对象类型  1为服务器使用的对象  2为客户端使用的对象
	zend_declare_property_long(CSocketCe, ZEND_STRL("socketType"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	return SUCCESS;
}

CMYFRAME_REGISTER_CLASS_RUN(CSocketClient)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CSocketClient",CSocketClient_functions);
	CSocketClientCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_string(CSocketClientCe, ZEND_STRL("message"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CSocketClientCe, ZEND_STRL("socketId"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CSocketClientCe, ZEND_STRL("socketType"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

void writeLogs(char *format, ...){
	
	va_list args;
    int ret;
    char *buffer;
    int size;

    va_start(args, format);
    size = vspprintf(&buffer, 0, format, args);

	char	*showDate;
	php_date("Y-m-d h:i:s",&showDate);
   
	php_printf("[%s]=>%s",showDate,buffer);
	
	efree(showDate);
    efree(buffer);
    va_end(args);
}

int startTcpListen(char *host,int port){

	int sock = socket(AF_INET,SOCK_STREAM,0);
    if( sock < 0 ){
		return -2;
    }

    //if TIME_WAIT server will restart
    int opt = 1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);// 地址为任意类型
    local.sin_port = htons(port);

	//设为非阻塞
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFD, 0)|O_NONBLOCK);

    if( bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0 ){
		return -3;
    }

    if( listen(sock,1020) < 0 ){
		return -4;
    }
    return sock;
}


//主进程 检查所有worker进程状态
void createWorkProcess(int forkNum,int listenfd,int lock,zval *object,int isFrist TSRMLS_DC);
void createMessage(char *event,char *message,int type,char **stringMessage,int useType TSRMLS_DC);

void checkSocketStatus(){


	if(clientStatus == 2){

		//若socket状态为 断开时，设为正在重试
		clientStatus == 3;

		//重连socket
		int restetStatus = resetSocketConnect();

	}
}

void timerChecker(void(*prompt_info)())
{
	struct sigaction tact;
	memset(&tact, 0, sizeof(tact));
	tact.sa_handler = prompt_info;
	tact.sa_flags = 0;
	sigemptyset(&tact.sa_mask);
	sigaction(SIGALRM, &tact, NULL);
}


void initTimer(){
	struct itimerval value;
	//设定执行任务的时间间隔为1秒0微秒
	value.it_value.tv_sec = 1;
	value.it_value.tv_usec = 0;
	value.it_interval = value.it_value;
	setitimer(ITIMER_REAL, &value, NULL);
}

void destoryTimer(){
	struct itimerval value;
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval = value.it_value;
	setitimer(ITIMER_REAL, &value, NULL);
}


int checkTcpServerWorkerStatus(zval *object TSRMLS_DC){

	//检查当前进程状态
	int		i,h,processStatus,aliveNum = 0,
			needCreate = 0,
			need = 0;
	zval	*pidList;
	char	*key;
	ulong	uKey;

	pidList = zend_read_property(CTcpServerCe,serverMainObject,ZEND_STRL("pidList"),1 TSRMLS_CC);
	h = zend_hash_num_elements(Z_ARRVAL_P(pidList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(pidList));

	aliveNum = 0;
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(pidList),&key,&uKey,0);
		//检查进程存活
		processStatus = -1;
		processStatus = kill(uKey,0);
		if(processStatus == 0){
			aliveNum++;
		}else{
			zend_hash_index_del(Z_ARRVAL_P(pidList),uKey);
		}
		zend_hash_move_forward(Z_ARRVAL_P(pidList));
	}

	need = getCreateWorkerNums(object TSRMLS_CC);
	if(need > aliveNum){
		needCreate = need - aliveNum;
		writeLogs("[CTcpServer] The master check child status,now alive [%d],need keep num[%d],need create[%d]\n",aliveNum,need,needCreate);
		
		createWorkProcess(needCreate,serverListenSocket,serverAccpetLock,serverMainObject,2 TSRMLS_CC);
	}
	

}

void catchTcpChildSig(int sig){
	writeLogs("[CTcpServer] [%d-%d] Receive a child process exit signal [%d]\n",getppid(),getpid(),sig);
	int endPid = wait(NULL);
	writeLogs("[CTcpServer] The process for determining the unexpected termination is [%d]\n",endPid);

	zval	*pidList = zend_read_property(CTcpServerCe,serverMainObject,ZEND_STRL("pidList"),1 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(pidList) && zend_hash_index_exists(Z_ARRVAL_P(pidList),endPid)){
		zend_hash_index_del(Z_ARRVAL_P(pidList),endPid);
	}
	checkTcpServerWorkerStatus(serverMainObject TSRMLS_CC);
}

//向网关服务器发送消息
void sendToGateway(char *stringMessage){

	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,serverToGatewaySocket);
	add_index_string(sendMessage,1,stringMessage,1);

	//判断客户端是否还在现在
	char socketIndex[100];
	sprintf(socketIndex,"%d_%d",getpid(),serverToGatewaySocket);

	//获取此socket的列表
	zval **thisSocketList;
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(serverSendList),serverToGatewaySocket,(void**)&thisSocketList) && IS_ARRAY == Z_TYPE_PP(thisSocketList) ){
	}else{
		//键不存在则创建一条
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_index_zval(serverSendList,serverToGatewaySocket,saveArray);
		zend_hash_index_find(Z_ARRVAL_P(serverSendList),serverToGatewaySocket,(void**)&thisSocketList);
	}

	add_next_index_zval(*thisSocketList,sendMessage);
}

//创建多个子进程
void createWorkProcess(int forkNum,int listenfd,int lock,zval *object,int isFrist TSRMLS_DC){

	int		i = 0;

	zval	*pidList = zend_read_property(CTcpServerCe,object,ZEND_STRL("pidList"),1 TSRMLS_CC);

	for(i = 0 ; i < forkNum ;i++){

		int forkPid = -1;
		forkPid=fork();
		if(forkPid==-1){
			continue;
		}else if(forkPid == 0){

			//初始化子进程
			workProcessInit(listenfd,lock,object TSRMLS_CC);

		}else{

			signal(SIGCHLD, catchTcpChildSig);

			//将所有子进程的PID保存
			add_index_long(pidList,forkPid,1);

			if(i == forkNum - 1 && isFrist == 1){

				while(1){

					//每30秒检测一次子进程状态
					sleep(30);
					checkTcpServerWorkerStatus(object TSRMLS_CC);
				}

			}

		}
	}

}

void timerCallback();
void catchTcpClientChildSig(int sig){
	writeLogs("[CTcpClient] [%d-%d] Receive a child process exit signal [%d]\n",getppid(),getpid(),sig);
	int endPid = wait(NULL);
	writeLogs("[CTcpClient] The process for determining the unexpected termination is [%d]\n",endPid);
	if(endPid == clientRecProcessId){
		if(clientStatus != 4){
			clientStatus = 2;

			//安装定时器
			timerChecker(timerCallback);
			initTimer();

			//重启socket
			checkSocketStatus();
		}
	}
}

static int read_to_buf(const char *filename, void *buf, int len)
{
	int fd;
	int ret;
	
	if(buf == NULL || len < 0){
		printf("%s: illegal para\n", __func__);
		return -1;
	}
 
	memset(buf, 0, len);
	fd = open(filename, O_RDONLY);
	if(fd < 0){
		perror("open:");
		return -1;
	}
	ret = read(fd, buf, len);
	close(fd);
	return ret;
}
 
static char *getCmdline(int pid, char *buf, int len)
{
	char filename[32];
	char *name = NULL;
	int n = 0;
	
	if(pid < 1 || buf == NULL || len < 0){
		printf("%s: illegal para\n", __func__);
		return NULL;
	}
		
	snprintf(filename, 32, "/proc/%d/cmdline", pid);
	n = read_to_buf(filename, buf, len);
	if(n < 0)
		return NULL;
 
	if(buf[n-1]=='\n')
		buf[--n] = 0;
 
	name = buf;
	while(n) {
		if(((unsigned char)*name) < ' ')
			*name = ' ';
		name++;
		n--;
	}
	*name = 0;
	name = NULL;
 
	if(buf[0])
		return buf;
 
	return NULL;	
}

int throwClientException(char *message){

}

int throwServerException(char *message){
	if(serverErrorHandler == NULL){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,message);
		return 1;
	}

	//触发错误事件
	zval	*callback;
	char	*callback_name = NULL;
	callback = serverErrorHandler;
	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		efree(callback_name);
		zend_throw_exception(CMicroServerExceptionCe, message, 10000 TSRMLS_CC);
		return 0;
	}

	zval	constructVal,
			contruReturn,
			*paramsList[1];

	INIT_ZVAL(constructVal);
	ZVAL_STRING(&constructVal,callback_name,0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_STRING(paramsList[0],message,1);
	int callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,1, paramsList TSRMLS_CC);

	zval_dtor(&contruReturn);
	zval_ptr_dtor(&paramsList[0]);
	efree(callback_name);
	return 2;
}

void getSocketRemoteIp(int fd,char **remoteIp){

	 struct sockaddr_in clientaddr1;
     memset(&clientaddr1, 0x00, sizeof(clientaddr1));
     socklen_t nl=sizeof(clientaddr1);
     getpeername(fd,(struct sockaddr*)&clientaddr1,&nl);
     *remoteIp = estrdup(inet_ntoa(clientaddr1.sin_addr)); 
}

int server_sendMessage(int fd,char *stringMessage){
	
	//判断客户端是否还在现在
	char socketIndex[100];
	sprintf(socketIndex,"%d_%d",getpid(),fd);
	if(IS_ARRAY == Z_TYPE_P(serverSocketList) && zend_hash_exists(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1) ){
	}else{
		return -1;
	}

	//获取此socket的列表
	zval **thisSocketList;
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(serverSendList),fd,(void**)&thisSocketList) && IS_ARRAY == Z_TYPE_PP(thisSocketList) ){
	}else{
		//键不存在则创建一条
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_index_zval(serverSendList,fd,saveArray);
		zend_hash_index_find(Z_ARRVAL_P(serverSendList),fd,(void**)&thisSocketList);
	}

	if(IS_ARRAY != Z_TYPE_PP(thisSocketList)){
		return -3;
	}

	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,fd);
	char *sendStringMessage;
	spprintf(&sendStringMessage,0,"%s%c",stringMessage,'\n');
	add_index_string(sendMessage,1,sendStringMessage,0);

	//写入一条消息
	add_next_index_zval(*thisSocketList,sendMessage);
	return 1;
}

int processServerSystemMessage(int fd,zval *jsonDecoder,zval *object TSRMLS_DC){

	zval **messageEvent;

	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&messageEvent);

	//连接事件
	if(strcmp(Z_STRVAL_PP(messageEvent),"connect") == 0){


		//回送客户端消息 要求上报客户端基本信息  [2,"connect",""]
		server_sendMessage(fd,"WzIsImNvbm5lY3QiLCIiXQ==");

		return 1;
	}

	
	//处理系统事件
	if(strcmp(Z_STRVAL_PP(messageEvent),"clientHeartbeat") == 0){
		
		//存续socket最后激活时间
		zval	**socketInfo;
		char	socketIndex[100];
		sprintf(socketIndex,"%d_%d",getpid(),fd);
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1,(void**)&socketInfo) && IS_ARRAY == Z_TYPE_PP(socketInfo)){
		}else{
			return -1;
		}

		zval *timeZval;
		microtime(&timeZval);
		add_assoc_double(*socketInfo,"lastActiveTime",Z_DVAL_P(timeZval));
		zval_ptr_dtor(&timeZval);
	}

	//连接时客户端上报的基本信息
	if(strcmp(Z_STRVAL_PP(messageEvent),"clientReport") == 0){

			//存续socket最后激活时间
		zval	**socketInfo;
		char	socketIndex[100];
		sprintf(socketIndex,"%d_%d",getpid(),fd);
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1,(void**)&socketInfo) && IS_ARRAY == Z_TYPE_PP(socketInfo)){
		}else{
			return -1;
		}

		//解析2
		zval	**messageContent,
				*messageArr,
				**sessionIdZval;

		zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),2,(void**)&messageContent);
		json_decode(Z_STRVAL_PP(messageContent),&messageArr);

		char *sessionId;
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageArr),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval) ){
			sessionId = estrdup(Z_STRVAL_PP(sessionIdZval));
		}else{
			sessionId = estrdup("unknow");
		}
		add_assoc_string(*socketInfo,"sessionId",sessionId,1);
		if(IS_ARRAY == Z_TYPE_P(messageArr)){
			add_assoc_zval(*socketInfo,"client",messageArr);
		}else{
			zval_ptr_dtor(&messageArr);
			zval *clientInfo;
			MAKE_STD_ZVAL(clientInfo);
			array_init(clientInfo);
			add_assoc_zval(*socketInfo,"client",clientInfo);
		}

		//记录serverSessionList
		add_assoc_long(serverSessionList,sessionId,fd);

		//发送消息给网关服务器
		char	acceptMessage[200],
				*gatewayMessage;
		sprintf(acceptMessage,"{\"sessionId\":\"%s\"}",sessionId);
		createMessage("connect",acceptMessage,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
		sendToGateway(gatewayMessage);
		efree(gatewayMessage);
		efree(sessionId);


		//回送客户端消息 触发连接成功
		server_sendMessage(fd,"WzIsImNvbm5lY3RlZCIsIiJd");

		//收到客户端回复后出发connect事件 WzEsImNvbm5lY3QiLCIiXQ==  [1,"connect",""]
		return processMessage(fd,"WzEsImNvbm5lY3QiLCIiXQ==",object TSRMLS_CC);
	}

}

int processMessage(int fd,char *thisMessage,zval *object TSRMLS_DC){

	char	*base64Decoder;
	zval	*jsonDecoder,
			*eventTable,
			*eventDefault,
			**messageEvent,
			**eventCallback,
			**stringMessage,
			**messageType;

	base64Decode(thisMessage,&base64Decoder);
	json_decode(base64Decoder,&jsonDecoder);

	if(zend_hash_num_elements(Z_ARRVAL_P(jsonDecoder)) < 3){
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		char *errorMessage;
		spprintf(&errorMessage,0,"some message parse error : %s",thisMessage);
		throwServerException(errorMessage);
		efree(errorMessage);
		return -1;
	}

	//当前的事件名称
	if(	SUCCESS == zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),0,(void**)&messageType) &&
		SUCCESS == zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&messageEvent) &&
		SUCCESS == zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),2,(void**)&stringMessage) ){
	}else{
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		return -2;
	}

	if(IS_LONG == Z_TYPE_PP(messageType) && 2 == Z_LVAL_PP(messageType)){
		processServerSystemMessage(fd,jsonDecoder,object TSRMLS_CC);
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		return 1;
	}

	//检查是否存在指定的event
	eventTable = zend_read_property(CTcpServerCe,object,ZEND_STRL("eventTable"), 0 TSRMLS_CC);

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventTable),Z_STRVAL_PP(messageEvent),strlen(Z_STRVAL_PP(messageEvent))+1,(void**)&eventCallback) && IS_OBJECT == Z_TYPE_PP(eventCallback) ){

		//向PHP层回调
		zval	*callParams;

		MAKE_STD_ZVAL(callParams);
		object_init_ex(callParams,CSocketCe);
		zend_update_property_string(CSocketCe,callParams,ZEND_STRL("message"),Z_STRVAL_PP(stringMessage) TSRMLS_CC);
		zend_update_property_long(CSocketCe,callParams,ZEND_STRL("socketId"),fd TSRMLS_CC);
		zend_update_property_long(CSocketCe,callParams,ZEND_STRL("socketType"),1 TSRMLS_CC);
		zend_update_property_long(CSocketCe,callParams,ZEND_STRL("processId"),getpid() TSRMLS_CC);

		//生成客户端信息
		zval **socketInfo;
		char socketIndex[100];
		sprintf(socketIndex,"%d_%d",getpid(),fd);
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1,(void**)&socketInfo) && IS_ARRAY == Z_TYPE_PP(socketInfo)){
			zval **thisInfoItem;
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(socketInfo),"remoteIp",strlen("remoteIp")+1,(void**)&thisInfoItem) && IS_STRING == Z_TYPE_PP(thisInfoItem) ){
				char *ip;
				ip = estrdup(Z_STRVAL_PP(thisInfoItem));
				zend_update_property_string(CSocketCe,callParams,ZEND_STRL("remoteIp"),ip TSRMLS_CC);
				efree(ip);
			}
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(socketInfo),"connectTime",strlen("connectTime")+1,(void**)&thisInfoItem) && IS_DOUBLE == Z_TYPE_PP(thisInfoItem) ){
				zend_update_property_double(CSocketCe,callParams,ZEND_STRL("connectTime"),Z_DVAL_PP(thisInfoItem) TSRMLS_CC);
			}
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(socketInfo),"client",strlen("client")+1,(void**)&thisInfoItem) && IS_ARRAY == Z_TYPE_PP(thisInfoItem) ){
				zval *saveItem;
				MAKE_STD_ZVAL(saveItem);
				ZVAL_ZVAL(saveItem,*thisInfoItem,1,0);
				zend_update_property(CSocketCe,callParams,ZEND_STRL("client"),saveItem TSRMLS_CC);
				zval_ptr_dtor(&saveItem);
			}
		}


		char	*callback_name = NULL;
		zval	*callback;
		callback = *eventCallback;
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			efree(base64Decoder);
			zval_ptr_dtor(&callParams);
			zval_ptr_dtor(&jsonDecoder);
			return 0;
		}

		zval	constructVal,
				contruReturn,
				*paramsList[1];

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callback_name,0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],callParams,1,0);
		int callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,1, paramsList TSRMLS_CC);

		zval_dtor(&contruReturn);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&callParams);
		efree(callback_name);
	}

	efree(base64Decoder);
	zval_ptr_dtor(&jsonDecoder);
}

void serverClearLocalData(char *sessionId){
	
	if(zend_hash_exists(Z_ARRVAL_P(serverGroup),sessionId,strlen(sessionId)+1)){
		zend_hash_del(Z_ARRVAL_P(serverGroup),sessionId,strlen(sessionId)+1);
	}

	if(zend_hash_exists(Z_ARRVAL_P(serverSessionList),sessionId,strlen(sessionId)+1)){
		zend_hash_del(Z_ARRVAL_P(serverSessionList),sessionId,strlen(sessionId)+1);
	}

	//清理本地uidsession
	int		i,h;
	zval	**thisSession;
	char	*key;
	ulong	uKey;
	h = zend_hash_num_elements(Z_ARRVAL_P(serverUidData));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverUidData));
	for(i = 0 ; i < h; i++){
		if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(serverUidData),(void**)&thisSession) && IS_STRING == Z_TYPE_PP(thisSession) && strcmp(Z_STRVAL_PP(thisSession),sessionId) == 0 ){
			zend_hash_get_current_key(Z_ARRVAL_P(serverUidData),&key,&uKey,0);
			zend_hash_del(Z_ARRVAL_P(serverUidData),key,strlen(key)+1);
		}
		zend_hash_move_forward(Z_ARRVAL_P(serverUidData));
	}

}

//服务器关闭客户端连接
void serverCloseClientSocket(int socketFd,zval *object){
	
	//检查socket列表
	char socketIndex[100];
	sprintf(socketIndex,"%d_%d",getpid(),socketFd);

	if(IS_ARRAY == Z_TYPE_P(serverSocketList) && zend_hash_exists(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1) ){
		//触发关闭事件
		//WzEsImRpc2Nvbm5lY3QiLCIiXQ==  [1,"disconnect",""]
		processMessage(socketFd,"WzEsImRpc2Nvbm5lY3QiLCIiXQ==",object TSRMLS_CC);

		//通知网关
		zval	**thisSocketInfo,
				**sessionIdZval;

		if( SUCCESS == zend_hash_find(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1,(void**)&thisSocketInfo) && IS_ARRAY == Z_TYPE_PP(thisSocketInfo) &&
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisSocketInfo),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval) && IS_STRING == Z_TYPE_PP(sessionIdZval)
		){
			char	acceptMessage[200],
					*gatewayMessage;

			//清理本地的群组信息 session数据
			serverClearLocalData(Z_STRVAL_PP(sessionIdZval));

			sprintf(acceptMessage,"{\"sessionId\":\"%s\"}",Z_STRVAL_PP(sessionIdZval));
			createMessage("disconnect",acceptMessage,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
			sendToGateway(gatewayMessage);
			efree(gatewayMessage);
		}


		zend_hash_del(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1);
	}

	//删除所有此socket未成功发送的记录
	if(serverSendList != NULL && IS_ARRAY == Z_TYPE_P(serverSendList) && zend_hash_index_exists(Z_ARRVAL_P(serverSendList),socketFd) ){
		zend_hash_index_del(Z_ARRVAL_P(serverSendList),socketFd);
	}


	//关闭socket
	close(socketFd);
	smart_str_0(&serverTempBuffer[socketFd]);
	smart_str_free(&serverTempBuffer[socketFd]);

}

//服务器worker内转发消息
void serverBroadcastMessage(char *message,zval *object TSRMLS_DC){

	//当前所有socket
	int		i,h;
	zval	**thisSocket,
			**socketId,
			*sendMessage;

	h = zend_hash_num_elements(Z_ARRVAL_P(serverSocketList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverSocketList));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(serverSocketList),(void**)&thisSocket);

		if(IS_ARRAY == Z_TYPE_PP(thisSocket) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisSocket),"socketId",strlen("socketId")+1,(void**)&socketId) && IS_LONG == Z_TYPE_PP(socketId) ){
		}else{
			zend_hash_move_forward(Z_ARRVAL_P(serverSocketList));
			continue;
		}

		zval **thisSocketList;
		if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_PP(socketId),(void**)&thisSocketList) && IS_ARRAY == Z_TYPE_PP(thisSocketList) ){
		}else{
			//键不存在则创建一条
			zval *saveArray;
			MAKE_STD_ZVAL(saveArray);
			array_init(saveArray);
			add_index_zval(serverSendList,Z_LVAL_PP(socketId),saveArray);
			zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_PP(socketId),(void**)&thisSocketList);
		}

		//写入一条消息
		zval *sendMessage;
		MAKE_STD_ZVAL(sendMessage);
		array_init(sendMessage);
		add_index_long(sendMessage,0,Z_LVAL_PP(socketId));
		char	*baseMessage,
				*sendMessageString;
		base64Encode(message,&baseMessage);
		spprintf(&sendMessageString,0,"%s%c",baseMessage,'\n');
		efree(baseMessage);
		add_index_string(sendMessage,1,sendMessageString,0);
		add_next_index_zval(*thisSocketList,sendMessage);


		zend_hash_move_forward(Z_ARRVAL_P(serverSocketList));

	}


}

void triggerEventBack(char *message,zval *object TSRMLS_DC){

	zval	*jsonDecoder,
			**eventId,
			**data,
			**eventCallback;
	
	json_decode(message,&jsonDecoder);

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(jsonDecoder),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId) && SUCCESS == zend_hash_find(Z_ARRVAL_P(jsonDecoder),"data",5,(void**)&data)){
	}else{
		zval_ptr_dtor(&jsonDecoder);
		writeLogs("[CTcpServer] can not parse event message:%s\n",message);
		return;
	}


	//寻找此事件的回调函数
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverAsyncList),Z_STRVAL_PP(eventId),strlen(Z_STRVAL_PP(eventId))+1,(void**)&eventCallback) && IS_OBJECT == Z_TYPE_PP(eventCallback) ){
	}else{
		zval_ptr_dtor(&jsonDecoder);
		return;
	}



	//触发事件回调
	char	*callback_name = NULL;
	zval	*callback;
	callback = *eventCallback;
	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
		efree(callback_name);
		zval_ptr_dtor(&jsonDecoder);
		return;
	}

	zval	constructVal,
			contruReturn,
			*paramsList[1];

	INIT_ZVAL(constructVal);
	ZVAL_STRING(&constructVal,callback_name,0);
	MAKE_STD_ZVAL(paramsList[0]);
	ZVAL_ZVAL(paramsList[0],*data,1,0);
	int callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,1, paramsList TSRMLS_CC);
	zval_ptr_dtor(&paramsList[0]);
	zval_dtor(&contruReturn);


	//删除事件
	zend_hash_del(Z_ARRVAL_P(serverAsyncList),Z_STRVAL_PP(eventId),strlen(Z_STRVAL_PP(eventId))+1);


	efree(callback_name);
	zval_ptr_dtor(&jsonDecoder);

}

void triggerForwordMessage(char *messageContent,zval *object TSRMLS_DC){


	zval	*message,
			**toUid,
			**toUidSession,
			**socketFd,
			**messageContentZval,
			**event;

	json_decode(messageContent,&message);

	if( SUCCESS == zend_hash_find(Z_ARRVAL_P(message),"toUid",strlen("toUid")+1,(void**)&toUid) && IS_STRING == Z_TYPE_PP(toUid) && 
	   	SUCCESS == zend_hash_find(Z_ARRVAL_P(message),"event",strlen("event")+1,(void**)&event) && IS_STRING == Z_TYPE_PP(event) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(message),"message",strlen("message")+1,(void**)&messageContentZval) ){
	}else{
		zval_ptr_dtor(&message);
		return;
	}

	//寻找当前toUid对应的sessionId
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverUidData),Z_STRVAL_PP(toUid),strlen(Z_STRVAL_PP(toUid))+1,(void**)&toUidSession) && IS_STRING == Z_TYPE_PP(toUidSession) ){
	}else{
		zval_ptr_dtor(&message);
		return;
	}
	
	//根据sessionID找socketfd
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverSessionList),Z_STRVAL_PP(toUidSession),strlen(Z_STRVAL_PP(toUidSession))+1,(void**)&socketFd) && IS_LONG == Z_TYPE_PP(socketFd) ){
	}else{
		zval_ptr_dtor(&message);
		return;
	}

	zval	*messageZval,
			*saveMessageContent;
	char	*jsonEncoder,
			*base64Encoder,
			*sendString;

	MAKE_STD_ZVAL(messageZval);
	array_init(messageZval);
	MAKE_STD_ZVAL(saveMessageContent);
	ZVAL_ZVAL(saveMessageContent,*messageContentZval,1,0);


	add_next_index_long(messageZval,1);
	add_next_index_string(messageZval,Z_STRVAL_PP(event),1);
	add_next_index_zval(messageZval,saveMessageContent);


	json_encode(messageZval,&jsonEncoder);
	base64Encode(jsonEncoder,&base64Encoder);
	spprintf(&sendString,0,"%s%c",base64Encoder,'\n');


	zval **thisSocketList;
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_PP(socketFd),(void**)&thisSocketList) && IS_ARRAY == Z_TYPE_PP(thisSocketList) ){
	}else{
		//键不存在则创建一条
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_index_zval(serverSendList,Z_LVAL_PP(socketFd),saveArray);
		zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_PP(socketFd),(void**)&thisSocketList);
	}

	//尾部插入一条消息
	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,Z_LVAL_PP(socketFd));
	add_index_string(sendMessage,1,sendString,1);
	add_next_index_zval(*thisSocketList,sendMessage);


	efree(sendString);
	efree(base64Encoder);
	efree(jsonEncoder);
	zval_ptr_dtor(&messageZval);
	zval_ptr_dtor(&message);

}

void serverProcessGatewayMessage(int fd,char *thisMessage,zval *object TSRMLS_DC){

	//记录与网关互动的最后时间
	serverGatewayLastActiveTime = getMicrotime();

	//解析消息
	char	*base64Decoder;
	zval	*jsonDecoder,
			**messageType,
			**messageContent,
			**senderSessionId;

	base64Decode(thisMessage,&base64Decoder);
	json_decode(base64Decoder,&jsonDecoder);

	if(IS_ARRAY != Z_TYPE_P(jsonDecoder) || zend_hash_num_elements(Z_ARRVAL_P(jsonDecoder)) != 4){
		zval_ptr_dtor(&jsonDecoder);
		efree(base64Decoder);
		return;
	}


	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&messageType);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),2,(void**)&messageContent);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),3,(void**)&senderSessionId);

	//网关要求转发消息
	if(strcmp(Z_STRVAL_PP(messageType),"broadcast") == 0){
		serverBroadcastMessage(Z_STRVAL_PP(messageContent),object TSRMLS_CC);

	}

	//触发PHP层回调事件
	if(	strcmp(Z_STRVAL_PP(messageType),"getAllConnection") == 0 ){
		triggerEventBack(Z_STRVAL_PP(messageContent),object TSRMLS_CC);
	}


	//每个worker记录一份group记录
	if(	strcmp(Z_STRVAL_PP(messageType),"joinGroup") == 0){
		triggerEventBack(Z_STRVAL_PP(messageContent),object TSRMLS_CC);
	}

	//绑定用户
	if(	strcmp(Z_STRVAL_PP(messageType),"bindUid") == 0){
		triggerEventBack(Z_STRVAL_PP(messageContent),object TSRMLS_CC);
	}

	//转发消息 forwardMessage
	if(	strcmp(Z_STRVAL_PP(messageType),"forwardMessage") == 0){
		triggerForwordMessage(Z_STRVAL_PP(messageContent),object TSRMLS_CC);
	}

	//触发转发消息回调
	if(	strcmp(Z_STRVAL_PP(messageType),"forwardMessageSuccess") == 0){
		triggerEventBack(Z_STRVAL_PP(messageContent),object TSRMLS_CC);
	}



	zval_ptr_dtor(&jsonDecoder);
	efree(base64Decoder);
}

void serverCloseGatewaySocket(int fd,zval *object){

	writeLogs("[CTcpServer] woker[%d] disconnect from gateway ...\n",getpid());

	//关闭socket
	close(fd);

	//删除积累的消息
	zend_hash_index_del(Z_ARRVAL_P(serverSendList),serverToGatewaySocket);
	serverToGatewaySocket = 0;
	serverToGatewayStatus = 2;		//设为已断开连接


	//立即重新检查网关状态
	checkGatewayStatus();
}

int handlerTCPEvents(int epfd,struct epoll_event revs[],int num,int listen_sock,int semid,int isGotLock,zval *object TSRMLS_CC)
{
    struct epoll_event ev;
    int i = 0;
	int nowIsGetLock = isGotLock;

    for( ; i < num; i++ ){

		int fd = revs[i].data.fd;

		// 调用accept接受新连接
		if( fd == listen_sock && (revs[i].events & EPOLLIN) ){

			struct sockaddr_in client;
			socklen_t len = sizeof(client);
			extern int errno;
			int new_sock = accept(fd,(struct sockaddr *)&client,&len);

			if( new_sock < 0 ){
				if(errno == 11){
				}else{
					setLogs("[%d] accept fail,errorno:%d \n",getpid(),errno);
				}
				continue;
			}

			//socket设为非阻塞
			fcntl(new_sock, F_SETFL, fcntl(new_sock, F_GETFL, NULL) | O_NONBLOCK);

			//获取对端信息
			zval	*remoteInfo,
					*connectTime;
			char	*remoteIp;
			getSocketRemoteIp(new_sock,&remoteIp);
			MAKE_STD_ZVAL(remoteInfo);
			array_init(remoteInfo);
			microtime(&connectTime);
			add_assoc_string(remoteInfo,"remoteIp",remoteIp,0);
			add_assoc_double(remoteInfo,"connectTime",Z_DVAL_P(connectTime));
			add_assoc_long(remoteInfo,"socketId",new_sock);
			zval_ptr_dtor(&connectTime);
			char socketIndex[100];
			sprintf(socketIndex,"%d_%d",getpid(),new_sock);
			add_assoc_zval(serverSocketList,socketIndex,remoteInfo);

			//检测系统默认事件触发
			//WzIsImNvbm5lY3QiLCIiXQ==  [2,"connect",""]
			processMessage(new_sock,"WzIsImNvbm5lY3QiLCIiXQ==",object TSRMLS_CC);

			//新加入消息
			ev.events = EPOLLIN;
			ev.data.fd = new_sock;
			epoll_ctl(epfd,EPOLL_CTL_ADD,new_sock,&ev);


			continue;
		}

		//与网关通信的消息
		if( fd == serverToGatewaySocket && (revs[i].events & EPOLLIN) ){

			char		buf[2],
						*thisMessage;
			int			readLen = 0,k;

			smart_str	tempBuffer[10240] = {0};

			while(1){
				errno = 0;
				readLen = read(fd,buf,sizeof(buf)-1);

				if(readLen <= 0){
					if(readLen == 0){

						//释放Bbuffer
						smart_str_0(&tempBuffer[fd]);
						smart_str_free(&tempBuffer[fd]);

						epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
						serverCloseGatewaySocket(fd,object);
						
					}
					break;
				}

				if(buf[0] != '\n'){
					smart_str_appendc(&tempBuffer[fd],buf[0]);
				}else{
					smart_str_0(&tempBuffer[fd]);
					thisMessage = estrdup(tempBuffer[fd].c);
					smart_str_free(&tempBuffer[fd]);

					//处理消息
					serverProcessGatewayMessage(fd,thisMessage,object TSRMLS_CC);
					efree(thisMessage);
					break;
				}	
			}

			continue;
		}

		// 如果是普通文件描述符，则调用read提供读取数据的服务
		if(revs[i].events & EPOLLIN)	{

			char		buf[2],
						*thisMessage;
			int			readLen = 0,k;

			smart_str	tempBuffer[10240] = {0};

			//释放信号量 让其他进程竞争accept
			if(isGotLock){
				
				struct sembuf tryrelease;
				tryrelease.sem_num = 0;
				tryrelease.sem_op = 1;
				tryrelease.sem_flg = SEM_UNDO|IPC_NOWAIT;
				semop(semid , &tryrelease , 1);
				
				nowIsGetLock = 0;
			}

			while(1){
				errno = 0;
				readLen = read(fd,buf,sizeof(buf)-1);

				if(readLen <= 0){
					if(readLen == 0){

						//释放buffer
						smart_str_0(&tempBuffer[fd]);
						smart_str_free(&tempBuffer[fd]);

						epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
						serverCloseClientSocket(fd,object);
					}
					break;
				}

				if(buf[0] != '\n'){
					smart_str_appendc(&tempBuffer[fd],buf[0]);
				}else{
					smart_str_0(&tempBuffer[fd]);
					thisMessage = estrdup(tempBuffer[fd].c);
					smart_str_free(&tempBuffer[fd]);

					//处理消息
					processMessage(fd,thisMessage,object TSRMLS_CC);
					efree(thisMessage);
					break;
				}	
			}
		}

    }
	return nowIsGetLock;
}

//服务器写消息
void doServerSendMessage(zval *object TSRMLS_DC){

	int		i,h,j,k;
	zval	**thisMessage,
			**toSocket,
			**messageContent,
			**socketMessage;
	char	*key;
	ulong	uKey;

	if(serverSendList == NULL || IS_ARRAY != Z_TYPE_P(serverSendList)){
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(serverSendList));

	if(h == 0){
		return;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverSendList));
	for(i = 0 ; i < h;i++){


		if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(serverSendList),(void**)&socketMessage) && IS_ARRAY == Z_TYPE_PP(socketMessage)){
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(socketMessage));
			k = zend_hash_num_elements(Z_ARRVAL_PP(socketMessage));
			for(j = 0 ; j < k ;j++){
				zend_hash_get_current_key(Z_ARRVAL_PP(socketMessage),&key,&uKey,0);

				if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_PP(socketMessage),(void**)&thisMessage) && IS_ARRAY == Z_TYPE_PP(thisMessage)){
				}else{
					zend_hash_move_forward(Z_ARRVAL_PP(socketMessage));
					zend_hash_index_del(Z_ARRVAL_PP(socketMessage),uKey);
					continue;
				}


				if(SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(thisMessage),0,(void**)&toSocket) && IS_LONG == Z_TYPE_PP(toSocket) &&
				   SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(thisMessage),1,(void**)&messageContent) && IS_STRING == Z_TYPE_PP(messageContent)){
				}else{
					zend_hash_move_forward(Z_ARRVAL_PP(socketMessage));
					zend_hash_index_del(Z_ARRVAL_PP(socketMessage),uKey);
					continue;
				}

				errno = 0;
				int status = write(Z_LVAL_PP(toSocket),Z_STRVAL_PP(messageContent),Z_STRLEN_PP(messageContent));

				//写入失败
				if(status <= 0){	
					if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
						break;
					}else{
						
						//向网关发送失败时 
						if(Z_LVAL_PP(toSocket) == serverToGatewaySocket){
							serverCloseGatewaySocket(Z_LVAL_PP(toSocket),object);
							break;
						}

						//客户端已关闭 非网关的socket
						serverCloseClientSocket(Z_LVAL_PP(toSocket),object);
						break;
					}
				}
				
				//移除消息
				zend_hash_move_forward(Z_ARRVAL_PP(socketMessage));
				zend_hash_index_del(Z_ARRVAL_PP(socketMessage),uKey);
			}
		}

		zend_hash_move_forward(Z_ARRVAL_P(serverSendList));
	}

}

void doClientSendMessage(){

	int		i,h;
	zval	**thisMessage,
			**toSocket,
			**messageContent;
	char	*key;
	ulong	uKey;


	if(clientSendList == NULL || IS_ARRAY != Z_TYPE_P(clientSendList)){
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(clientSendList));
	if(h == 0){
		return;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(clientSendList));
	for(i = 0 ; i < h;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(clientSendList),(void**)&thisMessage);
		zend_hash_get_current_key(Z_ARRVAL_P(clientSendList),&key,&uKey,0);

		zend_hash_index_find(Z_ARRVAL_PP(thisMessage),0,(void**)&toSocket);
		zend_hash_index_find(Z_ARRVAL_PP(thisMessage),1,(void**)&messageContent);


		errno = 0;
		int status = write(clientSocketId,Z_STRVAL_PP(messageContent),Z_STRLEN_PP(messageContent));

		//写入失败 让出进程
		if(status <= 0){
			if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
				break;
			}else{

				//服务器有异常
				ClientCloseSocket(0,0);
				break;
			}
		}

		//仅写入一部分的消息
		if(status < Z_STRLEN_PP(messageContent)){
			char *notSendMessage;
			substr(Z_STRVAL_PP(messageContent),status,Z_STRLEN_PP(messageContent)-status,&notSendMessage);
			add_index_string(*thisMessage,1,notSendMessage,0);
			break;
		}

		
		//移除消息
		zend_hash_move_forward(Z_ARRVAL_P(clientSendList));
		zend_hash_index_del(Z_ARRVAL_P(clientSendList),uKey);
	}

}

void tcpClienthandleMaster_signal(int signo){
	//收到父进程推出的信号
	if (signo == SIGHUP){  
		zend_bailout();
	}
}

//将当前worker的玩家数上报给gateway
void reportWorkerUser(){

	int		i,h;
	zval	**thisSocket,
			**sessionId,
			**clientInfo;

	if(serverSocketList == NULL){
		return;
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(serverSocketList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverSocketList));


	for(i = 0 ; i < h ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(serverSocketList),(void**)&thisSocket);

		if( SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisSocket),"client",strlen("client")+1,(void**)&clientInfo) && IS_ARRAY == Z_TYPE_PP(clientInfo) &&  
			SUCCESS == zend_hash_find(Z_ARRVAL_PP(clientInfo),"sessionId",strlen("sessionId")+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) )
		{

			//批量上报给gateway
			char	acceptMessage[200],
					*gatewayMessage;
			sprintf(acceptMessage,"{\"sessionId\":\"%s\"}",Z_STRVAL_PP(sessionId));
			createMessage("connect",acceptMessage,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
			sendToGateway(gatewayMessage);
			efree(gatewayMessage);
		}


		zend_hash_move_forward(Z_ARRVAL_P(serverSocketList));
	}


	//上报本地记录的玩家群组
	char	*key;
	ulong	uKey;
	zval	**groupName;
	h = zend_hash_num_elements(Z_ARRVAL_P(serverGroup));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverGroup));

	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(serverGroup),&key,&uKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(serverGroup),(void**)&groupName);

			char	*gatewayMessage,
					eventCon[100];
			sprintf(eventCon,"{\"eventId\":\"none\",\"group\":\"%s\",\"clientSessionId\":\"%s\"}",Z_STRVAL_PP(groupName),key);
			createMessage("joinGroup",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
			sendToGateway(gatewayMessage);
			efree(gatewayMessage);
		zend_hash_move_forward(Z_ARRVAL_P(serverGroup));
	}


	//上报本地记录的uid-session列表
	h = zend_hash_num_elements(Z_ARRVAL_P(serverUidData));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(serverUidData));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(serverUidData),&key,&uKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(serverUidData),(void**)&sessionId);
			char	*gatewayMessage,
			eventCon[100];
			sprintf(eventCon,"{\"eventId\":\"none\",\"uid\":\"%s\",\"clientSessionId\":\"%s\"}",key,Z_STRVAL_PP(sessionId));
			createMessage("bindUid",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
			sendToGateway(gatewayMessage);
			efree(gatewayMessage);

		zend_hash_move_forward(Z_ARRVAL_P(serverUidData));
	}




}

//连接至网关服务器
int serverConnectToGateway(zval *object TSRMLS_DC){

	zval *useZval = zend_read_property(CTcpServerCe,object,ZEND_STRL("gatewayUse"), 0 TSRMLS_CC);
	if(0 == Z_LVAL_P(useZval)){
		return 0;
	}

	zval *hostZval = zend_read_property(CTcpServerCe,object,ZEND_STRL("gatewayHost"), 0 TSRMLS_CC);
	zval *portZval = zend_read_property(CTcpServerCe,object,ZEND_STRL("gatewayPort"), 0 TSRMLS_CC);

	char  *host;
	int	  port;

	host = Z_STRVAL_P(hostZval);
	port = Z_LVAL_P(portZval);

	int socket = connectServerPort(host,port TSRMLS_CC);
	if(socket < 0){
		serverToGatewayStatus = 2;
		writeLogs("[FatalException] [%d] TCP Server can connect to gateway server %s:%d ,errorno : %d \n",getpid(),host,port,socket);
	}

	if(socket > 0){
		serverToGatewaySocket = socket;
		serverToGatewayStatus = 1;
		writeLogs("[CTcpServer] worker[%d] Successfully connected to gateway server ...\n",getpid());
		registerTcpWorker();
		serverGatewayLastActiveTime = getMicrotime();

		//上报当前worker的在线记录
		reportWorkerUser();

		//网关fd加入epoll
		struct epoll_event  ev2;
		ev2.events = EPOLLIN; 
		ev2.data.fd = serverToGatewaySocket;
		epoll_ctl(serverEpollFd,EPOLL_CTL_ADD,serverToGatewaySocket,&ev2);

	}

	return socket;
}

//服务器worker进程向网关服务器注册
int registerTcpWorker(){

	if(serverToGatewaySocket <= 0){
		return -1;
	}

	if(serverSessionId == NULL){
		char *sessionId;
		getSessionID(&sessionId);
		serverSessionId = sessionId;
	}

	serverToGatewayStatus = 1;

	char	*stringMessage;
	createMessage("serverRegister","[]",MSG_USER,&stringMessage,3 TSRMLS_CC);

	sendToGateway(stringMessage);
	efree(stringMessage);

	return 1;
}

//检查网关服务器状态
int checkGatewayStatus(){
	
	//正常连接或者主动断开时 不再重新连接
	if(serverToGatewayStatus != 2){
		return 1;
	}

	//尝试重连
	TSRMLS_FETCH();
	serverToGatewayStatus = 3;	//正在重试中
	serverToGatewaySocket = serverConnectToGateway(serverMainObject TSRMLS_CC);
	if(serverToGatewaySocket > 0){
		serverToGatewayStatus = 1;
	}else{
		serverToGatewayStatus = 2;
	}
}

void checkGatewayHeartbeat(){

	
	int timenow = getMicrotime();



	if(serverToGatewayStatus == 1 && serverGatewayLastActiveTime > 0 && timenow - serverGatewayLastActiveTime >= 30){
		writeLogs("[CTcpServer] The interaction with the gateway has taken a long time, sending heartbeat packets ...\n");
		char *gatewayMessage;
		createMessage("heartbeat","[]",MSG_USER,&gatewayMessage,3 TSRMLS_CC);
		sendToGateway(gatewayMessage);
		efree(gatewayMessage);
		serverGatewayLastActiveTime = timenow;
	}

}

void serverTimerCallback(){

	//获取当前时间
	int timestamp = getMicrotime();

	//距离上次检查连接大于10秒时 检查与网关之间的状态
	if(timestamp - serverLastCheckStaus >= 10){

		//检查与网关的状态
		checkGatewayStatus();

		//重置检查时间
		serverLastCheckStaus = timestamp;
	}





	//检查serverGatewayLastActiveTime 如最后活动时间超过30秒才心跳
	checkGatewayHeartbeat();

}

int initHashTable(){

	//初始化1个hash表记录所有socket
	if(serverSocketList == NULL){
		MAKE_STD_ZVAL(serverSocketList);
		array_init(serverSocketList);
	}

	if(serverAsyncList == NULL){
		MAKE_STD_ZVAL(serverAsyncList);
		array_init(serverAsyncList);
	}

	if(serverSessionList == NULL){
		MAKE_STD_ZVAL(serverSessionList);
		array_init(serverSessionList);
	}

	if(serverGroup == NULL){
		MAKE_STD_ZVAL(serverGroup);
		array_init(serverGroup);
	}

	if(serverLocalSessionData == NULL){
		MAKE_STD_ZVAL(serverLocalSessionData);
		array_init(serverLocalSessionData);
	}

	if(serverUidData == NULL){
		MAKE_STD_ZVAL(serverUidData);
		array_init(serverUidData);
	}

	serverLastCheckStaus = getMicrotime();
}

//初始化子进程
//设置进程标识
//每个子进程绑定epoll
int workProcessInit(int listenfd,int semid,zval *object TSRMLS_DC){

	if(serverSendList == NULL){
		MAKE_STD_ZVAL(serverSendList);
		array_init(serverSendList);
	}

	ini_seti("memory_limit",-1);

	signal(SIGHUP, tcpClienthandleMaster_signal);	//收到父进程退出信号 自身退出
	prctl(PR_SET_PDEATHSIG, SIGHUP);		//要求父进程退出时发出信号
	prctl(PR_SET_NAME, "CTcpServerWorker");

	//添加epoll事件
	int epfd = epoll_create(1024);
	serverEpollFd = epfd;
	if(epfd <= 0){
		//创建失败  子进程自行退出
		return -1;
	}

	//绑定读入流
	struct epoll_event  ev;
    ev.events = EPOLLIN; 
    ev.data.fd = listenfd;

	//设置信号 阻止进程退出
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	//分配各list空间
	initHashTable();


	//连接gateway服务器
	serverToGatewaySocket = serverConnectToGateway(object TSRMLS_CC);

	
	struct epoll_event revs[128];
    int n = sizeof(revs)/sizeof(revs[0]);
    int timeout = 3000;
    int num = 0;
	int isGotLock = 0;

	//阻塞进程
	while(1) {

		//检查信号量
		errno = 0;
		struct sembuf trywait;
		trywait.sem_num = 0;
		trywait.sem_op = -1;
		trywait.sem_flg = SEM_UNDO|IPC_NOWAIT;
        int tryNum = semop(semid , &trywait , 1);

		if (tryNum == 0) {
			if (isGotLock == 0) {
				isGotLock = 1;
				//注册事件
				epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
			}
		}else{
			if (isGotLock) {
				//删除事件
				epoll_ctl(epfd, EPOLL_CTL_DEL, listenfd, NULL);
				isGotLock = 0;
			}
		}


       //开始epoll事件等待
	   errno = 0;
       num = epoll_wait(epfd,revs,n,500);

	   //消息前触发函数，检查上次心跳间隔
	   serverTimerCallback();
	   
	   if(num > 0){
			isGotLock = handlerTCPEvents(epfd,revs,num,listenfd,semid,isGotLock,object TSRMLS_CC);
	   }

		doServerSendMessage(object TSRMLS_CC);

	   if (isGotLock) {
		
		   	struct sembuf tryrelease;
			tryrelease.sem_num = 0;
			tryrelease.sem_op = 1;
			tryrelease.sem_flg = SEM_UNDO|IPC_NOWAIT;
			int releaseStatus = semop(semid , &tryrelease , 1);
			isGotLock = 0;
		}
    }

    close(epfd);
    return 0;

}


PHP_METHOD(CTcpServer,__construct){
	zval	*eventTable;

	ini_seti("memory_limit",-1);

	eventTable = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("eventTable"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(eventTable)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CTcpServerCe,getThis(),ZEND_STRL("eventTable"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		eventTable = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("eventTable"),1 TSRMLS_CC);
	}
}

PHP_METHOD(CTcpServer,__destruct){

	if(serverErrorHandler != NULL){
		zval_ptr_dtor(&serverErrorHandler);
	}

	if(serverGroup != NULL){
		zval_ptr_dtor(&serverGroup);
	}

	if(serverSessionId != NULL){
		efree(serverSessionId);
	}

	if(serverAsyncList != NULL){
		zval_ptr_dtor(&serverAsyncList);
	}

	if(serverSocketList != NULL){
		zval_ptr_dtor(&serverSocketList);
	}

	if(serverSessionList != NULL){
		zval_ptr_dtor(&serverSessionList);
	}

	if(serverLocalSessionData != NULL){
		zval_ptr_dtor(&serverLocalSessionData);
	}

	if(serverUidData != NULL){
		zval_ptr_dtor(&serverUidData);
	}
}

PHP_METHOD(CTcpServer,getInstance){

	char	*key,
			*saveKey;
	int		keyLen = 0;

	zval	*selfInstace,
			**instaceSaveZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	if(keyLen == 0){
		key = "main";
	}


	selfInstace = zend_read_static_property(CTcpServerCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CTcpServerCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CTcpServerCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CTcpServerCe);

		//执行其构造器 并传入参数
		if (CTcpServerCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CTcpServerCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CTcpServerCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}

int getCreateWorkerNums(zval *object TSRMLS_DC){
	
	zval	*workerNum;
	workerNum = zend_read_property(CTcpServerCe,object,ZEND_STRL("worker"), 0 TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(workerNum) && Z_LVAL_P(workerNum) > 0){
		return Z_LVAL_P(workerNum);
	}

	//返回CPU核心数
	int cpuNum =  get_nprocs();
	return cpuNum*2;
}

PHP_METHOD(CTcpServer,listen){

	zval	*host,
			*port,
			*object,
			**argv,
			**SERVER,
			*pidList;

	int		errorCode = 0,
			isDaemon = 0,
			isMaster = 1;

	char	appPath[2024],
			codePath[2024];

	RETVAL_ZVAL(getThis(),1,0);

	host = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("host"), 0 TSRMLS_CC);
	port = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("port"), 0 TSRMLS_CC);

	//init framework
	getcwd(appPath,sizeof(appPath));
	sprintf(codePath,"%s%s",appPath,"/application");
	php_define("APP_PATH",appPath TSRMLS_CC);
	php_define("CODE_PATH",codePath TSRMLS_CC);
	CWebApp_createApp(&object TSRMLS_CC);
	zval_ptr_dtor(&object);


	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
	if(zend_hash_find(Z_ARRVAL_PP(SERVER),"argv",strlen("argv")+1,(void**)&argv) == SUCCESS && IS_ARRAY == Z_TYPE_PP(argv)){
		int	i,h;
		zval **thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_PP(argv));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(argv));
		for(i = 0 ; i < h;i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(argv),(void**)&thisVal);
			if(strstr(Z_STRVAL_PP(thisVal),"--daemon") != NULL){
				isDaemon = 1;
			}
			if(strstr(Z_STRVAL_PP(thisVal),"--worker") != NULL){
				isMaster = 0;
			}
			zend_hash_move_forward(Z_ARRVAL_PP(argv));
		}
	}

	//daemon
	if(isDaemon){
		writeLogs("run as a daemon process..\n");
		int s = daemon(1, 0);
	}

	//创建多进程池
	pidList = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("pidList"),1 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(pidList)){
		zval *saveData;
		MAKE_STD_ZVAL(saveData);
		array_init(saveData);
		zend_update_property(CTcpServerCe,getThis(),ZEND_STRL("pidList"),saveData TSRMLS_CC);
		zval_ptr_dtor(&saveData);
		pidList = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("pidList"),1 TSRMLS_CC);
	}
	
	//创建socket套接字 而后fork子进程 以共享此套接字
	int listenSocket = startTcpListen(Z_STRVAL_P(host),Z_LVAL_P(port));
	if(listenSocket < 0){
		//监听异常
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] TCP Server can not bind %s:%d ",Z_STRVAL_P(host),Z_LVAL_P(port));
		return;
	}

	//使用systemV信号量
	char	*nowFile,
			workDir[180],
			indexFile[250];   
    getcwd(workDir,sizeof(workDir)); 
	getServerParam("PHP_SELF",&nowFile);
	sprintf(indexFile,"%s/%s",workDir,nowFile);
	efree(nowFile);
	key_t semKey = ftok(indexFile,1);
	errno = 0;
	int semid = semget((key_t)semKey,1,IPC_CREAT|0664);
	if(semid < 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] can not create systemV semget ,errno[%d]",errno);
		close(listenSocket);
		return;
	}
	//初始化
	int initStatus = semctl(semid , 0 , SETVAL , 1);


	//设置全局变量 用于状态检查
	serverAccpetLock = semid;
	serverListenSocket = listenSocket;
	serverMainObject = getThis();

	//fork多子进程
	int workerNum = getCreateWorkerNums(getThis() TSRMLS_CC);
	createWorkProcess(workerNum,listenSocket,semid,getThis(),1 TSRMLS_CC);
}

PHP_METHOD(CTcpServer,setWorker){
	long	num = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&num) == FAILURE){
		RETURN_FALSE;
	}
	zend_update_property_long(CTcpServerCe,getThis(),ZEND_STRL("worker"),num TSRMLS_CC);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,bind){
	char	*host;
	int		hostLen = 0;
	long	port = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&host,&hostLen,&port) == FAILURE){
		RETURN_FALSE;
	}

	if(port == 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CTCPServerException] call [CTCPServer->bind] the port must available port");
		return;
	}

	zend_update_property_string(CTcpServerCe,getThis(),ZEND_STRL("host"),host TSRMLS_CC);
	zend_update_property_long(CTcpServerCe,getThis(),ZEND_STRL("port"),port TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,on){

	zval	*callback,
			*saveCallback;
	char	*callback_name,
			*eventName;
	int		eventNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&eventName,&eventNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		throwServerException("[CTcpServerException] call [CTcpServer->on] the params is not a callback function");
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }

	zval	*eventTable;


	//save to callbackObject
	eventTable = zend_read_property(CTcpServerCe,getThis(),ZEND_STRL("eventTable"), 0 TSRMLS_CC);
	MAKE_STD_ZVAL(saveCallback);
	ZVAL_ZVAL(saveCallback,callback,1,0);
	add_assoc_zval(eventTable,eventName,saveCallback);
	zend_update_property(CTcpServerCe,getThis(),ZEND_STRL("eventTable"),eventTable TSRMLS_CC);

    efree(callback_name);
	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,onData){

	zval	*callback;
	char	*callback_name;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		throwServerException("[CTcpServerException] call [CTcpServer->onData] the params is not a callback function");
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }


	//save to callbackObject
	zend_update_property(CTcpServerCe,getThis(),ZEND_STRL("eventDefault"),callback TSRMLS_CC);

    efree(callback_name);
	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CTcpClient,__construct){

	zval *eventTable;

	ini_seti("memory_limit",-1);

	eventTable = zend_read_property(CTcpClientCe,getThis(),ZEND_STRL("eventTable"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(eventTable)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CTcpClientCe,getThis(),ZEND_STRL("eventTable"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		eventTable = zend_read_property(CTcpClientCe,getThis(),ZEND_STRL("eventTable"),1 TSRMLS_CC);
	}
}


PHP_METHOD(CTcpClient,__destruct){

	//析构函数 关闭socket
	if(clientSocketId){

		//关闭socket
		close(clientSocketId);
	}


	//删除
	if(clientHost != NULL){
		efree(clientHost);
		clientHost = NULL;
	}

	if(clientErrorHandler != NULL){
		zval_ptr_dtor(&clientErrorHandler);
	}

	if(clientSessionId != NULL){
		efree(clientSessionId);
		clientSessionId = NULL;
	}
}

PHP_METHOD(CTcpClient,setHeartbeatInterval){

	int		timeInterval = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&timeInterval) == FAILURE){
		RETURN_FALSE;
	}

	RETVAL_ZVAL(getThis(),1,0);
	zend_update_property_long(CTcpClientCe,getThis(),ZEND_STRL("heartbeatInterval"),timeInterval TSRMLS_CC);
}

PHP_METHOD(CTcpClient,getInstance){
	char	*key,
			*saveKey;
	int		keyLen = 0;

	zval	*selfInstace,
			**instaceSaveZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	if(keyLen == 0){
		key = "main";
	}


	selfInstace = zend_read_static_property(CTcpClientCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CTcpClientCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CTcpClientCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CTcpClientCe);

		//执行其构造器 并传入参数
		if (CTcpClientCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CTcpClientCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CTcpClientCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}

//连接远程socket
int connectServerPort(char *host,int port TSRMLS_DC){


	int sockfd, num;    
    char buf[1024];  
    struct sockaddr_in server;


	if((sockfd=socket(AF_INET,SOCK_STREAM, 0))==-1){
		return -2;
	}

	bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

	//调用连接
	errno = 0;
	if(connect(sockfd, (struct sockaddr *)&server, sizeof(server))==-1){
		return -3;
	}

	return sockfd;
}

int ClientCloseSocket(int epfd,int listen_sock){

	//移除epoll事件
	if(epfd > 0){
		epoll_ctl(epfd, EPOLL_CTL_DEL, listen_sock, NULL);
	}

	//关闭当前socket
	if(listen_sock > 0){
		close(listen_sock);
	}

	//退出 当前子进程
	zend_bailout();
}

int checkServerStatus(int epfd,int listen_sock){
	
	//尝试发送确认包
	writeLogs("server happend exception,waite to reconnect\n");

	//关闭socket 设为断开 等待重试
	ClientCloseSocket(epfd,listen_sock);


}

int processMasterSendMessage(int fd,char *thisMessage TSRMLS_DC){
	char *saveMessage;
	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,fd);
	spprintf(&saveMessage,0,"%s%c",thisMessage,'\n');
	add_index_string(sendMessage,1,saveMessage,0);
	add_next_index_zval(clientSendList,sendMessage);
	return 1;
}

//生成sessionID 时间戳+5位随机数 md5
int getSessionID(char **sessionId){

	zval *timestamp;
	microtimeTrue(&timestamp);
	srand(time(NULL));
	int randNum = rand();
	char randString[100];
	sprintf(randString,"%.8f%d",Z_DVAL_P(timestamp),randNum);
	md5(randString,sessionId);
	zval_ptr_dtor(&timestamp);
}

int clientGetSDKInfo(char **sdkInfoString TSRMLS_DC){

	zval *sdkInfo;

	//当前SDK版本
	MAKE_STD_ZVAL(sdkInfo);
	array_init(sdkInfo);
	add_assoc_long(sdkInfo,"sdkVer",clientVersion);
	add_assoc_string(sdkInfo,"sdkName","php extension",1);
	add_assoc_long(sdkInfo,"cpuCore",get_nprocs());
	struct sysinfo si;
    if(0 == sysinfo(&si)){
		add_assoc_long(sdkInfo,"memoryAll",si.totalram);
		add_assoc_long(sdkInfo,"memoryFree",si.freeram);
	}else{
		add_assoc_long(sdkInfo,"memoryAll",-1);
		add_assoc_long(sdkInfo,"memoryFree",-1);
	}
	struct utsname osinfo;
	if(0 == uname(&osinfo)){
		add_assoc_string(sdkInfo,"osName",osinfo.sysname,1);
		add_assoc_string(sdkInfo,"osNode",osinfo.nodename,1);
		add_assoc_string(sdkInfo,"osRelease",osinfo.release,1);
		add_assoc_string(sdkInfo,"osVersion",osinfo.version,1);
	}else{
		add_assoc_string(sdkInfo,"osName","",1);
		add_assoc_string(sdkInfo,"osNode","",1);
		add_assoc_string(sdkInfo,"osRelease","",1);
		add_assoc_string(sdkInfo,"osVersion","",1);
	}

	//sessionID
	add_assoc_string(sdkInfo,"sessionId",clientSessionId,1);
	
	//转为json
	json_encode(sdkInfo,sdkInfoString);

}


int processClientSystemMessage(int fd,zval *jsonDecoder,char *stringMessage TSRMLS_DC){

	zval **messageEvent;

	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&messageEvent);


	//处理系统事件
	if(strcmp(Z_STRVAL_PP(messageEvent),"connect") == 0){
		
		//回复服务器当前客户端信息
		char	*sdkInfo,
				*reportMessage;
		clientGetSDKInfo(&sdkInfo TSRMLS_CC);
		createMessage("clientReport",sdkInfo,2,&reportMessage,2 TSRMLS_CC);

		//插入到发送队列
		zval *sendMessage;
		MAKE_STD_ZVAL(sendMessage);
		array_init(sendMessage);
		add_index_long(sendMessage,0,fd);
		add_index_string(sendMessage,1,reportMessage,0);
		add_next_index_zval(clientSendList,sendMessage);

		efree(sdkInfo);
	}


	//与服务器沟通完毕后  返回连接成功
	if(strcmp(Z_STRVAL_PP(messageEvent),"connected") == 0){

		//触发用户态connect事件 WzEsImNvbm5lY3QiLCIiXQ==  [1,"connect",""]
		processClientMessage(fd,"WzEsImNvbm5lY3QiLCIiXQ==" TSRMLS_CC);
	}

}

int processClientMessage(int fd,char *thisMessage TSRMLS_DC){

	char	*base64Decoder;
	zval	*jsonDecoder,
			*eventTable,
			*eventDefault,
			**messageEvent,
			**eventCallback,
			**stringMessage,
			**messageType;

	base64Decode(thisMessage,&base64Decoder);
	json_decode(base64Decoder,&jsonDecoder);

	if(zend_hash_num_elements(Z_ARRVAL_P(jsonDecoder)) != 3){
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		char *errorMessage;
		spprintf(&errorMessage,0,"some message parse error : %s",thisMessage);
		throwClientException(errorMessage);
		efree(errorMessage);
		return -1;
	}

	
	//当前的事件名称
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),0,(void**)&messageType);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&messageEvent);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),2,(void**)&stringMessage);

	if(IS_LONG == Z_TYPE_PP(messageType) && 2 == Z_LVAL_PP(messageType)){
		processClientSystemMessage(fd,jsonDecoder,thisMessage TSRMLS_CC);
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		return 1;
	}


	//检查是否存在指定的event
	eventTable = zend_read_property(CTcpClientCe,clientPHPObject,ZEND_STRL("eventTable"), 0 TSRMLS_CC);

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventTable),Z_STRVAL_PP(messageEvent),strlen(Z_STRVAL_PP(messageEvent))+1,(void**)&eventCallback) && IS_OBJECT == Z_TYPE_PP(eventCallback) ){

		//向PHP层回调
		zval	*callParams,
				*connectTimeZval;

		MAKE_STD_ZVAL(callParams);
		object_init_ex(callParams,CSocketClientCe);
		zend_update_property_string(CSocketClientCe,callParams,ZEND_STRL("message"),Z_STRVAL_PP(stringMessage) TSRMLS_CC);
		zend_update_property_long(CSocketClientCe,callParams,ZEND_STRL("socketId"),fd TSRMLS_CC);
		zend_update_property_long(CSocketClientCe,callParams,ZEND_STRL("socketType"),2 TSRMLS_CC);

		char	*callback_name = NULL;
		zval	*callback;
		callback = *eventCallback;
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			efree(base64Decoder);
			zval_ptr_dtor(&callParams);
			zval_ptr_dtor(&jsonDecoder);
			return 0;
		}

		zval	constructVal,
				contruReturn,
				*paramsList[1];

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,callback_name,0);
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],callParams,1,0);
		int callStatus = call_user_function(NULL, &callback, &constructVal, &contruReturn,1, paramsList TSRMLS_CC);

		zval_dtor(&contruReturn);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&callParams);
		efree(callback_name);
	}


	efree(base64Decoder);
	zval_ptr_dtor(&jsonDecoder);
}

int handerTcpClientMessage(int epfd,struct epoll_event revs[],int num,int listen_sock TSRMLS_CC){

	struct epoll_event ev;
    int i = 0;
    for( ; i < num; i++ ){

		int fd = revs[i].data.fd;

		if(fd == listen_sock && revs[i].events & EPOLLIN)	{

			char		buf[2],
						*thisMessage;
			int			readLen = 0,
						k,
						findMessage = 0;
			
			while(1){
				errno = 0;
				readLen = read(fd,buf,sizeof(buf)-1);

				if(readLen <= 0){
					if(readLen == 0){

						smart_str_0(&serverTempBuffer[fd]);
						smart_str_free(&serverTempBuffer[fd]);

						ClientCloseSocket(fd,epfd);
					}
					break;
				}
				for(k = 0 ; k < readLen;k++){

					//依次检查字符
					if(buf[k] != '\n'){
						smart_str_appendc(&serverTempBuffer[fd],buf[k]);
					}else if(buf[k] == '\n' || readLen < sizeof(buf)-1){
						smart_str_0(&serverTempBuffer[fd]);
						thisMessage = estrdup(serverTempBuffer[fd].c);
						
						//处理消息
						processClientMessage(fd,thisMessage TSRMLS_CC);

						findMessage = 1;
						efree(thisMessage);
						smart_str_free(&serverTempBuffer[fd]);
						break;
					}
				}
				if(findMessage){
					findMessage = 0;
					break;
				}
			}
		}


		if(fd == clientRecProcessReadPipeFd && revs[i].events & EPOLLIN)	{

			char		buf[2],
						*thisMessage;
			int			readLen = 0,
						k,
						findMessage = 0;
			smart_str	masterMessage = {0};
			
			while(1){
				readLen = read(fd,buf,sizeof(buf)-1);

				if(readLen <= 0){
					smart_str_0(&masterMessage);
					smart_str_free(&masterMessage);
					break;
				}
				for(k = 0 ; k < readLen;k++){

					//依次检查字符
					if(buf[k] != '\n'){
						smart_str_appendc(&masterMessage,buf[k]);
					}else if(buf[k] == '\n' || readLen < sizeof(buf)-1){
						smart_str_0(&masterMessage);
						thisMessage = estrdup(masterMessage.c);
						
						//处理消息 
						processMasterSendMessage(clientSocketId,thisMessage TSRMLS_CC);

						findMessage = 1;
						efree(thisMessage);
						smart_str_free(&masterMessage);
						break;
					}
				}
				if(findMessage){
					findMessage = 0;
					break;
				}
			}
		}
    }
}


int initRecProcess(int socket,int epfd,int readFd){
	//初始化子进程
	signal(SIGHUP, tcpClienthandleMaster_signal);	//收到父进程退出信号 自身退出
	prctl(PR_SET_PDEATHSIG, SIGHUP);		//要求父进程退出时发出信号
	prctl(PR_SET_NAME, "CTcpClientRevProcess"); 

	//管道 读事件设为非阻塞
	clientRecProcessReadPipeFd = readFd;
	fcntl(clientRecProcessReadPipeFd, F_SETFL, fcntl(clientRecProcessReadPipeFd, F_GETFL, 0) | O_NONBLOCK);


	//初始化发送队列
	if(clientSendList == NULL){
		MAKE_STD_ZVAL(clientSendList);
		array_init(clientSendList);
	}

	//设置信号 阻止进程退出
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);


	//socket非阻塞
	fcntl(socket, F_SETFL, fcntl(socket, F_GETFD, 0)|O_NONBLOCK);

	//加入socket事件
	struct epoll_event  ev;
	ev.events = EPOLLIN; 
	ev.data.fd = socket;
	struct epoll_event revs[128];
	int n = sizeof(revs)/sizeof(revs[0]);
	int timeout = 3000;
	int num = 0;
	epoll_ctl(epfd,EPOLL_CTL_ADD,socket,&ev);

	//将管道加入epoll
	struct epoll_event  ev2;
	ev2.events = EPOLLIN; 
	ev2.data.fd = clientRecProcessReadPipeFd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,clientRecProcessReadPipeFd,&ev2);


	//阻塞进程
	while(1) {

	   //开始epoll事件等待
	   num = epoll_wait(epfd,revs,n,1000);

	   if(num > 0){
			
		   handerTcpClientMessage(epfd,revs,num,socket TSRMLS_CC);

	   }

	   doClientSendMessage();
	}
}


//发送消息
//type 1为自定义消息
int Client_sendMessage(int socket,char *message,int tryNums TSRMLS_DC){

	if(tryNums >= 10){
		return -1;
	}

	errno = 0;
	tryNums++;
	int writeTime = write(socket,message,strlen(message));

	//记录最后一次正常socket io操作的时间
	if(writeTime > 0){
		clientLastMessageTime = getMicrotime();
	}

	//socket连接失效
	if(writeTime < 0){

		if(errno == 1 || errno == 4){
		}else if(errno == 11){
			usleep(500);
			return Client_sendMessage(socket,message,tryNums TSRMLS_CC);
		}else{
			kill(clientRecProcessId,SIGKILL);
		}
	}

	return writeTime;
}

//重连socket
int resetSocketConnect(){

	writeLogs("[%d] is try to reconnect server,old socket[%d] ...\n",getpid(),clientSocketId);

	//关闭socket
	close(clientSocketId);
	
	//重新连接
	int socket = connectServerPort(clientHost,clientPort TSRMLS_CC);
	if(socket < 0){
		clientStatus = 2;
		writeLogs("[%d] is try to reconnect server Fail ...\n",getpid());
		return -1;
	}

	clientStatus = 1;
	clientSocketId = socket;

	//创建管道
	int masterWriteFd[2];
	if(pipe(masterWriteFd) < 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CTCPServerException] can not create pipe..");
		return;
	}

	//fork 子进程 用于循环读消息
	int i = 0;
	for(i = 0 ; i < 1 ;i++){

		int forkPid = -1;
		forkPid=fork();
		if(forkPid==-1){

		}else if(forkPid == 0){

			close(masterWriteFd[1]);
			initRecProcess(socket,clientMainEpollFd,masterWriteFd[0]);

		}else{

			clientRecProcessWritePipeFd = masterWriteFd[1];
			close(masterWriteFd[0]);

			clientRecProcessId = forkPid;

		}
	}

	writeLogs("[%d] is try to reconnect server Success,new socket[%d] ...\n",getpid(),socket);
	destoryTimer();
	clientStatus = 1;
	return 1;
}


//客户端心跳
void clientHeartbeat(){
	
	//WzIsImNsaWVudEhlYXJ0YmVhdCIsIiJd  [2,"clientHeartbeat",""]
	char stringMessage[100];
	sprintf(stringMessage,"%s%c","WzIsImNsaWVudEhlYXJ0YmVhdCIsIiJd",'\n');
	
	//连接正常时方进行心跳
	if(clientStatus != 1){
		return;
	}

	int status = -1;
	while(1){
		errno = 0;
		status = write(clientRecProcessWritePipeFd,stringMessage,strlen(stringMessage));

		//子进程已结束出于重连状态
		if(status < 0 && errno == 32){
			break;
		}

		if(status == strlen(stringMessage)){
			writeLogs("[%d] send heartbeat package send sucess ...\n",getpid());
			break;
		}
		//发送失败则阻塞进程
		usleep(500);
	}
}

void timerCallback(){

	int timestamp = getMicrotime();

	//每10秒 检查socket状态
	if(timestamp - clientLastCheckSocket >= 5){

		//检查socket状态
		checkSocketStatus();

		clientLastCheckSocket = timestamp;
	}
	
	//读取心跳间隔
	zval *interval = zend_read_property(CTcpClientCe,clientPHPObject,ZEND_STRL("heartbeatInterval"), 0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(interval) && Z_LVAL_P(interval) > 0 && timestamp -  clientLastHeartbeat >= Z_LVAL_P(interval) ){

		//客户端心跳
		clientHeartbeat();

		clientLastHeartbeat = timestamp;
	}
}

void clientProcessExit(void){
	
}

PHP_METHOD(CTcpClient,connect){
	
	char	*host = NULL;
	int		hostLen = 0;
	long	port = 0;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&host,&hostLen,&port) == FAILURE){
		RETURN_FALSE;
	}

	RETVAL_ZVAL(getThis(),1,0);

	zend_update_property_string(CTcpClientCe,getThis(),ZEND_STRL("host"),host TSRMLS_CC);
	zend_update_property_long(CTcpClientCe,getThis(),ZEND_STRL("port"),port TSRMLS_CC);

	//记录全局变量
	clientHost = estrdup(host);
	clientPort = port;

	//获取/tmp目录下是否存在
	char *sessionId = NULL;
	/*if(SUCCESS == fileExist("/tmp/CQuickFrameworkPHPExtensionCTcpClient")){
		file_get_contents("/tmp/CQuickFrameworkPHPExtensionCTcpClient",&sessionId);
		if(strlen(sessionId) != 32){
			efree(sessionId);
			sessionId = NULL;
		}
	}*/

	//没有缓存文件则重新生成
	if(sessionId == NULL){

		getSessionID(&sessionId);
		
		//写入缓存文件
		//file_put_contents("/tmp/CQuickFrameworkPHPExtensionCTcpClient",sessionId);
	}

	clientSessionId = estrdup(sessionId);
	efree(sessionId);

	//连接服务器
	int socket = connectServerPort(host,port TSRMLS_CC);
	if(socket < 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] TCP Client can connect to %s:%d ,errorno : %d",host,port,socket);
		return;
	}

	clientStatus = 1;
	clientSocketId = socket;

	//记录连接时间
	zval *connectTimeZval;
	microtime(&connectTimeZval);
	zend_update_property_double(CTcpClientCe,getThis(),ZEND_STRL("connectTime"),Z_DVAL_P(connectTimeZval) TSRMLS_CC);
	zval_ptr_dtor(&connectTimeZval);

	if(clientPHPObject == NULL){
		clientPHPObject = getThis();
	}

	//开始循环等待
	clientMainEpollFd = epoll_create(1024);
	if(clientMainEpollFd <= 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] TCP Client can connect create epoll event");
		return;
	}

	//创建管道
	int masterWriteFd[2];
	if(pipe(masterWriteFd) < 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CTCPServerException] can not create pipe..");
		return;
	}

	//fork 子进程 用于循环读消息
	int i = 0;
	for(i = 0 ; i < 1 ;i++){


		int forkPid = -1;
		forkPid=fork();
		if(forkPid==-1){

		}else if(forkPid == 0){

			close(masterWriteFd[1]);
			initRecProcess(socket,clientMainEpollFd,masterWriteFd[0]);

		}else{

			//设置管道
			clientRecProcessWritePipeFd = masterWriteFd[1];
			close(masterWriteFd[0]);

			//记录读线程ID
			clientRecProcessId = forkPid;
	
			signal(SIGCHLD, catchTcpClientChildSig);

			atexit(clientProcessExit);
		}
	}
}

PHP_METHOD(CTcpClient,on){
	zval	*callback,
			*saveCallback;
	char	*callback_name,
			*eventName;
	int		eventNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&eventName,&eventNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		throwClientException("[CTcpClientException] call [CTcpClient->on] the params is not a callback function");
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }

	zval	*eventTable;


	//save to callbackObject
	eventTable = zend_read_property(CTcpClientCe,getThis(),ZEND_STRL("eventTable"), 0 TSRMLS_CC);
	MAKE_STD_ZVAL(saveCallback);
	ZVAL_ZVAL(saveCallback,callback,1,0);
	add_assoc_zval(eventTable,eventName,saveCallback);
	zend_update_property(CTcpClientCe,getThis(),ZEND_STRL("eventTable"),eventTable TSRMLS_CC);

	clientPHPObject = getThis();

    efree(callback_name);
	RETVAL_ZVAL(getThis(),1,0);
}


//useType 1服务器 2客户端
void createMessage(char *event,char *message,int type,char **stringMessage,int useType TSRMLS_DC){

	//构造消息
	zval	*messageZval;
	char	*jsonEncoder,
			*base64Encoder;

	MAKE_STD_ZVAL(messageZval);
	array_init(messageZval);

	add_next_index_long(messageZval,type);
	add_next_index_string(messageZval,event,1);
	add_next_index_string(messageZval,message,1);

	if(useType == 2){
		if(clientSessionId == NULL){
			add_next_index_string(messageZval,"",1);
		}else{
			add_next_index_string(messageZval,clientSessionId,1);
		}
	}
	if(useType == 3){
		if(serverSessionId == NULL){
			add_next_index_string(messageZval,"",1);
		}else{
			add_next_index_string(messageZval,serverSessionId,1);
		}
	}
	
	json_encode(messageZval,&jsonEncoder);
	base64Encode(jsonEncoder,&base64Encoder);
	spprintf(stringMessage,0,"%s%c",base64Encoder,'\n');

	//destroy
	zval_ptr_dtor(&messageZval);
	efree(jsonEncoder);
	efree(base64Encoder);
}


PHP_METHOD(CTcpClient,send){

	char	*message = NULL,
			*event = NULL;
	int		messageLen = 0,
			eventLen = 0;
	zval	*socket,
			*sendList;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&event,&eventLen,&message,&messageLen) == FAILURE){
		RETURN_FALSE;
	}

	if(event == NULL || message == NULL){
		RETURN_FALSE;
	}

	//记录一条发送消息 
	char *stringMessage;
	createMessage(event,message,MSG_USER,&stringMessage,2 TSRMLS_CC);

	//发送至管道
	int status = -1;
	while(1){
		status = write(clientRecProcessWritePipeFd,stringMessage,strlen(stringMessage));
		if(status == strlen(stringMessage)){
			break;
		}
		//发送失败则阻塞进程
		usleep(500);
	}

	efree(stringMessage);

	if(status <= 0){
		RETVAL_FALSE;
	}

	RETVAL_TRUE;
}


PHP_METHOD(CSocket,close)
{
	zval *socketId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("socketId"), 0 TSRMLS_CC);
	serverCloseClientSocket(Z_LVAL_P(socketId),getThis());
}

PHP_METHOD(CSocket,getClientInfo)
{
	zval *message = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	RETVAL_ZVAL(message,1,0);
}

PHP_METHOD(CSocket,read)
{
	zval *message = zend_read_property(CSocketCe,getThis(),ZEND_STRL("message"), 0 TSRMLS_CC);
	RETVAL_ZVAL(message,1,0);
}

PHP_METHOD(CSocket,send)
{
	char	*message,
			*event;
	int		messageLen = 0,
			eventLen = 0;
	zval	*socket,
			*sendList;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&event,&eventLen,&message,&messageLen) == FAILURE){
		throwServerException("[CTcpServerException] call [CTcpServer->send] params error ,need 2 string");
		RETURN_FALSE;
	}

	zval *socketId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("socketId"), 0 TSRMLS_CC);
	zval *socketType = zend_read_property(CSocketCe,getThis(),ZEND_STRL("socketType"), 0 TSRMLS_CC);

	//记录一条发送消息
	char *stringMessage;
	createMessage(event,message,MSG_USER,&stringMessage,1 TSRMLS_CC);

	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,Z_LVAL_P(socketId));
	add_index_string(sendMessage,1,stringMessage,0);

	//判断客户端是否还在现在
	char socketIndex[100];
	sprintf(socketIndex,"%d_%d",getpid(),Z_LVAL_P(socketId));
	if(IS_ARRAY == Z_TYPE_P(serverSocketList) && zend_hash_exists(Z_ARRVAL_P(serverSocketList),socketIndex,strlen(socketIndex)+1) ){
	}else{
		zval_ptr_dtor(&sendMessage);
		RETURN_FALSE;
	}

	//获取此socket的列表
	zval **thisSocketList;
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_P(socketId),(void**)&thisSocketList) && IS_ARRAY == Z_TYPE_PP(thisSocketList) ){
		if(zend_hash_num_elements(Z_ARRVAL_PP(thisSocketList)) >= MAX_SENDLIST_LEN ){
			zval_ptr_dtor(&sendMessage);
			RETURN_FALSE;
		}
	}else{
		//键不存在则创建一条
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_index_zval(serverSendList,Z_LVAL_P(socketId),saveArray);
		zend_hash_index_find(Z_ARRVAL_P(serverSendList),Z_LVAL_P(socketId),(void**)&thisSocketList);
	}

	if(IS_ARRAY != Z_TYPE_PP(thisSocketList)){
		zval_ptr_dtor(&sendMessage);
		RETURN_FALSE;
	}

	//重新读取
	int n =zend_hash_num_elements(Z_ARRVAL_PP(thisSocketList));
	add_next_index_zval(*thisSocketList,sendMessage);
	RETURN_LONG(n+1);
}


PHP_METHOD(CSocket,getSocketId)
{
	zval *socketId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("socketId"), 0 TSRMLS_CC);
	RETVAL_ZVAL(socketId,1,0);
}

PHP_METHOD(CSocket,getRemoteIp)
{
	zval *remoteIp = zend_read_property(CSocketCe,getThis(),ZEND_STRL("remoteIp"), 0 TSRMLS_CC);
	RETVAL_ZVAL(remoteIp,1,0);
}

PHP_METHOD(CSocket,getConnectTime)
{
	zval *data = zend_read_property(CSocketCe,getThis(),ZEND_STRL("connectTime"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CSocket,getSessionId)
{
	zval	**sessionId;

	zval *client = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(client) && SUCCESS == zend_hash_find(Z_ARRVAL_P(client),"sessionId",strlen("sessionId")+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) ){
		RETURN_STRING(Z_STRVAL_PP(sessionId),1);
	}
	RETURN_FALSE;
}


PHP_METHOD(CSocket,getProcessId)
{
	zval *data = zend_read_property(CSocketCe,getThis(),ZEND_STRL("processId"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}

PHP_METHOD(CSocket,getLastActiveTime)
{
	zval *data = zend_read_property(CSocketCe,getThis(),ZEND_STRL("lastActiveTime"), 0 TSRMLS_CC);
	RETVAL_ZVAL(data,1,0);
}


PHP_METHOD(CTcpServer,getAllConnection)
{

	//异步回调函数
	zval	*callback,
			*saveHandler,
			**thisEvent;

	char	*callback_name;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }
	efree(callback_name);

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	////向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	MAKE_STD_ZVAL(saveHandler);
	ZVAL_ZVAL(saveHandler,callback,1,0);
	add_assoc_zval(serverAsyncList,eventId,saveHandler);


	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\"}",eventId);
	createMessage("getAllConnection",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);

}


PHP_METHOD(CTcpServer,onError)
{
	zval	*callback,
			*saveHandler;
	char	*callback_name;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }
	efree(callback_name);

	MAKE_STD_ZVAL(serverErrorHandler);
	ZVAL_ZVAL(serverErrorHandler,callback,1,0);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpClient,close)
{
	clientStatus = 4;

	//clientMainEpollFd clientSocketId
	ClientCloseSocket(clientMainEpollFd,clientSocketId);

	kill(clientRecProcessId,SIGKILL);
}

PHP_METHOD(CTcpClient,onError){
	zval	*callback,
			*saveHandler;
	char	*callback_name;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }
	efree(callback_name);

	MAKE_STD_ZVAL(clientErrorHandler);
	ZVAL_ZVAL(clientErrorHandler,callback,1,0);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpClient,sleep){

}

PHP_METHOD(CTcpServer,sendToSessionId)
{}


void CTcpServer_sendToUid(char *toUid,char *event,zval *message,zval *callback,zval *object TSRMLS_DC){

	zval	*saveHandler,
			*sessionId;
	char	*callback_name,
			*sessionIdString = NULL;

	if(callback != NULL){
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			throwServerException("[CTcpServerException] call [CTcpServer->sendToUid] the sec param is not a callback function");
			return;
		}
		efree(callback_name);
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		return;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);

		php_printf("wait event callback\n");
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	if(callback != NULL){
		MAKE_STD_ZVAL(saveHandler);
		ZVAL_ZVAL(saveHandler,callback,1,0);
		add_assoc_zval(serverAsyncList,eventId,saveHandler);
	}

	//向网关服务器查询
	char	*gatewayMessage,
			*eventCon,
			trueToUid[120];

	zval	*sendMessage;

	sprintf(trueToUid,"u_%s",toUid);
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_assoc_string(sendMessage,"eventId",eventId,1);
	add_assoc_string(sendMessage,"toUid",trueToUid,1);
	add_assoc_string(sendMessage,"event",event,1);
	add_assoc_zval(sendMessage,"message",message);
	json_encode(sendMessage,&eventCon);
	createMessage("forwardMessage",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);
	efree(eventCon);
	zval_ptr_dtor(&sendMessage);
}

PHP_METHOD(CTcpServer,sendToUid)
{
	char	*uid,
			*event;
	int		uidLen = 0,
			eventLen = 0;
	zval	*message,
			*callback = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssz|z",&uid,&uidLen,&event,&eventLen,&message,&callback) == FAILURE){
		RETURN_FALSE;
	}

	//转发消息给指定UID
	CTcpServer_sendToUid(uid,event,message,callback,getThis() TSRMLS_CC);

}

PHP_METHOD(CSocket,bindUid)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId;
	char	*uid = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		uidLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&uid,&uidLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if(callback != NULL){
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			throwServerException("[CTcpServerException] call [CTcpServer->bindUid] the sec param is not a callback function");
			RETVAL_ZVAL(getThis(),1,0);
			return;
		}
		efree(callback_name);
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	if(callback != NULL){
		MAKE_STD_ZVAL(saveHandler);
		ZVAL_ZVAL(saveHandler,callback,1,0);
		add_assoc_zval(serverAsyncList,eventId,saveHandler);
	}

	//本地记录bindUid记录
	char trueUid[120];
	sprintf(trueUid,"u_%s",uid);
	add_assoc_string(serverUidData,trueUid,sessionIdString,1);

	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"uid\":\"%s\",\"clientSessionId\":\"%s\"}",eventId,trueUid,sessionIdString);
	createMessage("bindUid",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CSocket,unBindUid)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId;
	char	*uid = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		uidLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&uid,&uidLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if(callback != NULL){
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			throwServerException("[CTcpServerException] call [CTcpServer->bindUid] the sec param is not a callback function");
			RETVAL_ZVAL(getThis(),1,0);
			return;
		}
		efree(callback_name);
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	if(callback != NULL){
		MAKE_STD_ZVAL(saveHandler);
		ZVAL_ZVAL(saveHandler,callback,1,0);
		add_assoc_zval(serverAsyncList,eventId,saveHandler);
	}


	//本地记录bindUid记录
	char trueUid[120];
	sprintf(trueUid,"u_%s",uid);
	zend_hash_del(Z_ARRVAL_P(serverUidData),trueUid,strlen(uid)+1);

	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"uid\":\"%s\",\"clientSessionId\":\"%s\"}",eventId,trueUid,sessionIdString);
	createMessage("unBindUid",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CSocket,joinGroup)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId;
	char	*groupName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		groupNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&groupName,&groupNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if(callback != NULL){
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			throwServerException("[CTcpServerException] call [CTcpServer->joinGroup] the sec param is not a callback function");
			RETVAL_ZVAL(getThis(),1,0);
			return;
		}
		efree(callback_name);
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	if(callback != NULL){
		MAKE_STD_ZVAL(saveHandler);
		ZVAL_ZVAL(saveHandler,callback,1,0);
		add_assoc_zval(serverAsyncList,eventId,saveHandler);
	}

	add_assoc_string(serverGroup,Z_STRVAL_PP(sessionIdZval),groupName,1);

	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"group\":\"%s\",\"clientSessionId\":\"%s\"}",eventId,groupName,sessionIdString);
	createMessage("joinGroup",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);

}

PHP_METHOD(CSocket,getGroup)
{
	zval	*callback,
			*saveHandler,
			*sessionId;
	char	*groupName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		groupNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&groupName,&groupNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		throwServerException("[CTcpServerException] call [CTcpServer->getGroup] the sec param is not a callback function");
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }
	efree(callback_name);

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	MAKE_STD_ZVAL(saveHandler);
	ZVAL_ZVAL(saveHandler,callback,1,0);
	add_assoc_zval(serverAsyncList,eventId,saveHandler);


	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"group\":\"%s\"}",eventId,groupName);
	createMessage("getGroup",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,getGroup)
{
	zval	*callback,
			*saveHandler,
			*sessionId;
	char	*groupName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		groupNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&groupName,&groupNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
        efree(callback_name);
		throwServerException("[CTcpServerException] call [CTcpServer->getGroup] the sec param is not a callback function");
        RETVAL_ZVAL(getThis(),1,0);
        return;
    }
	efree(callback_name);

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);
	MAKE_STD_ZVAL(saveHandler);
	ZVAL_ZVAL(saveHandler,callback,1,0);
	add_assoc_zval(serverAsyncList,eventId,saveHandler);

	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"group\":\"%s\"}",eventId,groupName);
	createMessage("getGroup",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CSocket,leaveGroup)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId;
	char	*groupName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		groupNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|z",&groupName,&groupNameLen,&callback) == FAILURE){
		RETURN_FALSE;
	}

	if(callback != NULL){
		if (!zend_is_callable(callback, 0, &callback_name TSRMLS_CC)) {
			efree(callback_name);
			throwServerException("[CTcpServerException] call [CTcpServer->joinGroup] the sec param is not a callback function");
			RETVAL_ZVAL(getThis(),1,0);
			return;
		}
		efree(callback_name);
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	while(1){
		if(zend_hash_num_elements(Z_ARRVAL_P(serverAsyncList)) <= 1024){
			break;
		}
		//等待事件触发消耗完成  防止过度积累
		usleep(200);
	}

	//向此对象添加回调 生成事件随机值
	char *eventId;
	getSessionID(&eventId);

	if(callback != NULL){
		MAKE_STD_ZVAL(saveHandler);
		ZVAL_ZVAL(saveHandler,callback,1,0);
		add_assoc_zval(serverAsyncList,eventId,saveHandler);
	}


	zend_hash_del(Z_ARRVAL_P(serverGroup),Z_STRVAL_PP(sessionIdZval),strlen(Z_STRVAL_PP(sessionIdZval))+1);


	//向网关服务器查询
	char	*gatewayMessage,
			eventCon[100];
	sprintf(eventCon,"{\"eventId\":\"%s\",\"group\":\"%s\",\"clientSessionId\":\"%s\"}",eventId,groupName,sessionIdString);
	createMessage("leaveGroup",eventCon,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	efree(gatewayMessage);
	efree(eventId);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,broadcastToGroup)
{
}

PHP_METHOD(CTcpServer,broadcast)
{

	char	*message,
			*event;
	int		messageLen = 0,
			eventLen = 0;
	zval	*socket,
			*sendList;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&event,&eventLen,&message,&messageLen) == FAILURE){
		throwServerException("[CTcpServerException] call [CTcpServer->send] params error ,need 2 string");
		RETURN_FALSE;
	}

	//如果网关失败 则抛弃此消息
	if(serverToGatewayStatus != 1){
		return;
	}

	//记录一条发送消息
	zval	*messageJson;
	char	*jsonString;
	MAKE_STD_ZVAL(messageJson);
	array_init(messageJson);
	add_index_long(messageJson,0,MSG_USER);
	add_index_string(messageJson,1,event,1);
	add_index_string(messageJson,2,message,1);
	json_encode(messageJson,&jsonString);

	char *gatewayMessage;
	createMessage("broadcast",jsonString,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	sendToGateway(gatewayMessage);

	zval_ptr_dtor(&messageJson);
	efree(jsonString);
	efree(gatewayMessage);
}

PHP_METHOD(CSocket,setSession)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId,
			*keyData = NULL,
			**thisSessionData;

	char	*keyName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		keyNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&keyName,&keyNameLen,&keyData) == FAILURE){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(IS_ARRAY == Z_TYPE_P(sessionId) && SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	//本地记录session serverLocalSessionData
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1,(void**)&thisSessionData) && IS_ARRAY == Z_TYPE_PP(thisSessionData) ){
	}else{
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_assoc_zval(serverLocalSessionData,sessionIdString,saveArray);
		zend_hash_find(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1,(void**)&thisSessionData);
	}

	//将消息保存至本地sessionData中
	zval *saveLocal;
	MAKE_STD_ZVAL(saveLocal);
	ZVAL_ZVAL(saveLocal,keyData,1,0);
	add_assoc_zval(*thisSessionData,keyName,saveLocal);

	RETURN_TRUE;
}

PHP_METHOD(CSocket,getSession)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId,
			*keyData = NULL,
			**thisSessionData,
			**thisReturnData;

	char	*keyName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		keyNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&keyName,&keyNameLen) == FAILURE){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(IS_ARRAY == Z_TYPE_P(sessionId) && SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	//本地记录session serverLocalSessionData
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1,(void**)&thisSessionData) && IS_ARRAY == Z_TYPE_PP(thisSessionData) ){

		if(0 == zend_hash_num_elements(Z_ARRVAL_PP(thisSessionData)) ){
			zend_hash_del(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1);
			RETURN_NULL();
		}

		if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisSessionData),keyName,strlen(keyName)+1,(void**)&thisReturnData)){
			RETURN_ZVAL(*thisReturnData,1,0);
		}
	}

	RETURN_NULL();
}

PHP_METHOD(CSocket,delSession)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId,
			*keyData = NULL,
			**thisSessionData,
			**thisReturnData;

	char	*keyName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		keyNameLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&keyName,&keyNameLen) == FAILURE){
		RETURN_FALSE;
	}

	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	//本地记录session serverLocalSessionData
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1,(void**)&thisSessionData) && IS_ARRAY == Z_TYPE_PP(thisSessionData) ){

		if(0 == zend_hash_num_elements(Z_ARRVAL_PP(thisSessionData)) ){
			zend_hash_del(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1);
			RETURN_FALSE;
		}

		if(zend_hash_exists(Z_ARRVAL_PP(thisSessionData),keyName,strlen(keyName)+1)){
			zend_hash_del(Z_ARRVAL_PP(thisSessionData),keyName,strlen(keyName)+1);
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}

PHP_METHOD(CSocket,clearSession)
{
	zval	*callback = NULL,
			*saveHandler,
			*sessionId,
			*keyData = NULL,
			**thisSessionData,
			**thisReturnData;

	char	*keyName = NULL,
			*callback_name,
			*sessionIdString = NULL;
	int		keyNameLen = 0;


	//玩家client session
	sessionId = zend_read_property(CSocketCe,getThis(),ZEND_STRL("client"), 0 TSRMLS_CC);
	zval **sessionIdZval;
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(sessionId),"sessionId",strlen("sessionId")+1,(void**)&sessionIdZval)){
		sessionIdString = Z_STRVAL_PP(sessionIdZval);
	}else{
		RETURN_FALSE;
	}

	//本地记录session serverLocalSessionData
	if(zend_hash_exists(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1) ){
		zend_hash_del(Z_ARRVAL_P(serverLocalSessionData),sessionIdString,strlen(sessionIdString)+1);
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(CSocketClient,read)
{
	zval *message = zend_read_property(CSocketClientCe,getThis(),ZEND_STRL("message"), 0 TSRMLS_CC);
	RETVAL_ZVAL(message,1,0);
}

PHP_METHOD(CSocketClient,send)
{
	char	*message,
			*event;
	int		messageLen = 0,
			eventLen = 0;
	zval	*socket,
			*sendList;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&event,&eventLen,&message,&messageLen) == FAILURE){
		throwServerException("[CTcpClientException] call [CSocketClient->send] params error ,need 2 string");
		RETURN_FALSE;
	}


	//记录一条发送消息
	char *stringMessage;
	createMessage(event,message,MSG_USER,&stringMessage,2 TSRMLS_CC);

	zval *socketId = zend_read_property(CSocketClientCe,getThis(),ZEND_STRL("socketId"), 0 TSRMLS_CC);
	zval *socketType = zend_read_property(CSocketClientCe,getThis(),ZEND_STRL("socketType"), 0 TSRMLS_CC);

	zval *sendMessage;
	MAKE_STD_ZVAL(sendMessage);
	array_init(sendMessage);
	add_index_long(sendMessage,0,Z_LVAL_P(socketId));
	add_index_string(sendMessage,1,stringMessage,0);

	//同步消息发送
	int len = zend_hash_num_elements(Z_ARRVAL_P(clientSendList));
	if(len >= MAX_SENDLIST_LEN){
		zval_ptr_dtor(&sendMessage);
		RETURN_FALSE;
	}
	add_next_index_zval(clientSendList,sendMessage);
	RETVAL_LONG(len+1);
}

PHP_METHOD(CTcpGateway,__construct)
{}

PHP_METHOD(CTcpGateway,__destruct)
{
	if(gatewaySocketList != NULL){
		zval_ptr_dtor(&gatewaySocketList);
	}
	if(gatewaySessionList != NULL){
		zval_ptr_dtor(&gatewaySessionList);
	}
	if(gatewayUserSessionList != NULL){
		zval_ptr_dtor(&gatewayUserSessionList);
	}
	if(gatewaySendList != NULL){
		zval_ptr_dtor(&gatewaySendList);
	}
	if(gatewayGroupList != NULL){
		zval_ptr_dtor(&gatewayGroupList);
	}
	if(gatewayUidSessionList != NULL){
		zval_ptr_dtor(&gatewayUidSessionList);
	}
	if(gatewaySessionData != NULL){
		zval_ptr_dtor(&gatewaySessionData);
	}
}

PHP_METHOD(CTcpGateway,getInstance)
{
		char	*key,
			*saveKey;
	int		keyLen = 0;

	zval	*selfInstace,
			**instaceSaveZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	if(keyLen == 0){
		key = "main";
	}


	selfInstace = zend_read_static_property(CTcpGatewayCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CTcpGatewayCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CTcpGatewayCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		RETVAL_ZVAL(*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CTcpGatewayCe);

		//执行其构造器 并传入参数
		if (CTcpGatewayCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CTcpGatewayCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CTcpGatewayCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);

		RETURN_ZVAL(object,1,0);
	}
}

PHP_METHOD(CTcpGateway,bind)
{
	char	*host;
	int		hostLen = 0;
	long	port = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&host,&hostLen,&port) == FAILURE){
		RETURN_FALSE;
	}

	if(port == 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CTCPGatewayException] call [CTCPGateway->bind] the port must available port");
		return;
	}

	zend_update_property_string(CTcpGatewayCe,getThis(),ZEND_STRL("host"),host TSRMLS_CC);
	zend_update_property_long(CTcpGatewayCe,getThis(),ZEND_STRL("port"),port TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

PHP_METHOD(CTcpServer,gateway){

	char	*host;
	int		hostLen = 0;
	long	port = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&host,&hostLen,&port) == FAILURE){
		RETURN_FALSE;
	}

	if(port == 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CTCPServerException] call [CTCPServer->gateway] the port must available port");
		return;
	}

	zend_update_property_string(CTcpServerCe,getThis(),ZEND_STRL("gatewayHost"),host TSRMLS_CC);
	zend_update_property_long(CTcpServerCe,getThis(),ZEND_STRL("gatewayPort"),port TSRMLS_CC);
	zend_update_property_long(CTcpServerCe,getThis(),ZEND_STRL("gatewayUse"),1 TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);
}

//遍历发送队列 向各worker发送消息
void gatewayCloseWokerSocket(int fd);
void doGatewaySendMessageList(){

	int		i,h,j,k,writeStatus;
	zval	**thisSocketList,
			**thisMessage;
	char	*socketIdString,
			*messageIdString;
	ulong	socketId,
			messageId;

	h = zend_hash_num_elements(Z_ARRVAL_P(gatewaySendList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewaySendList));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_key(Z_ARRVAL_P(gatewaySendList),&socketIdString,&socketId,0);
		zend_hash_get_current_data(Z_ARRVAL_P(gatewaySendList),(void**)&thisSocketList);

		//为空则略过此消息
		if(IS_ARRAY != Z_TYPE_PP(thisSocketList)){
			zend_hash_move_forward(Z_ARRVAL_P(gatewaySendList));
			continue;
		}

		k = zend_hash_num_elements(Z_ARRVAL_PP(thisSocketList));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(thisSocketList));
		for(j = 0 ; j < k ; j++){
			zend_hash_get_current_data(Z_ARRVAL_PP(thisSocketList),(void**)&thisMessage);
			zend_hash_get_current_key(Z_ARRVAL_PP(thisSocketList),&messageIdString,&messageId,0);

			//转发给socketId
			errno = 0;
			writeStatus = write(socketId,Z_STRVAL_PP(thisMessage),Z_STRLEN_PP(thisMessage));

			//写入失败
			if(writeStatus <= 0){	
				if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN){
					break;
				}else{
				
					//worker发生异常 
					gatewayCloseWokerSocket(socketId);
					break;
				}
			}

			//写入一半
			if(writeStatus < Z_STRLEN_PP(thisMessage)){
				char *notSendMessage;
				substr(Z_STRVAL_PP(thisMessage),writeStatus,Z_STRLEN_PP(thisMessage)-writeStatus,&notSendMessage);
				add_index_string(*thisMessage,1,notSendMessage,0);
				break;
			}



			//删除此消息
			zend_hash_move_forward(Z_ARRVAL_PP(thisSocketList));
			zend_hash_index_del(Z_ARRVAL_PP(thisSocketList),messageId);
		}

		zend_hash_move_forward(Z_ARRVAL_P(gatewaySendList));
	}
}


//转发消息给所有的worker
void gatewayBroadcastMessage(char *thisMessage,zval *object TSRMLS_CC){

	int		i,h;
	zval	**thisSocketSendList;
	char	*key;
	ulong	uKey;

	h = zend_hash_num_elements(Z_ARRVAL_P(gatewaySocketList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewaySocketList));

	for(i = 0 ; i < h;i++){
		zend_hash_get_current_key(Z_ARRVAL_P(gatewaySocketList),&key,&uKey,0);

		//检查sendList
		if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(gatewaySendList),(int)uKey,(void**)&thisSocketSendList) && IS_ARRAY == Z_TYPE_PP(thisSocketSendList) ){
		}else{
			zval *saveArray;
			MAKE_STD_ZVAL(saveArray);
			array_init(saveArray);
			add_index_zval(gatewaySendList,(int)uKey,saveArray);
			zend_hash_index_find(Z_ARRVAL_P(gatewaySendList),(int)uKey,(void**)&thisSocketSendList);
		}

		//当前的消息
		char *sendMessage;
		spprintf(&sendMessage,0,"%s%c",thisMessage,'\n');
		add_next_index_string(*thisSocketSendList,sendMessage,0);

		zend_hash_move_forward(Z_ARRVAL_P(gatewaySocketList));
	}
}

void gatewaySendMessageToSocket(int fd,char *thisMessage,zval *object TSRMLS_DC){

	zval	**thisSocketSendList;

	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(gatewaySendList),fd,(void**)&thisSocketSendList) && IS_ARRAY == Z_TYPE_PP(thisSocketSendList) ){
	}else{
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_index_zval(gatewaySendList,fd,saveArray);
		zend_hash_index_find(Z_ARRVAL_P(gatewaySendList),fd,(void**)&thisSocketSendList);
	}

	add_next_index_string(*thisSocketSendList,thisMessage,1);
}

//响应所有的sessionList
void gatewayResponseAllSession(int fd,char *message,zval *object TSRMLS_DC){

	//获取事件id
	zval	*eventJson,
			**eventId;
	json_decode(message,&eventJson);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId)){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}


	zval	*allSessionList;
	char	*key;
	ulong	uKey;

	MAKE_STD_ZVAL(allSessionList);
	array_init(allSessionList);

	int		i,h;
	h = zend_hash_num_elements(Z_ARRVAL_P(gatewayUserSessionList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewayUserSessionList));
	for(i = 0 ; i < h;i++){
		zend_hash_get_current_key(Z_ARRVAL_P(gatewayUserSessionList),&key,&uKey,0);
		add_next_index_string(allSessionList,key,1);
		zend_hash_move_forward(Z_ARRVAL_P(gatewayUserSessionList));
	}

	//最终数组
	zval	*responseData;
	MAKE_STD_ZVAL(responseData);
	array_init(responseData);
	add_assoc_string(responseData,"eventId",Z_STRVAL_PP(eventId),1);
	add_assoc_zval(responseData,"data",allSessionList);


	//转json
	char	*jsonString,
			*gatewayMessage;

	json_encode(responseData,&jsonString);
	createMessage("getAllConnection",jsonString,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(fd,gatewayMessage,object TSRMLS_CC);

	//destory
	zval_ptr_dtor(&eventJson);
	efree(gatewayMessage);
	zval_ptr_dtor(&allSessionList);
	zval_ptr_dtor(&responseData);
}

//type 1加入群组  2退出群组
void gatewayEditGroup(int fd,char *message,int type,zval *object TSRMLS_DC){

	//获取事件id
	zval	*eventJson,
			**group,
			**sessionId,
			**thisGroup,
			**eventId;

	json_decode(message,&eventJson);

	//eventId
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId)){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"group",6,(void**)&group) && IS_STRING == Z_TYPE_PP(group) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"clientSessionId",strlen("clientSessionId")+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId)	){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	//判断当前群组是否存在
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(gatewayGroupList),Z_STRVAL_PP(group),strlen(Z_STRVAL_PP(group))+1,(void**)&thisGroup) && IS_ARRAY == Z_TYPE_PP(thisGroup)){
	}else{
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_assoc_zval(gatewayGroupList,Z_STRVAL_PP(group),saveArray);
		zend_hash_find(Z_ARRVAL_P(gatewayGroupList),Z_STRVAL_PP(group),strlen(Z_STRVAL_PP(group))+1,(void**)&thisGroup);
	}
	
	//向群组中添加客户端sessionId
	if(type == 1){
		add_assoc_long(*thisGroup,Z_STRVAL_PP(sessionId),0);
		writeLogs("[CTcpGateway] worker call joinGroup now gourp[%s] list is len:[%d] \n",Z_STRVAL_PP(group),zend_hash_num_elements(Z_ARRVAL_PP(thisGroup)));
	}
	
	//删除群组
	if(type == 2){
		zend_hash_del(Z_ARRVAL_PP(thisGroup),Z_STRVAL_PP(sessionId),strlen(Z_STRVAL_PP(sessionId))+1);
		writeLogs("[CTcpGateway] worker call leaveGroup now gourp[%s] list is len:[%d] \n",Z_STRVAL_PP(group),zend_hash_num_elements(Z_ARRVAL_PP(thisGroup)));
	}

	//触发回调
	char	callbackInfo[200],
			*gatewayMessage;
	int	nums = zend_hash_num_elements(Z_ARRVAL_PP(thisGroup));
	sprintf(callbackInfo,"{\"eventId\":\"%s\",\"data\":{\"result\":true,\"members\":%d,\"sessionId\":\"%s\"}}",Z_STRVAL_PP(eventId),nums,Z_STRVAL_PP(sessionId));
	createMessage("joinGroup",callbackInfo,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(fd,gatewayMessage,object TSRMLS_CC);
	efree(gatewayMessage);

	//destory
	zval_ptr_dtor(&eventJson);
}

void gatewayGetGroup(int fd,char *message,zval *object TSRMLS_DC){

	//获取事件id
	zval	*eventJson,
			**group,
			**sessionId,
			**thisGroup,
			**eventId;

	json_decode(message,&eventJson);

	//eventId
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId)){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"group",6,(void**)&group) && IS_STRING == Z_TYPE_PP(group)	){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}


	//判断当前群组是否存在
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(gatewayGroupList),Z_STRVAL_PP(group),strlen(Z_STRVAL_PP(group))+1,(void**)&thisGroup) && IS_ARRAY == Z_TYPE_PP(thisGroup)){
	}else{
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_assoc_zval(gatewayGroupList,Z_STRVAL_PP(group),saveArray);
		zend_hash_find(Z_ARRVAL_P(gatewayGroupList),Z_STRVAL_PP(group),strlen(Z_STRVAL_PP(group))+1,(void**)&thisGroup);
	}

	zval	*returnMessage;
	MAKE_STD_ZVAL(returnMessage);
	array_init(returnMessage);

	int		i,h;
	char	*key;
	ulong	uKey;
	h = zend_hash_num_elements(Z_ARRVAL_PP(thisGroup));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(thisGroup));
	for(i = 0 ; i < h ;i++){
		zend_hash_get_current_key(Z_ARRVAL_PP(thisGroup),&key,&uKey,0);
		add_next_index_string(returnMessage,key,1);
		zend_hash_move_forward(Z_ARRVAL_PP(thisGroup));
	}

	//转为json
	zval	*callbackMessage;
	char	*callbackJson;
	MAKE_STD_ZVAL(callbackMessage);
	array_init(callbackMessage);
	add_assoc_string(callbackMessage,"eventId",Z_STRVAL_PP(eventId),1);
	add_assoc_zval(callbackMessage,"data",returnMessage);
	json_encode(callbackMessage,&callbackJson);


	//响应消息
	char	*gatewayMessage;
	createMessage("joinGroup",callbackJson,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(fd,gatewayMessage,object TSRMLS_CC);
	efree(gatewayMessage);

	//destory
	efree(callbackJson);
	zval_ptr_dtor(&eventJson);
	zval_ptr_dtor(&returnMessage);
	zval_ptr_dtor(&callbackMessage);
}

//处理UID
void gatewayEditUid(int fd,char *message,int type,zval *object TSRMLS_DC)
{
	//获取事件id
	zval	*eventJson,
			**uid,
			**sessionId,
			**thisGroup,
			**eventId;

	json_decode(message,&eventJson);

	//eventId
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId)){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"uid",strlen("uid")+1,(void**)&uid) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"clientSessionId",strlen("clientSessionId")+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) ){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	char trueUid[120];
	if(IS_STRING == Z_TYPE_PP(uid)){
		sprintf(trueUid,"%s",Z_STRVAL_PP(uid));
	}else if(IS_LONG == Z_TYPE_PP(uid)){
		sprintf(trueUid,"%d",Z_LVAL_PP(uid));
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	if(type == 1){
		add_assoc_string(gatewayUidSessionList,trueUid,Z_STRVAL_PP(sessionId),1);
		writeLogs("[CTcpGateway] worker call bindUser now uid-session list is len:[%d] \n",zend_hash_num_elements(Z_ARRVAL_P(gatewayUidSessionList)));
	}else{
		zend_hash_del(Z_ARRVAL_P(gatewayUidSessionList),trueUid,strlen(trueUid)+1);
		writeLogs("[CTcpGateway] worker call unBindUser now uid-session list is len:[%d] \n",zend_hash_num_elements(Z_ARRVAL_P(gatewayUidSessionList)));
	}

	char	callbackInfo[400],
			*gatewayMessage;

	sprintf(callbackInfo,"{\"eventId\":\"%s\",\"data\":{\"result\":true,\"sessionId\":\"%s\"}}",Z_STRVAL_PP(eventId),Z_STRVAL_PP(sessionId));
	createMessage("bindUid",callbackInfo,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(fd,gatewayMessage,object TSRMLS_CC);



	efree(gatewayMessage);
	zval_ptr_dtor(&eventJson);
}

void gatewayClearUserDisconnect(char *userSessionId){

	//删除session和worker映射记录
	zend_hash_del(Z_ARRVAL_P(gatewayUserSessionList),userSessionId,strlen(userSessionId)+1);

	//删除此session加入的群组
	int		i,h,j,k;
	zval	**thisGroupUser,
			**thisUidSessionId;
	char	*key;
	ulong	uKey;

	h = zend_hash_num_elements(Z_ARRVAL_P(gatewayGroupList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewayGroupList));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(gatewayGroupList),(void**)&thisGroupUser);


		if(IS_ARRAY == Z_TYPE_PP(thisGroupUser) && zend_hash_exists(Z_ARRVAL_PP(thisGroupUser),userSessionId,strlen(userSessionId)+1) ){
			zend_hash_del(Z_ARRVAL_PP(thisGroupUser),userSessionId,strlen(userSessionId)+1);
		}
		zend_hash_move_forward(Z_ARRVAL_P(gatewayGroupList));
	}

	//删除uid与session绑定 gatewayUidSessionList
	h = zend_hash_num_elements(Z_ARRVAL_P(gatewayUidSessionList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewayUidSessionList));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(gatewayUidSessionList));
	for(i = 0 ; i < h ; i++){
		if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(gatewayUidSessionList),(void**)&thisUidSessionId) && IS_STRING == Z_TYPE_PP(thisUidSessionId) && strcmp(Z_STRVAL_PP(thisUidSessionId),userSessionId) == 0 ){
			zend_hash_get_current_key(Z_ARRVAL_P(gatewayUidSessionList),&key,&uKey,0);
			zend_hash_del(Z_ARRVAL_P(gatewayUidSessionList),key,strlen(key)+1);
		}
		zend_hash_move_forward(Z_ARRVAL_P(gatewayUidSessionList));
	}


	//删除此session绑定的sessionData

}

//操作session
void gatewayEditSession(int fd,char *message,int type,zval *object TSRMLS_DC)
{

	//获取事件id
	zval	*eventJson,
			**uid,
			**sessionId,
			**thisGroup,
			**eventId;

	json_decode(message,&eventJson);

	//eventId
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",8,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId)){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"uid",strlen("uid")+1,(void**)&uid) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"clientSessionId",strlen("clientSessionId")+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) ){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}




	zval_ptr_dtor(&eventJson);
}

void gatewayForwardMessage(int fd,char *message,zval *object TSRMLS_DC){

	zval	*eventJson,
			**toUid,
			**eventId,
			**event,
			**sendMessage,
			**sessionId,
			**workerFd;

	json_decode(message,&eventJson);

	if(	SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"toUid",strlen("toUid")+1,(void**)&toUid) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"eventId",strlen("eventId")+1,(void**)&eventId) && IS_STRING == Z_TYPE_PP(eventId) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"event",strlen("event")+1,(void**)&event) && IS_STRING == Z_TYPE_PP(event) && 
		SUCCESS == zend_hash_find(Z_ARRVAL_P(eventJson),"message",strlen("message")+1,(void**)&sendMessage)
	){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	//寻找toUid 的session
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(gatewayUidSessionList),Z_STRVAL_PP(toUid),strlen(Z_STRVAL_PP(toUid))+1,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) ){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}

	//寻找session所在的worker的fd
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(gatewayUserSessionList),Z_STRVAL_PP(sessionId),strlen(Z_STRVAL_PP(sessionId))+1,(void**)&workerFd) && IS_LONG == Z_TYPE_PP(workerFd) ){
	}else{
		zval_ptr_dtor(&eventJson);
		return;
	}


	//向toUid所在worker回复消息 要求转发消息
	char	*gatewayMessage;
	createMessage("forwardMessage",message,MSG_USER,&gatewayMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(Z_LVAL_PP(workerFd),gatewayMessage,object TSRMLS_CC);
	efree(gatewayMessage);


	//回复查询者 触发成功回调
	char	*rebackMessage,
			callbackInfo[200];
	sprintf(callbackInfo,"{\"eventId\":\"%s\",\"data\":{\"result\":true,\"sessionId\":\"%s\"}}",Z_STRVAL_PP(eventId),Z_STRVAL_PP(sessionId));
	createMessage("forwardMessageSuccess",callbackInfo,MSG_USER,&rebackMessage,3 TSRMLS_CC);
	gatewaySendMessageToSocket(fd,rebackMessage,object TSRMLS_CC);
	efree(rebackMessage);

	zval_ptr_dtor(&eventJson);
}


void processGatewayMessage(int fd,char *thisMessage,zval *object TSRMLS_DC){

	char	*base64Decoder;
	zval	*jsonDecoder,
			**type,
			**message,
			**sessionId;

	base64Decode(thisMessage,&base64Decoder);
	json_decode(base64Decoder,&jsonDecoder);

	if(IS_ARRAY != Z_TYPE_P(jsonDecoder) || zend_hash_num_elements(Z_ARRVAL_P(jsonDecoder)) != 4){
		efree(base64Decoder);
		zval_ptr_dtor(&jsonDecoder);
		return;
	}

	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),1,(void**)&type);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),2,(void**)&message);
	zend_hash_index_find(Z_ARRVAL_P(jsonDecoder),3,(void**)&sessionId);

	//worker 注册
	if(strcmp(Z_STRVAL_PP(type),"serverRegister") == 0){
		//记录socket gatewaySocketList
		zval	*sessionData,
				*nowTime;
		microtime(&nowTime);
		MAKE_STD_ZVAL(sessionData);
		array_init(sessionData);
		add_index_long(sessionData,0,fd);
		add_index_double(sessionData,1,Z_DVAL_P(nowTime));
		zval_ptr_dtor(&nowTime);
		add_assoc_zval(gatewaySessionList,Z_STRVAL_PP(sessionId),sessionData);
		add_index_string(gatewaySocketList,fd,Z_STRVAL_PP(sessionId),1);
		writeLogs("[CTcpGateway] worker[%s] is register success...\n",Z_STRVAL_PP(sessionId));
	}

	//socket上线
	if(strcmp(Z_STRVAL_PP(type),"connect") == 0){

		//读取session保存到  gatewayUserSessionList
		zval	*messageContent,
				**clientSessionId;
		json_decode(Z_STRVAL_PP(message),&messageContent);
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageContent),"sessionId",strlen("sessionId")+1,(void**)&clientSessionId) ){
			//保存session映射记录
			add_assoc_long(gatewayUserSessionList,Z_STRVAL_PP(clientSessionId),fd);
		}
		zval_ptr_dtor(&messageContent);
		writeLogs("[CTcpGateway] User connect,now session num is [%d] ...\n",zend_hash_num_elements(Z_ARRVAL_P(gatewayUserSessionList)));
	}

	//socket下线
	if(strcmp(Z_STRVAL_PP(type),"disconnect") == 0){

		//读取session 从gatewayUserSessionList移除
		zval	*messageContent,
				**clientSessionId;
		json_decode(Z_STRVAL_PP(message),&messageContent);
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(messageContent),"sessionId",strlen("sessionId")+1,(void**)&clientSessionId) ){
			
			//删除网关绑定的数据
			gatewayClearUserDisconnect(Z_STRVAL_PP(clientSessionId));
		}

		zval_ptr_dtor(&messageContent);
		writeLogs("[CTcpGateway] User disconnect,now session num is [%d] ...\n",zend_hash_num_elements(Z_ARRVAL_P(gatewayUserSessionList)));
	}

	//消息广播
	if(strcmp(Z_STRVAL_PP(type),"broadcast") == 0){
	
		//将这个 thisMessage 转发给所有在线的woker
		gatewayBroadcastMessage(thisMessage,object TSRMLS_CC);
	
	}

	//查询所有连接
	if(strcmp(Z_STRVAL_PP(type),"getAllConnection") == 0){
		gatewayResponseAllSession(fd,Z_STRVAL_PP(message),object TSRMLS_CC);
	}

	//客户端绑定群组
	if(strcmp(Z_STRVAL_PP(type),"joinGroup") == 0){
		gatewayEditGroup(fd,Z_STRVAL_PP(message),1,object TSRMLS_CC);
	}

	//客户端解除群组
	if(strcmp(Z_STRVAL_PP(type),"leaveGroup") == 0){
		gatewayEditGroup(fd,Z_STRVAL_PP(message),2,object TSRMLS_CC);
	}

	//获取群组信息
	if(strcmp(Z_STRVAL_PP(type),"getGroup") == 0){
		gatewayGetGroup(fd,Z_STRVAL_PP(message),object TSRMLS_CC);
	}

	//绑定UID
	if(strcmp(Z_STRVAL_PP(type),"bindUid") == 0){
		gatewayEditUid(fd,Z_STRVAL_PP(message),1,object TSRMLS_CC);
	}

	//解绑UID
	if(strcmp(Z_STRVAL_PP(type),"unBindUid") == 0){
		gatewayEditUid(fd,Z_STRVAL_PP(message),2,object TSRMLS_CC);
	}

	//操作session gatewaySessionData
	if(strcmp(Z_STRVAL_PP(type),"setSession") == 0){
		gatewayEditSession(fd,Z_STRVAL_PP(message),1,object TSRMLS_CC);
	}

	//转发消息
	if(strcmp(Z_STRVAL_PP(type),"forwardMessage") == 0){
		gatewayForwardMessage(fd,Z_STRVAL_PP(message),object TSRMLS_CC);
	}


	efree(base64Decoder);
	zval_ptr_dtor(&jsonDecoder);

}

//检测到worker断线
void gatewayCloseWokerSocket(int fd){

	//找出worker session
	zval	**sessionId;
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(gatewaySocketList),fd,(void**)&sessionId) && IS_STRING == Z_TYPE_PP(sessionId) ){

		writeLogs("[CTcpGateway] worker[%s] is unRegister...\n",Z_STRVAL_PP(sessionId));

		//删除session
		if(zend_hash_exists(Z_ARRVAL_P(gatewaySessionList),Z_STRVAL_PP(sessionId),strlen(Z_STRVAL_PP(sessionId))+1) ){
			zend_hash_del(Z_ARRVAL_P(gatewaySessionList),Z_STRVAL_PP(sessionId),strlen(Z_STRVAL_PP(sessionId))+1);
		}
		zend_hash_index_del(Z_ARRVAL_P(gatewaySocketList),fd);
	}

	if(zend_hash_index_exists(Z_ARRVAL_P(gatewaySendList),fd)){
		zend_hash_index_del(Z_ARRVAL_P(gatewaySendList),fd);
	}

	close(fd);

}

//网关服务器 每30秒检查一次 各worker状态
void checkServerSocketStatus(){

	int num = zend_hash_num_elements(Z_ARRVAL_P(gatewaySessionList));
	writeLogs("[CTcpGateway] begin to check worker status, now worker num is [%d]...\n",num);



}

void gateWaytimerCallback(){

	int timestamp = getMicrotime();

	if(timestamp - gatewayLastCheckTime >= 30){

		checkServerSocketStatus();

		gatewayLastCheckTime = getMicrotime();
	}
}

PHP_METHOD(CTcpGateway,listen)
{
	zval	*host,
			*port,
			*object,
			**argv,
			**SERVER,
			*pidList;

	int		errorCode = 0,
			isDaemon = 0;

	char	appPath[2024],
			codePath[2024];

	RETVAL_ZVAL(getThis(),1,0);

	ini_seti("memory_limit",-1);

	host = zend_read_property(CTcpGatewayCe,getThis(),ZEND_STRL("host"), 0 TSRMLS_CC);
	port = zend_read_property(CTcpGatewayCe,getThis(),ZEND_STRL("port"), 0 TSRMLS_CC);

	//init framework
	getcwd(appPath,sizeof(appPath));
	sprintf(codePath,"%s%s",appPath,"/application");
	php_define("APP_PATH",appPath TSRMLS_CC);
	php_define("CODE_PATH",codePath TSRMLS_CC);
	CWebApp_createApp(&object TSRMLS_CC);
	zval_ptr_dtor(&object);


	(void)zend_hash_find(&EG(symbol_table),ZEND_STRS("_SERVER"), (void **)&SERVER);
	if(zend_hash_find(Z_ARRVAL_PP(SERVER),"argv",strlen("argv")+1,(void**)&argv) == SUCCESS && IS_ARRAY == Z_TYPE_PP(argv)){
		int	i,h;
		zval **thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_PP(argv));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(argv));
		for(i = 0 ; i < h;i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(argv),(void**)&thisVal);
			if(strstr(Z_STRVAL_PP(thisVal),"--daemon") != NULL){
				isDaemon = 1;
			}
			zend_hash_move_forward(Z_ARRVAL_PP(argv));
		}
	}

	//设置信号 阻止进程退出
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, 0);

	//daemon
	if(isDaemon){
		writeLogs("run as a daemon process..\n");
		int s = daemon(1, 0);
	}

	if(gatewaySocketList == NULL){
		MAKE_STD_ZVAL(gatewaySocketList);
		array_init(gatewaySocketList);
	}
	if(gatewaySessionList == NULL){
		MAKE_STD_ZVAL(gatewaySessionList);
		array_init(gatewaySessionList);
	}
	if(gatewayUserSessionList == NULL){
		MAKE_STD_ZVAL(gatewayUserSessionList);
		array_init(gatewayUserSessionList);
	}
	if(gatewaySendList == NULL){
		MAKE_STD_ZVAL(gatewaySendList);
		array_init(gatewaySendList);
	}
	if(gatewayGroupList == NULL){
		MAKE_STD_ZVAL(gatewayGroupList);
		array_init(gatewayGroupList);
	}
	if(gatewayUidSessionList == NULL){
		MAKE_STD_ZVAL(gatewayUidSessionList);
		array_init(gatewayUidSessionList);
	}
	if(gatewaySessionData == NULL){
		MAKE_STD_ZVAL(gatewaySessionData);
		array_init(gatewaySessionData);
	}

	//创建socket套接字 而后fork子进程 以共享此套接字
	int listenSocket = startTcpListen(Z_STRVAL_P(host),Z_LVAL_P(port));
	if(listenSocket < 0){
		//监听异常
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] TCP Gateway can not bind %s:%d ",Z_STRVAL_P(host),Z_LVAL_P(port));
		return;
	}

	//加入epoll
	int epfd = epoll_create(1024);
	if(epfd <= 0){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[FatalException] TCP Gateway can not create epoll fd");
		return;
	}

	//绑定读入流
	struct epoll_event  ev;
    ev.events = EPOLLIN; 
    ev.data.fd = listenSocket;
	epoll_ctl(epfd,EPOLL_CTL_ADD,listenSocket,&ev);

	struct epoll_event revs[128];
    int n = sizeof(revs)/sizeof(revs[0]);
    int timeout = 3000;
    int i,num = 0;

	//开始网络请求
	while(1){

		//开始epoll事件等待
       num = epoll_wait(epfd,revs,n,500);

	   //检查心跳状态
	   gateWaytimerCallback();

	   //先检查发送消息队列
	   doGatewaySendMessageList();

	   for(i = 0 ; i < num; i++){

			int fd = revs[i].data.fd;

			// 调用accept接受新连接
			if( fd == listenSocket && (revs[i].events & EPOLLIN) ){

				struct sockaddr_in client;
				socklen_t len = sizeof(client);
				extern int errno;
				int new_sock = accept(fd,(struct sockaddr *)&client,&len);

				if( new_sock < 0 ){
					if(errno == 11){
					}else{
						setLogs("CTcpGateway [%d] accept fail,errorno:%d \n",getpid(),errno);
					}
					continue;
				}

				//socket设为非阻塞
				fcntl(new_sock, F_SETFL, fcntl(new_sock, F_GETFL, NULL) | O_NONBLOCK);

				//新加入消息
				ev.events = EPOLLIN;
				ev.data.fd = new_sock;
				epoll_ctl(epfd,EPOLL_CTL_ADD,new_sock,&ev);
				continue;
			}

			if(revs[i].events & EPOLLIN)	{

				char		buf[2],
							*thisMessage;
				int			readLen = 0,k;

				smart_str	tempBuffer[10240] = {0};

				while(1){
					errno = 0;
					readLen = read(fd,buf,sizeof(buf)-1);

					if(readLen <= 0){
						if(readLen == 0){

							smart_str_0(&tempBuffer[fd]);
							smart_str_free(&tempBuffer[fd]);

							epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
							gatewayCloseWokerSocket(fd);
						}
						break;
					}

					if(buf[0] != '\n'){
						smart_str_appendc(&tempBuffer[fd],buf[0]);
					}else{
						smart_str_0(&tempBuffer[fd]);
						thisMessage = estrdup(tempBuffer[fd].c);
						smart_str_free(&tempBuffer[fd]);
						processGatewayMessage(fd,thisMessage,object TSRMLS_CC);
						efree(thisMessage);
						break;
					}	
				}
			}
	   }

	}

}

PHP_METHOD(CTcpGateway,on)
{}

PHP_METHOD(CTcpGateway,onData)
{}

PHP_METHOD(CTcpGateway,onError)
{}


#endif