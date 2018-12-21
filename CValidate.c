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
#include "php_CValidate.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CValidate_functions[] = {
	PHP_ME(CValidate,isPhone,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isEmail,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isBetween,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isIDCard,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isNumber,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isUrl,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isSimpleString,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isComplexString,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,checkMustField,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CValidate)
{
	//注册CThread类
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CValidate",CValidate_functions);
	CValidateCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

int CValidate_isPhone(char *phone){

	zval *match;
	int	 isTrue = 0;

	if(strlen(phone) != 11){
		return 0;
	}

	if(preg_match("/^1[3|4|5|7|8][0-9]\\d{4,8}$/",phone,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
} 

PHP_METHOD(CValidate,isPhone)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isPhone(data);
	RETVAL_BOOL(isTrue);
}

int CValidate_isEmail(char *str){

	zval *match;
	int	 isTrue = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/([a-z0-9]*[-_\\.]?[a-z0-9]+)*@([a-z0-9]*[-_]?[a-z0-9]+)+[\\.][a-z]{2,3}([\\.][a-z]{2})?/i",str,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isEmail)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isEmail(data);
	RETVAL_BOOL(isTrue);
}

//判断字符串范围
int CValidate_isBetween(char *str,int b,int e){

	if(strlen(str) < b){
		return 0;
	}

	if(strlen(str) > e && e > 0){
		return 0;
	}

	return 1;
}

PHP_METHOD(CValidate,isBetween)
{
	char	*data;
	
	int		dataLen = 0,
			b = 0,
			e = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|ll",&data,&dataLen,&b,&e) == FAILURE){
		RETURN_FALSE;
	}

	
	isTrue = CValidate_isBetween(data,b,e);
	RETVAL_BOOL(isTrue);
}

//判断是否为公民身份证
int CValidate_isIDCard(char *str TSRMLS_DC){

	int		isTrue = 0;

	char	*areaArr = "{\"11\":\"0\",\"12\":\"0\",\"13\":\"0\",\"14\":\"0\",\"15\":\"0\",\"21\":\"0\",\"22\":\"0\",\"23\":\"0\",\"31\":\"0\",\"32\":\"0\",\"33\":\"0\",\"34\":\"0\",\"35\":\"0\",\"36\":\"0\",\"37\":\"0\",\"41\":\"0\",\"42\":\"0\",\"43\":\"0\",\"44\":\"0\",\"45\":\"0\",\"46\":\"0\",\"50\":\"0\",\"51\":\"0\",\"52\":\"0\",\"53\":\"0\",\"54\":\"0\",\"61\":\"0\",\"62\":\"0\",\"63\":\"0\",\"64\":\"0\",\"65\":\"0\",\"71\":\"0\",\"81\":\"0\",\"82\":\"0\",\"91\":\"0\"}",
			*pro;

	zval	*area;

	if(strlen(str) != 18){
		return 0;
	}

	//身份数组
	json_decode(areaArr,&area);

	//提取身份证前2位 判断省份
	MODULE_BEGIN
		int  proLen;
		substr(str,0,2,&pro);
		proLen = toInt(pro);
		if(!zend_hash_index_exists(Z_ARRVAL_P(area),proLen)){
			zval_ptr_dtor(&area);
			efree(pro);
			return 0;
		}
		efree(pro);
	MODULE_END

	//验证出生年
	MODULE_BEGIN
		char	*year,
				*reg;
		int		yearLen = 0;
		zval	*match;

		substr(str,6,4,&year);
		yearLen = toInt(year);

		//判断闰年
		if ( yearLen % 4 == 0 || yearLen % 100 == 0 ){
			reg = "/^[1-9][0-9]{5}19[0-9]{2}((01|03|05|07|08|10|12)(0[1-9]|[1-2][0-9]|3[0-1])|(04|06|09|11)(0[1-9]|[1-2][0-9]|30)|02(0[1-9]|[1-2][0-9]))[0-9]{3}[0-9Xx]$/";
		}else{
			reg = "/^[1-9][0-9]{5}19[0-9]{2}((01|03|05|07|08|10|12)(0[1-9]|[1-2][0-9]|3[0-1])|(04|06|09|11)(0[1-9]|[1-2][0-9]|30)|02(0[1-9]|1[0-9]|2[0-8]))[0-9]{3}[0-9Xx]$/";
		}
		
		//不匹配
		if(!preg_match(reg,str,&match)){
			efree(year);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&area);
			return 0;
		}
		efree(year);
		zval_ptr_dtor(&match);
	MODULE_END

	//校验校验位
	MODULE_BEGIN
	
		zval	*wi,
				*ai,
				**wZval,
				**checkNumZval;

		int		sigma = 0,
				i,
				b,
				w,
				snumber,
				checkNum,
				lastNumInt;

		char	*wiString = "[7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2]",
				*aiString = "[\"1\", \"0\", \"X\", \"9\", \"8\", \"7\", \"6\", \"5\", \"4\", \"3\", \"2\"]",
				bbype[4],
				lastNum[4];

		json_decode(wiString,&wi);
		json_decode(aiString,&ai);

		//统一大写
		php_strtoupper(str,strlen(str) + 1);

		for(i = 0 ; i < 17 ; i++){

			sprintf(bbype,"%c",str[i]);
			b = toInt(bbype);

			zend_hash_index_find(Z_ARRVAL_P(wi),i,(void**)&wZval);
			w = Z_LVAL_PP(wZval);
			sigma = sigma + (w*b);
		}

		//序号
		snumber = sigma % 11;

		zend_hash_index_find(Z_ARRVAL_P(ai),snumber,(void**)&checkNumZval);
		checkNum = toInt(Z_STRVAL_PP(checkNumZval));

		//验证最后一位
		sprintf(lastNum,"%c",str[17]);
		lastNumInt = toInt(lastNum);

		if(checkNum != lastNumInt){
			zval_ptr_dtor(&area);
			zval_ptr_dtor(&wi);
			zval_ptr_dtor(&ai);
			return 0;
		}
		zval_ptr_dtor(&wi);
		zval_ptr_dtor(&ai);
	MODULE_END

	//需销毁的值
	zval_ptr_dtor(&area);
	return 1;
}


PHP_METHOD(CValidate,isIDCard)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isIDCard(data TSRMLS_CC);
	RETVAL_BOOL(isTrue);
}

PHP_METHOD(CValidate,isNumber)
{
	long data;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&data) == FAILURE){
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

int CValidate_isUrl(char *str){

	zval *match;
	int	 isTrue = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/^(http|https|ftp|ftps)\\:\\/\\/(.*)/",str,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isUrl)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isUrl(data);
	RETVAL_BOOL(isTrue);
}

int CValidate_isSimpleString(char *str){

	zval *match;
	int	 isTrue = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/^[A-Za-z]{1}([A-Za-z0-9]|[_])+$/",str,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isSimpleString)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isSimpleString(data);
	RETVAL_BOOL(isTrue);
}

int CValidate_isComplexString(char *str){

	zval *match;
	int	 isTrue = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/(?=.*[\\d]+)(?=.*[a-zA-Z]+)(?=.*[^a-zA-Z0-9]+)/",str,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isComplexString)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isComplexString(data);
	RETVAL_BOOL(isTrue);
}

PHP_METHOD(CValidate,checkMustField)
{
	zval	*params,
			*checked,
			**rowZval,
			**checkedRows;

	int		i,j,k,n;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"aa",&params,&checked) == FAILURE){
		RETURN_FALSE;
	}

	j = zend_hash_num_elements(Z_ARRVAL_P(params));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(params));
	for(i = 0 ; i < j ; i ++){
		zend_hash_get_current_data(Z_ARRVAL_P(params),(void**)&rowZval);
		if(IS_ARRAY != Z_TYPE_PP(rowZval)){
			RETURN_FALSE;
		}

		//只要一次不存在则直接返回失败
		n = zend_hash_num_elements(Z_ARRVAL_P(checked));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(checked));
		for(k = 0 ; k < n ; k++){
			zend_hash_get_current_data(Z_ARRVAL_P(checked),(void**)&checkedRows);

			if(IS_STRING == Z_TYPE_PP(checkedRows)){
				if(!zend_hash_exists(Z_ARRVAL_PP(rowZval),Z_STRVAL_PP(checkedRows),strlen(Z_STRVAL_PP(checkedRows))+1)){
					RETURN_FALSE;
				}
			}else if(IS_LONG == Z_TYPE_PP(checkedRows)){
				if(!zend_hash_index_exists(Z_ARRVAL_PP(rowZval),Z_LVAL_PP(checkedRows))){
					RETURN_FALSE;
				}
			}

			zend_hash_move_forward(Z_ARRVAL_P(checked));
		}


		zend_hash_move_forward(Z_ARRVAL_P(params));
	}
	RETURN_TRUE;
}
