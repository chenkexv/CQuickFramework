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
#include "php_CEncrypt.h"
#include "php_CWebApp.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CEncrypt_functions[] = {
	PHP_ME(CEncrypt,AesDecode,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CEncrypt,AesEncode,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CEncrypt)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CEncrypt",CEncrypt_functions);
	CEncryptCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

void pkcs5_pad(char *input,int size,char **returnString){

	int		padLess = 0,
			pad = 0,
			i = 0;
	char	chrPad,
			repeat[1024];

	padLess = strlen(input) % size;
	pad = size - padLess;
	chrPad = pad;

	for(i = 0; i < pad ; i++){
		repeat[i] = chrPad;
	}
	repeat[pad] = '\0';

	strcat2(returnString,input,repeat,NULL);
}

void CEncrypt_AesEncode(char *val,char *key,zval **returnObjectZval TSRMLS_DC){

	char	*paddingInput;

	zval	*sizeObject = NULL,
			sizeObjectReturn,
			tdObjectReturn,
			*tdObject = NULL,
			ivsizeReturn,
			*ivSizeObject,
			ivReturn,
			*ivObject,
			dataReturn,
			*dataObject;

	MAKE_STD_ZVAL(*returnObjectZval);
	ZVAL_FALSE(*returnObjectZval);

	//mcrypt_get_block_size
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[2],
				params1,
				params2;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_STRING(paramsList[0],"rijndael-128",1);
		ZVAL_STRING(paramsList[1],"ecb",1);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_get_block_size", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &sizeObjectReturn, 2, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		sizeObject = &sizeObjectReturn;
	MODULE_END

	//如果不为空
	if(IS_LONG != Z_TYPE_P(sizeObject)){
		zval_dtor(&sizeObjectReturn);
		return;
	}

	//计算pkcspad
	pkcs5_pad(val,Z_LVAL_P(sizeObject),&paddingInput);

	//mcrypt_module_open
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[4],
				params1,
				params2,
				params3,
				params4;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		paramsList[2] = &params3;
		paramsList[3] = &params4;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);

		ZVAL_STRING(paramsList[0],"rijndael-128",1);
		ZVAL_STRING(paramsList[1],"",1);
		ZVAL_STRING(paramsList[2],"ecb",1);
		ZVAL_STRING(paramsList[3],"",1);

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_module_open", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &tdObjectReturn, 4, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		tdObject = &tdObjectReturn;
	MODULE_END

	if(IS_RESOURCE != Z_TYPE_P(tdObject)){
		efree(paddingInput);
		zval_dtor(&tdObjectReturn);
		zval_dtor(&sizeObjectReturn);
		return;
	}
		
	//mcrypt_enc_get_iv_size
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[1],
				params1;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],tdObject,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_enc_get_iv_size", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &ivsizeReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		ivSizeObject = &ivsizeReturn;
	MODULE_END

	//mcrypt_create_iv
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[2],
				params1,
				params2;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_ZVAL(paramsList[0],ivSizeObject,1,0)
		ZVAL_LONG(paramsList[1],2);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_create_iv", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &ivReturn, 2, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		ivObject = &ivReturn;
	MODULE_END

	//mcrypt_generic_init
	MODULE_BEGIN
		zval	constructVal,
				returnZval,
				*paramsList[3],
				params1,
				params2,
				params3;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		paramsList[2] = &params3;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);

		ZVAL_ZVAL(paramsList[0],tdObject,1,0);
		ZVAL_STRING(paramsList[1],key,1);
		ZVAL_ZVAL(paramsList[2],ivObject,1,0);

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_generic_init", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &returnZval, 3, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_dtor(&returnZval);
	MODULE_END

	//mcrypt_generic
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[2],
				params1,
				params2;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);

		ZVAL_ZVAL(paramsList[0],tdObject,1,0);
		ZVAL_STRING(paramsList[1],paddingInput,1);

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_generic", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &dataReturn, 2, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		dataObject = &dataReturn;
	MODULE_END

	ZVAL_ZVAL(*returnObjectZval,dataObject,1,0);

	//关闭资源 mcrypt_generic_deinit
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*paramsList[1],
				params1;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],tdObject,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_generic_deinit", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&returnObject);
	MODULE_END

	//mcrypt_module_close
	MODULE_BEGIN
		zval	constructVal,
				returnObject,
				*paramsList[1],
				params1;
		paramsList[0] = &params1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],tdObject,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_module_close", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &returnObject, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&returnObject);
	MODULE_END


	//销毁的变量
	efree(paddingInput);
	zval_dtor(&sizeObjectReturn);
	zval_dtor(&tdObjectReturn);
	zval_dtor(&ivsizeReturn);
	zval_dtor(&ivReturn);
	zval_dtor(&dataReturn);
}


//aes加密
PHP_METHOD(CEncrypt,AesEncode)
{
	char	*key,
			*val;
	int		keyLen = 0,
			valLen = 0;

	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&val,&valLen,&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	CEncrypt_AesEncode(val,key,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

void CEncrypt_AesDecode(zval *val,char *key,zval **returnObjectZval TSRMLS_DC)
{
	char	*resultString,
			*returnString,
			lastChar;

	int		lastAscii;

	zval	decodeReturn,
			*decodeObject;


	MAKE_STD_ZVAL(*returnObjectZval);
	ZVAL_NULL(*returnObjectZval);

	//调用mcrypt_decrypt
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[4],
				params1,
				params2,
				params3,
				params4;
		paramsList[0] = &params1;
		paramsList[1] = &params2;
		paramsList[2] = &params3;
		paramsList[3] = &params4;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);

		ZVAL_STRING(paramsList[0],"rijndael-128",1);
		ZVAL_STRING(paramsList[1],key,1);
		ZVAL_ZVAL(paramsList[2],val,1,0);
		ZVAL_STRING(paramsList[3],"ecb",1);

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"mcrypt_decrypt", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &decodeReturn, 4, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		decodeObject = &decodeReturn;
	MODULE_END

	//失败
	if(IS_STRING != Z_TYPE_P(decodeObject)){
		zval_dtor(&decodeReturn);
		return;
	}

	resultString = estrdup(Z_STRVAL_P(decodeObject));

	//取末尾一位
	lastChar = resultString[strlen(resultString)-1];

	//将其转为ascii码
	lastAscii = (int)lastChar;

	substr(resultString,0,0 - lastAscii,&returnString);

	ZVAL_STRING(*returnObjectZval,returnString,1);

	//销毁
	zval_dtor(&decodeReturn);
	efree(resultString);
	efree(returnString);
}

PHP_METHOD(CEncrypt,AesDecode)
{
	char	*key;
	int		keyLen = 0;

	zval	*val,
			*returnZval;

	char	lastChar;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&val,&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	CEncrypt_AesDecode(val,key,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}