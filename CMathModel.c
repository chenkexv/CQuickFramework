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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


#include "php_CQuickFramework.h"
#include "php_CMathModel.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include <math.h>



//zend类方法
zend_function_entry CMathModel_functions[] = {
	PHP_ME(CMathModel,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CMathModel,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMathModel,setData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getParams,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getModelType,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getFunction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getR2,NULL,ZEND_ACC_PUBLIC)

	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CMathModel)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CMathModel",CMathModel_functions);
	CMathModelCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//全局变量
	zend_declare_property_null(CMathModelCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CMathModelCe, ZEND_STRL("modelType"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CMathModelCe, ZEND_STRL("data"),ZEND_ACC_PRIVATE TSRMLS_CC);

	//幂函数
	zend_declare_property_long(CMathModelCe, ZEND_STRL("powA"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("powB"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("powR"),1,ZEND_ACC_PRIVATE TSRMLS_CC);

	//指数函数
	zend_declare_property_long(CMathModelCe, ZEND_STRL("expA"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("expB"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("expR"),1,ZEND_ACC_PRIVATE TSRMLS_CC);

	//对数函数
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lnA"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lnB"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lnR"),1,ZEND_ACC_PRIVATE TSRMLS_CC);

	//线性函数
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lineA"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lineB"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CMathModelCe, ZEND_STRL("lineR"),1,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

int CMathModel_getInstance(zval **returnZval,char *key TSRMLS_DC)
{

	zval	*selfInstace,
			**instaceSaveZval;

	selfInstace = zend_read_static_property(CMathModelCe,ZEND_STRL("instance"),1 TSRMLS_CC);

	MAKE_STD_ZVAL(*returnZval);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(selfInstace)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CMathModelCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		selfInstace = zend_read_static_property(CMathModelCe,ZEND_STRL("instance"),1 TSRMLS_CC);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(selfInstace),key,strlen(key)+1,(void**)&instaceSaveZval) ){
		ZVAL_ZVAL(*returnZval,*instaceSaveZval,1,0);
	}else{

		zval	*object;

		MAKE_STD_ZVAL(object);
		object_init_ex(object,CMathModelCe);

		//执行其构造器 并传入参数
		if (CMathModelCe->constructor) {
			zval	constructVal,
					constructReturn;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CMathModelCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将构造器返回值存入instance静态变量
		add_assoc_zval(selfInstace,key,object);
		zend_update_static_property(CMathModelCe,ZEND_STRL("instance"),selfInstace TSRMLS_CC);
		ZVAL_ZVAL(*returnZval,object,1,0);
	}
}

PHP_METHOD(CMathModel,getInstance)
{
	zval *instanceZval;
	char	*type = NULL,
			*nowModel;
	int		typeLen = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &type,&typeLen) == FAILURE) {
		return;
	}

	if(type == NULL || typeLen == 0){
		nowModel = estrdup("auto");
	}else{
		nowModel = estrdup(type);
	}

	CMathModel_getInstance(&instanceZval,nowModel TSRMLS_CC);
	zend_update_property_string(CMathModelCe,instanceZval,ZEND_STRL("modelType"),nowModel TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,1);
	efree(nowModel);
}

PHP_METHOD(CMathModel,__construct){
	zval *saveArray;
	MAKE_STD_ZVAL(saveArray);
	array_init(saveArray);
	zend_update_property(CMathModelCe,getThis(),ZEND_STRL("data"),saveArray TSRMLS_CC);
	zval_ptr_dtor(&saveArray);
}

void CMathModel_getPowFunction(zval *object TSRMLS_DC){

	zval	*data,
			**thisVal;

	double	allLogY = 0.0,
			allLogX = 0.0,
			thisX = 0.0,
			thisY = 0.0,
			argX = 0.0,
			argY = 0.0,
			a1 = 0.0,
			b1 = 0.0,
			r1 = 0.0,
			r2 = 0.0,
			r3 = 0.0,
			b = 0.0,
			a = 0.0,
			r = 0.0;

	ulong	thisIntKey;
	char	*otherKey;
	int		nums = 0,
			i;

	data = zend_read_property(CMathModelCe,object,ZEND_STRL("data"),1 TSRMLS_CC);
	nums = zend_hash_num_elements(Z_ARRVAL_P(data));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
		allLogX += log(thisX);
		allLogY += log(thisY);
		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	//平均值
	argX = allLogX / (double)nums;
	argY = allLogY / (double)nums;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
	
		a1 += (log(thisX)-argX)*(log(thisY)-argY);
		b1 += pow(log(thisX)-argX,2);
		r1 += (log(thisX)-argX)*(log(thisY)-argY);
		r2 += pow(log(thisX)-argX,2);
		r3 += pow(log(thisY)-argY,2);

		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	b = a1/b1;
	a = exp(argY - b*argX);
	r = r1/(sqrt(r2*r3));

	zend_update_property_double(CMathModelCe,object,ZEND_STRL("powA"),a TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("powB"),b TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("powR"),pow(r,2) TSRMLS_CC);
}

void CMathModel_getExpFunction(zval *object TSRMLS_DC){
	zval	*data,
			**thisVal;

	double	allLogY = 0.0,
			allLogX = 0.0,
			thisX = 0.0,
			thisY = 0.0,
			argX = 0.0,
			argY = 0.0,
			a1 = 0.0,
			b1 = 0.0,
			r1 = 0.0,
			r2 = 0.0,
			r3 = 0.0,
			b = 0.0,
			a = 0.0,
			r = 0.0;

	ulong	thisIntKey;
	char	*otherKey;
	int		nums = 0,
			i;

	data = zend_read_property(CMathModelCe,object,ZEND_STRL("data"),1 TSRMLS_CC);
	nums = zend_hash_num_elements(Z_ARRVAL_P(data));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
		allLogX += thisX;
		allLogY += log(thisY);
		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	//平均值
	argX = allLogX / (double)nums;
	argY = allLogY / (double)nums;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
	
		a1 += (thisX-argX)*(log(thisY)-argY);
		b1 += pow(thisX-argX,2);
		r1 += (thisX-argX)*(log(thisY)-argY);
		r2 += pow(thisX-argX,2);
		r3 += pow(log(thisY)-argY,2);

		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	b = a1/b1;
	a = exp(argY - b*argX);
	r = r1/(sqrt(r2*r3));

	zend_update_property_double(CMathModelCe,object,ZEND_STRL("expA"),a TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("expB"),b TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("expR"),pow(r,2) TSRMLS_CC);
}

void CMathModel_getLnFunction(zval *object TSRMLS_DC){
	zval	*data,
			**thisVal;

	double	allLogY = 0.0,
			allLogX = 0.0,
			thisX = 0.0,
			thisY = 0.0,
			argX = 0.0,
			argY = 0.0,
			a1 = 0.0,
			b1 = 0.0,
			r1 = 0.0,
			r2 = 0.0,
			r3 = 0.0,
			b = 0.0,
			a = 0.0,
			r = 0.0;

	ulong	thisIntKey;
	char	*otherKey;
	int		nums = 0,
			i;

	data = zend_read_property(CMathModelCe,object,ZEND_STRL("data"),1 TSRMLS_CC);
	nums = zend_hash_num_elements(Z_ARRVAL_P(data));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
		allLogX += log(thisX);
		allLogY += thisY;
		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	//平均值
	argX = allLogX / (double)nums;
	argY = allLogY / (double)nums;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
	
		a1 += (log(thisX)-argX)*(thisY-argY);
		b1 += pow(log(thisX)-argX,2);
		r1 += (log(thisX)-argX)*(thisY-argY);
		r2 += pow(log(thisX)-argX,2);
		r3 += pow(thisY-argY,2);

		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	b = a1/b1;
	a = (argY - b*argX);
	r = r1/(sqrt(r2*r3));

	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lnA"),a TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lnB"),b TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lnR"),pow(r,2) TSRMLS_CC);
}

void CMathModel_getLineFunction(zval *object TSRMLS_DC){

	zval	*data,
			**thisVal;

	double	allLogY = 0.0,
			allLogX = 0.0,
			thisX = 0.0,
			thisY = 0.0,
			argX = 0.0,
			argY = 0.0,
			a1 = 0.0,
			b1 = 0.0,
			r1 = 0.0,
			r2 = 0.0,
			r3 = 0.0,
			b = 0.0,
			a = 0.0,
			r = 0.0;

	ulong	thisIntKey;
	char	*otherKey;
	int		nums = 0,
			i;

	data = zend_read_property(CMathModelCe,object,ZEND_STRL("data"),1 TSRMLS_CC);
	nums = zend_hash_num_elements(Z_ARRVAL_P(data));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
		allLogX += (thisX);
		allLogY += (thisY);
		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	//平均值
	argX = allLogX / (double)nums;
	argY = allLogY / (double)nums;

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	for(i = 0 ; i < nums;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
		thisX = atof(otherKey);
		thisY = Z_DVAL_PP(thisVal);
	
		a1 += ((thisX)-argX)*((thisY)-argY);
		b1 += pow((thisX)-argX,2);
		r1 += ((thisX)-argX)*((thisY)-argY);
		r2 += pow((thisX)-argX,2);
		r3 += pow((thisY)-argY,2);

		zend_hash_move_forward(Z_ARRVAL_P(data));
	}

	b = a1/b1;
	a = (argY - b*argX);
	r = r1/(sqrt(r2*r3));

	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lineA"),a TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lineB"),b TSRMLS_CC);
	zend_update_property_double(CMathModelCe,object,ZEND_STRL("lineR"),pow(r,2) TSRMLS_CC);
}

void CMathModel_getMathMode(zval *object TSRMLS_DC){
	
	zval	*modelType;

	modelType = zend_read_property(CMathModelCe,object,ZEND_STRL("modelType"),1 TSRMLS_CC);

	CMathModel_getPowFunction(object TSRMLS_CC);
	CMathModel_getExpFunction(object TSRMLS_CC);
	CMathModel_getLnFunction(object TSRMLS_CC);
	CMathModel_getLineFunction(object TSRMLS_CC);

	//确定当前指定的模型类型
	php_strtolower(Z_STRVAL_P(modelType),strlen(Z_STRVAL_P(modelType))+1);
	if(strcmp(Z_STRVAL_P(modelType),"line") == 0 || strcmp(Z_STRVAL_P(modelType),"exp") == 0 || strcmp(Z_STRVAL_P(modelType),"pow") == 0 || strcmp(Z_STRVAL_P(modelType),"ln") == 0){
		//不做变更
	}else{
		//选择R2 最大的模型
		zval	*r1,
				*r2,
				*r3,
				*r4;

		char	maxModel[16];
		double	maxTop = 0.0;

		r1 = zend_read_property(CMathModelCe,object,ZEND_STRL("powR"),1 TSRMLS_CC);
		r2 = zend_read_property(CMathModelCe,object,ZEND_STRL("expR"),1 TSRMLS_CC);
		r3 = zend_read_property(CMathModelCe,object,ZEND_STRL("lnR"),1 TSRMLS_CC);
		r4 = zend_read_property(CMathModelCe,object,ZEND_STRL("lineR"),1 TSRMLS_CC);
		
		sprintf(maxModel,"%s","pow");
		if(Z_DVAL_P(r1) > maxTop){
			maxTop = Z_DVAL_P(r1);
			sprintf(maxModel,"%s","pow");
		}

		if(Z_DVAL_P(r2) > maxTop){
			maxTop = Z_DVAL_P(r2);
			sprintf(maxModel,"%s","exp");
		}

		if(Z_DVAL_P(r3) > maxTop){
			maxTop = Z_DVAL_P(r3);
			sprintf(maxModel,"%s","ln");
		}

		if(Z_DVAL_P(r4) > maxTop){
			maxTop = Z_DVAL_P(r4);
			sprintf(maxModel,"%s","line");
		}

		zend_update_property_string(CMathModelCe,object,ZEND_STRL("modelType"),maxModel TSRMLS_CC);
	}
}

PHP_METHOD(CMathModel,setData){

	zval	*data;
	long	isAppend = 0;
	int		i,h;
	char	*otherKey,
			keyChar[1024];
	ulong	thisIntKey;
	zval	**thisVal,
			*nowData,
			*nowKey;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|b", &data,&isAppend) == FAILURE) {
		RETVAL_FALSE;
		return;
	}

	MAKE_STD_ZVAL(nowData);
	if(isAppend == 0){
		array_init(nowData);
	}else{

		//向后附加
		zval *oldData = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("data"),1 TSRMLS_CC);
		ZVAL_ZVAL(nowData,oldData,1,0);
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
	h = zend_hash_num_elements(Z_ARRVAL_P(data));
	for(i = 0 ; i < h;i++){

		zend_hash_get_current_data(Z_ARRVAL_P(data),(void**)&thisVal);

		if(IS_LONG != Z_TYPE_PP(thisVal) && IS_DOUBLE != Z_TYPE_PP(thisVal) && IS_STRING != Z_TYPE_PP(thisVal)){
			zend_hash_move_forward(Z_ARRVAL_P(data));
			continue;
		}
		convert_to_double(*thisVal);
		MAKE_STD_ZVAL(nowKey);
		if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_P(data))){
			zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
			ZVAL_LONG(nowKey,thisIntKey);
		}else if(HASH_KEY_IS_STRING == zend_hash_get_current_key_type(Z_ARRVAL_P(data))){
			zend_hash_get_current_key(Z_ARRVAL_P(data), &otherKey, &thisIntKey, 0);
			ZVAL_STRING(nowKey,otherKey,1);
		}
		convert_to_double(nowKey);
		if(Z_DVAL_P(nowKey) > 0){
			sprintf(keyChar,"%.8f",Z_DVAL_P(nowKey));
			add_assoc_double(nowData,keyChar,Z_DVAL_PP(thisVal));
		}

		zend_hash_move_forward(Z_ARRVAL_P(data));
		zval_ptr_dtor(&nowKey);
	}

	zend_update_property(CMathModelCe,getThis(),ZEND_STRL("data"),nowData TSRMLS_CC);
	zval_ptr_dtor(&nowData);
	RETVAL_ZVAL(getThis(),1,0);

	//重新调用模型获取系数
	CMathModel_getMathMode(getThis() TSRMLS_CC);
}

PHP_METHOD(CMathModel,get){

	zval	*modelType,
			*a,
			*b,
			*x;

	double	y = 0.0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &x) == FAILURE) {
		RETVAL_FALSE;
		return;
	}

	//判断key类型
	convert_to_double(x);

	modelType = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("modelType"),1 TSRMLS_CC);

	if(strcmp(Z_STRVAL_P(modelType),"pow") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powB"),1 TSRMLS_CC);
		y = Z_DVAL_P(a)*pow(Z_DVAL_P(x),Z_DVAL_P(b));
	}

	if(strcmp(Z_STRVAL_P(modelType),"exp") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expB"),1 TSRMLS_CC);
		y = Z_DVAL_P(a)*exp(Z_DVAL_P(x)*Z_DVAL_P(b));
	}

	if(strcmp(Z_STRVAL_P(modelType),"ln") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnB"),1 TSRMLS_CC);
		y = Z_DVAL_P(b)*log(Z_DVAL_P(x))+Z_DVAL_P(a);
	}

	if(strcmp(Z_STRVAL_P(modelType),"line") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineB"),1 TSRMLS_CC);
		y = Z_DVAL_P(b)*Z_DVAL_P(x)+Z_DVAL_P(a);
	}

	RETVAL_DOUBLE(y);
}

PHP_METHOD(CMathModel,getParams){

	zval	*modelType,
			*a,
			*b,
			*returnArray;

	MAKE_STD_ZVAL(returnArray);
	array_init(returnArray);

	add_assoc_double(returnArray,"a",0.0);
	add_assoc_double(returnArray,"b",0.0);

	modelType = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("modelType"),1 TSRMLS_CC);

	if(strcmp(Z_STRVAL_P(modelType),"pow") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powB"),1 TSRMLS_CC);
		add_assoc_double(returnArray,"a",Z_DVAL_P(a));
		add_assoc_double(returnArray,"b",Z_DVAL_P(b));
	}

	if(strcmp(Z_STRVAL_P(modelType),"exp") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expB"),1 TSRMLS_CC);
		add_assoc_double(returnArray,"a",Z_DVAL_P(a));
		add_assoc_double(returnArray,"b",Z_DVAL_P(b));
	}

	if(strcmp(Z_STRVAL_P(modelType),"ln") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnB"),1 TSRMLS_CC);
		add_assoc_double(returnArray,"a",Z_DVAL_P(a));
		add_assoc_double(returnArray,"b",Z_DVAL_P(b));
	}

	if(strcmp(Z_STRVAL_P(modelType),"line") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineB"),1 TSRMLS_CC);
		add_assoc_double(returnArray,"a",Z_DVAL_P(a));
		add_assoc_double(returnArray,"b",Z_DVAL_P(b));
	}

	RETVAL_ZVAL(returnArray,1,1);
}

PHP_METHOD(CMathModel,getModelType){
	zval *modelType = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("modelType"),1 TSRMLS_CC);
	RETVAL_ZVAL(modelType,1,0);
}

PHP_METHOD(CMathModel,getFunction){

	zval	*modelType,
			*a,
			*b,
			*r;

	char	function[128];


	modelType = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("modelType"),1 TSRMLS_CC);

	if(strcmp(Z_STRVAL_P(modelType),"pow") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powB"),1 TSRMLS_CC);
		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powR"),1 TSRMLS_CC);

		sprintf(function,"Y = %.4fx^%.4f  R2=%.4f",Z_DVAL_P(a),Z_DVAL_P(b),Z_DVAL_P(r));

	}

	if(strcmp(Z_STRVAL_P(modelType),"exp") == 0){

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expB"),1 TSRMLS_CC);
		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expR"),1 TSRMLS_CC);

		sprintf(function,"Y = %.4f*exp(%.4fx)  R2=%.4f",Z_DVAL_P(a),Z_DVAL_P(b),Z_DVAL_P(r));

	}

	if(strcmp(Z_STRVAL_P(modelType),"ln") == 0){

		char	fh[2];

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnB"),1 TSRMLS_CC);
		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnR"),1 TSRMLS_CC);

		if(Z_DVAL_P(a) > 0.0){
			sprintf(fh,"+");
		}else{
			sprintf(fh,"");
		}

		sprintf(function,"Y = %.4f*ln(x)%s%.4f  R2=%.4f",Z_DVAL_P(b),fh,Z_DVAL_P(a),Z_DVAL_P(r));

	}

	if(strcmp(Z_STRVAL_P(modelType),"line") == 0){
		char	fh[2];

		a = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineA"),1 TSRMLS_CC);
		b = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineB"),1 TSRMLS_CC);
		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineR"),1 TSRMLS_CC);

		if(Z_DVAL_P(a) > 0.0){
			sprintf(fh,"+");
		}else{
			sprintf(fh,"");
		}
		sprintf(function,"Y = %.4f*x%s%.4f  R2=%.4f",Z_DVAL_P(b),fh,Z_DVAL_P(a),Z_DVAL_P(r));
	}

	RETVAL_STRING(function,1);

}
PHP_METHOD(CMathModel,getR2){
	zval	*modelType,
			*r;

	modelType = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("modelType"),1 TSRMLS_CC);

	if(strcmp(Z_STRVAL_P(modelType),"pow") == 0){

		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("powR"),1 TSRMLS_CC);
	}

	if(strcmp(Z_STRVAL_P(modelType),"exp") == 0){

		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("expR"),1 TSRMLS_CC);
	}

	if(strcmp(Z_STRVAL_P(modelType),"ln") == 0){

		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lnR"),1 TSRMLS_CC);
	}

	if(strcmp(Z_STRVAL_P(modelType),"line") == 0){

		r = zend_read_property(CMathModelCe,getThis(),ZEND_STRL("lineR"),1 TSRMLS_CC);
	}

	RETVAL_ZVAL(r,1,0);
}
