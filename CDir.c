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
#include "php_CQuickFramework.h"
#include "php_CException.h"
#include "php_CDir.h"



//zend类方法
zend_function_entry CDir_functions[] = {
	PHP_ME(CDir,getDirName,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,getDirFiles,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,getDirSize,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,getDirAllFiles,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,exists,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,delete,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,deleteAllFiles,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDir,rename,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDir)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CDir",CDir_functions);
	CDirCe = zend_register_internal_class(&funCe TSRMLS_CC);
	return SUCCESS;
}

PHP_METHOD(CDir,getDirName)
{
	char	*string, 
			*suffix = NULL, 
			*ret;

	int		string_len, 
			suffix_len = 0;

	size_t	ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &string, &string_len, &suffix, &suffix_len) == FAILURE) {
		return;
	}

	php_basename(string, string_len, suffix, suffix_len, &ret, &ret_len TSRMLS_CC);
	RETURN_STRINGL(ret, (int)ret_len, 0);
}

void CDidr_getDirFiles(char *dirn,long flags,zval *returnData TSRMLS_DC){

	int		status = 0,
			i = 0,
			h = 0;

	char	*key;

	ulong	ukey;

	zval	*params[2],
			function,
			funcReturn,
			*returnZval,
			*tempZval,
			**thisVal;

	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],dirn,1);

	MAKE_STD_ZVAL(params[1]);
	ZVAL_LONG(params[1],flags);

	INIT_ZVAL(function);
	ZVAL_STRING(&function,"scandir",0);

	status = call_user_function(EG(function_table), NULL, &function, &funcReturn, 2, params TSRMLS_CC);
	
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);

	if(status != SUCCESS){
		zval_dtor(&funcReturn);
		return;
	}

	tempZval = &funcReturn;

	if(IS_ARRAY != Z_TYPE_P(tempZval)){
		zval_dtor(&funcReturn);
		return;
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(tempZval));
	h = zend_hash_num_elements(Z_ARRVAL_P(tempZval));
	for(i = 0 ; i < h ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(tempZval),(void**)&thisVal);
		zend_hash_get_current_key(Z_ARRVAL_P(tempZval),&key,&ukey,0);

		if(IS_STRING == Z_TYPE_PP(thisVal) && strcmp(Z_STRVAL_PP(thisVal),".") != 0 && strcmp(Z_STRVAL_PP(thisVal),"..") != 0  ){
			//插入数组
			add_next_index_string(returnData,Z_STRVAL_PP(thisVal),1);
		}

		zend_hash_move_forward(Z_ARRVAL_P(tempZval));
	}

	zval_dtor(&funcReturn);

}

PHP_METHOD(CDir,getDirFiles)
{
	char	*dirn;

	int		dirn_len;

	long	flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &dirn, &dirn_len, &flags) == FAILURE) {
		return;
	}

	array_init(return_value);
	CDidr_getDirFiles(dirn,flags,return_value TSRMLS_CC);

}

void CDir_getDirAllFiles(char *path,zval *files TSRMLS_DC){

	if(php_is_dir(path)){

		zval	*thisDirFiles,
				**thisVal;

		int		i,h;

		char	*fullFile,
				*tempPath;

		MAKE_STD_ZVAL(thisDirFiles);
		array_init(thisDirFiles);
		CDidr_getDirFiles(path,0,thisDirFiles TSRMLS_CC);

		zend_hash_internal_pointer_reset(Z_ARRVAL_P(thisDirFiles));
		h = zend_hash_num_elements(Z_ARRVAL_P(thisDirFiles));
		for(i = 0 ; i < h ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(thisDirFiles),(void**)&thisVal);
			php_rtrim(path,"/",&tempPath);
			spprintf(&fullFile,0,"%s/%s",tempPath,Z_STRVAL_PP(thisVal));
			efree(tempPath);

			CDir_getDirAllFiles(fullFile,files TSRMLS_CC);
			zend_hash_move_forward(Z_ARRVAL_P(thisDirFiles));
		}
		zval_ptr_dtor(&thisDirFiles);

	}else{
		add_next_index_string(files,path,1);
	}
	efree(path);
}

PHP_METHOD(CDir,getDirAllFiles)
{
	char	*dirn;

	int		dirn_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dirn, &dirn_len) == FAILURE) {
		return;
	}
	
	array_init(return_value);
	CDir_getDirAllFiles(estrdup(dirn),return_value TSRMLS_CC);
}

PHP_METHOD(CDir,getDirFilesToTree)
{

}

size_t CDir_getDirSize(char *path TSRMLS_DC){

	size_t	allSize = 0;

	zval	*files;

	int		i,h;

	zval	**thisVal;

	char	*tempPath,
			*fullPath;

	MAKE_STD_ZVAL(files);
	array_init(files);
	CDidr_getDirFiles(path,0,files TSRMLS_CC);

	h = zend_hash_num_elements(Z_ARRVAL_P(files));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(files));

	for(i = 0 ; i < h ; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(files),(void**)&thisVal);

		if(strcmp(Z_STRVAL_PP(thisVal),".") == 0 || strcmp(Z_STRVAL_PP(thisVal),"..") == 0){
			zend_hash_move_forward(Z_ARRVAL_P(files));
			continue;
		}

		php_rtrim(path,"/",&tempPath);
		spprintf(&fullPath,0,"%s/%s",path,Z_STRVAL_PP(thisVal));
		
		if(php_is_dir(fullPath)){
			size_t thisSize = 0;
			thisSize = CDir_getDirSize(fullPath TSRMLS_CC);
			allSize = allSize + thisSize;
		}else{
			size_t thisSize = 0;
			thisSize = php_filesize(fullPath);
			allSize = allSize + thisSize;
		}


		efree(fullPath);
		efree(tempPath);

		zend_hash_move_forward(Z_ARRVAL_P(files));
	}

	zval_ptr_dtor(&files);

	return allSize;
}

PHP_METHOD(CDir,getDirSize)
{
	size_t	allSize = 0;

	char	*dirn;

	int		dirn_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dirn, &dirn_len) == FAILURE) {
		return;
	}

	allSize = CDir_getDirSize(dirn TSRMLS_CC);

	RETURN_LONG(allSize);
}

PHP_METHOD(CDir,exists)
{
	char	*dirn;

	int		dirn_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dirn, &dirn_len) == FAILURE) {
		return;
	}

	if(php_is_dir(dirn)){
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(CDir,isAbsolute)
{

}

PHP_METHOD(CDir,isRoot)
{

}

PHP_METHOD(CDir,isReadable)
{
}

PHP_METHOD(CDir,delete)
{
	char	*dirn;

	int		dirn_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dirn, &dirn_len) == FAILURE) {
		return;
	}

	//delete this path 
	if( rmdir(dirn) == 0 ){
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

void CDir_deleteAllFiles(char *dirn TSRMLS_DC){

	zval	*files,
			**thisVal;

	int		i,h;

	char	*tempPath,
			*fullPath;

	MAKE_STD_ZVAL(files);
	array_init(files);
	CDidr_getDirFiles(dirn,0,files TSRMLS_CC);

	add_next_index_string(files,"",1);

	h = zend_hash_num_elements(Z_ARRVAL_P(files));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(files));

	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(files),(void**)&thisVal);

		php_rtrim(dirn,"/",&tempPath);
		spprintf(&fullPath,0,"%s/%s",tempPath,Z_STRVAL_PP(thisVal));


		remove(fullPath);

		if(php_is_dir(fullPath)){

			CDir_deleteAllFiles(fullPath TSRMLS_CC);
		}

		efree(tempPath);
		efree(fullPath);

		zend_hash_move_forward(Z_ARRVAL_P(files));
	}

	zval_ptr_dtor(&files);
}

PHP_METHOD(CDir,deleteAllFiles)
{
	char	*dirn;

	int		dirn_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &dirn, &dirn_len) == FAILURE) {
		return;
	}


	CDir_deleteAllFiles(dirn TSRMLS_CC);


}

PHP_METHOD(CDir,rename)
{
	char	*old_name, 
			*new_name;

	int		old_name_len, 
			new_name_len,
			paramsNum = 2,
			returnStatus = 0;

	zval	*zcontext = NULL,
			*params[3],
			function,
			funcReturn;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "pp|r", &old_name, &old_name_len, &new_name, &new_name_len, &zcontext) == FAILURE) {
		RETURN_FALSE;
	}

	MAKE_STD_ZVAL(params[0]);
	ZVAL_STRING(params[0],old_name,1);
	MAKE_STD_ZVAL(params[1]);
	ZVAL_STRING(params[1],new_name,1);

	if(zcontext != NULL){
		MAKE_STD_ZVAL(params[2]);
		paramsNum = 3;
	}


	INIT_ZVAL(function);
	ZVAL_STRING(&function,"rename",0);
	call_user_function(EG(function_table), NULL, &function, &funcReturn, paramsNum, params TSRMLS_CC);

	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[1]);
	if(zcontext != NULL){
		zval_ptr_dtor(&params[2]);
	}

	if(Z_TYPE(funcReturn) == IS_BOOL && Z_BVAL(funcReturn) == 1){
		returnStatus = 1;
	}

	zval_dtor(&funcReturn);

	RETURN_BOOL(returnStatus);
}