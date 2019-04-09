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
#include "php_CArraySort.h"


//zend类方法
zend_function_entry CArraySort_functions[] = {
	PHP_ME(CArraySort,sortArrayAsc,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CArraySort,sortArrayDesc,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CArraySort)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CArraySort",CArraySort_functions);
	CArraySortCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

//二维数据反序排布
void CArraySort_sortArrayAsc(zval *list,char *sortKey,zval **returnZval TSRMLS_DC)
{
	zval		**thisVal,
				**this2Val,
				**sortKeyZval,
				**sort2KeyZval,
				*sortData1 = NULL,
				*sortData2 = NULL,
				**this3Val,
				*sortData;

	char		*key,
				*key2;

	ulong		uKey,
				uKey2;

	int			i,
				num,
				sort_total,
				j,
				key1Val = 0,
				key2Val = 0,
				min_key = 0,
				h,
				n;

	//不为数组时返回空
	if(IS_ARRAY != Z_TYPE_P(list)){
		return;
	}

	//分配空间
	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);

	MAKE_STD_ZVAL(sortData);
	array_init(sortData);

	num = zend_hash_num_elements(Z_ARRVAL_P(list));

	//遍历数组
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(list));
	for(i = 0 ; i < num ; i++){

		zend_hash_get_current_key(Z_ARRVAL_P(list),&key,&uKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(list),(void**)&thisVal);

		//指定key的值
		zend_hash_find(Z_ARRVAL_PP(thisVal),sortKey,strlen(sortKey)+1,(void**)&sortKeyZval);
		if(IS_LONG != Z_TYPE_PP(sortKeyZval)){
			key1Val = 0;
		}else{
			key1Val = Z_LVAL_PP(sortKeyZval);
		}

		min_key = -1;
		sort_total = zend_hash_num_elements(Z_ARRVAL_P(sortData));
		
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(sortData));
		for(j = 0 ; j < sort_total; j++){
			zend_hash_get_current_data(Z_ARRVAL_P(sortData),(void**)&this2Val);
			zend_hash_find(Z_ARRVAL_PP(this2Val),sortKey,strlen(sortKey)+1,(void**)&sort2KeyZval);
			if(IS_LONG != Z_TYPE_PP(sort2KeyZval)){
				key2Val = 0;
			}else{
				key2Val = Z_LVAL_PP(sort2KeyZval);
			}
			if(key1Val < key2Val){
				min_key = j + 1;
				break;
			}
			zend_hash_move_forward(Z_ARRVAL_P(sortData));
		}

		//添加值
		if(min_key == -1){
			add_next_index_zval(sortData,*thisVal);
			zval_add_ref(thisVal);
		}else{
			if(sortData1 != NULL){
				zval_ptr_dtor(&sortData1);
			}
			array_slice(Z_ARRVAL_P(sortData), 0, min_key-1,&sortData1); 
			add_next_index_zval(sortData1,*thisVal);
			zval_add_ref(thisVal);
			if((min_key-1) < sort_total){ 
				zend_module_entry tmp;
				int sortDataLen = 0;
				sortDataLen = zend_hash_num_elements(Z_ARRVAL_P(sortData));
				if(sortData2 != NULL){
					zval_ptr_dtor(&sortData2);
				}
				array_slice(Z_ARRVAL_P(sortData), min_key-1,sortDataLen - (min_key-1),&sortData2);
				
				h = zend_hash_num_elements(Z_ARRVAL_P(sortData2));
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(sortData2));
				for(n = 0 ; n < h ; n++){
					zend_hash_get_current_data(Z_ARRVAL_P(sortData2),(void**)&this3Val);
					zend_hash_next_index_insert(Z_ARRVAL_P(sortData1),this3Val,sizeof(zval*),NULL);
					zval_add_ref(this3Val);
					zend_hash_move_forward(Z_ARRVAL_P(sortData2));
				}
				zend_hash_copy(Z_ARRVAL_P(sortData),Z_ARRVAL_P(sortData1),(copy_ctor_func_t)zval_add_ref, &tmp, sizeof(zend_module_entry));
			}
		}
		zend_hash_move_forward(Z_ARRVAL_P(list));
	}

	ZVAL_ZVAL(*returnZval,sortData,1,0);

	//if(sortData2 != NULL) zval_ptr_dtor(&sortData2);
	//if(sortData1 != NULL) zval_ptr_dtor(&sortData1);
}

PHP_METHOD(CArraySort,sortArrayAsc)
{
	zval *list,
		 *returnZval;
	char *sortKey;
	int  sortKeyLen;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&list,&sortKey,&sortKeyLen) == FAILURE){
		array_init(return_value);
		return;
	}

	CArraySort_sortArrayAsc(list,sortKey,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}

//二维数据反序排布
void CArraySort_sortArrayDesc(zval *list,char *sortKey,zval **returnZval TSRMLS_DC)
{
	zval		**thisVal,
				**this2Val,
				**sortKeyZval,
				**sort2KeyZval,
				*sortData1,
				*sortData2,
				**this3Val;
			
	HashTable	*sortData;

	char		*key,
				*key2;

	ulong		uKey,
				uKey2;

	int			i,
				num,
				sort_total,
				j,
				key1Val = 0,
				key2Val = 0,
				min_key = 0,
				h,
				n,
				oldSortDataNum;

	//不为数组时返回空
	if(IS_ARRAY != Z_TYPE_P(list)){
		return;
	}


	//遍历数组
	num = zend_hash_num_elements(Z_ARRVAL_P(list));

	//分配空间
	MAKE_STD_ZVAL(*returnZval);


	ALLOC_HASHTABLE(sortData);
	zend_hash_init(sortData,num,NULL,NULL,0);

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(list));
	for(i = 0 ; i < num ; i++){

		zend_hash_get_current_key(Z_ARRVAL_P(list),&key,&uKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(list),(void**)&thisVal);

		//指定key的值
		zend_hash_find(Z_ARRVAL_PP(thisVal),sortKey,strlen(sortKey)+1,(void**)&sortKeyZval);

		if(IS_LONG != Z_TYPE_PP(sortKeyZval)){
			key1Val = 0;
		}else{
			key1Val = Z_LVAL_PP(sortKeyZval);
		}

		min_key = 0;
		sort_total = zend_hash_num_elements(sortData);

		zend_hash_internal_pointer_reset(sortData);
		for(j = 0 ; j < sort_total; j++){
			zend_hash_get_current_data(sortData,(void**)&this2Val);
			zend_hash_find(Z_ARRVAL_PP(this2Val),sortKey,strlen(sortKey)+1,(void**)&sort2KeyZval);
			if(IS_LONG != Z_TYPE_PP(sort2KeyZval)){
				key2Val = 0;
			}else{
				key2Val = Z_LVAL_PP(sort2KeyZval);
			}

			if(key1Val > key2Val){
				min_key = j + 1;
				break;
			}
			zend_hash_move_forward(sortData);
		}

		//添加值
		if(min_key == 0){
			zend_hash_next_index_insert(sortData,&*thisVal,sizeof(zval*),NULL);
		}else{
			array_slice(sortData, 0, min_key-1,&sortData1); 
			zend_hash_next_index_insert(Z_ARRVAL_P(sortData1),&*thisVal,sizeof(zval*),NULL);
			if((min_key-1) < sort_total){ 
				zend_module_entry tmp;
				int sortDataLen = 0;
				sortDataLen = zend_hash_num_elements(sortData);
				array_slice(sortData, min_key-1,sortDataLen - (min_key-1),&sortData2);
				h = zend_hash_num_elements(Z_ARRVAL_P(sortData2));
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(sortData2));
				for(n = 0 ; n < h ; n++){
					zend_hash_get_current_data(Z_ARRVAL_P(sortData2),(void**)&this3Val);
					zend_hash_next_index_insert(Z_ARRVAL_P(sortData1),&*this3Val,sizeof(zval*),NULL);
					zend_hash_move_forward(Z_ARRVAL_P(sortData2));
				}
				zend_hash_copy(sortData,Z_ARRVAL_P(sortData1),NULL, &tmp, sizeof(zend_module_entry));
			}
		}
		zend_hash_move_forward(Z_ARRVAL_P(list));
	}

	Z_TYPE_P(*returnZval) = IS_ARRAY;
	Z_ARRVAL_P(*returnZval) = sortData;
}

PHP_METHOD(CArraySort,sortArrayDesc)
{
	zval *list,
		 *returnZval;

	char *sortKey;

	int  sortKeyLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zs",&list,&sortKey,&sortKeyLen) == FAILURE){
		array_init(return_value);
		return;
	}

	CArraySort_sortArrayDesc(list,sortKey,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,1);
}
