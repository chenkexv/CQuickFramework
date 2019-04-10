#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_CMyFrameExtension.h"
#include "Zend/zend_list.h"
#include "Zend/zend_interfaces.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


void preg_replace(char *find,char * replace,char *str,char **returnStr){

	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_STRING(params[0],find,1);
	ZVAL_STRING(params[1],replace,1);
	ZVAL_STRING(params[2],str,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_replace",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(str);
	return;
}

int php_t_strftime(char *format,char *timestamp,zval **thisReturnZval){

	zval	returnZval,
			*params[2],
			function,
			*sthisReturnZval;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],format,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],timestamp,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"strftime",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	
	sthisReturnZval = &returnZval;
	
	MAKE_STD_ZVAL(*thisReturnZval);
	ZVAL_ZVAL(*thisReturnZval,sthisReturnZval,1,0);
	zval_dtor(&returnZval);
}

int php_strtotime(char *string){

	zval	returnZval,
			*params[1],
			function,
			*sthisReturnZval;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],string,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"strtotime",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	if(IS_LONG == Z_TYPE(returnZval)){
		zval_dtor(&returnZval);
		return Z_LVAL(returnZval);
	}else{
		zval_dtor(&returnZval);
		return 0;
	}
}

void htmlspecialchars(char *string,char **val)
{
	zval	returnZval,
			*params[1],
			function,
			*sthisReturnZval;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],string,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"htmlspecialchars",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*val = estrdup(Z_STRVAL(returnZval));
	}else{
		*val = estrdup(string);
	}
}

int php_define(char *string,char *val){

	zval	returnZval,
			*params[2],
			function,
			*sthisReturnZval;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],string,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],val,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"define",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(IS_LONG == Z_TYPE(returnZval)){
		zval_dtor(&returnZval);
		return Z_LVAL(returnZval);
	}else{
		zval_dtor(&returnZval);
		return 0;
	}
}

int php_eval(char *string){


}

void php_t_mktime(int s1,int s2,int s3,int s4,int s5,int s6,zval **thisReturnZval){

	zval	returnZval,
			*params[6],
			function,
			*sthisReturnZval;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	MAKE_STD_ZVAL(params[3]);
	MAKE_STD_ZVAL(params[4]);
	MAKE_STD_ZVAL(params[5]);
	ZVAL_LONG(params[0],s1);
	ZVAL_LONG(params[1],s2);
	ZVAL_LONG(params[2],s3);
	ZVAL_LONG(params[3],s4);
	ZVAL_LONG(params[4],s5);
	ZVAL_LONG(params[5],s6);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"mktime",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval, 6, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_ptr_dtor(&params[3]);
	zval_ptr_dtor(&params[4]);
	zval_ptr_dtor(&params[5]);

	sthisReturnZval = &returnZval;
	
	MAKE_STD_ZVAL(*thisReturnZval);
	ZVAL_ZVAL(*thisReturnZval,sthisReturnZval,1,0);
	zval_dtor(&returnZval);

}

void php_t_sprintf(char *find,char * replace,char **returnStr){

	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],find,1);
	ZVAL_STRING(params[1],replace,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"sprintf",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(replace);
	return;
}

//调用PHP str_replace函数
void str_replace(char *find,char * replace,char *str,char **returnStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_STRING(params[0],find,1);
	ZVAL_STRING(params[1],replace,1);
	ZVAL_STRING(params[2],str,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"str_replace",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(str);
	return;
}

int array_count_values(zval *array,zval **regAllResult){
	zval	returnZval,
			*tempRetrun,
			*params[1],
			param1,
			function;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);

	ZVAL_ZVAL(params[0],array,1,0);
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"array_count_values",0);
	if(SUCCESS != call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC)){
		MAKE_STD_ZVAL(*regAllResult);
		array_init(*regAllResult);
		return 0;
	}

	tempRetrun = &returnZval;
	MAKE_STD_ZVAL(*regAllResult);
	ZVAL_ZVAL(*regAllResult,tempRetrun,1,0);
	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);

	return 1;
}

void str_replaceArray(zval *find,zval *replace,char *str,char **returnStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_ZVAL(params[0],find,1,0);
	ZVAL_ZVAL(params[1],replace,1,0);
	ZVAL_STRING(params[2],str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"str_replace",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);


	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(str);
	return;
}

void php_strtr(char *find,zval *replace,char **returnStr)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],find,1);
	ZVAL_ZVAL(params[1],replace,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"strtr",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);


	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(find);
	return;
}

void preg_quote(char *reg,char * replaceOffset,char **returnStr){
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_quote",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(reg);
	return;
}

//调用PHP preg_repalce
void preg_repalce(char *reg,char * replaceOffset,char *waitToReplace,char **returnStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);
	ZVAL_STRING(params[2],waitToReplace,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_replace",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(waitToReplace);
	return;
}

void preg_repalceArray(zval *reg,zval *replaceOffset,char *waitToReplace,char **returnStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_ZVAL(params[0],reg,1,0);
	ZVAL_ZVAL(params[1],replaceOffset,1,0);
	ZVAL_STRING(params[2],waitToReplace,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_repalce",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(waitToReplace);
	return;
}

//调用preg_match函数
int preg_match(char *reg,char * replaceOffset, zval **regAllResult)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;
	HashTable *returnTable;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);


	ALLOC_HASHTABLE(returnTable);
	zend_hash_init(returnTable,8,NULL,NULL,0);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	Z_TYPE_P(params[2]) = IS_ARRAY;
	Z_ARRVAL_P(params[2]) = returnTable;

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_match",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	*regAllResult = params[2];

	return Z_LVAL(returnZval);
}

int function_exists(char *fun){
	int		i,n;
	char	*funLowName;
	zend_function *requsetAction;
	TSRMLS_FETCH();
	funLowName = estrdup(fun);
	php_strtolower(funLowName,strlen(funLowName)+1);
	if(zend_hash_find(EG(function_table),funLowName,strlen(funLowName)+1,(void**)&requsetAction) == SUCCESS){
		efree(funLowName);
		return 1;
	}

	efree(funLowName);
	return 0;
}

int method_exists(zval *object,char *fun){

	int		i,n;
	char	*funLowName;
	zend_function *requsetAction;
	TSRMLS_FETCH();
	funLowName = estrdup(fun);
	php_strtolower(funLowName,strlen(funLowName)+1);
	if(zend_hash_find( &(Z_OBJCE_P(object)->function_table),funLowName,strlen(funLowName)+1,(void**)&requsetAction) == SUCCESS){
		efree(funLowName);
		return 1;
	}

	efree(funLowName);
	return 0;
}

int is_callable(zval *callInfo)
{
	zval	*params[1],
			param1,
			function,
			returnZval;

	TSRMLS_FETCH();

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],callInfo,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"is_callable",0);
	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC)){
		if(IS_BOOL == Z_TYPE(returnZval) && 1 == Z_BVAL(returnZval) ){
			zval_dtor(&returnZval);
			zval_ptr_dtor(&params[0]);
			return 1;
		}
	}

	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);

	return 0;
}

int preg_split1(char *reg,char *replaceOffset,zval **regAllResult)
{
	zval	returnZval,
			*tempRetrun,
			*params[2],
			param1,
			param2,
			function;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_split",0);
	if(SUCCESS != call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC)){
		MAKE_STD_ZVAL(*regAllResult);
		zval_dtor(&returnZval);
		array_init(*regAllResult);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		return 0;
	}

	tempRetrun = &returnZval;

	MAKE_STD_ZVAL(*regAllResult);
	ZVAL_ZVAL(*regAllResult,tempRetrun,1,0);
	zval_dtor(&returnZval);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	return 1;
}

int preg_split(char *reg,char *replaceOffset,int params2,int params3,zval **regAllResult)
{
	zval	returnZval,
			*tempRetrun,
			*params[4],
			param1,
			param2,
			param3,
			param4,
			function;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	params[3] = &param4;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	MAKE_STD_ZVAL(params[3]);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);
	ZVAL_LONG(params[2],params2);
	ZVAL_LONG(params[3],params3);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_split",0);
	if(SUCCESS != call_user_function(EG(function_table), NULL, &function, &returnZval, 4, params TSRMLS_CC)){
		MAKE_STD_ZVAL(*regAllResult);
		array_init(*regAllResult);
		return 0;
	}

	tempRetrun = &returnZval;

	MAKE_STD_ZVAL(*regAllResult);
	ZVAL_ZVAL(*regAllResult,tempRetrun,1,0);
	zval_dtor(&returnZval);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_ptr_dtor(&params[3]);

	return 1;
}

//调用preg_match函数
int preg_match_all(char *reg,char * replaceOffset, zval **regAllResult)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;
	HashTable *returnTable;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);


	ALLOC_HASHTABLE(returnTable);
	zend_hash_init(returnTable,8,NULL,NULL,0);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	Z_TYPE_P(params[2]) = IS_ARRAY;
	Z_ARRVAL_P(params[2]) = returnTable;

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_match_all",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	*regAllResult = params[2];

	return Z_LVAL(returnZval);
}

int preg_match_all_ex(char *reg,char * replaceOffset,int flags, zval **regAllResult)
{
	zval	returnZval,
			*params[4],
			param1,
			param2,
			param3,
			param4,
			function;

	int status = FAILURE;
	HashTable *returnTable;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	params[3] = &param4;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	MAKE_STD_ZVAL(params[3]);


	ALLOC_HASHTABLE(returnTable);
	zend_hash_init(returnTable,8,NULL,NULL,0);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	Z_TYPE_P(params[2]) = IS_ARRAY;
	Z_ARRVAL_P(params[2]) = returnTable;

	ZVAL_LONG(params[3],flags);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"preg_match_all",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,4, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[3]);

	*regAllResult = params[2];

	return Z_LVAL(returnZval);
}


int json_decode(char *reg, zval **array)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],reg,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_BOOL(params[1],1);

	MAKE_STD_ZVAL(*array);


	INIT_ZVAL(function);
	ZVAL_STRING(&function,"json_decode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	
	if(IS_ARRAY == Z_TYPE(returnZval)){
		zval *tempSave;
		tempSave = &returnZval;
		ZVAL_ZVAL(*array,tempSave,1,0);
	}else{
		array_init(*array);
	}

	zval_dtor(&returnZval);
	return SUCCESS;
}

int json_encode(zval *getParams, char **returnString)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],getParams,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"json_encode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	
	if(IS_STRING == Z_TYPE(returnZval)){
		*returnString = estrdup(Z_STRVAL(returnZval));
	}else{
		*returnString = estrdup("[]");
	}

	zval_dtor(&returnZval);
	return SUCCESS;
}

int in_array(char *reg, zval *array)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],reg,1);
	ZVAL_ZVAL(params[1],array,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"in_array",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status =  Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}


//调用PHP trim
void php_trim(char *reg,char * replaceOffset,char **returnStr)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],reg,1);
	ZVAL_STRING(params[1],replaceOffset,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"trim",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(reg);
	return;
}

//通过SPL注册自动载入
void spl_autoload_register(zval *classMethod TSRMLS_DC)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],classMethod,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"spl_autoload_register",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}


//判断扩展载入
int extension_loaded(char *reg)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;
	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],reg,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"extension_loaded",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}


int ini_set(char *param1Str,char *param2Str)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],param1Str,1);
	ZVAL_STRING(params[1],param2Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ini_set",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void ini_get(char *param1Str,char **param2Str)
{
	char	*str;
	str = zend_ini_string(param1Str, strlen(param1Str)+1, 0);
	*param2Str = estrdup(str);
}

int ini_seti(char *param1Str,int param2Str)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],param1Str,1);
	ZVAL_LONG(params[1],param2Str);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ini_set",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int date_default_timezone_set(char *param1Str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"date_default_timezone_set",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int error_reporting(int param1Str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_LONG(params[0],param1Str);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"error_reporting",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void strip_tags(char *param1Str,char **returnStr)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"strip_tags",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*returnStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*returnStr = estrdup(param1Str);
	return;
}

int php_mkdir(char *param1Str)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;

	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	MAKE_STD_ZVAL(params[1]);
	ZVAL_LONG(params[1],0755);

	MAKE_STD_ZVAL(params[2]);
	ZVAL_BOOL(params[2],1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"mkdir",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void php_explode(char *tag,char *val,zval **getZval)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],tag,1);
	ZVAL_STRING(params[1],val,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"explode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);
	
	returnData = &returnZval;

	MAKE_STD_ZVAL(*getZval);
	ZVAL_ZVAL(*getZval,returnData,1,0);

	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
}

void php_implode(char *tag,zval *val,zval **getZval)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],tag,1);
	ZVAL_ZVAL(params[1],val,1,0);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"implode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);
	
	returnData = &returnZval;

	MAKE_STD_ZVAL(*getZval);
	ZVAL_ZVAL(*getZval,returnData,1,0);

	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
}

void php_date(char *tag,char **getZval)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],tag,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"date",0);

	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC)){
		*getZval = estrdup(Z_STRVAL(returnZval));
	}else{
		*getZval = estrdup("0000-00-00");
	}

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
	return;
}

void php_date_ex(char *tag,int timestamp,char **getZval)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],tag,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_LONG(params[1],timestamp);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"date",0);

	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC)){
		*getZval = estrdup(Z_STRVAL(returnZval));
	}else{
		*getZval = estrdup("0000-00-00");
	}

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_dtor(&returnZval);
	return;
}

int exec_shell(char *cmdCommand){

#ifdef PHP_WIN32
	return -1;
#else
	zval *returnData;
	int returnInt = -1;
	TSRMLS_FETCH();
	MAKE_STD_ZVAL(returnData);
	php_exec(0, cmdCommand, NULL, returnData TSRMLS_CC);
	returnInt = toInt(Z_STRVAL_P(returnData));
	zval_ptr_dtor(&returnData);
	return returnInt;
#endif
}

int exec_shell_return(char *cmdCommand,char **getStr){

	FILE *in;
	size_t total_readbytes;
	char *ret;
	php_stream *stream;
	TSRMLS_FETCH();

#ifdef PHP_WIN32
	if ((in=VCWD_POPEN(cmdCommand, "rt"))==NULL) {
#else
	if ((in=VCWD_POPEN(cmdCommand, "r"))==NULL) {
#endif
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to execute '%s'", cmdCommand);
		*getStr = estrdup("");
		return;
	}

	stream = php_stream_fopen_from_pipe(in, "rb");
	total_readbytes = php_stream_copy_to_mem(stream, &ret, PHP_STREAM_COPY_ALL, 0);
	php_stream_close(stream); 
	
	if (total_readbytes > 0) {

		*getStr = ret;

	} else {
		*getStr = estrdup("");
	}

}

void set_exception_handler(char *param1Str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"set_exception_handler",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);
}

void dumpMemory(){
	zval	returnZval,
			function,
			*returnData;
	TSRMLS_FETCH();
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"memory_get_usage",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	returnData = &returnZval;
	php_printf("=========================memroyUsed:%d\n",Z_LVAL_P(returnData));
	zval_dtor(&returnZval);
}

int php_flock(zval *fp,int status)
{
	zval	*params[2],
			param1,
			param2,
			function,
			returnZval,
			*backZval;


	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_ZVAL(params[0],fp,1,0);
	ZVAL_LONG(params[1],status);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"flock",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void php_fwrite(zval *fp,char *content)
{
	zval	*params[2],
			param1,
			param2,
			function,
			returnZval,
			*backZval;


	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_ZVAL(params[0],fp,1,0);
	ZVAL_STRING(params[1],content,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"fwrite",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_dtor(&returnZval);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
}

void php_fopen(char *file,char *type,zval **getreturnZval)
{
	zval	*params[2],
			param1,
			param2,
			function,
			returnZval,
			*backZval,
			*backData;


	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],file,1);
	ZVAL_STRING(params[1],type,1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"fopen",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	backZval = &returnZval;
	MAKE_STD_ZVAL(backData);
	ZVAL_ZVAL(backData,backZval,1,0);
	MAKE_STD_ZVAL(*getreturnZval);
	ZVAL_ZVAL(*getreturnZval,backData,1,0);
	zval_ptr_dtor(&backData);
	zval_dtor(&returnZval);
}

void php_fclose(zval *fp)
{
	zval	*params[2],
			param1,
			param2,
			function,
			returnZval,
			*backZval;


	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],fp,1,0);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"fclose",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

void register_shutdown_function(char *param1Str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"register_shutdown_function",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

void set_error_handler(char *param1Str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"set_error_handler",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

void error_get_last(zval **getZval)
{
	zval	returnZval,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"error_get_last",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}

void php_scandir(char *param1Str,zval **getZval){

	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"scandir",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}


void md5(char *param1Str,char **getStr){
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"md5",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(param1Str);
	zval_dtor(&returnZval);
	return;
}

int file_put_contents(char *param1Str,char *param2Str){
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],param1Str,1);
	ZVAL_STRING(params[1],param2Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"file_put_contents",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void getHostName(char **hostname){
	char	*tempHost;
	exec_shell_return("hostname",&tempHost);
	php_trim(tempHost,"\n",hostname);
	efree(tempHost);
}

void file_get_contents(char *param1Str,char **getStr){

	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],param1Str,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"file_get_contents",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(param1Str);
	zval_dtor(&returnZval);
	return;
}

void serialize(zval *param1Str,zval **getZval){

	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],param1Str,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"serialize",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

void unserialize(zval *param1Str,zval **getZval){
	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;


	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],param1Str,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"unserialize",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	returnData = &returnZval;
	MAKE_STD_ZVAL(*getZval);
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}

int php_chmod(char *file,int mod){

	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],file,1);
	ZVAL_LONG(params[1],mod);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"chmod",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	
	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int touch(char *file,int time){
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],file,1);
	ZVAL_LONG(params[1],time);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"touch",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int filemtime(char *file){
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);

	ZVAL_STRING(params[0],file,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"filemtime",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);


	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int ob_start(){

	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_start",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int ob_end_clean(){
	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_end_clean",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	
	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

int ob_flush(){
	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_flush",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	
	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void ob_get_contents(char **getStr){

	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_get_contents",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr =  estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*getStr = estrdup("");
	return;
}


void ob_end_flush(){
	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_end_flush",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	zval_dtor(&returnZval);
}

void ob_get_clean(char **getStr){
	zval	returnZval,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"ob_get_clean",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup("");
	zval_dtor(&returnZval);
	return;
}

int krsort(HashTable *key){

	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	Z_TYPE_P(params[0]) = IS_ARRAY;
	Z_ARRVAL_P(params[0]) = key;

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"krsort",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

#define MICRO_IN_SEC 1000000.00
void microtime(zval **getZval){

	char ret[100];
	struct timeval tp = {0};

	MAKE_STD_ZVAL(*getZval);

	if (gettimeofday(&tp, NULL)) {
		ZVAL_DOUBLE(*getZval,(double)1000000000.00);
		return;
	}

	ZVAL_DOUBLE(*getZval,(double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC));
}

void microtimeTrue(zval **getZval){
	zval	returnZval,
			function,
			*returnData,
			*returnBack,
			*params[1],
			param1;

	int status = FAILURE;

	TSRMLS_FETCH();

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_BOOL(params[0],1);
	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"microtime",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}

int setcookie(char *key,char *value){
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	params[1] = &param2;
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[0],key,1);
	ZVAL_STRING(params[1],value,1);


	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"setcookie",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	status = Z_LVAL(returnZval);
	zval_dtor(&returnZval);
	return status;
}

void inputCheck(char *str,char **getStr){

	zval	returnZval,
			*params[1],
			param1,
			param2,
			function;

	int status = FAILURE;
	TSRMLS_FETCH();


	//不在对特殊符号做操作
	*getStr = estrdup(str);
	return;

	if(1 == INI_BOOL("magic_quotes_gpc")){
		*getStr = estrdup(str);
		return;
	}

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],str,1);


	INIT_ZVAL(function);
	ZVAL_STRING(&function,"addslashes",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	zval_dtor(&returnZval);
	*getStr = estrdup(str);
}

void php_rawurldecode(char *str,char **decode)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;
	TSRMLS_FETCH();

	*decode = estrdup(str);
	return;

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],str,1);


	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"rawurldecode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*decode = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*decode = estrdup(str);
	zval_dtor(&returnZval);
}

void php_urldecode(char *str,char **decode)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;
	TSRMLS_FETCH();

	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],str,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"urldecode",0);

	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC)){

		zval_ptr_dtor(&params[0]);

		if(IS_STRING == Z_TYPE(returnZval)){
			*decode = estrdup(Z_STRVAL(returnZval));
			zval_dtor(&returnZval);
			return;
		}
	}
	*decode = estrdup(str);
	zval_dtor(&returnZval);
}

void array_unique(zval *array,zval **returnArray){
	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData;

	int status = FAILURE;
	TSRMLS_FETCH();

	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],array,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"array_unique",0);

	MAKE_STD_ZVAL(*returnArray);
	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC)){
		zval_ptr_dtor(&params[0]);
		returnData = &returnZval;
		ZVAL_ZVAL(*returnArray,returnData,1,0);
		zval_dtor(&returnZval);
		return;
	}
	ZVAL_ZVAL(*returnArray,array,1,0);
	zval_dtor(&returnZval);
}

void array_slice(HashTable *inputTable,int offsetBegin,int offsetEnd,zval **getStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();

	if(offsetBegin == 0 && offsetEnd == 0){
		MAKE_STD_ZVAL(*getStr);
		array_init(*getStr);
		return;
	}

	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);

	Z_TYPE_P(params[0]) = IS_ARRAY;
	Z_ARRVAL_P(params[0]) = inputTable;

	ZVAL_LONG(params[1],offsetBegin);
	ZVAL_LONG(params[2],offsetEnd);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"array_slice",0);
	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC)){
		MAKE_STD_ZVAL(*getStr);
		returnData = &returnZval;
		ZVAL_ZVAL(*getStr,returnData,1,0);
		zval_dtor(&returnZval);
	}else{
		MAKE_STD_ZVAL(*getStr);
		ZVAL_ZVAL(*getStr,params[0],1,0);
	}

	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
}

void array_sliceZval(zval *inputTable,int offsetBegin,int offsetEnd,zval **getStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();


	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	
	ZVAL_ZVAL(params[0],inputTable,1,0);
	ZVAL_LONG(params[1],offsetBegin);
	ZVAL_LONG(params[2],offsetEnd);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"array_slice",0);
	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC)){
		MAKE_STD_ZVAL(*getStr);
		returnData = &returnZval;
		ZVAL_ZVAL(*getStr,returnData,1,0);
		zval_dtor(&returnZval);
	}else{
		MAKE_STD_ZVAL(*getStr);
		ZVAL_ZVAL(*getStr,inputTable,1,0);
	}

	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
}


void substr(char *input,int offsetBegin,int offsetEnd,char **getStr)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);

	ZVAL_STRING(params[0],input,1);
	ZVAL_LONG(params[1],offsetBegin);
	ZVAL_LONG(params[2],offsetEnd);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"substr",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(input);
	zval_dtor(&returnZval);
	return;
}

void substr1(char *input,int offsetBegin,char **getStr)
{
	zval	returnZval,
			*params[2],
			param1,
			param2,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);

	ZVAL_STRING(params[0],input,1);
	ZVAL_LONG(params[1],offsetBegin);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"substr",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 2, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(input);
	zval_dtor(&returnZval);
	return;
}

void toChar(int a,char **getStr)
{
	char b[128];
	sprintf(b,"%d",a);
	*getStr = estrdup(b);
}

void php_addslashes(char *reg,char **getStr)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],reg,1);
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"addslashes",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}
	*getStr = estrdup(reg);
	zval_dtor(&returnZval);
	return;
}

void base64Encode(char *reg,char **getStr)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);


	ZVAL_STRING(params[0],reg,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"base64_encode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(reg);
	zval_dtor(&returnZval);
	return;
}

void base64Decode(char *reg,char **getStr)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);


	ZVAL_STRING(params[0],reg,1);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"base64_decode",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup(reg);
	zval_dtor(&returnZval);
	return;
}


//返回调用trace
int get_magic_quotes_gpc(){
zval	returnZval,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"get_magic_quotes_gpc",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC);
	return Z_LVAL(returnZval);

	zval_dtor(&returnZval);
}

void set_time_limit(int input)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_LONG(params[0],input);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"set_time_limit",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

int php_rand_call(int prams1,int params2)
{
	zval	function,
			returnZval,
			*nowReturn,
			*params[2];

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_LONG(params[0],prams1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_LONG(params[1],params2);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"rand",0);
	call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	if(IS_LONG == Z_TYPE(returnZval)){
		zval_dtor(&returnZval);
		return Z_LVAL(returnZval);
	}else{
		zval_dtor(&returnZval);
		return 0;
	}

}

void http_build_query(zval *input,char **getStr)
{
	zval	returnZval,
			*params[3],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;

	MAKE_STD_ZVAL(params[0]);


	ZVAL_ZVAL(params[0],input,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"http_build_query",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	if(IS_STRING == Z_TYPE(returnZval)){
		*getStr = estrdup(Z_STRVAL(returnZval));
		zval_dtor(&returnZval);
		return;
	}

	*getStr = estrdup("");
	zval_dtor(&returnZval);
	return;
}


void curl_setopt(zval *param1Str,int param2Str,zval *param3Str)
{
	zval	returnZval,
			*params[3],
			param1,
			param2,
			param3,
			function,
			*returnData,
			*returnBack,
			**getZval;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	params[1] = &param2;
	params[2] = &param3;
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);

	ZVAL_ZVAL(params[0],param1Str,1,0);
	ZVAL_LONG(params[1],param2Str);
	ZVAL_ZVAL(params[2],param3Str,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"curl_setopt",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 3, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	zval_ptr_dtor(&params[2]);
	zval_dtor(&returnZval);
}

void curl_init(zval **getZval)
{
	zval	returnZval,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"curl_init",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 0, NULL TSRMLS_CC);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}



void curl_exec(zval *object,zval **getZval)
{
	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],object,1,0);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"curl_exec",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;

	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}

void curl_getinfo(zval *object,zval **getZval)
{
	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);

	ZVAL_ZVAL(params[0],object,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"curl_getinfo",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}

void curl_error(zval *object,zval **getZval)
{
	zval	returnZval,
			*params[1],
			param1,
			function,
			*returnData,
			*returnBack;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);

	ZVAL_ZVAL(params[0],object,1,0);

	
	INIT_ZVAL(function);
	ZVAL_STRING(&function,"curl_error",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval, 1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);

	MAKE_STD_ZVAL(*getZval);
	returnData = &returnZval;
	ZVAL_ZVAL(*getZval,returnData,1,0);
	zval_dtor(&returnZval);
	return;
}


void get_include_path(char **decode)
{
	zval	returnZval,
			function;

	int status = FAILURE;
	TSRMLS_FETCH();

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"get_include_path",0);

	if(SUCCESS == call_user_function(EG(function_table), NULL, &function, &returnZval,0, NULL TSRMLS_CC)){

		if(IS_STRING == Z_TYPE(returnZval)){
			*decode = estrdup(Z_STRVAL(returnZval));
			zval_dtor(&returnZval);
			return;
		}
	}
	*decode = estrdup("");
	zval_dtor(&returnZval);
}

void set_include_path(char *str)
{
	zval	returnZval,
			*params[1],
			param1,
			function;

	int status = FAILURE;

	TSRMLS_FETCH();
	params[0] = &param1;
	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],str,1);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"set_include_path",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,1, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}

void parse_str(char *str,zval **backZval)
{
	zval	returnZval,
			*params[2],
			function;

	int status = FAILURE;

	TSRMLS_FETCH();

	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],str,1);

	MAKE_STD_ZVAL(params[1]);
	ZVAL_NULL(params[1]);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"parse_str",0);
	status = call_user_function(EG(function_table), NULL, &function, &returnZval,2, params TSRMLS_CC);

	MAKE_STD_ZVAL(*backZval);
	ZVAL_ZVAL(*backZval,params[1],1,1);

	zval_ptr_dtor(&params[0]);
	zval_dtor(&returnZval);
}