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
#include "php_CString.h"
#include "php_CWord.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CString_functions[] = {
	PHP_ME(CString,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CString,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CString,setData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,getFirstChar,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,getCharst,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,toAscii,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,toChar,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,at,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,substr,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,length,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,toUTF8,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,toGBK,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,toArray,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isUTF8,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isGBK,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isSimpleString,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isComplexString,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isIp,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isUrl,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isEmail,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isDate,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isJson,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isPhone,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isIDCard,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CString,isNumber,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CString)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CString",CString_functions);

	CStringCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CStringCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CStringCe, ZEND_STRL("data"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CStringCe, ZEND_STRL("charst"),"",ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CString,__construct){

	char	*string;
	int		stringLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	//check module 
	if (!zend_hash_exists(&module_registry, "mbstring", strlen("mbstring")+1)) {
		zend_throw_exception(CExceptionCe, "[CStringException] Call [CString->__construct] need install the [mbstring] extension ", 10001 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CStringCe,getThis(),ZEND_STRL("data"),string TSRMLS_CC);
}

PHP_METHOD(CString,getFirstChar){

	char	*endString,
			*string;

	zval	*stringZval;

	int		stringLen = 0,
			returnAll = 0;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|b",&returnAll) == FAILURE){
		return;
	}

	stringZval = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	string = Z_STRVAL_P(stringZval);
	if(strlen(string) <= 0){
		RETURN_STRING("",1);
	}

	if(returnAll == 0){
		CWord_getStringFirstCharOne(string,&endString TSRMLS_CC);
		RETVAL_STRING(endString,0);
	}else{

		char	*gbString,
				*tempStr1,
				*tempStr2,
				*endString;

		smart_str fullString = {0};

		int		i,h,
				thisAscii;

		php_iconv("UTF-8", "GB18030",string,&gbString);
		h = strlen(gbString);
		for(i = 0 ; i < h ;i++){
			substr(gbString,i,1,&tempStr1);
			
			thisAscii = (unsigned char)tempStr1[0];
			if(thisAscii > 160){
				substr(gbString,i++,2,&tempStr2);
				CWord_getStringFirstCharOne(tempStr2,&endString TSRMLS_CC);
				smart_str_appends(&fullString,endString);
				efree(endString);
				efree(tempStr2);
			}else{
				smart_str_appends(&fullString,tempStr1);
			}
			efree(tempStr1);
		}
		smart_str_0(&fullString);
		RETVAL_STRING(fullString.c,1);

		smart_str_free(&fullString);
		efree(gbString);
	}
}

void CString_getCharst(zval *object,char **charst TSRMLS_DC){
	zval	*nowData,
			callFunction,
			callReturn,
			*params[2],
			*saveReturn,
			*nowCharst;

	nowData = zend_read_property(CStringCe,object,ZEND_STRL("data"), 0 TSRMLS_CC);
	INIT_ZVAL(callFunction);
	ZVAL_STRING(&callFunction,"mb_detect_encoding",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],nowData,1,0);
	MAKE_STD_ZVAL(params[1]);
	array_init(params[1]);
	add_next_index_string(params[1],"UTF-8",1);
	add_next_index_string(params[1],"GB2312",1);
	call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	saveReturn = &callReturn;
	if(IS_STRING == Z_TYPE_P(saveReturn)){
		*charst = estrdup(Z_STRVAL_P(saveReturn));
	}else{
		*charst = estrdup("");
	}
	zend_update_property_string(CStringCe,object,ZEND_STRL("charst"),*charst TSRMLS_CC);
	zval_dtor(&callReturn);
}

PHP_METHOD(CString,getCharst)
{
	char *charst;
	CString_getCharst(getThis(),&charst TSRMLS_CC);
	RETVAL_STRING(charst,0);
}

PHP_METHOD(CString,toAscii)
{
	zval	*nowData;
	int		i,h;
	zval	*endZval;

	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	
	h = strlen(Z_STRVAL_P(nowData));
	MAKE_STD_ZVAL(endZval);
	array_init(endZval);
	for(i = 0 ; i < h ; i++){
		add_next_index_long(endZval,Z_STRVAL_P(nowData)[i]);
	}
	RETVAL_ZVAL(endZval,1,1);
}

PHP_METHOD(CString,toChar)
{
	zval	*nowData;
	int		i,h;
	zval	*endZval;
	char	thisChar[6];

	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	
	h = strlen(Z_STRVAL_P(nowData));
	MAKE_STD_ZVAL(endZval);
	array_init(endZval);
	for(i = 0 ; i < h ; i++){
		sprintf(thisChar,"%c",Z_STRVAL_P(nowData)[i]);
		add_next_index_string(endZval,thisChar,1);
	}
	RETVAL_ZVAL(endZval,1,1);
}

PHP_METHOD(CString,at){

	int		index = 0;
	char	*endString;
	zval	*nowData;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&index) == FAILURE){
		return;
	}
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	mb_substr(Z_STRVAL_P(nowData),((index-1 < 0) ? 0 : index - 1),1,"utf-8",&endString);
	RETVAL_STRING(endString,0);
}

PHP_METHOD(CString,substr)
{
	long	offset = 0,
			len = 1;
	char	*endString;
	zval	*nowData;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ll",&offset,&len) == FAILURE){
		return;
	}

	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	mb_substr(Z_STRVAL_P(nowData),offset,len,"utf-8",&endString);
	RETVAL_STRING(endString,0);
}

PHP_METHOD(CString,length)
{
	zval	*nowData;
	int		len = 0;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	len = mb_strlen(Z_STRVAL_P(nowData),"utf-8");
	RETVAL_LONG(len);
}

PHP_METHOD(CString,toUTF8)
{
	char	*charst;
	zval	*nowData;
	CString_getCharst(getThis(),&charst TSRMLS_CC);
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	if(strcmp(charst,"UTF-8") == 0){
		RETVAL_STRING(Z_STRVAL_P(nowData),1);
	}else{
		zval	callFunction,
				callReturn,
				*params[2];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"mb_convert_encoding",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],nowData,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],"UTF-8",1);
		call_user_function(EG(function_table), NULL, &callFunction, &callReturn, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		if(IS_STRING == Z_TYPE(callReturn)){
			RETVAL_STRING(Z_STRVAL(callReturn),1);
		}else{
			RETVAL_STRING("",1);
		}
		zval_dtor(&callReturn);
	}

	efree(charst);
}

PHP_METHOD(CString,toGBK){
	zval	*nowData;
	char	*gbString;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	php_iconv("UTF-8", "GB18030",Z_STRVAL_P(nowData),&gbString);
	RETVAL_STRING(gbString,0);
}

PHP_METHOD(CString,toArray){
	zval	*nowData,
			*array;
	int		i,h;
	char	*thisChar;
	MAKE_STD_ZVAL(array);
	array_init(array);
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	h = mb_strlen(Z_STRVAL_P(nowData),"UTF-8");
	for(i = 0 ; i < h ; i++){
		mb_substr(Z_STRVAL_P(nowData),i,1,"UTF-8",&thisChar);
		add_next_index_string(array,thisChar,0);
	}
	RETVAL_ZVAL(array,1,1);
}

PHP_METHOD(CString,isUTF8){
	char	*charst;
	zval	*nowData;
	CString_getCharst(getThis(),&charst TSRMLS_CC);
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	if(strcmp(charst,"UTF-8") == 0){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
	efree(charst);
}

PHP_METHOD(CString,isGBK){
	char	*charst;
	zval	*nowData;
	CString_getCharst(getThis(),&charst TSRMLS_CC);
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);
	if(strcmp(charst,"EUC-CN") == 0){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
	efree(charst);
}

PHP_METHOD(CString,isSimpleString){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isSimpleString(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

PHP_METHOD(CString,isComplexString)
{
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isComplexString(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isIp){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isIp(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

PHP_METHOD(CString,isUrl){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isUrl(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isEmail){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isEmail(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isDate){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isDate(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isJson){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isJson(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isPhone){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isPhone(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isIDCard){
	zval	*nowData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	if(CValidate_isIDCard(Z_STRVAL_P(nowData))){
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
PHP_METHOD(CString,isNumber){
	zval	*nowData,
			*newData;
	nowData = zend_read_property(CStringCe,getThis(),ZEND_STRL("data"), 0 TSRMLS_CC);

	MAKE_STD_ZVAL(newData);
	ZVAL_ZVAL(newData,nowData,1,0);

	convert_to_long(newData);
	convert_to_string(newData);

	if(strcmp(Z_STRVAL_P(newData),Z_STRVAL_P(nowData)) == 0){
		zval_ptr_dtor(&newData);
		RETURN_TRUE;
	}

	zval_ptr_dtor(&newData);
	RETURN_FALSE;
}
PHP_METHOD(CString,getInstance){}


PHP_METHOD(CString,setData){

	char	*string;
	int		stringLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	//check module 
	if (!zend_hash_exists(&module_registry, "mbstring", strlen("mbstring")+1)) {
		zend_throw_exception(CExceptionCe, "[CStringException] Call [CString->__construct] need install the [mbstring] extension ", 10001 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CStringCe,getThis(),ZEND_STRL("data"),string TSRMLS_CC);
}