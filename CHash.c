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
#include "php_CException.h"
#include "php_CHash.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CHash_functions[] = {
	PHP_ME(CHash,rand,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CHash)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CHash",CHash_functions);
	CHashCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

void CHash_rand(long len,long type,char **returnString TSRMLS_DC){

	char	*string1 = "0123456789",
			*string2 = "abcdefghijklmnopqrstuvwxyz",
			*string3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
			*string4 = "~@#$%^&*(){}[]|",
			*useString;

	int		endCount = 0,
			i,
			nowIndex;

	smart_str	endString = {0};


	if(type == 0){
		spprintf(&useString,0,"%s%s%s",string1,string2,string3);
	}else if(type == -1){
		spprintf(&useString,0,"%s%s%s%s",string1,string2,string3,string4);
	}else{
		if(type == 1){
			useString = estrdup(string1);
		}else if(type == 2){
			useString = estrdup(string2);
		}else if(type == 3){
			useString = estrdup(string3);
		}else if(type == 4){
			useString = estrdup(string4);
		}else{
			useString = estrdup("");
		}
	}

	endCount = strlen(useString) - 1;


	for(i = 0 ; i < len;i++){
		nowIndex = php_rand_call(0,endCount);
		smart_str_appendc(&endString,useString[nowIndex]);
	}

	smart_str_0(&endString);
	*returnString = estrdup(endString.c);

	efree(useString);
	smart_str_free(&endString);

}

PHP_METHOD(CHash,rand)
{
	char	*returnString;
	long	returnLen = 0,
			inputType = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l|l",&returnLen,&inputType) == FAILURE){
		RETVAL_NULL();
		return;
	}

	if(returnLen == 0){
		RETVAL_NULL();
		return;
	}

	CHash_rand(returnLen,inputType,&returnString TSRMLS_CC);
	RETVAL_STRING(returnString,1);
	efree(returnString);
}