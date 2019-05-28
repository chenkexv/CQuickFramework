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
#include "php_CWord.h"
#include "php_CRedis.h"
#include "php_CWebApp.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CWord_functions[] = {
	PHP_ME(CWord,getStringFirstChar,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CWord,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CWord,setDictionary,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,getHitWord,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,check,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,replace,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,mark,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,cache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CWord,clearCache,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CWord)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CWord",CWord_functions);

	CWordCe = zend_register_internal_class(&funCe TSRMLS_CC);


	zend_declare_property_null(CWordCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CWordCe, ZEND_STRL("contnet"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CWordCe, ZEND_STRL("dictPath"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CWordCe, ZEND_STRL("cacheKey"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CWordCe, ZEND_STRL("cacheTime"),86400*7,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CWordCe, ZEND_STRL("useCache"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CWordCe, ZEND_STRL("treeArray"),ZEND_ACC_PRIVATE TSRMLS_CC);


	return SUCCESS;
}

void CWord_getStringFirstCharOne(char *string,char **returnString TSRMLS_DC){

	unsigned char	fchar,
					schar,
					gfchar;
	int				fascii = 0,
					gfascii = 0,
					sascii = 0,
					asc = 0;
	char			*gbString;

	fchar = (unsigned char)string[0];
	fascii = fchar;

	if(fascii >= 65 && fascii <= 122){
		char	endString[2];
		sprintf(endString,"%c",fascii);
		php_strtoupper(endString,2);
		*returnString = estrdup(endString);
		return;
	}

	if(fascii >= 48 && fascii <= 57){
		char	endString[2];
		sprintf(endString,"%c",fascii);
		*returnString = estrdup(endString);
		return;
	}

	//change chart to GB18030
	php_iconv("UTF-8", "GB18030",string,&gbString);

	schar = (unsigned char)gbString[1];
	sascii = schar;
	gfchar = (unsigned char)gbString[0];
	gfascii = gfchar;

	asc = gfascii*256+sascii-65536;

	if(asc >= -20319 && asc <= -20284) {
		efree(gbString);
		*returnString = estrdup("A");
		return;
	}

	if((asc >= -20283 && asc <= -19776) || asc==-8231){ 
		efree(gbString);
		*returnString = estrdup("B");
		return;
	}
	if(asc >= -19775 && asc <= -19219) {
		efree(gbString);
		*returnString = estrdup("C");
		return;
	}
	if(asc >= -19218 && asc <= -18711) {
		efree(gbString);
		*returnString = estrdup("D");
		return;
	}
	if(asc >= -18710 && asc <= -18527) {
		efree(gbString);
		*returnString = estrdup("E");
		return;
	}
	if(asc >= -18526 && asc <= -18240) {
		efree(gbString);
		*returnString = estrdup("F");
		return;
	}
	if(asc >= -18239 && asc <= -17923) {
		efree(gbString);
		*returnString = estrdup("G");
		return;
	}
	if((asc >= -17922 && asc <= -17418) || asc==-7989 || asc==-5445 || asc==-6743) {
		efree(gbString);
		*returnString = estrdup("H");
		return;
	}
	if((asc >= -17417 && asc <= -16475) || asc==-4949 || asc==-4398 || asc==-21158 || asc==-30747) {
		efree(gbString);
		*returnString = estrdup("J");
		return;
	}
	if((asc >= -16474 && asc <= -16213) || asc==-4360 || asc==-7427 || asc==-8249) {
		efree(gbString);
		*returnString = estrdup("K");
		return;
	}
	if(asc >= -16212 && asc <= -15641) {
		efree(gbString);
		*returnString = estrdup("L");
		return;
	}
	if((asc >= -15640 && asc <= -15166) || asc== -2104 || asc== -8220 || asc==-7512 || asc==-8201) {
		efree(gbString);
		*returnString = estrdup("M");
		return;
	}
	if(asc >= -15165 && asc <= -14923) {
		efree(gbString);
		*returnString = estrdup("N");
		return;
	}
	if(asc >= -14922 && asc <= -14915) {
		efree(gbString);
		*returnString = estrdup("O");
		return;
	}
	if(asc >= -14914 && asc <= -14631) {
		efree(gbString);
		*returnString = estrdup("P");
		return;
	}
	if(asc >= -14630 && asc <= -14150) {
		efree(gbString);
		*returnString = estrdup("Q");
		return;
	}
	if((asc >= -14149 && asc <= -14091) || asc==-4445) {
		efree(gbString);
		*returnString = estrdup("R");
		return;
	}
	if(asc >= -14090 && asc <= -13319) {
		efree(gbString);
		*returnString = estrdup("S");
		return;
	}
	if(asc >= -13318 && asc <= -12839) {
		efree(gbString);
		*returnString = estrdup("T");
		return;
	}
	if(asc >= -12838 && asc <= -12557) {
		efree(gbString);
		*returnString = estrdup("W");
		return;
	}
	if((asc >= -12556 && asc <= -11848) || asc==-4923 || asc==-6446) {
		efree(gbString);
		*returnString = estrdup("X");
		return;
	}
	if( (asc >= -11847 && asc <= -11056) || asc==-6704 || asc==-7431 || asc==-8508 || asc==-5417 || asc==-5143 || asc==-10061) {
		efree(gbString);
		*returnString = estrdup("Y");
		return;
	}
	if((asc >= -11055 && asc <= -10247) || asc==-5154) {
		efree(gbString);
		*returnString = estrdup("Z");
		return;
	}
	
	*returnString = estrdup("0");

	//destory
	efree(gbString);
}

//返回汉字的第一个字母
PHP_METHOD(CWord,getStringFirstChar)
{
	char	*string,
			*endString;
	int		stringLen = 0,
			returnAll = 0;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|b",&string,&stringLen,&returnAll) == FAILURE){
		return;
	}

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

int CWord_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CWordCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CWordCe);

		//执行构造器
		if (CWordCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CWordCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CWordCe,ZEND_STRL("instance"),object TSRMLS_CC);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}


PHP_METHOD(CWord,getInstance)
{
	zval *instanceZval;

	CWord_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,1);
}

void CWowrd_loadDictFile(zval *object,char *filePath TSRMLS_DC){

	zval	*dictPath;
	char	*md5String;

	dictPath = zend_read_property(CWordCe,object,ZEND_STRL("dictPath"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(dictPath)){
		zval	*saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CWordCe,object,ZEND_STRL("dictPath"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		dictPath = zend_read_property(CWordCe,object,ZEND_STRL("dictPath"),0 TSRMLS_CC);
	}

	if(SUCCESS != fileExist(filePath)){
		char	errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CWordException] Call [CWord->setDictionary] the dict file not exists : ",filePath);
		zend_throw_exception(CClassNotFoundExceptionCe,errorMessage, 10000 TSRMLS_CC);
		return;
	}

	//set to object
	md5(filePath,&md5String);
	add_assoc_string(dictPath,md5String,filePath,1);
	efree(md5String);
}

PHP_METHOD(CWord,setDictionary)
{
	zval	*filePath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&filePath) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->setDictionary] must set a string or an array", 10000 TSRMLS_CC);
		return;
	}

	ZVAL_ZVAL(return_value,getThis(),1,0);

	if(IS_STRING == Z_TYPE_P(filePath)){
		CWowrd_loadDictFile(getThis(),Z_STRVAL_P(filePath) TSRMLS_CC);
		return;
	}else if(IS_ARRAY == Z_TYPE_P(filePath)){

		int		i,h;
		zval	**thisVal;
		h = zend_hash_num_elements(Z_ARRVAL_P(filePath));
		for(i = 0 ; i < h ; i ++){
			zend_hash_get_current_data(Z_ARRVAL_P(filePath),(void**)&thisVal);
			
			if(IS_STRING != Z_TYPE_PP(thisVal)){
				zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->setDictionary] the dict list must all string", 10000 TSRMLS_CC);
				return;
			}

			CWowrd_loadDictFile(getThis(),Z_STRVAL_PP(thisVal) TSRMLS_CC);

			zend_hash_move_forward(Z_ARRVAL_P(filePath));
		}

	}else{
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->setDictionary] must set a string or an array", 10000 TSRMLS_CC);
		return;
	}
}


void CWord_getDictFilesAsArray(zval *object,zval **returnArray TSRMLS_DC){

	zval	*dictPath,
			**thisFilePath,
			*nowFileArray,
			**thisRowString;

	int		i,h;

	char	*thisFileString;

	dictPath = zend_read_property(CWordCe,object,ZEND_STRL("dictPath"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(*returnArray);
	array_init(*returnArray);

	if(IS_ARRAY != Z_TYPE_P(dictPath)){
		return;
	}


	h = zend_hash_num_elements(Z_ARRVAL_P(dictPath));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(dictPath));
	for( i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(dictPath),(void**)&thisFilePath);
		file_get_contents(Z_STRVAL_PP(thisFilePath),&thisFileString);
		php_explode(PHP_EOL,thisFileString,&nowFileArray);
		php_array_merge(Z_ARRVAL_PP(returnArray), Z_ARRVAL_P(nowFileArray), 1 TSRMLS_CC);
		efree(thisFileString);
		zval_ptr_dtor(&nowFileArray);
		zend_hash_move_forward(Z_ARRVAL_P(dictPath));
	}
}

void CWord_getCacheKey(zval *object,char **cacheKey TSRMLS_DC){

	zval	*dictPath,
			*cacheKeyZval;

	char	*md5Key;

	cacheKeyZval = zend_read_property(CWordCe,object,ZEND_STRL("cacheKey"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(cacheKeyZval) && strlen(Z_STRVAL_P(cacheKeyZval)) > 0){
		*cacheKey = estrdup(Z_STRVAL_P(cacheKeyZval));
		return;
	}

	dictPath = zend_read_property(CWordCe,object,ZEND_STRL("dictPath"),0 TSRMLS_CC);
	

	if(IS_ARRAY != Z_TYPE_P(dictPath)){
		md5("nokey",&md5Key);
	}else{
		zval	*stringZval;
		php_implode(",",dictPath,&stringZval);
		md5(Z_STRVAL_P(stringZval),&md5Key);
		zval_ptr_dtor(&stringZval);
	}

	strcat2(cacheKey,"sensitiveDict:",md5Key,NULL);
	efree(md5Key);
}


void CWord_saveDictArrayToRedis(zval *object,zval *array TSRMLS_DC){

	char	cacheKey[60],
			*saveString,
			*cacheKeyMd5String;

	zval	*callRedisParams,
			*dataFromRedis,
			*useCache;

	//disable cache
	useCache = zend_read_property(CWordCe,object,ZEND_STRL("useCache"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(useCache) && 0 == Z_LVAL_P(useCache)){
		return;
	}

	CWord_getCacheKey(object,&cacheKeyMd5String TSRMLS_CC);

	json_encode(array,&saveString);

	//read share memory
	MAKE_STD_ZVAL(callRedisParams);
	array_init(callRedisParams);
	add_next_index_string(callRedisParams,cacheKeyMd5String,1);
	add_next_index_string(callRedisParams,saveString,1);
	add_next_index_long(callRedisParams,86400*7);
	CRedis_callFunction("main","set",callRedisParams,&dataFromRedis TSRMLS_CC);
	zval_ptr_dtor(&callRedisParams);
	zval_ptr_dtor(&dataFromRedis);

	efree(saveString);
	efree(cacheKeyMd5String);
}

void CWord_createWordToTree(zval *object,char *string TSRMLS_DC){

	int		i,length;
	
	char	*thisString,
			*thisChar;

	zval	*treeArray,
			**tempTree;

	if(strlen(string) <= 0){
		return;
	}

	php_trim(string," \n\r\n",&thisString);

	treeArray = zend_read_property(CWordCe,object,ZEND_STRL("treeArray"),0 TSRMLS_CC);

	length = mb_strlen(thisString,"utf-8");

	for(i = 0 ; i < length; i++){
		
		mb_substr(thisString, i, 1, "utf-8",&thisChar);

		if(IS_ARRAY == Z_TYPE_P(treeArray) && SUCCESS == zend_hash_find(Z_ARRVAL_P(treeArray),thisChar,strlen(thisChar)+1,(void**)&tempTree) && IS_ARRAY == Z_TYPE_PP(tempTree)){
			treeArray = *tempTree;
		}else{
			zval *newTree;
			MAKE_STD_ZVAL(newTree);
			array_init(newTree);
			add_assoc_bool(newTree,"ending",0);
			add_assoc_zval(treeArray,thisChar,newTree);
			treeArray = newTree;
		}

		if (i == length - 1) {
			add_assoc_bool(treeArray,"ending",1);
		}

		efree(thisChar);
	}
		
	efree(thisString);
}

void CWord_createDictArrayToTree(zval *object,zval *array,zval **returnArray TSRMLS_DC){
	
	zval	*treeArray,
			**thisVal;

	int		i,h;

	treeArray = zend_read_property(CWordCe,object,ZEND_STRL("treeArray"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(treeArray)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_property(CWordCe,object,ZEND_STRL("treeArray"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		treeArray = zend_read_property(CWordCe,object,ZEND_STRL("treeArray"),0 TSRMLS_CC);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(array));

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(array));
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(array),(void**)&thisVal);
		CWord_createWordToTree(object,Z_STRVAL_PP(thisVal) TSRMLS_CC);
		zend_hash_move_forward(Z_ARRVAL_P(array));
	}

	MAKE_STD_ZVAL(*returnArray);
	ZVAL_ZVAL(*returnArray,treeArray,1,0);
}

//get from redis cache
void CWord_getDictArrayFromRedis(zval *object,zval **returnData TSRMLS_DC){
	
	zval	*useCache,
			*callRedisParams,
			*dataFromRedis,
			*jsonDecodeZval;

	char	*cacheKeyMd5String;

	MAKE_STD_ZVAL(*returnData);

	useCache = zend_read_property(CWordCe,object,ZEND_STRL("useCache"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(useCache) && 0 == Z_LVAL_P(useCache)){
		ZVAL_BOOL(*returnData,0);
		return;
	}

	CWord_getCacheKey(object,&cacheKeyMd5String TSRMLS_CC);
	

	MAKE_STD_ZVAL(callRedisParams);
	array_init(callRedisParams);
	add_next_index_string(callRedisParams,cacheKeyMd5String,1);
	CRedis_callFunction("main","get",callRedisParams,&dataFromRedis TSRMLS_CC);
	zval_ptr_dtor(&callRedisParams);


	if(IS_STRING != Z_TYPE_P(dataFromRedis)){
		zval_ptr_dtor(&dataFromRedis);
		efree(cacheKeyMd5String);
		ZVAL_BOOL(*returnData,0);
		return;
	}

	//json_decode
	json_decode(Z_STRVAL_P(dataFromRedis),&jsonDecodeZval);
	ZVAL_ZVAL(*returnData,jsonDecodeZval,1,0);
	
	zval_ptr_dtor(&jsonDecodeZval);
	zval_ptr_dtor(&dataFromRedis);
	efree(cacheKeyMd5String);
}

void CWord_getDictTree(zval *object,zval **treeData TSRMLS_DC){

	zval	*dictWordArray,
			*treeArray,
			*cacheArray,
			*fromClass;


	//read class prop treeArray
	fromClass = zend_read_property(CWordCe,object,ZEND_STRL("treeArray"),0 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(fromClass)){
		MAKE_STD_ZVAL(*treeData);
		ZVAL_ZVAL(*treeData,fromClass,1,0);
		return;
	}

	//check from cache
	CWord_getDictArrayFromRedis(object,&cacheArray TSRMLS_CC);
	if(IS_BOOL != Z_TYPE_P(cacheArray)){
		zend_update_property(CWordCe,object,ZEND_STRL("treeArray"),cacheArray TSRMLS_CC);
		MAKE_STD_ZVAL(*treeData);
		ZVAL_ZVAL(*treeData,cacheArray,1,1);
		return;
	}
	zval_ptr_dtor(&cacheArray);

	//get file
	CWord_getDictFilesAsArray(object,&dictWordArray TSRMLS_CC);

	//create Tree
	CWord_createDictArrayToTree(object,dictWordArray,&treeArray TSRMLS_CC);

	//save to Reids
	CWord_saveDictArrayToRedis(object,treeArray TSRMLS_CC);

	//return
	MAKE_STD_ZVAL(*treeData);
	ZVAL_ZVAL(*treeData,treeArray,1,0);

	zval_ptr_dtor(&dictWordArray);
	zval_ptr_dtor(&treeArray);
}

void CWord_getHitWord(zval *object,char *content,int matchType,zval **returnArray TSRMLS_DC){

	zval	*treeData,
			**nowMap;

	char	*thisChar;

	int		contentLen = 0,
			mbLength,
			i,j;

	CWord_getDictTree(object,&treeData TSRMLS_CC);

	MAKE_STD_ZVAL(*returnArray);
	array_init(*returnArray);

	mbLength = mb_strlen(content,"utf-8");
	for(i = 0 ; i < mbLength ; i++){
		int		matchFlag = 0,
				flag = 0;
		zval	*tempTree,
				**isEnding;

		char	*badWord;

		tempTree = treeData;

		for(j = i; j < mbLength;j++){

			mb_substr(content,j,1,"utf-8",&thisChar);

			if(SUCCESS == zend_hash_find(Z_ARRVAL_P(tempTree),thisChar,strlen(thisChar)+1,(void**)&nowMap) && IS_ARRAY == Z_TYPE_PP(nowMap) ){
			}else{
				efree(thisChar);
				break;
			}

			tempTree = *nowMap;

			//offset +1
			matchFlag++;

			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(nowMap),"ending",strlen("ending")+1,(void**)&isEnding) && IS_BOOL == Z_TYPE_PP(isEnding) && 0 == Z_LVAL_PP(isEnding) ){
				efree(thisChar);
				continue;
			}

			flag = 1;

			if (1 == matchType)  {
				efree(thisChar);
                break;
            }

			efree(thisChar);
		}
		
		if (flag == 0) {
			matchFlag = 0;
        }

		if(matchFlag <= 0){
			continue;
		}

		mb_substr(content, i, matchFlag, "utf-8",&badWord);

		add_next_index_string(*returnArray,badWord,0);

		i = i+ matchFlag - 1;
	}


	zval_ptr_dtor(&treeData);
}

PHP_METHOD(CWord,getHitWord)
{

	char	*content;

	zval	*returnArray;

	int		contentLen = 0,
			matchType = 1;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&content,&contentLen,&matchType) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->getHitWord] params error", 10000 TSRMLS_CC);
		return;
	}

	CWord_getHitWord(getThis(),content,matchType,&returnArray TSRMLS_CC);
	RETVAL_ZVAL(returnArray,1,1);
}

int CWord_check(zval *object,char *content TSRMLS_DC){

	zval	*treeData,
			**nowMap;

	char	*thisChar;

	int		contentLen = 0,
			mbLength,
			i,j;

	CWord_getDictTree(object,&treeData TSRMLS_CC);

	mbLength = mb_strlen(content,"utf-8");
	for(i = 0 ; i < mbLength ; i++){
		int		matchFlag = 0,
				flag = 0;
		zval	*tempTree,
				**isEnding;

		char	*badWord;

		tempTree = treeData;

		for(j = i; j < mbLength;j++){

			mb_substr(content,j,1,"utf-8",&thisChar);

			if(SUCCESS == zend_hash_find(Z_ARRVAL_P(tempTree),thisChar,strlen(thisChar)+1,(void**)&nowMap) && IS_ARRAY == Z_TYPE_PP(nowMap) ){
			}else{
				efree(thisChar);
				break;
			}

			tempTree = *nowMap;

			//offset +1
			matchFlag++;

			if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(nowMap),"ending",strlen("ending")+1,(void**)&isEnding) && IS_BOOL == Z_TYPE_PP(isEnding) && 0 == Z_LVAL_PP(isEnding) ){
				efree(thisChar);
				continue;
			}

			efree(thisChar);
			zval_ptr_dtor(&treeData);
			return 1;
		}
		
		if(matchFlag <= 0){
			continue;
		}

		i = i+ matchFlag - 1;
	}


	zval_ptr_dtor(&treeData);
	return 0;
}

PHP_METHOD(CWord,check)
{

	char	*content;

	int		contentLen = 0,
			status = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&content,&contentLen) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->check] params error", 10000 TSRMLS_CC);
		return;
	}

	status = CWord_check(getThis(),content TSRMLS_CC);

	RETVAL_BOOL(status);
}

PHP_METHOD(CWord,replace)
{
	char	*content,
			*replaceString,
			*trueReplace,
			*newContent;

	int		contentLen = 0,
			replaceStringLen = 0,
			i,h;

	zval	*returnArray,
			**thisVal,
			*replaceZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|s",&content,&contentLen,&replaceString,&replaceStringLen) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->replace] params error", 10000 TSRMLS_CC);
		return;
	}

	CWord_getHitWord(getThis(),content,1,&returnArray TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(returnArray)){
		zval_ptr_dtor(&returnArray);
		RETURN_STRING(content,1);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(returnArray));
	if(h <= 0){
		RETURN_STRING(content,1);
	}

	if(replaceStringLen == 0){
		trueReplace = estrdup("**");
	}else{
		trueReplace = estrdup(replaceString);
	}

	MAKE_STD_ZVAL(replaceZval);
	array_init(replaceZval);
	for(i = 0 ; i < h ; i++){
		add_next_index_string(replaceZval,trueReplace,1);
	}

	str_replaceArray(returnArray,replaceZval,content,&newContent);

	RETVAL_STRING(newContent,0);

	efree(trueReplace);
	zval_ptr_dtor(&returnArray);
	zval_ptr_dtor(&replaceZval);
}

PHP_METHOD(CWord,mark)
{
	char	*content,
			*sltag,
			*srtag,
			*ltag,
			*rtag,
			*newContent,
			*markString;

	int		contentLen = 0,
			ltagLen = 0,
			rtagLen = 0,
			i,
			h;

	zval	*returnArray,
			**thisVal,
			*replaceZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|ss",&content,&contentLen,&sltag,&ltagLen,&rtag,&rtagLen) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->mark] params error", 10000 TSRMLS_CC);
		return;
	}

	if(ltagLen == 0){
		ltag = estrdup("<span class=\"mark\">");
	}else{
		ltag = estrdup(sltag);
	}

	if(rtagLen == 0){
		rtag = estrdup("</span>");
	}else{
		rtag = estrdup(srtag);
	}

	CWord_getHitWord(getThis(),content,1,&returnArray TSRMLS_CC);

	if(IS_ARRAY != Z_TYPE_P(returnArray)){
		zval_ptr_dtor(&returnArray);
		RETURN_STRING(content,1);
	}

	h = zend_hash_num_elements(Z_ARRVAL_P(returnArray));
	if(h <= 0){
		RETURN_STRING(content,1);
	}

	zend_hash_internal_pointer_reset(Z_ARRVAL_P(returnArray));

	MAKE_STD_ZVAL(replaceZval);
	array_init(replaceZval);
	for(i = 0 ; i < h ; i++){
		zend_hash_get_current_data(Z_ARRVAL_P(returnArray),(void**)&thisVal);
		convert_to_string(*thisVal);
		spprintf(&markString,0,"%s%s%s",ltag,Z_STRVAL_PP(thisVal),rtag);
		add_next_index_string(replaceZval,markString,0);
		zend_hash_move_forward(Z_ARRVAL_P(returnArray));
	}

	str_replaceArray(returnArray,replaceZval,content,&newContent);

	RETVAL_STRING(newContent,0);

	zval_ptr_dtor(&returnArray);
	zval_ptr_dtor(&replaceZval);
	efree(rtag);
	efree(ltag);
}

PHP_METHOD(CWord,clearCache)
{
	char	*cacheKeyMd5String;

	zval	*callRedisParams,
			*dataFromRedis;

	CWord_getCacheKey(getThis(),&cacheKeyMd5String TSRMLS_CC);

	MAKE_STD_ZVAL(callRedisParams);
	array_init(callRedisParams);
	add_next_index_string(callRedisParams,cacheKeyMd5String,1);
	CRedis_callFunction("main","del",callRedisParams,&dataFromRedis TSRMLS_CC);
	zval_ptr_dtor(&callRedisParams);
	zval_ptr_dtor(&dataFromRedis);
	efree(cacheKeyMd5String);
	RETVAL_ZVAL(getThis(),1,0);
}


PHP_METHOD(CWord,cache){

	zval	*cacheKey;
	long	cacheTime = 86400*7;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|l",&cacheKey,&cacheTime) == FAILURE){
		zend_throw_exception(CExceptionCe,"[CWordException] Call [CWord->cache] the params error", 10000 TSRMLS_CC);
		return;
	}

	if(IS_BOOL == Z_TYPE_P(cacheKey)){
		zend_update_property_long(CWordCe,getThis(),ZEND_STRL("useCache"),Z_LVAL_P(cacheKey) TSRMLS_CC);
	}else if(IS_STRING == Z_TYPE_P(cacheKey)){
		zend_update_property_string(CWordCe,getThis(),ZEND_STRL("cacheKey"),Z_STRVAL_P(cacheKey) TSRMLS_CC);
		zend_update_property_long(CWordCe,getThis(),ZEND_STRL("cacheTime"),cacheTime TSRMLS_CC);
	}

	RETVAL_ZVAL(getThis(),1,0);
}

