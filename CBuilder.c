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
#include "php_CBuilder.h"
#include "php_CException.h"
#include "php_CDatabase.h"
#include "php_CResult.h"
#include "php_CExec.h"
#include "php_CDbError.h"
#include "php_CHooks.h"


//zend类方法
zend_function_entry CBuilder_functions[] = {
	PHP_ME(CBuilder,insert,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,prepare,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,query,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,update,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,delete,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,select,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,value,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,from,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,join,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,on,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,where,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,groupBy,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,orderBy,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,execute,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,getSql,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,cache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,beginTransaction,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,commit,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,rollback,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,limit,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,setConnCheckInterval,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,getLastActiveTime,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CBuilder,destory,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CBuilder)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CBuilder",CBuilder_functions);
	CBuilderCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量
	zend_declare_property_string(CBuilderCe, ZEND_STRL("action"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("from"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("cols"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("join"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("joinTemp"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("on"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("where"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("val"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CBuilderCe, ZEND_STRL("value"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("groupBy"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("orderBy"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("limit")," LIMIT 10000 ",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("tablePre"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("_sql"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("_cache"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CBuilderCe, ZEND_STRL("_cacheTime"),3600,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("isMaster"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("configName"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(CBuilderCe, ZEND_STRL("prepare"),"",ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CBuilderCe, ZEND_STRL("whereValue"),ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}

//替换表前缀返回SQL
void CBuilder_getTablePre(zval *object,char **tableStr TSRMLS_DC)
{
	zval *thisVal,
		 **thisKeyZval;

	char *configName = "main",
		 *useMaster = "0",
		 thisKey[128],
		 *returnSql = "";
	int	 hasExist;

	//获取key
	thisVal = zend_read_property(CBuilderCe,object,ZEND_STRL("tablePre"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(thisVal)){
		*tableStr = estrdup(Z_STRVAL_P(thisVal));
		return;
	}

	*tableStr = estrdup("");
}

//类方法:创建应用对象
PHP_METHOD(CBuilder,insert)
{
	char *action;
	action = " INSERT INTO ";
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("action"),action TSRMLS_CC);
	ZVAL_ZVAL(return_value,getThis(),1,0);
}

PHP_METHOD(CBuilder,update)
{
	char *action;
	action = " UPDATE ";
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("action"),action TSRMLS_CC);
	ZVAL_ZVAL(return_value,getThis(),1,0);
}

PHP_METHOD(CBuilder,delete)
{
	char *action;
	action = " DELETE ";
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("action"),action TSRMLS_CC);
	ZVAL_ZVAL(return_value,getThis(),1,0);
}


PHP_METHOD(CBuilder,select)
{	
	//获取参数个数
	int argc = ZEND_NUM_ARGS(),
		i = 0;
	zval ***args,
		 **thisVal;

	char *cols,
		 *key;

	ulong ikey;

	cols = "";

	ZVAL_ZVAL(return_value,getThis(),1,0);

	args = (zval***)safe_emalloc(argc,sizeof(zval**),0);


	//设置动作
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("action"),"SELECT" TSRMLS_CC);


	if(ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc,args) == FAILURE){

		zend_update_property_string(CBuilderCe,getThis(), ZEND_STRL("cols"), "*" TSRMLS_CC);
		efree(args);
		return;

	}else if(ZEND_NUM_ARGS() == 1 && IS_ARRAY == Z_TYPE_PP(args[0])  ){
		
		int k,n,keyType;
		smart_str tempCols = {0};

		n = zend_hash_num_elements(Z_ARRVAL_PP(args[0]));

		if(n > 0){
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(args[0]));
			for(k = 0 ; k < n ; k++){
				zend_hash_get_current_data((HashTable*)Z_STRVAL_PP(args[0]),(void**)&thisVal);

				if(IS_STRING != Z_TYPE_PP(thisVal)){
					zend_hash_move_forward((HashTable*)Z_STRVAL_PP(args[0]));
					continue;
				}

				keyType = zend_hash_get_current_key_type((HashTable*)Z_STRVAL_PP(args[0]));
				if(keyType == 2){
					if(IS_STRING == Z_TYPE_PP(thisVal)){
					
						smart_str_appends(&tempCols,Z_STRVAL_PP(thisVal));
						smart_str_appends(&tempCols,",");

					}
				}else if(keyType == 1){
					char *getKey;
					ulong getUkey;
					zend_hash_get_current_key((HashTable*)Z_STRVAL_PP(args[0]),&getKey,&getUkey,0);

					smart_str_appends(&tempCols,getKey);
					smart_str_appends(&tempCols," AS ");
					smart_str_appends(&tempCols,Z_STRVAL_PP(thisVal));
					smart_str_appends(&tempCols,",");
				}

				zend_hash_move_forward((HashTable*)Z_STRVAL_PP(args[0]));
			}

			smart_str_0(&tempCols);

			if(tempCols.c == NULL){
				zend_update_property_string(CBuilderCe,getThis(), ZEND_STRL("cols"), "*" TSRMLS_CC);
				efree(args);
				return;
			}


			substr(tempCols.c,0,-1,&cols);
			smart_str_free(&tempCols);
			zend_update_property_string(CBuilderCe,getThis(), ZEND_STRL("cols"), cols TSRMLS_CC);
			efree(cols);
			efree(args);
			return;
		}else{
			zend_update_property_string(CBuilderCe,getThis(), ZEND_STRL("cols"), "*" TSRMLS_CC);
			efree(args);
			return;
		}
	}else{
		//处理任意数目参数
		smart_str tempCols = {0};

		for(i = 0 ; i < argc ; i++){

			if(IS_STRING == Z_TYPE_PP(args[i])){

				convert_to_string(*args[i]);

				if(i < argc - 1) {

					smart_str_appends(&tempCols,cols);
					smart_str_appends(&tempCols,Z_STRVAL_PP(args[i]));
					smart_str_appends(&tempCols,",");
				}else{
					smart_str_appends(&tempCols,cols);
					smart_str_appends(&tempCols,Z_STRVAL_PP(args[i]));
				}

			}else if(IS_ARRAY == Z_TYPE_PP(args[i])){

				zend_hash_get_current_key((HashTable*)Z_STRVAL_PP(args[i]),&key,&ikey,0);
				zend_hash_get_current_data((HashTable*)Z_STRVAL_PP(args[i]),(void**)&thisVal);

				if(IS_STRING != Z_TYPE_PP(thisVal)){
					efree(args);
					zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->select] Parameter error ", 5001 TSRMLS_CC);
					return;
				}

				if(i < argc - 1) {
					
					smart_str_appends(&tempCols,key);
					smart_str_appends(&tempCols," AS ");
					smart_str_appends(&tempCols,Z_STRVAL_PP(thisVal));
					smart_str_appends(&tempCols,",");

				}else{
					smart_str_appends(&tempCols,key);
					smart_str_appends(&tempCols," AS ");
					smart_str_appends(&tempCols,Z_STRVAL_PP(thisVal));
				}

			}else{
				efree(args);
				smart_str_0(&tempCols);
				smart_str_free(&tempCols);
				zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->select] Parameter error ", 5001 TSRMLS_CC);
				return;
			}
		}

		smart_str_0(&tempCols);
		cols = estrdup(tempCols.c);
		smart_str_free(&tempCols);
	}

	efree(args);
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("cols"), cols TSRMLS_CC);
	efree(cols);
}

PHP_METHOD(CBuilder,from)
{
	zval *tableName,
		 **tablePre;

	int tableNameLen;

	char *tableString,
		 *tablePreVal;

	ZVAL_ZVAL(return_value,getThis(),1,0);
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&tableName,&tableNameLen) == FAILURE){
        zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5002 TSRMLS_CC);
		return;
    }

	if(Z_TYPE_P(tableName) == IS_ARRAY){
		//解析数组
		char *key,
			 tempTableString[1024];
		ulong iKey = 0;
		int keyType;
		zval **value;
		if(zend_hash_num_elements(Z_ARRVAL_P(tableName)) != 1){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error , send a key->value array like array(\"tableName\"=>\"a\") ", 5002 TSRMLS_CC);
			return;
		}



		zend_hash_internal_pointer_reset(Z_ARRVAL_P(tableName));
		keyType = zend_hash_get_current_key(Z_ARRVAL_P(tableName),&key,&iKey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(tableName),(void**)&value);

		if(keyType == HASH_KEY_IS_LONG){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error , send a key->value array like array(\"tableName\"=>\"a\") ", 5002 TSRMLS_CC);
			return;
		}

		if(Z_TYPE_PP(value) != IS_STRING){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error , value must be a string ", 5002 TSRMLS_CC);
			return;
		}
		sprintf(tempTableString,"%s%s%s%s",key,"` AS `",Z_STRVAL_PP(value),"`");
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("from"),tempTableString TSRMLS_CC);
		return;

	}else if(Z_TYPE_P(tableName) == IS_STRING){
		char tempTableString[1024];
		sprintf(tempTableString,"%s%s",Z_STRVAL_P(tableName),"`");
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("from"),tempTableString TSRMLS_CC);
		return;

	}else{
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5002 TSRMLS_CC);
		return;
	}
}

PHP_METHOD(CBuilder,join)
{
	zval *joinTable;
	char *joinType,
		 *defaultJoinType = "LEFT",
		 joinTemp[10240],
		 *from = "",
		 *tablePre;
	int	joinTypeLen = 0;

	ZVAL_ZVAL(return_value,getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|s",&joinTable,&joinType,&joinTypeLen) == FAILURE){
        zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->join] Parameter error ", 5003 TSRMLS_CC);
		return;
    }

	//表前缀
	CBuilder_getTablePre(getThis(),&tablePre TSRMLS_CC);

	//join方式
	if(joinTypeLen > 0 ){
		//转为小写
		php_strtolower(joinType,strlen(joinType));
		if(strcmp(joinType,"left") != 0 && strcmp(joinType,"right") != 0 && strcmp(joinType,"inner") != 0){
		}else{
			php_strtoupper(joinType,strlen(joinType));
			defaultJoinType = joinType;
		}
	}

	if(IS_ARRAY == Z_TYPE_P(joinTable)){
		//数组个数
		int paramsNum;
		paramsNum = zend_hash_num_elements(Z_ARRVAL_P(joinTable));

		//一个key-value键值对
		if(1 == paramsNum){
			char *thisKey,
				 tempFrom[10240];
			ulong thisUkey;
			zval **thisVal;
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_key(Z_ARRVAL_P(joinTable),&thisKey,&thisUkey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&thisVal);
			if(IS_STRING != Z_TYPE_PP(thisVal)){
				zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->join] Parameter error ", 5003 TSRMLS_CC);
				return;
			}
			sprintf(tempFrom,"%s%s%s%s%s%s","`",tablePre,thisKey,"` AS `",Z_STRVAL_PP(thisVal),"` ");
			from = estrdup(tempFrom);
		}else if(2 == paramsNum){
			//值为2个数组
			zval **thisKey,
				 **thisVal;
			char tempFrom[10240];
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&thisKey);
			zend_hash_move_forward(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&thisVal);
			if(IS_STRING != Z_TYPE_PP(thisKey) || IS_STRING != Z_TYPE_PP(thisVal) ){
				zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->join] Parameter error ", 5003 TSRMLS_CC);
				return;
			}
			sprintf(tempFrom,"%s%s%s%s%s%s","`",tablePre,Z_STRVAL_PP(thisKey),"` AS `",Z_STRVAL_PP(thisVal),"` ");
			from = estrdup(tempFrom);
		}else if(3 == paramsNum){
			//值为2个数组
			zval **thisKey,
				 **thisVal,
				 **joinTypeZval;
			char tempFrom[10240];
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&thisKey);
			zend_hash_move_forward(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&thisVal);
			zend_hash_move_forward(Z_ARRVAL_P(joinTable));
			zend_hash_get_current_data(Z_ARRVAL_P(joinTable),(void**)&joinTypeZval);
			if(IS_STRING != Z_TYPE_PP(thisKey) || IS_STRING != Z_TYPE_PP(thisVal) ){
				zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->join] Parameter error ", 5003 TSRMLS_CC);
				return;
			}
			sprintf(tempFrom,"%s%s%s%s%s%s","`",tablePre,Z_STRVAL_PP(thisKey),"` AS `",Z_STRVAL_PP(thisVal),"` ");
			from = estrdup(tempFrom);
			if(IS_STRING == Z_TYPE_PP(joinTypeZval)){
				php_strtolower(Z_STRVAL_PP(joinTypeZval),strlen(Z_STRVAL_PP(joinTypeZval)));
				if(strcmp(Z_STRVAL_PP(joinTypeZval),"left") != 0 && strcmp(Z_STRVAL_PP(joinTypeZval),"right") != 0 && strcmp(Z_STRVAL_PP(joinTypeZval),"inner") != 0){
				}else{
					php_strtoupper(Z_STRVAL_PP(joinTypeZval),strlen(Z_STRVAL_PP(joinTypeZval)));
					defaultJoinType = (Z_STRVAL_PP(joinTypeZval));
				}
			}
		}else{
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->join] Parameter error ", 5003 TSRMLS_CC);
			return;
		}
	}else if(IS_STRING == Z_TYPE_P(joinTable)){
		from = estrdup(Z_STRVAL_P(joinTable));
	}

	sprintf(joinTemp,"%s%s%s%s"," ",defaultJoinType," JOIN ",from);
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("joinTemp"),joinTemp TSRMLS_CC);
	efree(from);
	efree(tablePre);
}

PHP_METHOD(CBuilder,on)
{
	int argc = ZEND_NUM_ARGS();
	char *where;

	ZVAL_ZVAL(return_value,getThis(),1,0);

	if(3 == argc){
		char *param1,
			 *param2,
			 tempWhere[10240];
		int paramLen1,
			paramLen2;

		zval *param3;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssz",&param1,&paramLen1,&param2,&paramLen2,&param3) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error ", 5005 TSRMLS_CC);
			return;
		}
		if(IS_LONG == Z_TYPE_P(param3)){
			sprintf(tempWhere,"%s %s %d",param1,param2,Z_LVAL_P(param3));
		}else if(IS_DOUBLE == Z_TYPE_P(param3)){
			sprintf(tempWhere,"%s %s %f",param1,param2,Z_DVAL_P(param3));
		}else if(IS_STRING == Z_TYPE_P(param3)){
			char *filterWhere;
			php_addslashes(Z_STRVAL_P(param3),&filterWhere);
			sprintf(tempWhere,"%s %s %s",param1,param2,filterWhere);
			efree(filterWhere);
		}
		where = estrdup(tempWhere);
	}else if(1 == argc){
		zval *paramArr,
			 **thisVal;
		int paramNum,
			i;
		char *key,
			 *on = "",
			 *valStr,
			 *equl,
			 *tempVal;
		ulong ukey;

		zval *hasLogic;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&paramArr) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->on] Parameter error ", 5004 TSRMLS_CC);
			return;
		}

		//类型不为数组
		if(IS_ARRAY != Z_TYPE_P(paramArr)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->on] Parameter error ", 5004 TSRMLS_CC);
			return;
		}

		//尝试遍历数组
		paramNum = zend_hash_num_elements(Z_ARRVAL_P(paramArr));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(paramArr));
		for(i = 0 ; i < paramNum; i++){

			//判断Key类型
			if(zend_hash_get_current_key_type(Z_ARRVAL_P(paramArr)) != 1){
				zend_hash_move_forward(Z_ARRVAL_P(paramArr));
				continue;
			}

			zend_hash_get_current_key(Z_ARRVAL_P(paramArr),&key,&ukey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(paramArr),(void**)&thisVal);

			//判断VAL类型
			if(IS_STRING == Z_TYPE_PP(thisVal) || IS_LONG == Z_TYPE_PP(thisVal)){}
			else{
				zend_hash_move_forward(Z_ARRVAL_P(paramArr));
				continue;
			}

			//判断key中是否存在逻辑符号
			if(preg_match("/[!=<>]/",key,&hasLogic)){
				equl = " ";
			}else{
				equl = " = ";
			}
			zval_ptr_dtor(&hasLogic);

			if(IS_STRING == Z_TYPE_PP(thisVal)){

				php_addslashes(Z_STRVAL_PP(thisVal),&valStr);
				strcat2(&tempVal," '",valStr,"' ",NULL);
				efree(valStr);

			}else if(IS_LONG == Z_TYPE_PP(thisVal)){
				strcat2(&tempVal," ",Z_LVAL_PP(thisVal)," ",NULL);
			}

			if(i < paramNum - 1){	
				strcat2(&on,on,key,equl,tempVal,"AND ",NULL);
			}else{
				strcat2(&on,on,key,equl,tempVal,NULL);
			}

			efree(tempVal);

			zend_hash_move_forward(Z_ARRVAL_P(paramArr));
		}
		where = on;
	}else{
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->on] Parameter error ", 5004 TSRMLS_CC);
		return;
	}


	//读取joinTemp变量
	MODULE_BEGIN
		zval *joinTempZval,
			 *joinOld;
		char joinResult[10240],
			 *joinOldStr = "";

		joinTempZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("joinTemp"),0 TSRMLS_CC);
		if(IS_NULL == Z_TYPE_P(joinTempZval) || IS_STRING != Z_TYPE_P(joinTempZval) ){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->on] Need to call the join call on method", 5004 TSRMLS_CC);
			return;
		}

		joinOld = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("join"),0 TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(joinOld)){
			joinOldStr = Z_STRVAL_P(joinOld);
		}
		

		if(strlen(joinOldStr) <= 0){
			sprintf(joinResult,"%s %s%s%s",joinOldStr,Z_STRVAL_P(joinTempZval),"ON ",(where));
		}else{
			sprintf(joinResult,"%s %s%s%s",joinOldStr,Z_STRVAL_P(joinTempZval),"ON ",(where));
		}

		//更新join变量
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("join"),joinResult TSRMLS_CC);

		//重置joinTemp变量
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("joinTemp"),"" TSRMLS_CC);
	MODULE_END

	efree(where);

}

PHP_METHOD(CBuilder,where)
{
	int argc = ZEND_NUM_ARGS();
	char *where = NULL;
	zval *whereValue;

	smart_str smarty_where = {0};

	ZVAL_ZVAL(return_value,getThis(),1,0);

	//读取whereVal
	whereValue = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(whereValue)){
		zval		*saveWhereZval;
		MAKE_STD_ZVAL(saveWhereZval);
		array_init(saveWhereZval);
		zend_update_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"),saveWhereZval TSRMLS_CC);
		whereValue = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"),0 TSRMLS_CC);
		zval_ptr_dtor(&saveWhereZval);
	}

	if(3 == argc){

		char *param1,
			 *param2,
			 *tempWhere;
		int paramLen1,
			paramLen2;

		zval *param3,
			 *saveValue;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ssz",&param1,&paramLen1,&param2,&paramLen2,&param3) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error ", 5005 TSRMLS_CC);
			return;
		}

		if(IS_NULL == Z_TYPE_P(param3)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error , Pass the where condition is parsed as empty, which may cause the whole table to be updated", 5005 TSRMLS_CC);
			return;
		}
		
		MAKE_STD_ZVAL(saveValue);
		ZVAL_ZVAL(saveValue,param3,1,0);
		if(IS_LONG == Z_TYPE_P(param3)){

			smart_str_appends(&smarty_where,param1);
			smart_str_appends(&smarty_where," ");
			smart_str_appends(&smarty_where,param2);
			smart_str_appends(&smarty_where," ?");

			add_next_index_zval(whereValue,saveValue);

		}else if(IS_DOUBLE == Z_TYPE_P(param3)){

			smart_str_appends(&smarty_where,param1);
			smart_str_appends(&smarty_where," ");
			smart_str_appends(&smarty_where,param2);
			smart_str_appends(&smarty_where," ?");

			zend_hash_next_index_insert(Z_ARRVAL_P(whereValue),&saveValue,sizeof(zval*),NULL);

		}else if(IS_STRING == Z_TYPE_P(param3)){

			smart_str_appends(&smarty_where,param1);
			smart_str_appends(&smarty_where," ");
			smart_str_appends(&smarty_where,param2);
			smart_str_appends(&smarty_where," ?");

			zend_hash_next_index_insert(Z_ARRVAL_P(whereValue),&saveValue,sizeof(zval*),NULL);

		}else if(IS_ARRAY == Z_TYPE_P(param3)){
			
			int i,num,qnum;
			zval	**thisVal,
					*saveVal;
			char	*waitNeedArray = "";

			smart_str smarty_inWhere = {0};

			
			qnum = zend_hash_num_elements(Z_ARRVAL_P(saveValue));

			zend_hash_internal_pointer_reset(Z_ARRVAL_P(saveValue));

			for(i = 0 ; i < qnum ; i++){

				if(qnum == 1){

					smart_str_appends(&smarty_where,param1);
					smart_str_appends(&smarty_where," = ");
					smart_str_appends(&smarty_where," ? ");

				}else{

					if(i < qnum - 1){
						smart_str_appends(&smarty_inWhere,"?,");
					}else{
						smart_str_appends(&smarty_inWhere,"?");
					}
				}


				zend_hash_get_current_data(Z_ARRVAL_P(saveValue),(void**)&thisVal);
				MAKE_STD_ZVAL(saveVal);
				ZVAL_ZVAL(saveVal,*thisVal,1,0);
				zend_hash_next_index_insert(Z_ARRVAL_P(whereValue),&saveVal,sizeof(zval*),NULL);
				zend_hash_move_forward(Z_ARRVAL_P(saveValue));
			}

			zval_ptr_dtor(&saveValue);
			smart_str_0(&smarty_inWhere);

			if(qnum > 1){
				smart_str_appends(&smarty_where,param1);
				smart_str_appends(&smarty_where," ");
				smart_str_appends(&smarty_where,param2);
				smart_str_appends(&smarty_where," (");
				smart_str_appends(&smarty_where,smarty_inWhere.c);
				smart_str_appends(&smarty_where,") ");
			}

			smart_str_free(&smarty_inWhere);

		}else{
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] the 3 parameter error ", 5005 TSRMLS_CC);
			return;
		}

	}else if(1 == argc){

		zval *paramArr,
			 **thisVal,
			 *saveZval;
		int paramNum,
			i;
		char *key,
			 *on = "",
			 *valStr,
			 *tempSave,
			 *equl;
		ulong ukey;

		zval *hasLogic;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&paramArr) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error ", 5005 TSRMLS_CC);
			return;
		}

		//类型不为数组
		if(IS_ARRAY != Z_TYPE_P(paramArr)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error ", 5005 TSRMLS_CC);
			return;
		}

		//尝试遍历数组
		paramNum = zend_hash_num_elements(Z_ARRVAL_P(paramArr));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(paramArr));
		for(i = 0 ; i < paramNum; i++){

			//判断Key类型
			if(zend_hash_get_current_key_type(Z_ARRVAL_P(paramArr)) != 1){
				zend_hash_move_forward(Z_ARRVAL_P(paramArr));
				continue;
			}

			zend_hash_get_current_key(Z_ARRVAL_P(paramArr),&key,&ukey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(paramArr),(void**)&thisVal);

			//判断VAL类型
			if(IS_STRING == Z_TYPE_PP(thisVal) || IS_LONG == Z_TYPE_PP(thisVal) || IS_DOUBLE == Z_TYPE_PP(thisVal) ){}
			else{
				char *errorMessage;
				char *moreInfo;
				json_encode(paramArr,&moreInfo);
				spprintf(&errorMessage,0,"%s%s","[QueryException] Call [CBuilder->where] Parameter error , the array's value must be string,long,double => %s ",moreInfo);
				zend_throw_exception(CDbExceptionCe, errorMessage, 5005 TSRMLS_CC);
				efree(errorMessage);
				efree(moreInfo);
				return;
			}


			//拼接key

			//判断key中是否存在逻辑符号
			if(preg_match("/[!=<>]/",key,&hasLogic)){
				//equl
				equl = " ";
			}else{
				//equl
				equl = " = ";
			}
			zval_ptr_dtor(&hasLogic);


			if(IS_STRING == Z_TYPE_PP(thisVal)){

				add_next_index_string(whereValue,Z_STRVAL_PP(thisVal),1);
	
			}else if(IS_LONG == Z_TYPE_PP(thisVal) || IS_DOUBLE == Z_TYPE_PP(thisVal) ){

				MAKE_STD_ZVAL(saveZval);
				ZVAL_ZVAL(saveZval,*thisVal,1,0);
				zend_hash_next_index_insert(Z_ARRVAL_P(whereValue),&saveZval,sizeof(zval*),NULL);
			}

			if(i < paramNum - 1){

				smart_str_appends(&smarty_where,key);
				smart_str_appends(&smarty_where,equl);
				smart_str_appends(&smarty_where,"? AND ");


			}else{
				smart_str_appends(&smarty_where,key);
				smart_str_appends(&smarty_where,equl);
				smart_str_appends(&smarty_where,"?");
			}

			zend_hash_move_forward(Z_ARRVAL_P(paramArr));
		}

		
	}else{
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error , must give 3 string or 1 array ", 5005 TSRMLS_CC);
		return;
	}

	smart_str_0(&smarty_where);

	if(IS_NULL != smarty_where.c && strlen(smarty_where.c) > 0){

		//读取已有的where变量
		zval *whereZval;
		smart_str smarty_whereResult = {0};

		whereZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("where"), 0 TSRMLS_CC);

		if(IS_STRING != Z_TYPE_P(whereZval) || (IS_STRING == Z_TYPE_P(whereZval) && Z_STRLEN_P(whereZval) <= 0) ){

			smart_str_appends(&smarty_whereResult," WHERE ");
			smart_str_appends(&smarty_whereResult,smarty_where.c);

		}else{

			smart_str_appends(&smarty_whereResult,Z_STRVAL_P(whereZval));
			smart_str_appends(&smarty_whereResult," AND ");
			smart_str_appends(&smarty_whereResult,smarty_where.c);
		}

		smart_str_0(&smarty_whereResult);

		//保存where变量
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("where"),smarty_whereResult.c TSRMLS_CC);
		
		smart_str_free(&smarty_whereResult);
		smart_str_free(&smarty_where);

	}else{

		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->where] Parameter error , Pass the where condition is parsed as empty, which may cause the whole table to be updated", 5005 TSRMLS_CC);
	}
}

PHP_METHOD(CBuilder,groupBy)
{
	int argc = ZEND_NUM_ARGS();
	char *groupBy = "";

	ZVAL_ZVAL(return_value,getThis(),1,0);

	if(1 == argc){
		char *groupParams,
			 groupTemp[10240];
		int groupParamsLen;
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&groupParams,&groupParamsLen) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->groupBy] Parameter error ", 5006 TSRMLS_CC);
			return;
		}
		if(groupParamsLen > 0){
			sprintf(groupTemp,"%s%s"," GROUP BY ",groupParams);
		}
		groupBy = estrdup(groupTemp);
	}else{
		zval ***args;
		int i;
		char groupTemp[10240],
			 *thisGroupBy = "";

		args = (zval***)safe_emalloc(argc,sizeof(zval**),0);
		if(ZEND_NUM_ARGS() == 0 || zend_get_parameters_array_ex(argc,args) == FAILURE){
		}else{
			//处理任意数目参数
			for(i = 0 ; i < argc ; i++){
				if(IS_STRING == Z_TYPE_PP(args[i])){	
					strcat2(&thisGroupBy,thisGroupBy,Z_STRVAL_PP(args[i]),NULL);
				}else{
					zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->groupBy] When passing multiple GroupBy keyword parameter error", 5006 TSRMLS_CC);
					return;
				}
				if(i < argc - 1) {
					strcat2(&thisGroupBy,thisGroupBy,",",NULL);
				}
			}
		}
		efree(args);
		if(strlen(thisGroupBy) > 0){
			sprintf(groupTemp,"%s%s"," GROUP BY ",thisGroupBy);
			efree(thisGroupBy);
		}
		groupBy = estrdup(groupTemp);
	}

	//更新GroupBy的值
	if(strlen(groupBy) > 0){
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("groupBy"),groupBy TSRMLS_CC);
	}

	efree(groupBy);
}

PHP_METHOD(CBuilder,orderBy)
{
	char	*order,
			*sort,
			*defaultSort = "DESC";
	int		orderLen = 0,
			sortLen = 0;

	ZVAL_ZVAL(return_value,getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&order,&orderLen,&sort,&sortLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->orderBy] Parameter error ", 5007 TSRMLS_CC);
		return;
	}

	if(orderLen <= 0){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->orderBy] Parameter error ", 5007 TSRMLS_CC);
		return;
	}

	if(sortLen > 0){
		defaultSort = sort;
	}

	//读取已有的orderBy变量
	MODULE_BEGIN
		zval *orderByZval;
	
		char orderByResult[10240];

		orderByZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("orderBy"), 0 TSRMLS_CC);
		if(IS_NULL == Z_TYPE_P(orderByZval) || strlen(Z_STRVAL_P(orderByZval)) <= 0 ){
			sprintf(orderByResult,"%s%s%s%s"," ORDER BY ",order," ",defaultSort);
		}else{
			sprintf(orderByResult,"%s%s%s%s%s",Z_STRVAL_P(orderByZval)," , ",order," ",defaultSort);
		}

		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("orderBy"),orderByResult TSRMLS_CC);
	MODULE_END
}


//替换表前缀返回SQL
void CBuilder_replaceTablePre(char *sql,zval *object,char **thisReturnSql TSRMLS_DC)
{
	zval *thisVal,
		 **thisKeyZval;

	char *configName = "main",
		 *useMaster = "0",
		 thisKey[128],
		 *returnSql = "";
	int	 hasExist;

	//获取key
	thisVal = zend_read_property(CBuilderCe,object,ZEND_STRL("configName"),0 TSRMLS_CC);
	configName = Z_STRVAL_P(thisVal);

	thisVal = zend_read_property(CBuilderCe,object,ZEND_STRL("isMaster"),0 TSRMLS_CC);
	useMaster = Z_STRVAL_P(thisVal);

	//配置key
	sprintf(thisKey,"%s%s",useMaster,configName);

	//读取CDatabase下的静态变量
	thisVal = zend_read_static_property(CDatabaseCe,ZEND_STRL("configData"),0 TSRMLS_CC);
	//不存在则去掉前缀
	if(IS_ARRAY != Z_TYPE_P(thisVal)){
		str_replace("MyFrameTablePre_","",sql,&returnSql);
		//更新值
		zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),returnSql TSRMLS_CC);
		*thisReturnSql = estrdup(returnSql);
		efree(returnSql);
		return;
	}

	//有无该配置的key
	hasExist = zend_hash_exists(Z_ARRVAL_P(thisVal),thisKey,strlen(thisKey)+1);
	if(hasExist == 0){
		str_replace("MyFrameTablePre_","",sql,&returnSql);
		//更新值
		zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),(returnSql) TSRMLS_CC);
		*thisReturnSql = estrdup(returnSql);
		efree(returnSql);
		return;
	}

	//获取thisKey中的tablePrefix字段
	zend_hash_find(Z_ARRVAL_P(thisVal),thisKey,strlen(thisKey)+1,(void**)&thisKeyZval);
	if(IS_ARRAY == Z_TYPE_PP(thisKeyZval) && zend_hash_exists(Z_ARRVAL_PP(thisKeyZval),"tablePrefix",strlen("tablePrefix")+1) == 1 ){
		zval **tableZval;
		zend_hash_find(Z_ARRVAL_PP(thisKeyZval),"tablePrefix",strlen("tablePrefix")+1,(void**)&tableZval);
		if(Z_TYPE_PP(tableZval) == IS_STRING && strlen(Z_STRVAL_PP(tableZval)) > 0){

			str_replace("MyFrameTablePre_",Z_STRVAL_PP(tableZval),sql,&returnSql);
		
			//更新值
			zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),returnSql TSRMLS_CC);
			*thisReturnSql = estrdup(returnSql);
			efree(returnSql);
			return;
		}
	}

	str_replace("MyFrameTablePre_","",sql,&returnSql);
	//更新值
	zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),(returnSql) TSRMLS_CC);
	*thisReturnSql = estrdup(returnSql);
	efree(returnSql);
}

//创建SELECT语句
void CBuilder_createSelectSQL(zval *object,char **getStr TSRMLS_DC)
{

	zval *thisVal,
		 *colsZval,
		 *fromZval,
		 *joinZval,
		 *whereZval,
		 *groupByZval,
		 *orderByZval,
		 *limitZval;

	char *tablePre;

	smart_str smarty_sqlStr = {0};

	//cols
	colsZval = zend_read_property(CBuilderCe,object,ZEND_STRL("cols"),0 TSRMLS_CC);
	
	//from
	fromZval = zend_read_property(CBuilderCe,object,ZEND_STRL("from"),0 TSRMLS_CC);
	if(strlen(Z_STRVAL_P(fromZval)) <= 0){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5010 TSRMLS_CC);
		*getStr = NULL;
		return;
	}

	//join
	joinZval = zend_read_property(CBuilderCe,object,ZEND_STRL("join"),0 TSRMLS_CC);

	//where
	whereZval = zend_read_property(CBuilderCe,object,ZEND_STRL("where"),0 TSRMLS_CC);
	
	//groupBy
	groupByZval = zend_read_property(CBuilderCe,object,ZEND_STRL("groupBy"),0 TSRMLS_CC);

	//orderBy
	orderByZval = zend_read_property(CBuilderCe,object,ZEND_STRL("orderBy"),0 TSRMLS_CC);

	//limit
	limitZval = zend_read_property(CBuilderCe,object,ZEND_STRL("limit"),0 TSRMLS_CC);

	//表前缀
	CBuilder_getTablePre(object,&tablePre TSRMLS_CC);

	//拼装SQL
	smart_str_appends(&smarty_sqlStr,"SELECT ");
	if(IS_STRING == Z_TYPE_P(colsZval)){
		smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(colsZval));
	}else{
		smart_str_appends(&smarty_sqlStr,"*");
	}
	smart_str_appends(&smarty_sqlStr," FROM `");
	smart_str_appends(&smarty_sqlStr,tablePre);
	smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(fromZval));

	if(Z_STRLEN_P(joinZval) > 0){
		smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(joinZval));
	}

	smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(whereZval));
	smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(groupByZval));
	smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(orderByZval));
	smart_str_appends(&smarty_sqlStr,Z_STRVAL_P(limitZval));

	smart_str_0(&smarty_sqlStr);

	*getStr = estrdup(smarty_sqlStr.c);

	zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),smarty_sqlStr.c TSRMLS_CC);

	smart_str_free(&smarty_sqlStr);
	efree(tablePre);
}

//创建UPdateSQL语句
void CBuilder_createUpdateSQL(zval *object,char **getStr TSRMLS_DC)
{

	char	*key,
		    *tablePre;
	ulong	ukey;

	int		i,n,whereLen,whereInt;

	zval *fromZval,
		 *thisVal,
		 **thisValZval,
		 *whereZval,
		 *limitZval,
		 *bindParams,
		 *saveParams,
		 *whereValue;

	smart_str  updateStr = {0},
			   sqlStr = {0};


	int clearWhere = 0,clearLimit = 0;

	//from
	fromZval = zend_read_property(CBuilderCe,object,ZEND_STRL("from"),0 TSRMLS_CC);
	if(strlen(Z_STRVAL_P(fromZval)) <= 0){
		*getStr = NULL;
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5010 TSRMLS_CC);
		return;
	}

	//value
	thisVal = zend_read_property(CBuilderCe,object,ZEND_STRL("value"),0 TSRMLS_CC);
	if( IS_ARRAY != Z_TYPE_P(thisVal) ){
		*getStr = NULL;
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter error ", 5018 TSRMLS_CC);
		return;
	}

	MAKE_STD_ZVAL(bindParams);
	array_init(bindParams);

	//遍历值
	n = zend_hash_num_elements(Z_ARRVAL_P(thisVal));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisVal));
	for(i = 0 ; i < n ; i++){

		zend_hash_get_current_key(Z_ARRVAL_P(thisVal),&key,&ukey,0);
		zend_hash_get_current_data(Z_ARRVAL_P(thisVal),(void**)&thisValZval);

		if(IS_STRING == Z_TYPE_PP(thisValZval)){
			/*if(Z_STRLEN_PP(thisValZval) > 65535){
				*getStr = NULL;
				zval_ptr_dtor(&bindParams);
				zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter length exceeds the limit of this method , Please use CDatabase::getInstance()->query(sql)", 5018 TSRMLS_CC);
				return;
			}*/
		}

		//如果待更新的值是整数且为自增自减时
		if(strstr(key,"+") != NULL && (IS_LONG == Z_TYPE_PP(thisValZval) || IS_DOUBLE == Z_TYPE_PP(thisValZval)) ){

			char *replaceMoreKey,
				 *trimReplaceKeyReplace;
	
			smart_str tempUpdateStr = {0};
			str_replace("+","",key,&trimReplaceKeyReplace);
			php_trim(trimReplaceKeyReplace," ",&replaceMoreKey);
			efree(trimReplaceKeyReplace);
	
			smart_str_appends(&tempUpdateStr,"`");
			smart_str_appends(&tempUpdateStr,replaceMoreKey);
			smart_str_appends(&tempUpdateStr,"` = `");
			smart_str_appends(&tempUpdateStr,replaceMoreKey);
			smart_str_appends(&tempUpdateStr,"`");
			smart_str_appends(&tempUpdateStr," + ");
			smart_str_appends(&tempUpdateStr,"?");

			MAKE_STD_ZVAL(saveParams);
			ZVAL_ZVAL(saveParams,*thisValZval,1,0);
			add_next_index_zval(bindParams,saveParams);

			if(i < n - 1){
				smart_str_appends(&tempUpdateStr," ,");
			}

			smart_str_0(&tempUpdateStr);
			smart_str_appends(&updateStr,tempUpdateStr.c);
			smart_str_free(&tempUpdateStr);

			efree(replaceMoreKey);

		}else if(strstr(key,"-") != NULL && (IS_LONG == Z_TYPE_PP(thisValZval) || IS_DOUBLE == Z_TYPE_PP(thisValZval)) ){

			char	*replaceMoreKey,
					*trimReplaceKeyReplace;

			smart_str tempUpdateStr = {0};
			
			str_replace("-","",key,&trimReplaceKeyReplace);
			php_trim(trimReplaceKeyReplace," ",&replaceMoreKey);
			efree(trimReplaceKeyReplace);
	
			smart_str_appends(&tempUpdateStr,"`");
			smart_str_appends(&tempUpdateStr,replaceMoreKey);
			smart_str_appends(&tempUpdateStr,"` = `");
			smart_str_appends(&tempUpdateStr,replaceMoreKey);
			smart_str_appends(&tempUpdateStr,"`");
			smart_str_appends(&tempUpdateStr," - ");
			smart_str_appends(&tempUpdateStr,"?");

			MAKE_STD_ZVAL(saveParams);
			ZVAL_ZVAL(saveParams,*thisValZval,1,0);
			add_next_index_zval(bindParams,saveParams);

			if(i < n - 1){
				smart_str_appends(&tempUpdateStr," ,");
			}
			smart_str_0(&tempUpdateStr);

			smart_str_appends(&updateStr,tempUpdateStr.c);

			smart_str_free(&tempUpdateStr);
			efree(replaceMoreKey);

		}else{

			char *tempUpdate;
			smart_str tempUpdateStr = {0};
			char *inputVal;

			smart_str_appends(&tempUpdateStr," `");
			smart_str_appends(&tempUpdateStr,key);
			smart_str_appends(&tempUpdateStr,"` = ?");
			if(i < n - 1){
				smart_str_appends(&tempUpdateStr,",");
			}

			MAKE_STD_ZVAL(saveParams);
			ZVAL_ZVAL(saveParams,*thisValZval,1,0);
			add_next_index_zval(bindParams,saveParams);

			smart_str_0(&tempUpdateStr);
			smart_str_appends(&updateStr,tempUpdateStr.c);
			smart_str_free(&tempUpdateStr);

		}
		zend_hash_move_forward(Z_ARRVAL_P(thisVal));
	}

	smart_str_0(&updateStr);


	if(strlen(updateStr.c) <= 0){
		*getStr = NULL;
		zval_ptr_dtor(&bindParams);
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter error ,Value type error", 5019 TSRMLS_CC);
		return;
	}

	//where
	whereZval = zend_read_property(CBuilderCe,object,ZEND_STRL("where"),0 TSRMLS_CC);

	//遍历where
	zend_update_property(CBuilderCe,object,ZEND_STRL("value"),bindParams TSRMLS_CC);
	zval_ptr_dtor(&bindParams);

	//limit
	limitZval = zend_read_property(CBuilderCe,object,ZEND_STRL("limit"),0 TSRMLS_CC);

	//表名
	CBuilder_getTablePre(object,&tablePre TSRMLS_CC);


	//拼装SQL
	smart_str_appends(&sqlStr,"UPDATE `");
	smart_str_appends(&sqlStr,tablePre);
	smart_str_appends(&sqlStr,Z_STRVAL_P(fromZval));
	smart_str_appends(&sqlStr," SET");
	smart_str_appends(&sqlStr,updateStr.c);
	smart_str_appends(&sqlStr," ");
	smart_str_appends(&sqlStr,Z_STRVAL_P(whereZval));

	if(strlen(Z_STRVAL_P(limitZval)) > 0 && strcmp(" LIMIT 10000 ",Z_STRVAL_P(limitZval)) != 0 ){
		smart_str_appends(&sqlStr," ");
		smart_str_appends(&sqlStr,Z_STRVAL_P(limitZval));
	}
	smart_str_0(&sqlStr);

	*getStr = estrdup(sqlStr.c);

	smart_str_free(&updateStr);
	smart_str_free(&sqlStr);
	efree(tablePre);
}

//创建插入语句
void CBuilder_createInsertSQL(zval *object,char **getStr TSRMLS_DC)
{
	char	*sql,
			*from,
			*keyStr = "",
			*valStr = "",
			*key,
			*where,
			*sqlTemp,
			*replaceTableSql;

	ulong	ukey;

	int		i,n;

	zval *thisVal,
		 **thisValZval,
		 *fromZval;
	char *tempSql,
		 *keyStrTemp,
		 *valStrTemp;

	char *valTemp;
	char *inputVal,
		 *tablePre;


	//from
	fromZval = zend_read_property(CBuilderCe,object,ZEND_STRL("from"),0 TSRMLS_CC);
	if(Z_STRLEN_P(fromZval) <= 0){
		*getStr = NULL;
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5010 TSRMLS_CC);
		return;
	}

	//value
	thisVal = zend_read_property(CBuilderCe,object,ZEND_STRL("value"),0 TSRMLS_CC);
	if( IS_ARRAY != Z_TYPE_P(thisVal) ){
		*getStr = NULL;
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter error ", 5018 TSRMLS_CC);
		return;
	}

	//判断是索引数组还是关联数组
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisVal));
	zend_hash_get_current_data(Z_ARRVAL_P(thisVal),(void**)&thisValZval);


	//标签缀
	CBuilder_getTablePre(object,&tablePre TSRMLS_CC);


	//二维数据
	if(IS_ARRAY == Z_TYPE_PP(thisValZval) && HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_P(thisVal)) ){

		int j,childNum;
		zval **childZval,
			 *saveValue,
			 *copyValues;

		smart_str smart_keyStr = {0};
		smart_str smart_valStr = {0};
		smart_str smart_sqlStr = {0};

		MAKE_STD_ZVAL(saveValue);
		array_init(saveValue);

		//遍历值
		n = zend_hash_num_elements(Z_ARRVAL_P(thisVal));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisVal));


		for(i = 0 ; i < n ; i++){

			zend_hash_get_current_data(Z_ARRVAL_P(thisVal),(void**)&thisValZval);

			if(IS_ARRAY != Z_TYPE_PP(thisValZval)){
				zend_hash_move_forward(Z_ARRVAL_P(thisVal));
				continue;
			}

			smart_str_appends(&smart_valStr,"(");
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(thisValZval));
			childNum = zend_hash_num_elements(Z_ARRVAL_PP(thisValZval));
			for(j = 0 ; j < childNum ; j++){

				zend_hash_get_current_key(Z_ARRVAL_PP(thisValZval),&key,&ukey,0);
				zend_hash_get_current_data(Z_ARRVAL_PP(thisValZval),(void**)&childZval);

				if(IS_NULL == Z_TYPE_PP(childZval)){
					zend_hash_move_forward(Z_ARRVAL_PP(thisValZval));
					continue;
				}

				if(i == 0){
					
					smart_str_appends(&smart_keyStr,"`");
					smart_str_appends(&smart_keyStr,key);
					if(j < childNum - 1){
						smart_str_appends(&smart_keyStr,"`,");	
					}else{
						smart_str_appends(&smart_keyStr,"`");
					}

				}

				smart_str_appends(&smart_valStr,"?");
				if(j < childNum - 1){
					smart_str_appends(&smart_valStr,",");
				}

				MAKE_STD_ZVAL(copyValues);
				ZVAL_ZVAL(copyValues,*childZval,1,0);
				add_next_index_zval(saveValue,copyValues);
				zend_hash_move_forward(Z_ARRVAL_PP(thisValZval));
			}

			//去掉updateStr最后一位
			if(i < n - 1){
				smart_str_appends(&smart_valStr,"),");
			}else{
				smart_str_appends(&smart_valStr,")");
			}

			zend_hash_move_forward(Z_ARRVAL_P(thisVal));
		}

		//保存value
		zend_update_property(CBuilderCe,object,ZEND_STRL("value"),saveValue TSRMLS_CC);
		zval_ptr_dtor(&saveValue);

		smart_str_0(&smart_keyStr);
		smart_str_0(&smart_valStr);

		smart_str_appends(&smart_sqlStr,"INSERT INTO `");
		smart_str_appends(&smart_sqlStr,tablePre);
		smart_str_appends(&smart_sqlStr,Z_STRVAL_P(fromZval));
		smart_str_appends(&smart_sqlStr,"(");
		smart_str_appends(&smart_sqlStr,smart_keyStr.c);
		smart_str_appends(&smart_sqlStr,") VALUES");
		smart_str_appends(&smart_sqlStr,smart_valStr.c);
		smart_str_0(&smart_sqlStr);
		*getStr = estrdup(smart_sqlStr.c);

		zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),smart_sqlStr.c TSRMLS_CC);

		smart_str_free(&smart_keyStr);
		smart_str_free(&smart_valStr);
		smart_str_free(&smart_sqlStr);
		efree(tablePre);

	}else{

		zval *saveValue,
			 *copyValues;

		smart_str smart_keyStr = {0};
		smart_str smart_valStr = {0};
		smart_str smart_sqlStr = {0};

		MAKE_STD_ZVAL(saveValue);
		array_init(saveValue);

		//遍历值
		n = zend_hash_num_elements(Z_ARRVAL_P(thisVal));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisVal));
		for(i = 0 ; i < n ; i++){

			zend_hash_get_current_key(Z_ARRVAL_P(thisVal),&key,&ukey,0);
			zend_hash_get_current_data(Z_ARRVAL_P(thisVal),(void**)&thisValZval);

			if(IS_NULL == Z_TYPE_PP(thisValZval)){
				zend_hash_move_forward(Z_ARRVAL_P(thisVal));
				continue;
			}	

			smart_str_appends(&smart_keyStr,"`");
			smart_str_appends(&smart_keyStr,key);

			if(i < n - 1){
				smart_str_appends(&smart_keyStr,"`,");	
			}else{
				smart_str_appends(&smart_keyStr,"`");
			}

			smart_str_appends(&smart_valStr,"?");
			
			if(i < n - 1){
				smart_str_appends(&smart_valStr,",");
			}

			MAKE_STD_ZVAL(copyValues);
			ZVAL_ZVAL(copyValues,*thisValZval,1,0);
			add_next_index_zval(saveValue,copyValues);

			zend_hash_move_forward(Z_ARRVAL_P(thisVal));
		}

		smart_str_0(&smart_keyStr);
		smart_str_0(&smart_valStr);

		smart_str_appends(&smart_sqlStr,"INSERT INTO `");
		smart_str_appends(&smart_sqlStr,tablePre);
		smart_str_appends(&smart_sqlStr,Z_STRVAL_P(fromZval));
		smart_str_appends(&smart_sqlStr,"(");
		smart_str_append(&smart_sqlStr,&smart_keyStr);
		smart_str_appends(&smart_sqlStr,") VALUES(");
		smart_str_append(&smart_sqlStr,&smart_valStr);
		smart_str_appends(&smart_sqlStr,")");
		smart_str_0(&smart_sqlStr);

		*getStr = estrdup(smart_sqlStr.c);

		//保存value
		zend_update_property(CBuilderCe,object,ZEND_STRL("value"),saveValue TSRMLS_CC);
		zval_ptr_dtor(&saveValue);

		zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),smart_sqlStr.c TSRMLS_CC);
		
		smart_str_free(&smart_keyStr);
		smart_str_free(&smart_valStr);
		smart_str_free(&smart_sqlStr);
		efree(tablePre);

	}

}

//创建删除语句
void CBuilder_createDeleteSQL(zval *object,char **getStr TSRMLS_DC)
{

	char *tablePre;

	ulong	ukey;

	int		i,n,clearWhere = 0,clearLimit = 0;

	smart_str sqlStr = {0};

	zval *thisVal,
		 **thisValZval,
		 *fromZval,
		 *whereZval,
		 *limitZval;

	char *tempStr;

	//from
	fromZval = zend_read_property(CBuilderCe,object,ZEND_STRL("from"),0 TSRMLS_CC);
	if(Z_STRVAL_P(fromZval) <= 0){
		*getStr = NULL;
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->from] Parameter error ", 5010 TSRMLS_CC);
		return;
	}
	
	//where
	whereZval = zend_read_property(CBuilderCe,object,ZEND_STRL("where"),0 TSRMLS_CC);


	//limit
	limitZval = zend_read_property(CBuilderCe,object,ZEND_STRL("limit"),0 TSRMLS_CC);
	
	//表前缀
	CBuilder_getTablePre(object,&tablePre TSRMLS_CC);


	smart_str_appends(&sqlStr,"DELETE FROM `");
	smart_str_appends(&sqlStr,tablePre);
	smart_str_appends(&sqlStr,Z_STRVAL_P(fromZval));
	smart_str_appends(&sqlStr,Z_STRVAL_P(whereZval));
	if(strlen(Z_STRVAL_P(limitZval)) > 0 && strcmp(" LIMIT 10000 ",Z_STRVAL_P(limitZval)) != 0 ){
		smart_str_appends(&sqlStr,Z_STRVAL_P(limitZval));
	}
	smart_str_0(&sqlStr);

	*getStr = estrdup(sqlStr.c);

	smart_str_free(&sqlStr);
	efree(tablePre);
}

void CBuilder_clearSelf(zval *object TSRMLS_DC){

	if(IS_OBJECT == Z_TYPE_P(object)){

		zend_update_property_string(CBuilderCe,object, ZEND_STRL("action"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("from"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("cols"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("join"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("joinTemp"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("on"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("where"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("val"),"" TSRMLS_CC);
		zend_update_property_null(CBuilderCe,object, ZEND_STRL("value") TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("groupBy"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("orderBy"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("limit")," LIMIT 10000 " TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("_sql"),"" TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("_cache"),"" TSRMLS_CC);
		zend_update_property_long(CBuilderCe,object, ZEND_STRL("_cacheTime"),3600 TSRMLS_CC);
		zend_update_property_string(CBuilderCe,object, ZEND_STRL("prepare"),"" TSRMLS_CC);
		zend_update_property_null(CBuilderCe,object, ZEND_STRL("whereValue") TSRMLS_CC);
	}
}

void CDatabase_setQueryCache(zval *object,zval *result TSRMLS_DC){

	zval	*resultArray,
			*callParams,
			*cacheKey,
			*cacheTime,
			*redisReturn;

	char	*jsonString;

	long	cacheTimeLong = 3600;

	
	cacheKey = zend_read_property(CBuilderCe,object,ZEND_STRL("_cache"), 0 TSRMLS_CC);
	if(strlen(Z_STRVAL_P(cacheKey)) <= 0){
		return;
	}

	resultArray = zend_read_property(CResultCe,result,ZEND_STRL("value"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(resultArray)){
		return;
	}

	cacheTime = zend_read_property(CBuilderCe,object,ZEND_STRL("_cacheTime"), 0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(cacheTime) && Z_LVAL_P(cacheTime) > 0){
		cacheTimeLong = Z_LVAL_P(cacheTime);
	}

	json_encode(resultArray,&jsonString);

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,Z_STRVAL_P(cacheKey),1);
	add_next_index_string(callParams,jsonString,0);
	add_next_index_long(callParams,cacheTimeLong);

	CRedis_callFunction("main","set",callParams,&redisReturn TSRMLS_CC);
	zval_ptr_dtor(&redisReturn);
	zval_ptr_dtor(&callParams);
}

//check return from cache
void CDatabase_checkQueryCache(zval *object,zval **cacheResult TSRMLS_DC)
{
	zval	*cacheKey,
			*callParams,
			*redisReturn,
			*jsonDecode;

	MAKE_STD_ZVAL(*cacheResult);
	cacheKey = zend_read_property(CBuilderCe,object,ZEND_STRL("_cache"), 0 TSRMLS_CC);
	convert_to_string(cacheKey);
	if(strlen(Z_STRVAL_P(cacheKey)) <= 0){
		ZVAL_BOOL(*cacheResult,0);
		return;
	}

	MAKE_STD_ZVAL(callParams);
	array_init(callParams);
	add_next_index_string(callParams,Z_STRVAL_P(cacheKey),1);
	CRedis_callFunction("main","get",callParams,&redisReturn TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(redisReturn)){
		zval_ptr_dtor(&redisReturn);
		zval_ptr_dtor(&callParams);
		ZVAL_BOOL(*cacheResult,0);
		return;
	}

	//decode
	json_decode(Z_STRVAL_P(redisReturn),&jsonDecode);
	ZVAL_ZVAL(*cacheResult,jsonDecode,1,1);
	zval_ptr_dtor(&redisReturn);
	zval_ptr_dtor(&callParams);
}

//实现execute参数 Call 
void CBuilder_executeParam1(zval *params,zval *object,zval **defaultZval TSRMLS_DC)
{
	zval	*prepareZval,
			*pdoObject,
			*configObject,
			*returnZval,
			*pdoMentReturn,
			fetchAllReturn,
			*fetchAllZval,
			*cResultZval;

	char	*configName = "main",
			*sql;

	MAKE_STD_ZVAL(*defaultZval);

	prepareZval = zend_read_property(CBuilderCe,object,ZEND_STRL("prepare"), 0 TSRMLS_CC);
	if(IS_STRING != Z_TYPE_P(prepareZval) || strlen(Z_STRVAL_P(prepareZval)) <= 0 ){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->execute(array condition)] Function, you must prepare () sets the executing SQL", 5021 TSRMLS_CC);
		return;
	}
	sql = Z_STRVAL_P(prepareZval);

	//获取PDO对象
	configObject = zend_read_property(CBuilderCe,object,ZEND_STRL("configName"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configObject)){
		configName = Z_STRVAL_P(configObject);
	}

	CDatabase_getDatabase(configName,1,&pdoObject TSRMLS_CC);

	//保存SQL
	zend_update_property_string(CBuilderCe,object,ZEND_STRL("_sql"),sql TSRMLS_CC);

	//调用PDO方法执行prepare方法
	MODULE_BEGIN
		zval	pdoAction,
				pdoReturn,
				dataReturn,
				*beginTime,
				*endTime,
				*paramList[3],
				param1,
				*savePdoReturn;

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"prepare",0);


		//触发查询前Hooks函数
		MODULE_BEGIN
			zval *paramsList[1],
				  param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],object,1,0);
			CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		MODULE_END

		microtime(&beginTime);
		
		//预执行
		paramList[0] = &param1;
		MAKE_STD_ZVAL(paramList[0]);
		ZVAL_STRING(paramList[0],sql,1);
		call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
		zval_ptr_dtor(&paramList[0]);

		//执行
		returnZval = &pdoReturn;

		if(IS_OBJECT != Z_TYPE_P(returnZval)){
			char *errorMessage;
			zval_ptr_dtor(&beginTime);
			zval_ptr_dtor(&pdoObject);
			strcat2(&errorMessage,"[QueryException] Call [CBuilder->prepare] Parameter error [",sql,"]",NULL);
			zend_throw_exception(CDbExceptionCe, errorMessage, 5021 TSRMLS_CC);
			efree(errorMessage);
			return;
		}

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"execute",0);
		MAKE_STD_ZVAL(paramList[0]);
		ZVAL_ZVAL(paramList[0],params,1,0);
		call_user_function(NULL, &returnZval, &pdoAction, &dataReturn,1, paramList TSRMLS_CC);
		pdoMentReturn = &dataReturn;
		zval_ptr_dtor(&paramList[0]);
		microtime(&endTime);	

		if(IS_BOOL == Z_TYPE_P(pdoMentReturn) && Z_LVAL_P(pdoMentReturn) == 0 ){
			char errorMessage[10240];
			zval_ptr_dtor(&beginTime);
			zval_ptr_dtor(&pdoObject);
			zval_ptr_dtor(&endTime);
			sprintf(errorMessage,"%s%s%s","[QueryException] Call [CBuilder->prepare] Parameter error [",sql,"]");
			zend_throw_exception(CDbExceptionCe, errorMessage, 5022 TSRMLS_CC);
			ZVAL_FALSE(*defaultZval);
			return;
		}

		//有异常则报告致命错误
		if(EG(exception)){
			//确定异常类是否为PDOException
			zend_class_entry *exceptionCe;
			exceptionCe = Z_OBJCE_P(EG(exception));
			if(strcmp(exceptionCe->name,"PDOException") == 0){					
				//读取其错误信息
				zval *exceptionMessage;
				char *errMessage;
				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&pdoObject);
				zval_ptr_dtor(&endTime);
				zval_dtor(&dataReturn);

				exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
				strcat2(&errMessage,"[QueryException]",Z_STRVAL_P(exceptionMessage)," - SQL[",sql,"]",NULL);
				Z_OBJ_HANDLE_P(EG(exception)) = 0;
				zend_clear_exception(TSRMLS_C);
				zend_throw_exception(CDbExceptionCe, errMessage, 1001 TSRMLS_CC);
				efree(errMessage);
				return;
			}
		}

		//调用fetchAll获取值
		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"fetchAll",0);
		call_user_function(NULL, &returnZval, &pdoAction, &fetchAllReturn,0, NULL TSRMLS_CC);
		fetchAllZval = &fetchAllReturn;
		MAKE_STD_ZVAL(savePdoReturn);
		ZVAL_ZVAL(savePdoReturn,fetchAllZval,1,0);

		//组装cResult对象
		MAKE_STD_ZVAL(cResultZval);
		object_init_ex(cResultZval,CResultCe);

		//向该对象设置值
		zend_update_property_string(CResultCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
		zend_update_property(CResultCe,cResultZval,ZEND_STRL("value"),savePdoReturn TSRMLS_CC);
		zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isFromCache"),0 TSRMLS_CC);
		zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isMaster"),1 TSRMLS_CC);
		zend_update_property_double(CResultCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

		//执行查询后的Hooks
		MODULE_BEGIN
			zval *paramsList[1],
				param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
			CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		MODULE_END

		ZVAL_ZVAL(*defaultZval,cResultZval,1,1);
	
		zval_ptr_dtor(&pdoObject);
		zval_ptr_dtor(&beginTime);
		zval_ptr_dtor(&endTime);
		zval_ptr_dtor(&savePdoReturn);
		zval_dtor(&pdoReturn);
		zval_dtor(&dataReturn);
		zval_dtor(&fetchAllReturn);

	MODULE_END


	return;
}

PHP_METHOD(CBuilder,execute)
{
	int argc = ZEND_NUM_ARGS();

	zval *pdoObject = NULL;


	if(1 == argc){
		//执行带有参数的方法
		zval *returnZval,
			 *params;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&params) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->execute(array condition)] Function, no Call method and the corresponding parameter type", 5021 TSRMLS_CC);
			return;
		}
		if(Z_TYPE_P(params) != IS_ARRAY){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->execute(array condition)] Function, no Call method and the corresponding parameter type", 5021 TSRMLS_CC);
			return;
		}

		CBuilder_executeParam1(params , getThis(),&returnZval TSRMLS_CC);
		if(IS_OBJECT == Z_TYPE_P(returnZval)){
			RETVAL_ZVAL(returnZval,1,1);
			return;
		}else{
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->execute(array condition)] Function, return the result failed", 5024 TSRMLS_CC);
			return;
		}
	}else{

		//执行不带参数方法
		zval	*actionZval,
				*masterZval,
				*runActionZval;
		char	*action,
				*configName;
		int		useMaster = 0;

		configName = "main";

		//配置名称
		actionZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"), 0 TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(actionZval) && strlen(Z_STRVAL_P(actionZval)) >0 ){
			configName = Z_STRVAL_P(actionZval);
		}

		//是否强制使用主库
		masterZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("isMaster"), 0 TSRMLS_CC);
		if(IS_STRING == Z_TYPE_P(masterZval) && strcmp(Z_STRVAL_P(masterZval),"1") == 0 ){
			useMaster = 1;
		}

		//动作名称
		runActionZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("action"), 0 TSRMLS_CC);
		if(IS_STRING != Z_TYPE_P(runActionZval)){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Object [CBuilder] parameter is flawed, CQuickFramework cannot execute the DB operation", 5008 TSRMLS_CC);
			return;
		}
		php_trim(Z_STRVAL_P(runActionZval)," ",&action);

		//判断操作类型
		if(strcmp(action,"SELECT") == 0){

			int thisUseMaster = 0,
				fromMaster = 0;

			char	*sql,
					*cacheKey;

			zval	*thisGetZval,
					*beginTime,
					*endTime,
					*pdoReturnZval,
					*cResultZval,
					*resultZval,
					*whereValue,
					*saveResult,
					pdoResultGet,
					*cacheResult;


			//获取PDO对象
			if(useMaster == 1){
				thisUseMaster = 1;
				fromMaster = 1;
			}

			//check cache
			CDatabase_checkQueryCache(getThis(),&cacheResult TSRMLS_CC);
			if(IS_ARRAY == Z_TYPE_P(cacheResult)){
				//has get success
				MAKE_STD_ZVAL(cResultZval);
				object_init_ex(cResultZval,CResultCe);
				zend_update_property_string(CResultCe,cResultZval,ZEND_STRL("sql"),"" TSRMLS_CC);
				zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isFromCache"),1 TSRMLS_CC);
				zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isMaster"),0 TSRMLS_CC);
				zend_update_property_null(CResultCe,cResultZval,ZEND_STRL("whereValue") TSRMLS_CC);
				zend_update_property(CResultCe,cResultZval,ZEND_STRL("value"),cacheResult TSRMLS_CC);
				zend_update_property_double(CResultCe,cResultZval,ZEND_STRL("castTime"),0 TSRMLS_CC);
				CBuilder_clearSelf(getThis() TSRMLS_CC);
				RETVAL_ZVAL(cResultZval,1,1);
				zval_ptr_dtor(&cacheResult);
				efree(action);
				return;
			}
			zval_ptr_dtor(&cacheResult);
		

			CDatabase_getDatabase(configName,thisUseMaster,&pdoObject TSRMLS_CC);

			if(pdoObject == NULL || Z_TYPE_P(pdoObject) != IS_OBJECT){
				efree(action);
				zval_ptr_dtor(&pdoObject);
				return;
			}

			//创建select语句
			CBuilder_createSelectSQL(getThis(),&sql TSRMLS_CC);

			if(sql == NULL){
				efree(action);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, "[QueryException] CQuickFramework can not complete the analysis of the SQL statement ", 5008 TSRMLS_CC);
				return;
			}

			//触发查询前Hooks函数
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],getThis(),1,0);
				CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			//调用PDO prepare 方法
			MODULE_BEGIN
				zval pdoAction,
					 pdoReturn,
					 *paramList[3],
					 param1,
					 param2,
					 param3;

				INIT_ZVAL(pdoAction);
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_STRING(paramList[0],sql,1);
				ZVAL_STRING(&pdoAction,"prepare",0);
				microtime(&beginTime);
				call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
				zval_ptr_dtor(&paramList[0]);
				pdoReturnZval = &pdoReturn;

				//where条件
				whereValue = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"), 0 TSRMLS_CC);

				//执行
				MODULE_BEGIN
					zval setAction,
						 setReturen,
						 *setParamList[1],
						 setParam1;

					INIT_ZVAL(setAction);
					setParamList[0] = &setParam1;
					MAKE_STD_ZVAL(setParamList[0]);
					ZVAL_ZVAL(setParamList[0],whereValue,1,0);
					ZVAL_STRING(&setAction,"execute",0);
					call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
					zval_ptr_dtor(&setParamList[0]);
				
					//发生错误
					if(IS_BOOL == Z_TYPE(setReturen) && 0 == Z_LVAL(setReturen) ){
						
						//抛出异常前触发HOOKS_EXECUTE_ERROR钩子函数
						zval	*paramsList[1],
								*dbErrorObject,
								param1,
								errAction,
								 errorMessage,
								*showError;
						char	errorMessageStr[10240];


						INIT_ZVAL(errAction);
						ZVAL_STRING(&errAction,"errorInfo",0);
						call_user_function(NULL, &pdoReturnZval, &errAction, &errorMessage, 0, NULL TSRMLS_CC);
						if(zend_hash_num_elements(Z_ARRVAL(errorMessage)) > 0){
								zval **errorData;
								zend_hash_move_forward(Z_ARRVAL(errorMessage));
								zend_hash_move_forward(Z_ARRVAL(errorMessage));
								zend_hash_get_current_data(Z_ARRVAL(errorMessage),(void**)&errorData);
								if(IS_STRING == Z_TYPE_PP(errorData)){
									sprintf(errorMessageStr,"%s%s%s%s%s","[SQLException] Execute SQL when an exception occurs[",Z_STRVAL_PP(errorData),"] - SQL[",sql,"]");
								}else{
									sprintf(errorMessageStr,"%s%s%s","[SQLException] Execute SQL when an exception occurs - SQL[",sql,"]");
								}
						}else{
							sprintf(errorMessageStr,"%s%s%s","[SQLException] Execute SQL when an exception occurs - SQL[",sql,"]");
						}

						MAKE_STD_ZVAL(dbErrorObject);
						object_init_ex(dbErrorObject,CDbErrorCe);
						zend_update_property_string(CDbErrorCe,dbErrorObject,"_errorMessage",strlen("_errorMessage"),errorMessageStr TSRMLS_CC);
						zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("_sql"),sql TSRMLS_CC);
						zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("action"),"SELECT" TSRMLS_CC);

						paramsList[0] = &param1;
						MAKE_STD_ZVAL(paramsList[0]);
						ZVAL_ZVAL(paramsList[0],dbErrorObject,1,0);
						CHooks_callHooks("HOOKS_EXECUTE_ERROR",paramsList,1 TSRMLS_CC);
						zval_ptr_dtor(&dbErrorObject);
						zval_ptr_dtor(&paramsList[0]);
						zval_dtor(&errorMessage);
						//抛出异常

						efree(sql);
						efree(action);
						zval_ptr_dtor(&pdoObject);
						zval_dtor(&pdoReturn);
						zval_ptr_dtor(&beginTime);
						CBuilder_clearSelf(getThis() TSRMLS_CC);


						zend_throw_exception(CDbExceptionCe, (errorMessageStr), 5010 TSRMLS_CC);
						return;
					}
					zval_dtor(&setReturen);

				MODULE_END


				//设置返回值方式为PDO::FETCH_OBJ
				MODULE_BEGIN
					zval setAction,
						 setReturen,
						 *setParamList[1],
						 setParam1;
					INIT_ZVAL(setAction);
					setParamList[0] = &setParam1;
					MAKE_STD_ZVAL(setParamList[0]);
					ZVAL_LONG(setParamList[0],2);
					ZVAL_STRING(&setAction,"setFetchMode",0);
					call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
					zval_ptr_dtor(&setParamList[0]);
					zval_dtor(&setReturen);
				MODULE_END


				//实例 查询对象结果 并设置值
				MAKE_STD_ZVAL(cResultZval);
				object_init_ex(cResultZval,CResultCe);
				zend_update_property_string(CResultCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
				zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isFromCache"),0 TSRMLS_CC);
				zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isMaster"),fromMaster TSRMLS_CC);
				zend_update_property(CResultCe,cResultZval,ZEND_STRL("whereValue"),whereValue TSRMLS_CC);

				//获取结果集
				MODULE_BEGIN
					zval fetchAction,
						 fetchReturen,
						 *saveResult,
						 *returnPointer,
						 *test1;
					INIT_ZVAL(fetchAction);

					ZVAL_STRING(&fetchAction,"fetchAll",0);
					call_user_function(NULL, &pdoReturnZval, &fetchAction, &pdoResultGet, 0, NULL TSRMLS_CC);
					returnPointer = &pdoResultGet;
					MAKE_STD_ZVAL(saveResult);
					ZVAL_ZVAL(saveResult,returnPointer,1,0);
					zend_update_property(CResultCe,cResultZval,ZEND_STRL("value"),saveResult TSRMLS_CC);	
					zval_ptr_dtor(&saveResult);
					zval_dtor(&pdoResultGet);
				MODULE_END
				microtime(&endTime);

				zend_update_property_double(CResultCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&endTime);
			
				//执行查询后的Hooks
				MODULE_BEGIN
					zval *paramsList[1],
						  param1;
					paramsList[0] = &param1;
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
				MODULE_END

				//set to cache
				CDatabase_setQueryCache(getThis(),cResultZval TSRMLS_CC);

				//清理自身
				CBuilder_clearSelf(getThis() TSRMLS_CC);
				efree(sql);

				//返回对象
				RETVAL_ZVAL(cResultZval,1,1);
				zval_ptr_dtor(&pdoObject);
				zval_dtor(&pdoReturn);
			
			MODULE_END

		}else if(strcmp(action,"UPDATE") == 0){

			int thisUseMaster = 1,
				fromMaster = 1,
				changeRow = 0;

			char	*sql,
					*cacheKey;

			zval	*thisGetZval,
					*beginTime,
					*endTime,
					*pdoReturnZval,
					*pdoReturnZvalTemp,
					*cResultZval,
					*resultZval,
					*whereValue,
					*saveParams,
					*bindParams;

			zval	setAction,
					setReturen,
					*setParamList[1],
					setParam1,
					*setReturnZval,
					**thisValZval,
					*saveWhereZval;

			int		whereLen,whereInt;

			//获取PDO对象
			thisUseMaster = 1;
			fromMaster = 1;

			CDatabase_getDatabase(configName,thisUseMaster,&pdoObject TSRMLS_CC);

			if(pdoObject == NULL || Z_TYPE_P(pdoObject) != IS_OBJECT){
				efree(action);
				return;
			}

			//创建update语句
			CBuilder_createUpdateSQL(getThis(),&sql TSRMLS_CC);

			if(sql == NULL){
				efree(action);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, "[QueryException] CQuickFramework can not complete the analysis of the SQL statement ", 5008 TSRMLS_CC);
				return;
			}


			//触发查询前Hooks函数
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],getThis(),1,0);
				CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			//values值
			bindParams = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);

			//where条件
			whereValue = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"), 0 TSRMLS_CC);
			whereLen = zend_hash_num_elements(Z_ARRVAL_P(whereValue));
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(whereValue));
			for(whereInt = 0 ; whereInt < whereLen;whereInt++){
				zend_hash_get_current_data(Z_ARRVAL_P(whereValue),(void**)&thisValZval);
				MAKE_STD_ZVAL(saveWhereZval);
				ZVAL_ZVAL(saveWhereZval,*thisValZval,1,0);
				add_next_index_zval(bindParams,saveWhereZval);
				zend_hash_move_forward(Z_ARRVAL_P(whereValue));
			}

			//调用PDO prepare 方法
			MODULE_BEGIN
				zval pdoAction,
					 pdoReturn,
					 rsReturn,
					 *paramList[3],
					 param1,
					 param2,
					 param3,
					 *rsZval;

				INIT_ZVAL(pdoAction);
			
				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_STRING(paramList[0],sql,1);

				ZVAL_STRING(&pdoAction,"prepare",0);
				microtime(&beginTime);
				call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
				zval_ptr_dtor(&paramList[0]);
			
				pdoReturnZval = &pdoReturn;

				//调用execute方法
				INIT_ZVAL(setAction);
				setParamList[0] = &setParam1;
				MAKE_STD_ZVAL(setParamList[0]);
				ZVAL_ZVAL(setParamList[0],bindParams,1,0);
				ZVAL_STRING(&setAction,"execute",0);
				call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
				zval_ptr_dtor(&setParamList[0]);
			
				setReturnZval = &setReturen;
				microtime(&endTime);

				//发生错误
				if(IS_BOOL == Z_TYPE_P(setReturnZval) && 0 == Z_LVAL_P(setReturnZval) ){
					char errorMessageStr[10240],
						 *sqlReturnErr = "";

					zval	errorInfoAction,
							errorInfoReturn,
							*errInfo,
							*dbErrorObject,
							*paramsList[1];

					//获取errorInfo
					ZVAL_STRING(&errorInfoAction,"errorInfo",0);
					call_user_function(NULL, &pdoReturnZval, &errorInfoAction, &errorInfoReturn, 0, NULL TSRMLS_CC);
					errInfo = &errorInfoReturn;
					if(IS_ARRAY == Z_TYPE_P(errInfo) && 3 == zend_hash_num_elements(Z_ARRVAL_P(errInfo)) ){
						zval **returnHashVal;
						zend_hash_index_find(Z_ARRVAL_P(errInfo),2,(void*)&returnHashVal);
						if(IS_STRING == Z_TYPE_PP(returnHashVal)){
							sqlReturnErr = Z_STRVAL_PP(returnHashVal);
						}
					}
		
					sprintf(errorMessageStr,"%s%s%s%s%s","[SQLException] ",sqlReturnErr," in SQL[",sql,"]");

					MAKE_STD_ZVAL(dbErrorObject);
					object_init_ex(dbErrorObject,CDbErrorCe);
					zend_update_property_string(CDbErrorCe,dbErrorObject,"_errorMessage",strlen("_errorMessage"),errorMessageStr TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("_sql"),sql TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("action"),"UPDATE" TSRMLS_CC);

					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],dbErrorObject,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_ERROR",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&dbErrorObject);
					zval_ptr_dtor(&paramsList[0]);

					zend_throw_exception(CDbExceptionCe, errorMessageStr, 5010 TSRMLS_CC);
					zval_dtor(&errorInfoReturn);
					efree(sql);
					efree(action);
					zval_ptr_dtor(&pdoObject);
					zval_ptr_dtor(&endTime);
					zval_ptr_dtor(&beginTime);
					zval_dtor(&pdoReturn);
					zval_dtor(&setReturen);
					CBuilder_clearSelf(getThis() TSRMLS_CC);

					RETVAL_FALSE;
					return;
				}

				//返回影响行数
				MODULE_BEGIN
					zval setAction,
						 setReturen;
					INIT_ZVAL(setAction);
					ZVAL_STRING(&setAction,"rowCount",0);
					call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 0, NULL TSRMLS_CC);
					if(IS_LONG == Z_TYPE(setReturen)){
						changeRow = Z_LVAL(setReturen);
					}
					zval_dtor(&setReturen);
				MODULE_END
			
				//实例 操作对象结果 并设置值
				MAKE_STD_ZVAL(cResultZval);
				object_init_ex(cResultZval,CExecCe);

				//向该对象设置值
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"UPDATE" TSRMLS_CC);
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
				zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),changeRow TSRMLS_CC);
				zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),0 TSRMLS_CC);
				zend_update_property(CExecCe,cResultZval,ZEND_STRL("whereValue"),bindParams TSRMLS_CC);
				zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

				//执行查询后的Hooks
				MODULE_BEGIN
					zval *paramsList[1],
						  param1;
					paramsList[0] = &param1;
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
				MODULE_END

				//清理自身
				CBuilder_clearSelf(getThis() TSRMLS_CC);

				//返回对象
				RETVAL_ZVAL(cResultZval,1,1);

				efree(sql);
				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&endTime);
				zval_ptr_dtor(&pdoObject);
				zval_dtor(&pdoReturn);
				zval_dtor(&setReturen);

			MODULE_END

		}else if(strcmp(action,"INSERT INTO") == 0){

			int thisUseMaster = 1,
				fromMaster = 1,
				rowCount = 1;

			char	*sql,
					*cacheKey,
					*lastInsertId = "0";

			zval	*thisGetZval,
					*beginTime,
					*endTime,
					*pdoReturnZval,
					*cResultZval,
					*resultZval,
					*values;

			zval	setAction,
					setReturen,
					*setParamList[1],
					setParam1,
					*setReturnZval;

			//获取PDO对象
			thisUseMaster = 1;
			fromMaster = 1;

			CDatabase_getDatabase(configName,thisUseMaster,&pdoObject TSRMLS_CC);

			if(pdoObject == NULL || Z_TYPE_P(pdoObject) != IS_OBJECT){
				zval_ptr_dtor(&pdoObject);
				efree(action);
				zend_throw_exception(CDbExceptionCe, "[QueryException] CQuickFramework can not get database driver ", 5008 TSRMLS_CC);
				return;
			}

			//创建update语句
			CBuilder_createInsertSQL(getThis(),&sql TSRMLS_CC);

			if(sql == NULL){
				efree(action);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, "[QueryException] CQuickFramework can not complete the analysis of the SQL statement ", 5008 TSRMLS_CC);
				return;
			}

			//数据
			values = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("value"), 0 TSRMLS_CC);

			//触发查询前Hooks函数
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],getThis(),1,0);
				CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			//调用PDO prepare 方法
			MODULE_BEGIN
				zval pdoAction,
					 pdoReturn,
					 *paramList[3],
					 param1,
					 param2,
					 param3;

				INIT_ZVAL(pdoAction);
				paramList[0] = &param1;

				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_STRING(paramList[0],sql,1);

				ZVAL_STRING(&pdoAction,"prepare",0);
				microtime(&beginTime);
				call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
				zval_ptr_dtor(&paramList[0]);
				pdoReturnZval = &pdoReturn;

				//调用execute方法
				INIT_ZVAL(setAction);
				MAKE_STD_ZVAL(setParamList[0]);
				ZVAL_ZVAL(setParamList[0],values,1,0);
				ZVAL_STRING(&setAction,"execute",0);
				call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
				zval_ptr_dtor(&setParamList[0]);
				setReturnZval = &setReturen;
				microtime(&endTime);

				//发生错误
				if(IS_BOOL == Z_TYPE_P(setReturnZval) && 0 == Z_LVAL_P(setReturnZval) ){
					
					char errorMessageStr[10240],
						 *sqlReturnErr = "";

					zval	errorInfoAction,
							errorInfoReturn,
							*errInfo,
							*dbErrorObject,
							*paramsList[1];

					//获取errorInfo
					ZVAL_STRING(&errorInfoAction,"errorInfo",0);
					call_user_function(NULL, &pdoReturnZval, &errorInfoAction, &errorInfoReturn, 0, NULL TSRMLS_CC);
					errInfo = &errorInfoReturn;
					if(IS_ARRAY == Z_TYPE_P(errInfo) && 3 == zend_hash_num_elements(Z_ARRVAL_P(errInfo)) ){
						zval **returnHashVal;
						zend_hash_index_find(Z_ARRVAL_P(errInfo),2,(void**)&returnHashVal);
						if(IS_STRING == Z_TYPE_PP(returnHashVal)){
							sqlReturnErr = Z_STRVAL_PP(returnHashVal);
						}
					}


					sprintf(errorMessageStr,"%s%s%s%s%s","[SQLException] ",sqlReturnErr," in SQL[",sql,"]");

					MAKE_STD_ZVAL(dbErrorObject);
					object_init_ex(dbErrorObject,CDbErrorCe);
					zend_update_property_string(CDbErrorCe,dbErrorObject,"_errorMessage",strlen("_errorMessage"),errorMessageStr TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("_sql"),sql TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("action"),"UPDATE" TSRMLS_CC);

					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],dbErrorObject,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_ERROR",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&dbErrorObject);
					zval_ptr_dtor(&paramsList[0]);

					zend_throw_exception(CDbExceptionCe, errorMessageStr, 5010 TSRMLS_CC);
					efree(sql);
					efree(action);
					zval_ptr_dtor(&pdoObject);
					zval_dtor(&errorInfoReturn);
					zval_ptr_dtor(&endTime);
					zval_ptr_dtor(&beginTime);
					zval_dtor(&pdoReturn);
					zval_dtor(&setReturen);
					CBuilder_clearSelf(getThis() TSRMLS_CC);
					return;
				}

				
				//获取最后插入ID
				MODULE_BEGIN
					zval	lastInsertAction,
							lastInserReturn;
					INIT_ZVAL(lastInsertAction);
					ZVAL_STRING(&lastInsertAction, "lastInsertId", 0);
					call_user_function(NULL, &pdoObject, &lastInsertAction, &lastInserReturn, 0, NULL TSRMLS_CC);
					if(IS_STRING == Z_TYPE(lastInserReturn)){
						lastInsertId = estrdup(Z_STRVAL(lastInserReturn));
					}else{
						lastInsertId = estrdup("0");
					}
					zval_dtor(&lastInserReturn);
				MODULE_END

				//影响行数
				MODULE_BEGIN
					zval	actionRun,
							actionReturn;
					INIT_ZVAL(actionRun);
					ZVAL_STRING(&actionRun, "rowCount", 0);
					call_user_function(NULL, &pdoReturnZval, &actionRun, &actionReturn, 0, NULL TSRMLS_CC);
					if(IS_LONG == Z_TYPE(actionReturn)){
						rowCount = Z_LVAL(actionReturn);
					}else{
						rowCount = 1;
					}
					zval_dtor(&actionReturn);
				MODULE_END

				//实例 操作对象结果 并设置值
				MAKE_STD_ZVAL(cResultZval);
				object_init_ex(cResultZval,CExecCe);

				//向该对象设置值
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"INSERT" TSRMLS_CC);
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
				zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),rowCount TSRMLS_CC);
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),lastInsertId TSRMLS_CC);
				zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&endTime);
				efree(lastInsertId);

				//执行查询后的Hooks
				MODULE_BEGIN
					zval *paramsList[1],
						  param1;
					paramsList[0] = &param1;
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
				MODULE_END

				//清理自身
				CBuilder_clearSelf(getThis() TSRMLS_CC);

				efree(sql);

				//返回对象
				RETVAL_ZVAL(cResultZval,1,1);
				zval_ptr_dtor(&pdoObject);
				zval_dtor(&pdoReturn);
			
			MODULE_END

		}else if(strcmp(action,"DELETE") == 0){

			int thisUseMaster = 1,
				fromMaster = 1,
				changeRow = 0;

			char	*sql,
					*cacheKey;

			zval	*thisGetZval,
					*beginTime,
					*endTime,
					*pdoReturnZval,
					*cResultZval,
					*resultZval,
					*whereValue;

			zval	setAction,
					setReturen,
					*setParamList[1],
					setParam1,
					*setReturnZval;
			

			//获取PDO对象
			thisUseMaster = 1;
			fromMaster = 1;

			CDatabase_getDatabase(configName,thisUseMaster,&pdoObject TSRMLS_CC);

			if(pdoObject == NULL || Z_TYPE_P(pdoObject) != IS_OBJECT){
				efree(action);
				return;
			}

			//创建update语句
			CBuilder_createDeleteSQL(getThis(),&sql TSRMLS_CC);

			if(sql == NULL){
				efree(action);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, "[QueryException] CQuickFramework can not complete the analysis of the SQL statement ", 5008 TSRMLS_CC);
				return;
			}

			//触发查询前Hooks函数
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],getThis(),1,0);
				CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			//where条件
			whereValue = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("whereValue"),0 TSRMLS_CC);

			//调用PDO prepare 方法
			MODULE_BEGIN
				zval pdoAction,
					 pdoReturn,
					 *paramList[3],
					 param1,
					 param2,
					 param3;

				INIT_ZVAL(pdoAction);

				paramList[0] = &param1;
				MAKE_STD_ZVAL(paramList[0]);
				ZVAL_STRING(paramList[0],sql,1);

				ZVAL_STRING(&pdoAction,"prepare",0);
				microtime(&beginTime);
				call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
				zval_ptr_dtor(&paramList[0]);
				pdoReturnZval = &pdoReturn;

				//调用execute方法
				INIT_ZVAL(setAction);
				setParamList[0] = &setParam1;
				MAKE_STD_ZVAL(setParamList[0]);
				ZVAL_ZVAL(setParamList[0],whereValue,1,0);
				ZVAL_STRING(&setAction,"execute",0);
				call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
				zval_ptr_dtor(&setParamList[0]);
				setReturnZval = &setReturen;
				microtime(&endTime);

				//发生错误
				if(IS_BOOL == Z_TYPE_P(setReturnZval) && 0 == Z_LVAL_P(setReturnZval) ){
					char errorMessageStr[10240],
						 *sqlReturnErr = "";

					zval	errorInfoAction,
							errorInfoReturn,
							*errInfo,
							*dbErrorObject,
							*paramsList[1];

					//获取errorInfo
					ZVAL_STRING(&errorInfoAction,"errorInfo",0);
					call_user_function(NULL, &pdoReturnZval, &errorInfoAction, &errorInfoReturn, 0, NULL TSRMLS_CC);
					errInfo = &errorInfoReturn;
					if(IS_ARRAY == Z_TYPE_P(errInfo) && 3 == zend_hash_num_elements(Z_ARRVAL_P(errInfo)) ){
						zval **returnHashVal;
						zend_hash_index_find(Z_ARRVAL_P(errInfo),2,(void**)&returnHashVal);
						if(IS_STRING == Z_TYPE_PP(returnHashVal)){
							sqlReturnErr = Z_STRVAL_PP(returnHashVal);
						}
					};

					sprintf(errorMessageStr,"%s%s%s%s%s","[SQLException] ",sqlReturnErr," in SQL[",sql,"]");

					MAKE_STD_ZVAL(dbErrorObject);
					object_init_ex(dbErrorObject,CDbErrorCe);
					zend_update_property_string(CDbErrorCe,dbErrorObject,"_errorMessage",strlen("_errorMessage"),errorMessageStr TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("_sql"),sql TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("action"),"UPDATE" TSRMLS_CC);

					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],dbErrorObject,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_ERROR",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&dbErrorObject);
					zval_ptr_dtor(&paramsList[0]);

					zend_throw_exception(CDbExceptionCe, errorMessageStr, 5010 TSRMLS_CC);

					efree(sql);
					efree(action);
					zval_ptr_dtor(&pdoObject);
					zval_ptr_dtor(&endTime);
					zval_ptr_dtor(&beginTime);
					zval_dtor(&errorInfoReturn);
					zval_dtor(&pdoReturn);
					zval_dtor(&setReturen);
					CBuilder_clearSelf(getThis() TSRMLS_CC);

					RETVAL_FALSE;
					return;
				}

				//返回影响行数
				MODULE_BEGIN
					zval setAction,
						 setReturen,
						 *test;
					INIT_ZVAL(setAction);
					ZVAL_STRING(&setAction,"rowCount",0);
					call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 0, NULL TSRMLS_CC);
					if(IS_LONG == Z_TYPE(setReturen)){
						changeRow = Z_LVAL(setReturen);
					}
					zval_dtor(&setReturen);
				MODULE_END
				
				//实例 操作对象结果 并设置值
				MAKE_STD_ZVAL(cResultZval);
				object_init_ex(cResultZval,CExecCe);

				//向该对象设置值
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"DELETE" TSRMLS_CC);
				zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
				zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),changeRow TSRMLS_CC);
				zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),0 TSRMLS_CC);
				zend_update_property(CExecCe,cResultZval,ZEND_STRL("whereValue"),whereValue TSRMLS_CC);
				zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

				//执行查询后的Hooks
				MODULE_BEGIN
					zval *paramsList[1],
						  param1;
					paramsList[0] = &param1;
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
				MODULE_END

				efree(sql);

				//清理自身
				CBuilder_clearSelf(getThis() TSRMLS_CC);

				//返回对象
				RETVAL_ZVAL(cResultZval,1,1);

				zval_ptr_dtor(&endTime);
				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&pdoObject);
				zval_dtor(&pdoReturn);
				zval_dtor(&setReturen);

			MODULE_END
		}else{
			efree(action);
			zend_throw_exception(CDbExceptionCe, "[CDbException] Not specified operating mode or operation mode does not exist", 5010 TSRMLS_CC);
			RETVAL_FALSE;
			return;
		}

		efree(action);
	}
}

PHP_METHOD(CBuilder,getSql)
{
	zval *returnZval;

	returnZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("_sql"), 0 TSRMLS_CC);
	if(Z_TYPE_P(returnZval) == IS_STRING){
		RETVAL_STRING(Z_STRVAL_P(returnZval),1);
	}else{
		RETVAL_STRING("",0);
	}
}

//设置缓存
PHP_METHOD(CBuilder,cache)
{
	char		*cacheKey;
	long		cacheTime = 3600;
	int			cacheKeyLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&cacheKey,&cacheKeyLen,&cacheTime) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->cache] Parameter error ", 5007 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("_cache"),cacheKey TSRMLS_CC);
	zend_update_property_long(CBuilderCe,getThis(),ZEND_STRL("_cacheTime"),cacheTime TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);

}

PHP_METHOD(CBuilder,prepare)
{
	//获取参数个数
	int argc = ZEND_NUM_ARGS(),
		i = 0,
		sqlLen = 0;

	char *sql;

	RETVAL_ZVAL(getThis(),1,0);

	if(argc != 1){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->prepare] Parameter error ", 5019 TSRMLS_CC);
		return;
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&sql,&sqlLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->prepare] Parameter error ", 5019 TSRMLS_CC);
		return;
	}

	//更新值
	zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("prepare"),sql TSRMLS_CC);
}

//直接执行SQL语句
PHP_METHOD(CBuilder,query)
{
	char *sql;
	int sqlLen = 0;

	zval *pdoObject,
		 *configObject,
		 *returnZval,
		 *beginTime,
		 *endTime,
		 *pdoReturnZval,
		 *resultZval,
		 *cResultZval;

	char *configName = "main";

	int fromMaster = 1;

	double castTime = 0.0;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&sql,&sqlLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->query] Parameter error ", 5018 TSRMLS_CC);
		return;
	}

	//配置名称
	configObject = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configObject)){
		configName = Z_STRVAL_P(configObject);
	}

	CDatabase_getDatabase(configName,1,&pdoObject TSRMLS_CC);

	//调用执行钩子前函数
	MODULE_BEGIN
		zval	*paramsList[1],
				param1;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],getThis(),1,0);
		CHooks_callHooks("HOOKS_EXECUTE_BEFORE",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	//调用PDO执行QUERY方法
	MODULE_BEGIN
		zval pdoAction,
			 pdoReturn,
			 *paramList[3],
			 param1,
			 param2,
			 param3;

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"query",0);
		paramList[0] = &param1;
		MAKE_STD_ZVAL(paramList[0]);
		ZVAL_STRING(paramList[0],sql,1);
		microtime(&beginTime);
		call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,1, paramList TSRMLS_CC);
		zval_ptr_dtor(&paramList[0]);
		pdoReturnZval = &pdoReturn;

		//发生错误
		if(IS_BOOL == Z_TYPE_P(pdoReturnZval)){
			//尝试获取SQL错误提示
			zval errAction,
				errorMessage,
				*errorObject;
			INIT_ZVAL(errAction);
			ZVAL_STRING(&errAction,"errorInfo",0);
			call_user_function(NULL, &pdoObject, &errAction, &errorMessage, 0, NULL TSRMLS_CC);

			if(zend_hash_num_elements(Z_ARRVAL(errorMessage)) > 0){
				char *errorText,
						*errorMessageStr;
				zval **errorData;
				zend_hash_move_forward(Z_ARRVAL(errorMessage));
				zend_hash_move_forward(Z_ARRVAL(errorMessage));
				zend_hash_get_current_data(Z_ARRVAL(errorMessage),(void**)&errorData);
				errorText = estrdup(Z_STRVAL_PP(errorData));

				//抛出异常前触发HOOKS_EXECUTE_ERROR钩子函数
				MODULE_BEGIN
					zval	*paramsList[1],
							*dbErrorObject,
							param1;
					MAKE_STD_ZVAL(dbErrorObject);
					object_init_ex(dbErrorObject,CDbErrorCe);
					zend_update_property_string(CDbErrorCe,dbErrorObject,"_errorMessage",strlen("_errorMessage"),errorText TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("_sql"),sql TSRMLS_CC);
					zend_update_property_string(CDbErrorCe,dbErrorObject,ZEND_STRL("action"),"SELECT" TSRMLS_CC);
			
					MAKE_STD_ZVAL(paramsList[0]);
					ZVAL_ZVAL(paramsList[0],dbErrorObject,1,0);
					CHooks_callHooks("HOOKS_EXECUTE_ERROR",paramsList,1 TSRMLS_CC);
					zval_ptr_dtor(&paramsList[0]);
					zval_ptr_dtor(&dbErrorObject);
				MODULE_END

				zval_dtor(&errorMessage);
				spprintf(&errorMessageStr,0,"%s%s%s%s%s","[SQLException]",errorText," - SQL[",sql,"]");
				efree(errorText);
				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, errorMessageStr, 5010 TSRMLS_CC);
				efree(errorMessageStr);
				RETVAL_FALSE;
				return;
			}else{
				char errMessage[10240];
				sprintf(errMessage,"%s%s","[ModelException] CQuickFramework execute SQL [%s] there was an error, CQuickFramework couldn't get the reason for the error from the PDO object",sql);
				zval_dtor(&errorMessage);
				zval_ptr_dtor(&beginTime);
				zval_ptr_dtor(&pdoObject);
				zend_throw_exception(CDbExceptionCe, errMessage, 5021 TSRMLS_CC);
				return;
			}
		}


		//设置返回值方式为PDO::FETCH_ARR
		MODULE_BEGIN
			zval setAction,
				setReturen,
				*setParamList[1],
				setParam1;
			INIT_ZVAL(setAction);
			setParamList[0] = &setParam1;
			MAKE_STD_ZVAL(setParamList[0]);
			ZVAL_LONG(setParamList[0],2);
			ZVAL_STRING(&setAction,"setFetchMode",0);
			call_user_function(NULL, &pdoReturnZval, &setAction, &setReturen, 1, setParamList TSRMLS_CC);
			zval_ptr_dtor(&setParamList[0]);
			zval_dtor(&setReturen);
		MODULE_END

		//实例 查询对象结果 并设置值
		MAKE_STD_ZVAL(cResultZval);
		object_init_ex(cResultZval,CResultCe);

		//获取结果集
		MODULE_BEGIN
			zval fetchAction,
				fetchReturen,
				*saveTemp;
			INIT_ZVAL(fetchAction);
			ZVAL_STRING(&fetchAction,"fetchAll",0);
			call_user_function(NULL, &pdoReturnZval, &fetchAction, &fetchReturen, 0, NULL TSRMLS_CC);
			saveTemp = &fetchReturen;
			MAKE_STD_ZVAL(resultZval);
			ZVAL_ZVAL(resultZval,saveTemp,1,0);
			zend_update_property(CResultCe,cResultZval,ZEND_STRL("value"),resultZval TSRMLS_CC);
			zval_ptr_dtor(&resultZval);
			zval_dtor(&fetchReturen);
		MODULE_END

		microtime(&endTime);
		castTime = Z_DVAL_P(endTime)-Z_DVAL_P(beginTime);

		//向该对象设置值
		zend_update_property_string(CResultCe,cResultZval,ZEND_STRL("sql"),sql TSRMLS_CC);
		zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isFromCache"),0 TSRMLS_CC);
		zend_update_property_bool(CResultCe,cResultZval,ZEND_STRL("isMaster"),fromMaster TSRMLS_CC);
		zend_update_property_double(CResultCe,cResultZval,ZEND_STRL("castTime"),castTime TSRMLS_CC);

		//执行查询后的Hooks
		MODULE_BEGIN
			zval *paramsList[1],
				param1;
			paramsList[0] = &param1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
			CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
		MODULE_END

		//返回对象
		ZVAL_ZVAL(return_value,cResultZval,1,1);

		zval_dtor(&pdoReturn);

	MODULE_END

	zval_ptr_dtor(&pdoObject);
	zval_ptr_dtor(&beginTime);
	zval_ptr_dtor(&endTime);
}

PHP_METHOD(CBuilder,value)
{
	zval *valueZval;

	ZVAL_ZVAL(return_value,getThis(),1,0);
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&valueZval) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter error ", 5014 TSRMLS_CC);
		return;
	}

	if(IS_ARRAY != Z_TYPE_P(valueZval) || ZEND_NUM_ARGS() != 1 ){
		zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->value] Parameter error ", 5014 TSRMLS_CC);
		return;
	}

	//更新保存value的值
	zend_update_property(CBuilderCe,getThis(),ZEND_STRL("value"),valueZval TSRMLS_CC);
}


PHP_METHOD(CBuilder,beginTransaction)
{
	zval *pdoObject,
		 *configObject,
		 *returnZval;
	char *configName = "main";

	//配置名称
	configObject = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configObject)){
		configName = Z_STRVAL_P(configObject);
	}

	CDatabase_getDatabase(configName,1,&pdoObject TSRMLS_CC);

	//调用pdo开启事务方法
	MODULE_BEGIN
		zval	pdoAction,
				pdoReturn,
				*beginTime,
				*endTime;

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"beginTransaction",0);

		microtime(&beginTime);
		call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,0, NULL TSRMLS_CC);
		microtime(&endTime);

		//有异常则报告致命错误
		if(EG(exception)){
			//确定异常类是否为PDOException
			zend_class_entry *exceptionCe;

			exceptionCe = Z_OBJCE_P(EG(exception));
			if(strcmp(exceptionCe->name,"PDOException") == 0){
							
				//读取其错误信息
				zval *exceptionMessage;
				char errMessage[10240];
				exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
				sprintf(errMessage,"%s%s","[TransactionException] There was an error begin transaction:",Z_STRVAL_P(exceptionMessage));
				Z_OBJ_HANDLE_P(EG(exception)) = 0;
				zend_clear_exception(TSRMLS_C);
				zend_throw_exception(CDbExceptionCe, errMessage, 5021 TSRMLS_CC);
				return;

			}
		}

		//调用执行完Hooks函数
		MODULE_BEGIN
			zval *cResultZval;
			MAKE_STD_ZVAL(cResultZval);
			object_init_ex(cResultZval,CExecCe);

			//向该对象设置值
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"BEGINTRANSACTION" TSRMLS_CC);
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),"PDO->beginTransaction" TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),0 TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),0 TSRMLS_CC);
			zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

			//执行查询后的Hooks
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
				CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			zval_ptr_dtor(&cResultZval);

		MODULE_END

		zval_dtor(&pdoReturn);

		zval_ptr_dtor(&beginTime);
		zval_ptr_dtor(&endTime);

	MODULE_END

	RETVAL_ZVAL(pdoObject,1,1);
}

PHP_METHOD(CBuilder,commit)
{
	zval *pdoObject,
		 *configObject,
		 *returnZval;
	char *configName = "main";

	//配置名称
	configObject = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configObject)){
		configName = Z_STRVAL_P(configObject);
	}

	CDatabase_getDatabase(configName,1,&pdoObject TSRMLS_CC);

	//调用pdo开启事务方法
	MODULE_BEGIN
		zval	pdoAction,
				pdoReturn,
				*beginTime,
				*endTime;

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"commit",0);

		microtime(&beginTime);
		call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,0, NULL TSRMLS_CC);
		microtime(&endTime);

		//有异常则报告致命错误
		if(EG(exception)){
			//确定异常类是否为PDOException
			zend_class_entry *exceptionCe;

			exceptionCe = Z_OBJCE_P(EG(exception));
			if(strcmp(exceptionCe->name,"PDOException") == 0){
							
				//读取其错误信息
				zval *exceptionMessage;
				char errMessage[10240];
				exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
				sprintf(errMessage,"%s%s","[TransactionException] An error occurred while trying to commit the transaction:",Z_STRVAL_P(exceptionMessage));
				Z_OBJ_HANDLE_P(EG(exception)) = 0;
				zend_clear_exception(TSRMLS_C);
				zend_throw_exception(CDbExceptionCe, errMessage, 5021 TSRMLS_CC);
				return;
			}
		}

		//调用执行完Hooks函数
		MODULE_BEGIN
			zval *cResultZval;

			MAKE_STD_ZVAL(cResultZval);
			object_init_ex(cResultZval,CExecCe);

			//向该对象设置值
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"COMMIT" TSRMLS_CC);
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),"PDO->commit" TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),0 TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),0 TSRMLS_CC);
			zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

			//执行查询后的Hooks
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
				CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			zval_ptr_dtor(&cResultZval);

		MODULE_END


		zval_dtor(&pdoReturn);
		zval_ptr_dtor(&beginTime);
		zval_ptr_dtor(&endTime);


	MODULE_END

	zval_ptr_dtor(&pdoObject);

}

PHP_METHOD(CBuilder,rollback)
{
	zval *pdoObject,
		 *configObject,
		 *returnZval;
	char *configName = "main";

	//配置名称
	configObject = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(configObject)){
		configName = Z_STRVAL_P(configObject);
	}

	CDatabase_getDatabase(configName,1,&pdoObject TSRMLS_CC);

	//调用pdo开启事务方法
	MODULE_BEGIN
		zval	pdoAction,
				pdoReturn,
				*beginTime,
				*endTime;

		INIT_ZVAL(pdoAction);
		ZVAL_STRING(&pdoAction,"rollback",0);

		microtime(&beginTime);
		call_user_function(NULL, &pdoObject, &pdoAction, &pdoReturn,0, NULL TSRMLS_CC);
		microtime(&endTime);

		//有异常则报告致命错误
		if(EG(exception)){
			//确定异常类是否为PDOException
			zend_class_entry *exceptionCe;

			exceptionCe = Z_OBJCE_P(EG(exception));
			if(strcmp(exceptionCe->name,"PDOException") == 0){
							
				//读取其错误信息
				zval *exceptionMessage;
				char errMessage[10240];
				exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
				sprintf(errMessage,"%s%s","[TransactionException] An error occurred while trying to roll back the transaction:",Z_STRVAL_P(exceptionMessage));
				Z_OBJ_HANDLE_P(EG(exception)) = 0;
				zend_clear_exception(TSRMLS_C);
				zend_throw_exception(CDbExceptionCe, errMessage, 5021 TSRMLS_CC);
				return;
			}
		}

		//调用执行完Hooks函数
		MODULE_BEGIN
			zval *cResultZval;
			MAKE_STD_ZVAL(cResultZval);
			object_init_ex(cResultZval,CExecCe);

			//向该对象设置值
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("action"),"ROLLBACK" TSRMLS_CC);
			zend_update_property_string(CExecCe,cResultZval,ZEND_STRL("sql"),"PDO->rollback" TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("rows"),0 TSRMLS_CC);
			zend_update_property_long(CExecCe,cResultZval,ZEND_STRL("lastInsertId"),0 TSRMLS_CC);
			zend_update_property_double(CExecCe,cResultZval,ZEND_STRL("castTime"),Z_DVAL_P(endTime) - Z_DVAL_P(beginTime) TSRMLS_CC);

			//执行查询后的Hooks
			MODULE_BEGIN
				zval *paramsList[1],
					  param1;
				paramsList[0] = &param1;
				MAKE_STD_ZVAL(paramsList[0]);
				ZVAL_ZVAL(paramsList[0],cResultZval,1,0);
				CHooks_callHooks("HOOKS_EXECUTE_END",paramsList,1 TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
			MODULE_END

			zval_ptr_dtor(&cResultZval);

		MODULE_END

		zval_dtor(&pdoReturn);
		zval_ptr_dtor(&beginTime);
		zval_ptr_dtor(&endTime);

	MODULE_END

	zval_ptr_dtor(&pdoObject);
}

PHP_METHOD(CBuilder,limit)
{
	int argc = ZEND_NUM_ARGS();

	char *limitStr = "";

	RETVAL_ZVAL(getThis(),1,0);

	if(argc == 1){
		char tempLimit[10240];

		zval *limit;

		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&limit) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->limit] Parameter error ", 5022 TSRMLS_CC);
			return;
		}
		if(IS_STRING == Z_TYPE_P(limit)){
			sprintf(tempLimit,"%s%s"," LIMIT ",Z_STRVAL_P(limit));
		}else if(IS_LONG == Z_TYPE_P(limit)){
			sprintf(tempLimit,"%s%d"," LIMIT ",Z_LVAL_P(limit));
		}else{
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->limit] Parameter error ", 5022 TSRMLS_CC);
			return;
		}
		limitStr = estrdup(tempLimit);

	}else if(argc == 2){

		char tempLimit[10240];
		
		zval	*offset,
				*limit;

		char offsetTempStr[10240],
			 limitTempStr[10240],
			 *offsetVal,
			 *limitVal;


		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz",&offset,&limit) == FAILURE){
			zend_throw_exception(CDbExceptionCe, "[QueryException] Call [CBuilder->limit] Parameter error ", 5022 TSRMLS_CC);
			return;
		}

		if(IS_LONG == Z_TYPE_P(offset)){
			sprintf(offsetTempStr,"%d",Z_LVAL_P(offset));
			offsetVal = estrdup(offsetTempStr);
		}else if(IS_STRING == Z_TYPE_P(offset)){
			offsetVal = estrdup(Z_STRVAL_P(offset));
		}

		if(IS_LONG == Z_TYPE_P(limit)){
			sprintf(limitTempStr,"%d",Z_LVAL_P(limit));
			limitVal = estrdup(limitTempStr);
		}else if(IS_STRING == Z_TYPE_P(limit)){
			limitVal = estrdup(Z_STRVAL_P(limit));
		}


		sprintf(tempLimit,"%s%s%s%s"," LIMIT ",offsetVal,",",limitVal);
		limitStr = estrdup(tempLimit);
	}
	
	if(strlen(limitStr) > 0){
		zend_update_property_string(CBuilderCe,getThis(),ZEND_STRL("limit"),limitStr TSRMLS_CC);
	}
	efree(limitStr);
}

//设置检查存货的间隔 默认20秒
PHP_METHOD(CBuilder,setConnCheckInterval)
{

}

//立即检查数据连接状态
PHP_METHOD(CBuilder,getLastActiveTime)
{

}

//立即销毁当前PDO对象
PHP_METHOD(CBuilder,destory)
{
	zval	*actionZval,
			*masterZval;

	char	*configName;
	int		useMaster = 0,
			clearStatus = 0;


	//获取当前的数据配置
	actionZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("configName"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(actionZval) && strlen(Z_STRVAL_P(actionZval)) >0 ){
		configName = Z_STRVAL_P(actionZval);
	}

	//是否强制使用主库
	masterZval = zend_read_property(CBuilderCe,getThis(),ZEND_STRL("isMaster"), 0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(masterZval) && strcmp(Z_STRVAL_P(masterZval),"1") == 0 ){
		useMaster = 1;
	}

	//删除对象
	clearStatus = CDatabase_deleteDatabase(useMaster,configName TSRMLS_CC);

	RETVAL_BOOL(clearStatus);
}