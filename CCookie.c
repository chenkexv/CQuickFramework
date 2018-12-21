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
#include "php_CCookie.h"
#include "php_CEncrypt.h"
#include "php_CWebApp.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CCookie_functions[] = {
	PHP_ME(CCookie,set,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CCookie,get,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CCookie,del,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

char *aesKey = "8jnbhgtvfdcxmkiu";

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CCookie)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CCookie",CCookie_functions);
	CCookieCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

int CCookie_set(char *key,char *val,int timeout TSRMLS_DC){

	zval	*encodeString,
			*timenow;

	char	*base64encode;

	int		timenowInt = 0;


	//对val进行加密
	CEncrypt_AesEncode(val,aesKey,&encodeString TSRMLS_CC);

	if(IS_STRING != Z_TYPE_P(encodeString)){
		zval_ptr_dtor(&encodeString);
		return 0;
	}
	
	//对加密的内容 base64
	base64Encode(Z_STRVAL_P(encodeString),&base64encode);

	microtime(&timenow);
	timenowInt = (int)(Z_DVAL_P(timenow));
	timenowInt = timenowInt + timeout;


	//设置cookie
	MODULE_BEGIN
		zval	constructVal,
				callReturn,
				*paramsList[7],
				params1,
				params2,
				params3,
				params4,
				params5,
				params6,
				params7;

		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);
		MAKE_STD_ZVAL(paramsList[4]);
		MAKE_STD_ZVAL(paramsList[5]);
		MAKE_STD_ZVAL(paramsList[6]);

		ZVAL_STRING(paramsList[0],key,1);
		ZVAL_STRING(paramsList[1],base64encode,1);
		ZVAL_LONG(paramsList[2],timenowInt);
		ZVAL_STRING(paramsList[3],"/",1);
		ZVAL_NULL(paramsList[4]);
		ZVAL_NULL(paramsList[5]);
		ZVAL_TRUE(paramsList[6]);

		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"setcookie", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &callReturn, 7, paramsList TSRMLS_CC);

		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		zval_ptr_dtor(&paramsList[4]);
		zval_ptr_dtor(&paramsList[5]);
		zval_ptr_dtor(&paramsList[6]);

		zval_dtor(&callReturn);

	MODULE_END


	//销毁
	zval_ptr_dtor(&encodeString);
	zval_ptr_dtor(&timenow);
	efree(base64encode);

	return 1;
}


PHP_METHOD(CCookie,set)
{

	char	*key,
			*val;
	int		keyLen = 0,
			valLen = 0,
			timeout = 3600,
			status;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|l",&key,&keyLen,&val,&valLen,&timeout) == FAILURE){
		RETURN_FALSE;
	}

	//设置Cookie
	status = CCookie_set(key,val,timeout TSRMLS_CC);

	RETVAL_BOOL(status);
}

//获取cookie
void CCookie_get(char *key,char **returnString TSRMLS_DC){

	zval	**array,
			**keVal,
			*paramsZval,
			*aesDecode;

	char	*decodeString;


	if( zend_hash_find(&EG(symbol_table),"_COOKIE",sizeof("_COOKIE"),(void**)&array) != SUCCESS ){
		*returnString = estrdup("");
		return;
	}

	if(IS_ARRAY != Z_TYPE_PP(array)){
		*returnString = estrdup("");
		return;
	}

	//查找key
	if( zend_hash_find(Z_ARRVAL_PP(array),key,strlen(key)+1,(void**)&keVal) != SUCCESS ){
		*returnString = estrdup("");
		return;
	}

	//不为string也失败
	if(IS_STRING != Z_TYPE_PP(keVal)){
		*returnString = estrdup("");
		return;
	}

	//将string解码
	base64Decode(Z_STRVAL_PP(keVal),&decodeString);


	//再将string解码
	MAKE_STD_ZVAL(paramsZval);
	ZVAL_STRING(paramsZval,decodeString,1);
	CEncrypt_AesDecode(paramsZval,aesKey,&aesDecode TSRMLS_CC);

	if(IS_STRING != Z_TYPE_P(aesDecode)){
		*returnString = estrdup("");
		efree(decodeString);
		zval_ptr_dtor(&paramsZval);
		zval_ptr_dtor(&aesDecode);
		return;
	}


	*returnString = estrdup(Z_STRVAL_P(aesDecode));

	//销毁
	efree(decodeString);
	zval_ptr_dtor(&paramsZval);
	zval_ptr_dtor(&aesDecode);
}

PHP_METHOD(CCookie,get)
{
	char	*key,
			*returnString;

	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	CCookie_get(key,&returnString TSRMLS_CC);
	RETVAL_STRING(returnString,1);
	efree(returnString);
}

PHP_METHOD(CCookie,del)
{
	char	*key;

	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETURN_FALSE;
	}

	CCookie_set(key,"",-3600 TSRMLS_CC);
}