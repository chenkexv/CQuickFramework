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
#include "php_CConfig.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CValidate_functions[] = {
	PHP_ME(CValidate,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,check,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(CValidate,__construct,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CValidate,set,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CValidate,getLastError,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CValidate,getLastErrorCode,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CValidate,isPhone,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isEmail,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isBetween,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isIDCard,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isNumber,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isUrl,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isSimpleString,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isComplexString,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isIp,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isJson,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CValidate,isDate,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
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

	zend_declare_property_null(CValidateCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CValidateCe, ZEND_STRL("name"),"default",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CValidateCe, ZEND_STRL("lastError"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CValidateCe, ZEND_STRL("errorCode"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CValidateCe, ZEND_STRL("rules"),ZEND_ACC_PRIVATE TSRMLS_CC);

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

	if(preg_match("/^[a-z0-9]+([._-][a-z0-9]+)*@([0-9a-z]+\\.[a-z]{2,14}(\\.[a-z]{2})?)$/i",str,&match)){
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

void CValidate_getInstance(char *groupName,zval **returnZval TSRMLS_DC){

	zval	*instanceZval,
			**instaceSaveZval;

	int hasExistConfig;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CValidateCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CValidateCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CValidateCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(instanceZval), groupName, strlen(groupName)+1);

	//为空时则实例化自身
	if(0 == hasExistConfig ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CValidateCe);

		//执行构造器
		if (CValidateCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 params1;
			zval *paramsList[1];
			paramsList[0] = &params1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],groupName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CValidateCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
		}


		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);

		zend_hash_add(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CValidateCe, ZEND_STRL("instance"),instanceZval TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	//直接取instace静态变量中的返回值
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,(void**)&instaceSaveZval) ){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,*instaceSaveZval,1,0);
		return;
	}
}

PHP_METHOD(CValidate,getInstance)
{
	char	*groupName,
			*dgroupName;
	int		groupNameLen = 0;
	zval	*object;

	//判断是否指定序列
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&dgroupName,&groupNameLen) == FAILURE){
		return;
	}

	if(groupNameLen == 0){
		groupName = estrdup("default");
	}else{
		groupName = estrdup(dgroupName);
	}
	
	CValidate_getInstance(groupName,&object TSRMLS_CC);
	ZVAL_ZVAL(return_value,object,1,1);
	efree(groupName);
}

int CValidate_isIp(char *str){

	zval *match;
	int	 isTrue = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$/",str,&match)){
		isTrue = 1;
	}

	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isIp)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isIp(data);
	RETVAL_BOOL(isTrue);
}

int CValidate_isDate(char *str){

	zval	*match;
	int		isTrue = 0,
			timestamp = 0;

	if(strlen(str) == 0){
		return 0;
	}


	if(preg_match("/^\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}$/s",str,&match)){
		isTrue = 1;
	}
	zval_ptr_dtor(&match);

	if(preg_match("/^\\d{4}-\\d{2}-\\d{2}$/s",str,&match)){
		isTrue = 1;
	}
	zval_ptr_dtor(&match);

	//check to time
	if(isTrue == 1){
		timestamp = php_strtotime(str);
		if(timestamp == 0){
			isTrue = 0;
		}
	}

	return isTrue;
}

int CValidate_isFloat(char *str){

	zval	*match;
	int		isTrue = 0,
			timestamp = 0;

	if(strlen(str) == 0){
		return 0;
	}

	if(preg_match("/^(-?\\d+)(\\.\\d+)?$/",str,&match)){
		isTrue = 1;
	}
	zval_ptr_dtor(&match);

	return isTrue;
}

PHP_METHOD(CValidate,isDate)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isDate(data);
	RETVAL_BOOL(isTrue);
}

int CValidate_isJson(char *str TSRMLS_DC){

	zval	*jsonDecode;
	int		isTrue = 0;

	json_decode(str,&jsonDecode);
	if(zend_hash_num_elements(Z_ARRVAL_P(jsonDecode)) > 0){
		isTrue = 1;
	}
	zval_ptr_dtor(&jsonDecode);
	return isTrue;
}

PHP_METHOD(CValidate,isJson)
{
	char	*data;
	int		dataLen = 0,
			isTrue = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&data,&dataLen) == FAILURE){
		RETURN_FALSE;
	}

	isTrue = CValidate_isJson(data TSRMLS_CC);
	RETVAL_BOOL(isTrue);
}


PHP_METHOD(CValidate,check)
{
	zval	*data,
			**thisVal,
			*rules,
			**thisRules,
			**rulesVal,
			**isMust;

	int		i,h;

	char	*key,
			*ruleKey;

	ulong	ukey,
			ruleUkey;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),"call check params error" TSRMLS_CC);
		RETURN_FALSE;
	}
	
	if(IS_ARRAY != Z_TYPE_P(data)){
		zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),"call check params error" TSRMLS_CC);
		RETURN_FALSE;
	}

	rules = zend_read_property(CValidateCe,getThis(),ZEND_STRL("rules"),1 TSRMLS_CC);

	//no rules
	if(IS_ARRAY != Z_TYPE_P(rules)){
		RETURN_TRUE;
	}

	//check Must key
	h = zend_hash_num_elements(Z_ARRVAL_P(rules));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(rules),(void**)&rulesVal);
		zend_hash_get_current_key(Z_ARRVAL_P(rules),&ruleKey,&ruleUkey,0);

		if(IS_ARRAY == Z_TYPE_PP(rulesVal) && SUCCESS == zend_hash_find(Z_ARRVAL_PP(rulesVal),"isMust",strlen("isMust")+1,(void**)&isMust) && IS_LONG == Z_TYPE_PP(isMust) && 1 == Z_LVAL_PP(isMust) ){
			//check has this key
			if(!zend_hash_exists(Z_ARRVAL_P(data),ruleKey,strlen(ruleKey)+1)){
				zval	**tips;
				zend_hash_find(Z_ARRVAL_PP(rulesVal),"tips",strlen("tips")+1,(void**)&tips);
				zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : ruleKey ) TSRMLS_CC);
				zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),5 TSRMLS_CC);
				RETURN_FALSE;
			}
		}
		zend_hash_move_forward(Z_ARRVAL_P(rules));
	}


	h = zend_hash_num_elements(Z_ARRVAL_P(data));
	for(i = 0 ; i < h ; i++){

		if(HASH_KEY_IS_STRING != zend_hash_get_current_key_type(Z_ARRVAL_P(data))){
			zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),"call check params error,the params'key must be a string" TSRMLS_CC);
			RETURN_FALSE;
		}

		//now key;
		zend_hash_get_current_key(Z_ARRVAL_P(data),&key,&ukey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);

		if(IS_LONG == Z_TYPE_PP(thisVal) || IS_DOUBLE == Z_TYPE_PP(thisVal)){
			convert_to_string(*thisVal);
		}

		if(IS_STRING != Z_TYPE_PP(thisVal)){
			zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),"call check params error,the params'value must be a string" TSRMLS_CC);
			RETURN_FALSE;
		}

		//find rules
		if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),key,strlen(key)+1,(void**)&thisRules) && IS_ARRAY == Z_TYPE_PP(thisRules) ){

			zval	**type,
					**min,
					**max,
					**minVal,
					**maxVal,
					**tips,
					**match;

			//min max
			zend_hash_find(Z_ARRVAL_PP(thisRules),"min",strlen("min")+1,(void**)&min);
			zend_hash_find(Z_ARRVAL_PP(thisRules),"max",strlen("max")+1,(void**)&max);
			zend_hash_find(Z_ARRVAL_PP(thisRules),"minVal",strlen("minVal")+1,(void**)&minVal);
			zend_hash_find(Z_ARRVAL_PP(thisRules),"maxVal",strlen("maxVal")+1,(void**)&maxVal);
			zend_hash_find(Z_ARRVAL_PP(thisRules),"tips",strlen("tips")+1,(void**)&tips);
			zend_hash_find(Z_ARRVAL_PP(thisRules),"match",strlen("match")+1,(void**)&match);

			//this rules type
			zend_hash_find(Z_ARRVAL_PP(thisRules),"type",strlen("type")+1,(void**)&type);

			//if set prel match , only match this
			if(Z_STRLEN_PP(match) > 0){
				zval	*matched;
				if(!preg_match(Z_STRVAL_PP(match),Z_STRVAL_PP(thisVal),&matched)){
					zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
					zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),4 TSRMLS_CC);
					zval_ptr_dtor(&matched);
					RETURN_FALSE;
				}
				zval_ptr_dtor(&matched);
			}else{

				//check username
				if(strcmp(Z_STRVAL_PP(type),"username") == 0){
					if(!CValidate_isSimpleString(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
					if((Z_LVAL_PP(min) != 0 || Z_LVAL_PP(max) != 0) && !CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),2 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//check password
				if(strcmp(Z_STRVAL_PP(type),"password") == 0){
					if(!CValidate_isComplexString(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
					if((Z_LVAL_PP(min) != 0 || Z_LVAL_PP(max) != 0) && !CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),2 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//phone
				if(strcmp(Z_STRVAL_PP(type),"phone") == 0){
					if(!CValidate_isPhone(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//mail
				if(strcmp(Z_STRVAL_PP(type),"mail") == 0){
					if(!CValidate_isEmail(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//simpleString
				if(strcmp(Z_STRVAL_PP(type),"simpleString") == 0){
					if(!CValidate_isSimpleString(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}
				
				//idcard
				if(strcmp(Z_STRVAL_PP(type),"idcard") == 0){
					if(!CValidate_isIDCard(Z_STRVAL_PP(thisVal) TSRMLS_CC)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//url
				if(strcmp(Z_STRVAL_PP(type),"url") == 0){
					if(!CValidate_isUrl(Z_STRVAL_PP(thisVal) TSRMLS_CC)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//json
				if(strcmp(Z_STRVAL_PP(type),"json") == 0){
					if(!CValidate_isJson(Z_STRVAL_PP(thisVal) TSRMLS_CC)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//date
				if(strcmp(Z_STRVAL_PP(type),"date") == 0){
					if(!CValidate_isDate(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//float
				if(strcmp(Z_STRVAL_PP(type),"float") == 0){
					if(!CValidate_isFloat(Z_STRVAL_PP(thisVal))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}

					//check length
					if((Z_LVAL_PP(min) != 0 || Z_LVAL_PP(max) != 0) && !CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),2 TSRMLS_CC);
						RETURN_FALSE;
					}
					
					//check minVal
					convert_to_double(*thisVal);
					if(Z_DVAL_PP(minVal) != 0 && Z_DVAL_PP(thisVal) <  Z_DVAL_PP(minVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type)) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
					if(Z_DVAL_PP(maxVal) != 0 && Z_DVAL_PP(thisVal) >  Z_DVAL_PP(maxVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//string
				if(strcmp(Z_STRVAL_PP(type),"string") == 0){
					if(!CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//number
				if(strcmp(Z_STRVAL_PP(type),"number") == 0){
					if(!isdigitstr(Z_STRVAL_PP(thisVal)) ){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}

					if((Z_LVAL_PP(min) != 0 || Z_LVAL_PP(max) != 0) && !CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),2 TSRMLS_CC);
						RETURN_FALSE;
					}

					//check minVal
					convert_to_double(*thisVal);
					if(Z_DVAL_PP(minVal) != 0 && Z_DVAL_PP(thisVal) <  Z_DVAL_PP(minVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
					if(Z_DVAL_PP(maxVal) != 0 && Z_DVAL_PP(thisVal) >  Z_DVAL_PP(maxVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
				}

				//timestamp
				if(strcmp(Z_STRVAL_PP(type),"timestamp") == 0){
					if(!isdigitstr(Z_STRVAL_PP(thisVal)) ){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
						RETURN_FALSE;
					}
					if((Z_LVAL_PP(min) != 0 || Z_LVAL_PP(max) != 0) && !CValidate_isBetween(Z_STRVAL_PP(thisVal),Z_LVAL_PP(min),Z_LVAL_PP(max))){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),2 TSRMLS_CC);
						RETURN_FALSE;
					}

					//check minVal
					convert_to_double(*thisVal);
					if(Z_DVAL_PP(minVal) != 0 && Z_DVAL_PP(thisVal) <  Z_DVAL_PP(minVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
					if(Z_DVAL_PP(maxVal) != 0 && Z_DVAL_PP(thisVal) >  Z_DVAL_PP(maxVal)){
						zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),(Z_STRVAL_PP(tips) > 0 ? Z_STRVAL_PP(tips) : Z_STRVAL_PP(type) ) TSRMLS_CC);
						zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),3 TSRMLS_CC);
						RETURN_FALSE;
					}
				}
			}

		}


		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	zend_update_property_string(CValidateCe,getThis(),ZEND_STRL("lastError"),"" TSRMLS_CC);
	zend_update_property_long(CValidateCe,getThis(),ZEND_STRL("errorCode"),0 TSRMLS_CC);
	RETURN_TRUE;
}

void CValidate_setRules(zval *object,char *name,char *type,char *match,int min,int max,double minVal,double maxVal,char *tips,int isMust TSRMLS_DC){
	
	zval	*rules,
			*saveData;

	rules = zend_read_property(CValidateCe,object,ZEND_STRL("rules"),1 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(rules)){
		zval	*saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CValidateCe,object,ZEND_STRL("rules"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		rules = zend_read_property(CValidateCe,object,ZEND_STRL("rules"),0 TSRMLS_CC);
	}

	//saveData
	MAKE_STD_ZVAL(saveData);
	array_init(saveData);
	add_assoc_string(saveData,"name",name,1);
	add_assoc_string(saveData,"type",type,1);
	add_assoc_string(saveData,"match",match,1);
	add_assoc_string(saveData,"tips",tips,1);
	add_assoc_long(saveData,"min",min);
	add_assoc_long(saveData,"max",max);
	add_assoc_double(saveData,"minVal",minVal);
	add_assoc_double(saveData,"maxVal",maxVal);
	add_assoc_long(saveData,"isMust",isMust);

	//save object
	add_assoc_zval(rules,name,saveData);
}

void CValidate_setDefaultRules(zval *object TSRMLS_DC){

	char	*usernameError,
			*passwordError,
			*phoneError,
			*mailError,
			*idcardError,
			*urlError,
			*stringError,
			*numberError,
			*ipError;
	
	CValidate_setRules(object,"username","username","",4,20,0.0,0.0,"username",0 TSRMLS_CC);
	CValidate_setRules(object,"password","password","",6,24,0.0,0.0,"password",0 TSRMLS_CC);
	CValidate_setRules(object,"phone","phone","",11,11,0.0,0.0,"phone",0 TSRMLS_CC);
	CValidate_setRules(object,"mail","mail","",4,60,0.0,0.0,"mail",0 TSRMLS_CC);
	CValidate_setRules(object,"idcard","idcard","",18,18,0.0,0.0,"idcard",0 TSRMLS_CC);
	CValidate_setRules(object,"url","url","",4,20,0.0,0.0,"url",0 TSRMLS_CC);
	CValidate_setRules(object,"simpleString","simpleString","",4,20,0.0,0.0,"simpleString",0 TSRMLS_CC);
	CValidate_setRules(object,"number","number","",0,0,0.0,0.0,"number",0 TSRMLS_CC);
	CValidate_setRules(object,"ip","ip","",7,15,0.0,0.0,"ip",0 TSRMLS_CC);
}

PHP_METHOD(CValidate,__construct)
{
	char	*groupName;

	int		groupNameLen = 0;

	zval	*cconfigInstanceZval,
			*thisConfigFile,
			*supportTyps;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&groupName,&groupNameLen) == FAILURE){
		return;
	}

	json_decode("[\"phone\",\"mail\",\"ip\",\"username\",\"password\",\"date\",\"timestamp\",\"idcard\",\"url\",\"string\",\"number\",\"float\",\"simpleString\",\"json\"]",&supportTyps);


	//set default rules
	if(strcmp(groupName,"default") == 0){
		CValidate_setDefaultRules(getThis() TSRMLS_CC);
	}

	//check has validate config file
	CConfig_getInstance("validate",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load(groupName,cconfigInstanceZval,&thisConfigFile TSRMLS_CC);
	
	//reset
	if(IS_ARRAY == Z_TYPE_P(thisConfigFile)){

		int		i,h,
				minInt = 0,
				maxInt = 0,
				isMustInt = 0;

		zval	**thisVal,
				**name,
				**type,
				**match,
				**min,
				**max,
				**tips,
				**minVal,
				**maxVal,
				*saveMinVal,
				*saveMaxVal,
				**isMust;

		char	*key,
				*matchString,
				*tipsString;
		ulong	ukey;

		h = zend_hash_num_elements(Z_ARRVAL_P(thisConfigFile));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_data(Z_ARRVAL_P(thisConfigFile),(void**)&thisVal);

			if(HASH_KEY_IS_STRING != zend_hash_get_current_key_type(Z_ARRVAL_P(thisConfigFile))){
				char	errorMessage[1024];
				sprintf(errorMessage,"%s%s%s","[CValidateException] Error reading configuration file ,the [",groupName,"] child keys must be string");
				zend_throw_exception(CExceptionCe, errorMessage, 12011 TSRMLS_CC);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&thisConfigFile);
				zval_ptr_dtor(&supportTyps);
				return;
			}

			zend_hash_get_current_key(Z_ARRVAL_P(thisConfigFile),&key,&ukey,0);

			//is not an array
			if(IS_ARRAY != Z_TYPE_PP(thisVal)){
				char	errorMessage[1024];
				sprintf(errorMessage,"%s%s%s%s%s","[CValidateException] Error reading configuration file [",groupName,"->",key,"], key value not an array");
				zend_throw_exception(CExceptionCe, errorMessage, 12011 TSRMLS_CC);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&thisConfigFile);
				zval_ptr_dtor(&supportTyps);
				return;
			}

			//get type
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"type",strlen("type")+1,(void**)&type)  && IS_STRING == Z_TYPE_PP(type) ){
			}else{
				char	errorMessage[1024];
				sprintf(errorMessage,"%s%s%s%s%s","[CValidateException] Error reading configuration file [",groupName,"->",key,"], this rule [type] must be a string ");
				zend_throw_exception(CExceptionCe, errorMessage, 12011 TSRMLS_CC);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&thisConfigFile);
				zval_ptr_dtor(&supportTyps);
				return;
			}

			//check type is support
			if(!in_array(Z_STRVAL_PP(type),supportTyps)){
				char	errorMessage[1024];
				sprintf(errorMessage,"%s%s%s%s%s","[CValidateException] Error reading configuration file [",groupName,"->",key,"], this rule [type] not support,is must in ([\"phone\",\"mail\",\"ip\",\"username\",\"password\",\"date\",\"timestamp\",\"idcard\",\"url\",\"string\",\"number\",\"float\",\"simpleString\",\"json\"]) ");
				zend_throw_exception(CExceptionCe, errorMessage, 12011 TSRMLS_CC);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&thisConfigFile);
				zval_ptr_dtor(&supportTyps);
				return;
			}

			//match
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"match",strlen("match")+1,(void**)&match)  && IS_STRING == Z_TYPE_PP(match) ){
				matchString = estrdup(Z_STRVAL_PP(match));
			}else{
				matchString = estrdup("");
			}

			//min
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"minLength",strlen("minLength")+1,(void**)&min)  && IS_LONG == Z_TYPE_PP(min) ){
				minInt = Z_LVAL_PP(min);
			}else{
				minInt = 0;
			}

			//max
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"maxLength",strlen("maxLength")+1,(void**)&max)  && IS_LONG == Z_TYPE_PP(max) ){
				maxInt = Z_LVAL_PP(max);
			}else{
				maxInt = 0;
			}		   

			//minVal
			MAKE_STD_ZVAL(saveMinVal);
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"minVal",strlen("minVal")+1,(void**)&minVal)  && (IS_LONG == Z_TYPE_PP(minVal) || IS_DOUBLE == Z_TYPE_PP(minVal)) ){
				ZVAL_ZVAL(saveMinVal,*minVal,1,0);
				convert_to_double(saveMinVal);
			}else{
				ZVAL_DOUBLE(saveMinVal,0.0);
			}
	
			//maxVal
			MAKE_STD_ZVAL(saveMaxVal);
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"maxVal",strlen("minVal")+1,(void**)&maxVal)  && (IS_LONG == Z_TYPE_PP(maxVal) || IS_DOUBLE == Z_TYPE_PP(maxVal)) ){
				ZVAL_ZVAL(saveMaxVal,*maxVal,1,0);
				convert_to_double(saveMaxVal);
			}else{
				ZVAL_DOUBLE(saveMaxVal,0.0);
			}

			//tips
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"tips",strlen("tips")+1,(void**)&tips)  && IS_STRING == Z_TYPE_PP(tips) ){
				tipsString = estrdup(Z_STRVAL_PP(tips));
			}else{
				tipsString = estrdup("");
			}

			//is Must
			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisVal),"must",strlen("must")+1,(void**)&isMust)  && IS_BOOL == Z_TYPE_PP(isMust) ){
				isMustInt = Z_LVAL_PP(isMust);
			}

			//set rules
			CValidate_setRules(getThis(),key,Z_STRVAL_PP(type),matchString,minInt,maxInt,Z_DVAL_P(saveMinVal),Z_DVAL_P(saveMaxVal),tipsString,isMustInt TSRMLS_CC);

			efree(tipsString);
			efree(matchString);
			zval_ptr_dtor(&saveMinVal);
			zval_ptr_dtor(&saveMaxVal);

			zend_hash_move_forward(Z_ARRVAL_P(thisConfigFile));
		}
	}

	//destroy
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&thisConfigFile);
	zval_ptr_dtor(&supportTyps);
}

PHP_METHOD(CValidate,set)
{

	char		*name,
				*type,
				*match,
				*tips;

	zval		*rules,
				**typeZval,
				**matchZval,
				**minLengthZval,
				**maxLengthZval,
				**tipsZval,
				*saveMinVal,
				*saveMaxVal,
				**minValZval,
				**maxValZval,
				**mustZval;
	
	int			nameLen = 0,
				typeLen = 0,
				matchLen = 0,
				tipsLen = 0,
				min = 0,
				max = 0,
				isMust = 0;

	double		minVal = 0,
				maxVal = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&name,&nameLen,&rules) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CValidateException] call set function ,must give 2 params,the type is string,array", 12011 TSRMLS_CC);
		RETURN_FALSE;
	}

	if(IS_ARRAY != Z_TYPE_P(rules)){
		zend_throw_exception(CExceptionCe, "[CValidateException] call set function ,must give 2 params,the type is string,array", 12011 TSRMLS_CC);
		RETURN_FALSE;
	}

	//type
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"type",strlen("type")+1,(void**)&typeZval) && IS_STRING == Z_TYPE_PP(typeZval)){
	}else{
		zend_throw_exception(CExceptionCe, "[CValidateException] call set function ,the params [type] must be a string", 12011 TSRMLS_CC);
		RETURN_FALSE;
	}

	//match
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"match",strlen("match")+1,(void**)&matchZval) && IS_STRING == Z_TYPE_PP(matchZval)){
		match = estrdup(Z_STRVAL_PP(matchZval));
	}else{
		match = estrdup("");
	}

	//tips
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"tips",strlen("tips")+1,(void**)&tipsZval) && IS_STRING == Z_TYPE_PP(tipsZval)){
		tips = estrdup(Z_STRVAL_PP(tipsZval));
	}else{
		tips = estrdup("");
	}

	//minLength
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"minLength",strlen("minLength")+1,(void**)&minLengthZval) && (IS_LONG == Z_TYPE_PP(minLengthZval) || IS_DOUBLE == Z_TYPE_PP(minLengthZval)) ){
		min = Z_LVAL_PP(minLengthZval);
	}

	//maxLength
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"maxLength",strlen("maxLength")+1,(void**)&maxLengthZval) && (IS_LONG == Z_TYPE_PP(maxLengthZval) || IS_DOUBLE == Z_TYPE_PP(maxLengthZval)) ){
		min = Z_LVAL_PP(maxLengthZval);
	}

	//minVal
	MAKE_STD_ZVAL(saveMinVal);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"minVal",strlen("minVal")+1,(void**)&minValZval) && (IS_LONG == Z_TYPE_PP(minValZval) || IS_DOUBLE == Z_TYPE_PP(minValZval)) ){	
		ZVAL_ZVAL(saveMinVal,*minValZval,1,0);
		convert_to_double(saveMinVal);
	}else{
		ZVAL_DOUBLE(saveMinVal,minVal);
	}

	//maxVal
	MAKE_STD_ZVAL(saveMaxVal);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"maxVal",strlen("maxVal")+1,(void**)&maxValZval) && (IS_LONG == Z_TYPE_PP(maxValZval) || IS_DOUBLE == Z_TYPE_PP(maxValZval)) ){
		ZVAL_ZVAL(saveMaxVal,*maxValZval,1,0);
		convert_to_double(saveMaxVal);
	}else{
		ZVAL_DOUBLE(saveMaxVal,maxVal);
	}

	//isMust
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(rules),"must",strlen("must")+1,(void**)&mustZval) && IS_BOOL == Z_TYPE_PP(mustZval)){
		isMust = Z_LVAL_PP(mustZval);
	}


	CValidate_setRules(getThis(),name,Z_STRVAL_PP(typeZval),match,min,max,Z_DVAL_P(saveMinVal),Z_DVAL_P(saveMaxVal),tips,isMust TSRMLS_CC);

	efree(match);
	efree(tips);
	zval_ptr_dtor(&saveMaxVal);
	zval_ptr_dtor(&saveMinVal);
	RETVAL_TRUE;	
}

PHP_METHOD(CValidate,getLastError)
{
	zval	*lastError;
	lastError = zend_read_property(CValidateCe,getThis(),ZEND_STRL("lastError"),1 TSRMLS_CC);
	ZVAL_ZVAL(return_value,lastError,1,0);
}

PHP_METHOD(CValidate,getLastErrorCode)
{
	zval	*lastError;
	lastError = zend_read_property(CValidateCe,getThis(),ZEND_STRL("errorCode"),1 TSRMLS_CC);
	if(1 == Z_LVAL_P(lastError)){
		RETURN_STRING("type",1);
	}else if(2 == Z_LVAL_P(lastError)){
		RETURN_STRING("length",1);
	}else if(3 == Z_LVAL_P(lastError)){
		RETURN_STRING("val",1);
	}else if(4 == Z_LVAL_P(lastError)){
		RETURN_STRING("match",1);
	}else if(5 == Z_LVAL_P(lastError)){
		RETURN_STRING("must",1);
	}else{
		RETURN_STRING("",1);
	}
}