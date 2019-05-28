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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"


#include "php_CMyFrameExtension.h"
#include "php_CWebApp.h"
#include <string.h>
#include "SAPI.h"


//获取CPUID
int getCPUID(zval **list){

	char *getCPUShell = "dmidecode -t 4 | grep ID",
		 *cpuList = "",
		 *matchRegx = "/ID:\\s(.*)\\n/";

	zval *matchList,
		 **matchIndexZval;

	TSRMLS_FETCH();

	MAKE_STD_ZVAL(*list);
	array_init(*list);


	//提取CPU
	exec_shell_return(getCPUShell,&cpuList);

	//未安装命令提取失败
	if(strstr(cpuList,"found") != NULL){
		efree(cpuList);
		return 0;
	}

	//正则提取其中的ID
	if(!preg_match_all(matchRegx,cpuList,&matchList)){
		zval_ptr_dtor(&matchList);
		efree(cpuList);
		return 0;
	}

	if(IS_ARRAY != Z_TYPE_P(matchList) || 2 < zend_hash_num_elements(Z_ARRVAL_P(matchList)) ){
		zval_ptr_dtor(&matchList);
		efree(cpuList);
		return 0;
	}

	//获取index1
	if(SUCCESS != zend_hash_index_find(Z_ARRVAL_P(matchList),1,(void**)&matchIndexZval)){
		zval_ptr_dtor(&matchList);
		efree(cpuList);
		return 0;
	}

	//复制
	ZVAL_ZVAL(*list,*matchIndexZval,1,0);

	efree(cpuList);
	zval_ptr_dtor(&matchList);

	return 1;
}

//返回服务中止时间
void getServiceExpire(char **limitVal){
	*limitVal = estrdup("1512230400.00");
}


void checkVersionAuth(TSRMLS_D){

	//版本信息
	float limitTime = 1512230400.00;
	zval *nowTime;
	char *serverIp = "47.52.216.43",
		 *nowServerIp;

	//读取系统时间
	microtime(&nowTime);
	if(limitTime - Z_DVAL_P(nowTime) <= 0){
		zval_ptr_dtor(&nowTime);
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[CopyrightException] Service authorization has expired, please contact Quick business to obtain technical support");
	}
	zval_ptr_dtor(&nowTime);

	//判断是否CLI
	if(strcmp(sapi_module.name,"cli") == 0){
		return;
	}

	//读取服务器IP
	getServerParam("SERVER_ADDR",&nowServerIp TSRMLS_CC);
	if(nowServerIp == NULL){
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[CopyrightException] Authorization fails, please contact Quick business to obtain technical support");
	}

	//判断IP
	if(strcmp(nowServerIp,serverIp) != 0){
		efree(nowServerIp);
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[CopyrightException] Authorization fails, please contact Quick business to obtain technical support");
	}

	efree(nowServerIp);
}

