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


#include "php_CQuickFramework.h"
#include "php_CMonitor.h"
#include "php_CWebApp.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CMonitor_functions[] = {
	PHP_ME(CMonitor,getCPU,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getMemory,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getSwap,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getNetworkStat,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getDisk,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getProcess,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getLoadAverage,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMonitor,getNetConnection,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CMonitor)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CMonitor",CMonitor_functions);
	CMonitorCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

#ifdef PHP_WIN32
PHP_METHOD(CMonitor,getCPU)
{}
PHP_METHOD(CMonitor,getMemory)
{}
PHP_METHOD(CMonitor,getSwap)
{}
PHP_METHOD(CMonitor,getNetworkStat)
{}
PHP_METHOD(CMonitor,getDisk)
{}
PHP_METHOD(CMonitor,getProcess)
{}
PHP_METHOD(CMonitor,getLoadAverage)
{}
PHP_METHOD(CMonitor,getNetConnection)
{}

void CMonitor_getCPU(zval **returnData,int num TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}
void CMonitor_getMemory(zval **returnData,int num TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

void CMonitor_getSwap(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

void CMonitor_getNetworkStat(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

void CMonitor_getDisk(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

void CMonitor_getProcess(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

void CMonitor_getNetConnection(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}
void CMonitor_getLoadAverage(zval **returnData TSRMLS_DC){
	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);
}

#else
void CMonitor_getCPU(zval **returnData,int num TSRMLS_DC){

	char	*runCommand,
			*returnString;

	zval	*thisData;

	if(num > 1){
		char numString[1024];
		sprintf(numString,"%s%d%s","top -bn ",num," -i -c");
		runCommand = estrdup(numString);
	}else{
		runCommand = estrdup("top -bn 1 -i -c");
	}

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);
	efree(runCommand);

	//正则提取
	preg_match_all("/Cpu(.*):\\s*(.*)us,\\s*(.*)sy,\\s*(.*)ni,\\s*(.*)id,\\s*(.*)wa,\\s*(.*)hi,\\s*(.*)si,\\s*(.*)st\\n/",returnString,&thisData);


	if(IS_ARRAY != Z_TYPE_P(thisData) || zend_hash_num_elements(Z_ARRVAL_P(thisData)) != 10){
		zval_ptr_dtor(&thisData);
		efree(returnString);
		return;
	}

	//组装数据 us
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),2,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"us",saveData);
	MODULE_END
	
	//sy
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),3,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"sy",saveData);
	MODULE_END

	//ni
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),4,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"ni",saveData);
	MODULE_END

	//ni
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),4,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"ni",saveData);
	MODULE_END

	//id
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),5,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"id",saveData);
	MODULE_END

	//wa
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),6,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"wa",saveData);
	MODULE_END

	//hi
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),7,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"hi",saveData);
	MODULE_END

	//si
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),8,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"si",saveData);
	MODULE_END

	//st
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),9,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"st",saveData);
	MODULE_END

	//销毁数据
	zval_ptr_dtor(&thisData);
	efree(returnString);
}


PHP_METHOD(CMonitor,getCPU)
{
	zval *data;
	int		getNum = 1;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&getNum) == FAILURE){
		RETURN_FALSE;
	}

	CMonitor_getCPU(&data,getNum TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getMemory(zval **returnData,int num TSRMLS_DC){

	char	*runCommand,
			*returnString;

	zval	*thisData;

	if(num > 1){
		char numString[11];
		sprintf(numString,"%d",num);
		strcat2(&runCommand,"top -bn ",numString,"  -i -c",NULL);
	}else{
		runCommand = estrdup("top -bn 1 -i -c");
	}

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//正则提取
	preg_match_all("/Mem:\\s*(\\d+)k total,\\s*(\\d+)k used,\\s*(\\d+)k free,\\s*(\\d+)k buffers\\n/",returnString,&thisData);
	if(IS_ARRAY != Z_TYPE_P(thisData) || zend_hash_num_elements(Z_ARRVAL_P(thisData)) != 5){
		zval_ptr_dtor(&thisData);
		efree(returnString);
		efree(runCommand);
		return;
	}

	//组装数据 total
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),1,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"total",saveData);
	MODULE_END

	//used
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),2,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"used",saveData);
	MODULE_END

	//free
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),3,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"free",saveData);
	MODULE_END

	//buffers
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),4,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"buffers",saveData);
	MODULE_END
	

	//销毁数据
	zval_ptr_dtor(&thisData);
	efree(returnString);
	efree(runCommand);
}


PHP_METHOD(CMonitor,getMemory)
{
	zval *data;
	int		getNum = 1;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&getNum) == FAILURE){
		RETURN_FALSE;
	}

	CMonitor_getMemory(&data,getNum TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getSwap(zval **returnData TSRMLS_DC){

	char	*runCommand = "top -bn 1 -i -c",
			*returnString;

	zval	*thisData;

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//正则提取
	preg_match_all("/Swap:\\s*(\\d+)k total,\\s*(\\d+)k used,\\s*(\\d+)k free,\\s*(\\d+)k\\s*cached\\n/",returnString,&thisData);

	if(IS_ARRAY != Z_TYPE_P(thisData) || zend_hash_num_elements(Z_ARRVAL_P(thisData)) != 5){
		zval_ptr_dtor(&thisData);
		efree(returnString);
		return;
	}

	//组装数据 total
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),1,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"total",saveData);
	MODULE_END

	//used
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),2,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"used",saveData);
	MODULE_END

	//free
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),3,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"free",saveData);
	MODULE_END

	//buffers
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),4,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"buffers",saveData);
	MODULE_END
	

	//销毁数据
	zval_ptr_dtor(&thisData);
	efree(returnString);
}

PHP_METHOD(CMonitor,getSwap)
{
	zval *data;
	CMonitor_getSwap(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getDataByNetCard(zval *cradData,char *cradName,zval **cardEndData TSRMLS_DC){

	int		i,j;
	zval	**rowZval;

	MAKE_STD_ZVAL(*cardEndData);
	array_init(*cardEndData);


	j = zend_hash_num_elements(Z_ARRVAL_P(cradData));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(cradData));
	for(i = 0 ; i < j ; i ++){
		zend_hash_get_current_data(Z_ARRVAL_P(cradData),(void**)&rowZval);

		//入网卡字节数
		if(i == 1){
			char	*byte,
					replaceString[1024];
			sprintf(replaceString,"%s%s",cradName,":");
			str_replace(replaceString,"",Z_STRVAL_PP(rowZval),&byte);
			add_assoc_string(*cardEndData,"in_bytes",byte,1);
			efree(byte);
		}

		//入网卡包数
		if(i == 2){
			add_assoc_string(*cardEndData,"in_packets",Z_STRVAL_PP(rowZval),1);
		}

		//入网卡错误包数
		if(i == 3){
			add_assoc_string(*cardEndData,"in_errpackets",Z_STRVAL_PP(rowZval),1);
		}

		//入网卡丢弃包数
		if(i == 4){
			add_assoc_string(*cardEndData,"in_droppackets",Z_STRVAL_PP(rowZval),1);
		}

		//出字节数
		if(i == 9){
			add_assoc_string(*cardEndData,"out_bytes",Z_STRVAL_PP(rowZval),1);
		}

		//出包数
		if(i == 10){
			add_assoc_string(*cardEndData,"out_packets",Z_STRVAL_PP(rowZval),1);
		}

		//出错误包数
		if(i == 11){
			add_assoc_string(*cardEndData,"out_errpackets",Z_STRVAL_PP(rowZval),1);
		}

		//出丢弃包数
		if(i == 12){
			add_assoc_string(*cardEndData,"out_droppackets",Z_STRVAL_PP(rowZval),1);
		}

		zend_hash_move_forward(Z_ARRVAL_P(cradData));
	}


}

void CMonitor_getNetworkStat(zval **returnData TSRMLS_DC){

	char	*runCommand = "cat /proc/net/dev",
			*returnString,
			*thisCradString,
			*cardName;

	zval	*thisData,
			*lineArray,
			**rowZval,
			*cardData,
			*cradTrueData,
			*cardNameZval,
			*replaceEmp;

	int		i,j;

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//按行拆分
	php_explode(PHP_EOL,returnString,&lineArray);
	if(IS_ARRAY != Z_TYPE_P(lineArray)){
		efree(returnString);
		zval_ptr_dtor(&lineArray);
		return;
	}

	j = zend_hash_num_elements(Z_ARRVAL_P(lineArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(lineArray));
	for(i = 0 ; i < j ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(lineArray),(void**)&rowZval);

		if(strstr(Z_STRVAL_PP(rowZval),"Receive") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		if(strstr(Z_STRVAL_PP(rowZval),"face") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//替换
		str_replace(": ",":",Z_STRVAL_PP(rowZval),&replaceEmp);

		//多个空格替换成横线
		preg_repalce("/[\\s]+/is","-",replaceEmp,&thisCradString);
		efree(replaceEmp);

		//提取不同的网卡 按空格分割每个网卡的数据
		php_explode("-",thisCradString,&cardData);
		if(IS_ARRAY != Z_TYPE_P(cardData)){
			zval_ptr_dtor(&cardData);
			efree(thisCradString);
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//提取网卡名字
		php_explode(":",thisCradString,&cardNameZval);
		if(IS_ARRAY == Z_TYPE_P(cardNameZval) && zend_hash_num_elements(Z_ARRVAL_P(cardNameZval)) >= 2){
			zval **cardNameIndex;
			zend_hash_index_find(Z_ARRVAL_P(cardNameZval),0,(void**)&cardNameIndex);
			php_trim(Z_STRVAL_PP(cardNameIndex),"-",&cardName);
		}else{
			char tempName[1024];
			sprintf(tempName,"%s%d","eth",i);
			cardName = estrdup(tempName);
		}

		//将这个数据组装成单个网卡
		CMonitor_getDataByNetCard(cardData,cardName,&cradTrueData TSRMLS_CC);

		//保存数据
		add_assoc_zval(*returnData,cardName,cradTrueData);

		//销毁数据
		zval_ptr_dtor(&cardData);
		zval_ptr_dtor(&cardNameZval);
		efree(thisCradString);
		efree(cardName);

		//移动浮标
		zend_hash_move_forward(Z_ARRVAL_P(lineArray));
	}



	//销毁数据
	efree(returnString);
	zval_ptr_dtor(&lineArray);
}

PHP_METHOD(CMonitor,getNetworkStat)
{
	zval *data;
	CMonitor_getNetworkStat(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getDataByDisk(zval *cradData,zval **cardEndData TSRMLS_DC)
{
	int		i,j;
	zval	**rowZval;

	MAKE_STD_ZVAL(*cardEndData);
	array_init(*cardEndData);

	j = zend_hash_num_elements(Z_ARRVAL_P(cradData));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(cradData));
	for(i = 0 ; i < j ; i ++){
		zend_hash_get_current_data(Z_ARRVAL_P(cradData),(void**)&rowZval);

		//分区名
		if(i == 0){

			if(IS_NULL == Z_TYPE_PP(rowZval) || (IS_STRING == Z_TYPE_PP(rowZval) && strlen(Z_STRVAL_PP(rowZval)) <= 0) ){
				zend_hash_move_forward(Z_ARRVAL_P(cradData));
				continue;
			}

			add_assoc_string(*cardEndData,"fileSystem",Z_STRVAL_PP(rowZval),1);
		}

		//size
		if(i == 1){
			add_assoc_string(*cardEndData,"size",Z_STRVAL_PP(rowZval),1);
		}

		//used
		if(i == 2){
			add_assoc_string(*cardEndData,"used",Z_STRVAL_PP(rowZval),1);
		}

		//Avail
		if(i == 3){
			add_assoc_string(*cardEndData,"avail",Z_STRVAL_PP(rowZval),1);
		}

		//Userate
		if(i == 4){
			add_assoc_string(*cardEndData,"useRate",Z_STRVAL_PP(rowZval),1);
		}

		//Mounted
		if(i == 5){
			add_assoc_string(*cardEndData,"mounted",Z_STRVAL_PP(rowZval),1);
		}

		zend_hash_move_forward(Z_ARRVAL_P(cradData));
	}
}


void CMonitor_getDisk(zval **returnData TSRMLS_DC){

	char	*runCommand = "df -h",
			*returnString,
			*thisCradString;

	zval	*thisData,
			*lineArray,
			**rowZval,
			*cardData,
			*cradTrueData;

	int		i,j;


	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);
	
	//按行拆分
	php_explode(PHP_EOL,returnString,&lineArray);
	if(IS_ARRAY != Z_TYPE_P(lineArray)){
		efree(returnString);
		zval_ptr_dtor(&lineArray);
		return;
	}

	j = zend_hash_num_elements(Z_ARRVAL_P(lineArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(lineArray));
	for(i = 0 ; i < j ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(lineArray),(void**)&rowZval);

		if(strstr(Z_STRVAL_PP(rowZval),"Filesystem") != NULL){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//多个空格替换成横线
		preg_repalce("/[\\s]+/is","-",Z_STRVAL_PP(rowZval),&thisCradString);

		//提取不同的磁盘 按空格分割每个网卡的数据
		php_explode("-",thisCradString,&cardData);
		if(IS_ARRAY != Z_TYPE_P(cardData)){
			zval_ptr_dtor(&cardData);
			efree(thisCradString);
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//将这个数据组装成单个分区
		CMonitor_getDataByDisk(cardData,&cradTrueData TSRMLS_CC);
	

		//保存数据
		if(zend_hash_num_elements(Z_ARRVAL_P(cradTrueData)) > 0){
			add_next_index_zval(*returnData,cradTrueData);
		}else{
			zval_ptr_dtor(&cradTrueData);
		}

		//销毁数据
		zval_ptr_dtor(&cardData);
		efree(thisCradString);

		//移动浮标
		zend_hash_move_forward(Z_ARRVAL_P(lineArray));
	}



	//销毁数据
	efree(returnString);
	zval_ptr_dtor(&lineArray);
}

PHP_METHOD(CMonitor,getDisk)
{
	zval *data;
	CMonitor_getDisk(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getProcess(zval **returnData TSRMLS_DC){

	char	*runCommand = "top -bn 1 -i -c",
			*returnString;

	zval	*thisData;

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//正则提取
	preg_match_all("/Tasks:\\s*(\\d+) total,\\s*(\\d+) running,\\s*(\\d+) sleeping,\\s*(\\d+) stopped,\\s*(\\d+) zombie\\n/",returnString,&thisData);


	//total
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),1,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"total",saveData);
	MODULE_END

	//running
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),2,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"running",saveData);
	MODULE_END

	//sleeping
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),3,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"sleeping",saveData);
	MODULE_END

	//stopped
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),4,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"stopped",saveData);
	MODULE_END

	//zombie
	MODULE_BEGIN
		zval	**thisIndex,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),5,(void**)&thisIndex);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*thisIndex,1,0);
		add_assoc_zval(*returnData,"zombie",saveData);
	MODULE_END
	
	//销毁数据
	zval_ptr_dtor(&thisData);
	efree(returnString);
}

PHP_METHOD(CMonitor,getProcess)
{
	zval *data;
	CMonitor_getProcess(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getLoadAverage(zval **returnData TSRMLS_DC){

	char	*runCommand = "top -bn 1 -i -c",
			*returnString;

	zval	*thisData;

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//正则提取
	preg_match_all("/load average: ([0-9\\.]+), ([0-9\\.]+), ([0-9\\.]+)\\n/",returnString,&thisData);

	//1min
	MODULE_BEGIN
		zval	**thisIndex,
				**indexIndexVal,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),1,(void**)&thisIndex);
		zend_hash_index_find(Z_ARRVAL_PP(thisIndex),0,(void**)&indexIndexVal);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*indexIndexVal,1,0);
		add_assoc_zval(*returnData,"loadaverage1",saveData);
	MODULE_END

	//5min
	MODULE_BEGIN
		zval	**thisIndex,
				**indexIndexVal,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),2,(void**)&thisIndex);
		zend_hash_index_find(Z_ARRVAL_PP(thisIndex),0,(void**)&indexIndexVal);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*indexIndexVal,1,0);
		add_assoc_zval(*returnData,"loadaverage5",saveData);
	MODULE_END

	//15min
	MODULE_BEGIN
		zval	**thisIndex,
				**indexIndexVal,
				*saveData;
		zend_hash_index_find(Z_ARRVAL_P(thisData),3,(void**)&thisIndex);
		zend_hash_index_find(Z_ARRVAL_PP(thisIndex),0,(void**)&indexIndexVal);
		MAKE_STD_ZVAL(saveData);
		ZVAL_ZVAL(saveData,*indexIndexVal,1,0);
		add_assoc_zval(*returnData,"loadaverage15",saveData);
	MODULE_END
	

	//销毁数据
	zval_ptr_dtor(&thisData);
	efree(returnString);
}

PHP_METHOD(CMonitor,getLoadAverage)
{
	zval *data;
	CMonitor_getLoadAverage(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}

void CMonitor_getNetConnection(zval **returnData TSRMLS_DC){

	char	*runCommand = "netstat -n | awk '/^tcp/ {++S[$NF]} END {for(a in S) print a, S[a]}'",
			*returnString;

	zval	*thisData,
			*lineArray,
			**rowZval,
			*statusRow,
			**netStatus,
			**netVal;

	int		i,j;

	MAKE_STD_ZVAL(*returnData);
	array_init(*returnData);

	exec_shell_return(runCommand,&returnString);

	//按行拆分
	php_explode(PHP_EOL,returnString,&lineArray);
	if(IS_ARRAY != Z_TYPE_P(lineArray)){
		zval_ptr_dtor(&lineArray);
		efree(returnString);
		return;
	}

	j = zend_hash_num_elements(Z_ARRVAL_P(lineArray));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(lineArray));
	for(i = 0 ; i < j ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(lineArray),(void**)&rowZval);
		
		if(Z_STRLEN_PP(rowZval) <= 0){
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//拆分状态
		php_explode(" ",Z_STRVAL_PP(rowZval),&statusRow);
		if(IS_ARRAY != Z_TYPE_P(statusRow) || zend_hash_num_elements(Z_ARRVAL_P(statusRow)) != 2){
			zval_ptr_dtor(&statusRow);
			zend_hash_move_forward(Z_ARRVAL_P(lineArray));
			continue;
		}

		//状态和值
		zend_hash_index_find(Z_ARRVAL_P(statusRow),0,(void**)&netStatus);
		zend_hash_index_find(Z_ARRVAL_P(statusRow),1,(void**)&netVal);

		add_assoc_string(*returnData,Z_STRVAL_PP(netStatus),Z_STRVAL_PP(netVal),1);
	

		zval_ptr_dtor(&statusRow);
		zend_hash_move_forward(Z_ARRVAL_P(lineArray));
	}
	
	//销毁数据
	zval_ptr_dtor(&lineArray);
	efree(returnString);
}

PHP_METHOD(CMonitor,getNetConnection)
{
	zval *data;
	CMonitor_getNetConnection(&data TSRMLS_CC);
	RETVAL_ZVAL(data,1,1);
}
#endif