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



//zend类方法
zend_function_entry CMathModel_functions[] = {
	PHP_ME(CMathModel,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CMathModel,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CMathModel,setData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getMathMode,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CMathModel,get,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getParams,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getModelType,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getFunction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getR2,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CMathModel,getLineFunction,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CMathModel,getLnFunction,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CMathModel,getExpFunction,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CMathModel,getPowFunction,NULL,ZEND_ACC_PRIVATE)

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
	ZVAL_ZVAL(return_value,instanceZval,1,1);
	efree(nowModel);
}

PHP_METHOD(CMathModel,__construct){
}

PHP_METHOD(CMathModel,setData){}
PHP_METHOD(CMathModel,getMathMode){}
PHP_METHOD(CMathModel,get){}
PHP_METHOD(CMathModel,getParams){}
PHP_METHOD(CMathModel,getModelType){}
PHP_METHOD(CMathModel,getFunction){}
PHP_METHOD(CMathModel,getR2){}
PHP_METHOD(CMathModel,getLineFunction){}
PHP_METHOD(CMathModel,getLnFunction){}
PHP_METHOD(CMathModel,getExpFunction){}
PHP_METHOD(CMathModel,getPowFunction){}