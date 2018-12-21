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
#include "php_CArrayHelper.h"


//zend类方法
zend_function_entry CArrayHelper_functions[] = {
	PHP_ME(CArrayHelper,resetKey,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CArrayHelper,get,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CArrayHelper,sortArrByOneField,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CArrayHelper)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CArrayHelper",CArrayHelper_functions);
	CArrayHelperCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}


PHP_METHOD(CArrayHelper,resetKey)
{
	zval	*array,
			**arrayVal,
			**keyZval,
			*saveZval;
	char	*key;
	int		keyLen = 0,
			i,j;

	array_init(return_value);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&array,&key,&keyLen) == FAILURE){
		return;
	}

	if(keyLen == 0){
		return;
	}

	if(IS_ARRAY != Z_TYPE_P(array)){
		return;
	}


	j = zend_hash_num_elements(Z_ARRVAL_P(array));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	for(i = 0 ; i < j ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(array),(void**)&arrayVal);


		if(IS_ARRAY != Z_TYPE_PP(arrayVal)){
			return;
		}

		//判断是否存在键
		if(!zend_hash_exists(Z_ARRVAL_PP(arrayVal),key,strlen(key)+1)){
			return;
		}

		if(SUCCESS != zend_hash_find(Z_ARRVAL_PP(arrayVal),key,strlen(key)+1,(void**)&keyZval)){
			return;
		}

		MAKE_STD_ZVAL(saveZval);
		ZVAL_ZVAL(saveZval,*arrayVal,1,0);

		if(Z_TYPE_PP(keyZval) == IS_STRING){

			add_assoc_zval(return_value,Z_STRVAL_PP(keyZval),saveZval);

		}else if(Z_TYPE_PP(keyZval) == IS_LONG){

			add_index_zval(return_value,Z_LVAL_PP(keyZval),saveZval);
		}

		zend_hash_move_forward(Z_ARRVAL_P(array));
	}

}

void CArrayHelper_getKeyInt(zval *array,int key,zval *defaultVal,zval **returnZval TSRMLS_DC){

	MAKE_STD_ZVAL(*returnZval);

	//直接存在key则直接返回
	if(zend_hash_index_exists(Z_ARRVAL_P(array),key)){
		zval	**thisZval;
		zend_hash_index_find(Z_ARRVAL_P(array),key,(void**)&thisZval);
		ZVAL_ZVAL(*returnZval,*thisZval,1,0);
		return;
	}else{
		ZVAL_STRING(*returnZval,"",1);
	}
}

void CArrayHelper_getKeyString(zval *array,char *key,zval *defaultVal,zval **returnZval TSRMLS_DC){

	MAKE_STD_ZVAL(*returnZval);

	//直接存在key则直接返回
	if(zend_hash_exists(Z_ARRVAL_P(array),key,strlen(key)+1)){
		zval	**thisZval;
		zend_hash_find(Z_ARRVAL_P(array),key,strlen(key)+1,(void**)&thisZval);
		ZVAL_ZVAL(*returnZval,*thisZval,1,0);
		return;
	}

	//判断是否存在分割号.
	if(strstr(key,".") != NULL){

		//将key按照.分割
		zval *cutArr;
		php_explode(".",key,&cutArr);

		if(IS_ARRAY == Z_TYPE_P(cutArr) && zend_hash_num_elements(Z_ARRVAL_P(cutArr)) > 1){
			
			//对configsData保存的Hashtable进行遍历
			int configTableNum,
				n;

			zval *currentData,
				 **nData,
				 **resetData;

			//当前值
			MAKE_STD_ZVAL(currentData);
			ZVAL_ZVAL(currentData,array,1,0);

			configTableNum = zend_hash_num_elements(Z_ARRVAL_P(cutArr));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(cutArr));

			for( n = 0 ; n < configTableNum ; n++){

				zend_hash_get_current_data(Z_ARRVAL_P(cutArr),(void**)&nData);
				if(IS_ARRAY != Z_TYPE_P(currentData)){
					ZVAL_ZVAL(*returnZval,defaultVal,1,0);
					zval_ptr_dtor(&cutArr);
					zval_ptr_dtor(&currentData);
					return;
				}

				//判断该键是否为字符串型但值为数字
				if(isdigitstr(Z_STRVAL_PP(nData))){
				
					int keyInt = toInt(Z_STRVAL_PP(nData));

					//不存在该层键则返回null
					if(!zend_hash_index_exists(Z_ARRVAL_P(currentData),keyInt)){
						ZVAL_ZVAL(*returnZval,defaultVal,1,0);
						zval_ptr_dtor(&cutArr);
						zval_ptr_dtor(&currentData);
						return;
					}

					//重置cutArr的值
					if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(currentData),keyInt,(void**)&resetData)){
						zval *tempZval;
						MAKE_STD_ZVAL(tempZval);
						ZVAL_ZVAL(tempZval,*resetData,1,0);
						zval_ptr_dtor(&currentData);
						MAKE_STD_ZVAL(currentData);
						ZVAL_ZVAL(currentData,tempZval,1,0);
						zval_ptr_dtor(&tempZval);
					}
					zend_hash_move_forward(Z_ARRVAL_P(cutArr));

				}else{

					//不存在该层键则返回null
					if(!zend_hash_exists(Z_ARRVAL_P(currentData),Z_STRVAL_PP(nData),strlen(Z_STRVAL_PP(nData)) + 1)){
						ZVAL_ZVAL(*returnZval,defaultVal,1,0);
						zval_ptr_dtor(&cutArr);
						zval_ptr_dtor(&currentData);
						return;
					}

					//重置cutArr的值
					if(SUCCESS == zend_hash_find(Z_ARRVAL_P(currentData),Z_STRVAL_PP(nData),Z_STRLEN_PP(nData) + 1,(void**)&resetData)){
						zval *tempZval;

						MAKE_STD_ZVAL(tempZval);
						ZVAL_ZVAL(tempZval,*resetData,1,0);
						zval_ptr_dtor(&currentData);

						MAKE_STD_ZVAL(currentData);
						ZVAL_ZVAL(currentData,tempZval,1,0);
						zval_ptr_dtor(&tempZval);
					}
					zend_hash_move_forward(Z_ARRVAL_P(cutArr));
				}
			}

			ZVAL_ZVAL(*returnZval,currentData,1,0);
			zval_ptr_dtor(&currentData);
			zval_ptr_dtor(&cutArr);
			return;
		}else{
			ZVAL_ZVAL(*returnZval,defaultVal,1,0);
		}
	}else{
		ZVAL_STRING(*returnZval,"",1);
	}
}


PHP_METHOD(CArrayHelper,get)
{
	zval	*array,
			*defaultVal = NULL,
			*sendDefault,
			*key,
			*returnZval;

	int		keyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz|z",&array,&key,&defaultVal) == FAILURE){
		return;
	}

	if(IS_ARRAY != Z_TYPE_P(array)){
		return;
	}

	MAKE_STD_ZVAL(sendDefault);
	if(NULL != defaultVal){
		ZVAL_ZVAL(sendDefault,defaultVal,1,0);
	}else{
		ZVAL_STRING(sendDefault,"",1);
	}

	if(IS_STRING == Z_TYPE_P(key)){
		CArrayHelper_getKeyString(array,Z_STRVAL_P(key),sendDefault,&returnZval TSRMLS_CC);
	}else{
		CArrayHelper_getKeyInt(array,Z_STRVAL_P(key),sendDefault,&returnZval TSRMLS_CC);
	}

	zval_ptr_dtor(&sendDefault);
	RETVAL_ZVAL(returnZval,1,1);
}

void CArrayHelper_sortArrByOneField(zval *array,char *field,int sort,zval **returnZval TSRMLS_DC){
	
	zval	*fieldArr,
			**nData,
			**fieleKeyVal,
			*functionReturn,
			sizeObjectReturn;

	int		i,j,keyInt;
	char	*key;

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	MAKE_STD_ZVAL(fieldArr);
	array_init(fieldArr);

	j = zend_hash_num_elements(Z_ARRVAL_P(array));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	for(i = 0 ; i < j ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(array),(void**)&nData);

		if(IS_ARRAY != Z_TYPE_PP(nData)){
			zval_ptr_dtor(&fieldArr);
			return;
		}

		zend_hash_find(Z_ARRVAL_PP(nData),field,strlen(field)+1,(void**)&fieleKeyVal);


		if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_P(array))){
			zval *saveTemp;
			zend_hash_get_current_key(Z_ARRVAL_P(array), &key, &keyInt, 0);
			MAKE_STD_ZVAL(saveTemp);
			ZVAL_ZVAL(saveTemp,*fieleKeyVal,1,0);
			add_index_zval(fieldArr,keyInt,saveTemp);	
		}else if(HASH_KEY_IS_STRING == zend_hash_get_current_key_type(Z_ARRVAL_P(array))){
			zval *saveTemp;
			zend_hash_get_current_key(Z_ARRVAL_P(array), &key, &keyInt, 0);
			MAKE_STD_ZVAL(saveTemp);
			ZVAL_ZVAL(saveTemp,*fieleKeyVal,1,0);
			add_assoc_zval(fieldArr,key,saveTemp);
		}else{
			zval_ptr_dtor(&fieldArr);
			return;
		}

		zend_hash_move_forward(Z_ARRVAL_P(array));
	}


	//调用php array_multisort
	MODULE_BEGIN
		zval	constructVal,
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
		ZVAL_ZVAL(paramsList[0],fieldArr,1,0);
		ZVAL_LONG(paramsList[1],sort);
		ZVAL_ZVAL(paramsList[2],array,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"array_multisort", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &sizeObjectReturn, 3, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_dtor(&sizeObjectReturn);
		ZVAL_ZVAL(*returnZval,paramsList[2],1,1);
	MODULE_END


	//销毁的资源
	zval_ptr_dtor(&fieldArr);

}

PHP_METHOD(CArrayHelper,sortArrByOneField)
{
	zval	*array,
			*returnZval;

	char	*field,
			*sort = "asc";

	int		fieldLen = 0,
			sortLen = 0,
			sortType = 4;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs|s",&array,&field,&fieldLen,&sort,&sortLen) == FAILURE){
		array_init(return_value);
		return;
	}

	//异常判断
	if(IS_ARRAY != Z_TYPE_P(array)){
		array_init(return_value);
		return;
	}

	//指段传递有误
	if(fieldLen == 0){
		array_init(return_value);
		return;
	}

	php_strtolower(sort,strlen(sort)+1);
	if(strcmp(sort,"asc") == 0){
		sortType = 4;
	}else{
		sortType = 3;
	}

	CArrayHelper_sortArrByOneField(array,field,sortType,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}