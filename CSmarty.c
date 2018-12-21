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


#include "php_CMyFrameExtension.h"
#include "php_CSmarty.h"
#include "php_CWebApp.h"
#include "php_CRequest.h"


zend_function_entry CSmarty_functions[] = {
	PHP_ME(CSmarty,setInitData,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSmarty,showPageData,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSmarty,cn_substr,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSmarty,sayTime,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSmarty,checkRight,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};


CMYFRAME_REGISTER_CLASS_RUN(CSmarty)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CSmarty",CSmarty_functions);
	CSmartyCe = zend_register_internal_class(&funCe TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CSmarty,setInitData)
{
	

}

PHP_METHOD(CSmarty,showPageData)
{
	zval	*contentGetZval,
			*other1,
			*other2,
			*other3,
			**contentGetContent,
			*contentZval,
			**titleSet,
			**keySet,
			**desSet;

	char	*titleDefault = "",
			*keyDefault = "",
			*descDefault = "",
			*titleString,
			*keywordsString,
			*desString,
			*showString;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|zzz",&contentGetZval,&other1,&other2,&other3) == FAILURE){
		RETURN_FALSE;
	}

	if(IS_ARRAY != Z_TYPE_P(contentGetZval)){
		RETURN_FALSE;
	}

	if(SUCCESS != zend_hash_find(Z_ARRVAL_P(contentGetZval),"content",strlen("content")+1,(void**)&contentGetContent)){
		RETURN_FALSE;
	}

	if(IS_ARRAY != Z_TYPE_PP(contentGetContent)){
		RETURN_FALSE;
	}

	contentZval = *contentGetContent;
	

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(contentZval),"title",strlen("title")+1,(void**)&titleSet) && IS_STRING == Z_TYPE_PP(titleSet) ){
		titleDefault = Z_STRVAL_PP(titleSet);
	}


	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(contentZval),"keyword",strlen("keyword")+1,(void**)&keySet) && IS_STRING == Z_TYPE_PP(keySet) ){
		keyDefault = Z_STRVAL_PP(keySet);
	}


	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(contentZval),"desc",strlen("desc")+1,(void**)&desSet) && IS_STRING == Z_TYPE_PP(desSet) ){
		descDefault = Z_STRVAL_PP(desSet);
	}




	strcat2(&titleString,"<title>",titleDefault,"</title>\r\n",NULL);
	strcat2(&keywordsString,"\t<meta name=\"keywords\" content=\"",keyDefault,"\" />\r\n",NULL);
	strcat2(&desString,"\t<meta name=\"description\" content=\"",descDefault,"\" />\r\n",NULL);


	strcat2(&showString,titleString,keywordsString,desString,NULL);

	php_printf("%s",showString);

	efree(titleString);
	efree(keywordsString);
	efree(desString);
	efree(showString);

}

void CSmarty_cnSubstr(char *sourcestr,int llen,char *appendStr,char **returnString TSRMLS_DC)
{
	int		i = 0,
			str_length = 0,
			ascnum,
			cutLength;

	float	n = 0.0;

	char	*tempStr;

	smart_str returnstr = {0};


	str_length = strlen(sourcestr);
	cutLength = llen;

	if(str_length == 0){
		*returnString = estrdup(sourcestr);
		return;
	}

	while( n < cutLength && i < str_length)
	{
		substr(sourcestr,i,1,&tempStr);
		
		ascnum = (int)((unsigned char)tempStr[0]);

		if (ascnum >= 252) 
		{
			char *nowAppend;
			substr(sourcestr,i,6,&nowAppend);
			i = i + 6;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum >= 248){

			char *nowAppend;
			substr(sourcestr,i,5,&nowAppend);
			i = i + 5;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum >= 240){

			char *nowAppend;
			substr(sourcestr,i,4,&nowAppend);
			i = i + 4;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum >= 224){

			char *nowAppend;
			substr(sourcestr,i,3,&nowAppend);
			i = i + 3;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum >= 192){

			char *nowAppend;
			substr(sourcestr,i,2,&nowAppend);
			i = i + 2;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum >= 65 && ascnum <=90 && ascnum != 73){

			char *nowAppend;
			substr(sourcestr,i,1,&nowAppend);
			i = i + 1;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else if(ascnum == 37 || ascnum == 38 || ascnum == 64 || ascnum == 109 || ascnum == 119){ //%,&,@,m,w
			
			char *nowAppend;
			substr(sourcestr,i,1,&nowAppend);
			i = i + 1;
			n++;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}else{

			char *nowAppend;
			substr(sourcestr,i,1,&nowAppend);
			i = i + 1;
			n = n+0.5;
			smart_str_appends(&returnstr,nowAppend);
			efree(nowAppend);

		}
		efree(tempStr);
	}

	if(i < str_length){
		smart_str_appends(&returnstr,appendStr);
	}

	smart_str_0(&returnstr);

	*returnString = estrdup(returnstr.c);

	smart_str_free(&returnstr);
}

PHP_METHOD(CSmarty,cn_substr)
{
	zval	*params,
			**str,
			**l,
			**append,
			*other1,
			*other2,
			*other3;

	int		llen = 0;

	char	*appendStr,
			*string,
			*returnString;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|zzz",&params,&other1,&other2,&other3) == FAILURE){
		return;
	}	

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(params),"str",4,(void**)&str) && IS_STRING == Z_TYPE_PP(str) ){
		string = estrdup(Z_STRVAL_PP(str));
	}else{
		return;
	}


	if(SUCCESS != zend_hash_find(Z_ARRVAL_P(params),"l",2,(void**)&l)){
		llen = 80;
	}else{
		if(IS_LONG == Z_TYPE_PP(l)){
			llen = Z_LVAL_PP(l);
		}else if(IS_STRING == Z_TYPE_PP(l)){
			llen = toInt(Z_STRVAL_PP(l));
		}else{
			llen = 80;
		}
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(params),"append",strlen("append")+1,(void**)&append) && IS_STRING == Z_TYPE_PP(append)){
		appendStr = estrdup(Z_STRVAL_PP(append));
	}else{
		appendStr = estrdup("..");
	}



	CSmarty_cnSubstr(string,llen,appendStr,&returnString TSRMLS_CC);


	RETVAL_STRING(returnString,1);


	efree(appendStr);
	efree(string);
	efree(returnString);
}

void CSmarty_sayTime(long timestamp,char **returnString TSRMLS_DC){

	zval	*nowTime;

	long	nowTimeStamp = 0,
			toCast = 0,
			showInt;

	char	returnChar[1024],
			*sayType;

	//当前时间戳
	microtime(&nowTime);
	
	nowTimeStamp = (long)Z_DVAL_P(nowTime);

	toCast = nowTimeStamp - timestamp;


	if(toCast < 86400){
		showInt = (int)ceil(toCast/3600);
		base64Decode("5bCP5pe25YmN",&sayType);	//小时前
	}else if(toCast >= 86400 && toCast < 30*86400){
		showInt = (int)ceil(toCast/86400);
		base64Decode("5aSp5YmN",&sayType);//天前
	}else if(toCast >= 30*86400 && toCast < 30*86400*12){
		showInt = (int)ceil(toCast / (86400*30));
		base64Decode("5pyI5YmN",&sayType);//月前
	}else if(toCast >= 30*86400*12){
		showInt = (int)ceil(toCast / (86400*30*12));
		base64Decode("5bm05YmN",&sayType);//月前
	}else{
		showInt = (int)ceil(toCast / (86400));
		base64Decode("5bCP5pe25YmN",&sayType);	//小时前
	}
	
	sprintf(returnChar,"%d%s",showInt,sayType);

	*returnString = estrdup(returnChar);

	zval_ptr_dtor(&nowTime);
	efree(sayType);
}

PHP_METHOD(CSmarty,sayTime)
{
	zval	*params,
			**append,
			*other1,
			*other2,
			*other3,
			**time;

	long	timestamp = 0;

	char	*sayString;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|zzz",&params,&other1,&other2,&other3) == FAILURE){
		return;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(params),"time",5,(void**)&time) && IS_LONG == Z_TYPE_PP(time) ){
		timestamp = Z_LVAL_PP(time);
	}

	//返回空
	if(0 == timestamp){
		RETURN_STRING("",1);
	}

	CSmarty_sayTime(timestamp,&sayString TSRMLS_CC);

	RETVAL_STRING(sayString,1);
	efree(sayString);
}

//为兼容php的函数 新版应废弃此函数
PHP_METHOD(CSmarty,checkRight)
{
	zval	*params,
			*other1,
			*other2,
			*other3,
			**controller,
			**action,
			constructVal,
			callReturn,
			*returnObject,
			*paramsList[2],
			param1,
			param2,
			*appPath,
			adminPath[1024],
			*object;

	zend_class_entry	**adminControllerPP,
						*adminController;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz|zz",&params,&other1,&other2,&other3) == FAILURE){
		RETURN_NULL();
	}

	if(IS_ARRAY != Z_TYPE_P(params)){
		RETURN_NULL();
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(params),"c",strlen("c")+1,(void**)&controller) && IS_STRING == Z_TYPE_PP(controller) ){	
	}else{
		RETURN_NULL();
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(params),"a",strlen("a")+1,(void**)&action) && IS_STRING == Z_TYPE_PP(action) ){	
	}else{
		RETURN_NULL();
	}


	//判断文件存在 CLoader_loadFile
	if(zend_hash_find(EG(class_table),"quickadmincontroller",strlen("quickadmincontroller")+1,(void**)&adminControllerPP) == FAILURE){
		appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);
		sprintf(adminPath,"%s%s",Z_STRVAL_P(appPath),"/application/classes/QuickAdminController.php");
		CLoader_loadFile(adminPath);
	}

	//调用 QuickAdminController::checkRight 验证
	if(zend_hash_find(EG(class_table),"quickadmincontroller",strlen("quickadmincontroller")+1,(void**)&adminControllerPP) == FAILURE){
		RETURN_NULL();
	}

	adminController = *adminControllerPP;

	if(!zend_hash_exists(&adminController->function_table,"checkright",strlen("checkright")+1)){ 
		RETURN_NULL();
	}

	MAKE_STD_ZVAL(object);
	object_init_ex(object,adminController);

	//判断是否存在checkRight方法
	paramsList[0] = &param1;
	paramsList[1] = &param2;
	MAKE_STD_ZVAL(paramsList[0]);
	MAKE_STD_ZVAL(paramsList[1]);
	ZVAL_STRING(paramsList[0],Z_STRVAL_PP(controller),1);
	ZVAL_STRING(paramsList[1],Z_STRVAL_PP(action),1);
	INIT_ZVAL(constructVal);
	ZVAL_STRING(&constructVal,"checkRight", 0);
	call_user_function(NULL, &object, &constructVal, &callReturn, 2, paramsList TSRMLS_CC);
	returnObject = &callReturn;

	//成功
	if(IS_BOOL == Z_TYPE_P(returnObject) && 1 == Z_LVAL_P(returnObject)){
		RETVAL_ZVAL(other1,1,0);
	}else{
		RETVAL_NULL();
	}

	zval_dtor(&callReturn);
	zval_ptr_dtor(&paramsList[0]);
	zval_ptr_dtor(&paramsList[1]);
	zval_ptr_dtor(&object);
}