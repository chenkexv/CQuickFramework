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
#include "php_CController.h"
#include "php_CWebApp.h"
#include "php_CException.h"

ZEND_BEGIN_ARG_INFO_EX(session_set_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

//zend类方法
zend_function_entry CController_functions[] = {
	PHP_ME(CController,Args,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,File,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,createUrl,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,getView,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,display,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,fetch,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,assign,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,setTitle,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,setKeyword,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,setDescription,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,setPageData,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,showMessage,NULL,ZEND_ACC_PROTECTED)
	PHP_ME(CController,showMessageData,NULL,ZEND_ACC_PROTECTED)
	PHP_ME(CController,showErrorMessage,NULL,ZEND_ACC_PROTECTED)
	PHP_ME(CController,setLanguage,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CController,__set,session_set_arginfo,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CController)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CController",CController_functions);
	CControllerCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量
	zend_declare_property_null(CControllerCe, ZEND_STRL("viewObject"),ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(CControllerCe, ZEND_STRL("pageInfo"),ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(CControllerCe, ZEND_STRL("langType"),"",ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(CControllerCe, ZEND_STRL("lang"),ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CController,Args)
{
	char *key = "",
		 *type = "string",
		 *from = "REQUEST";

	zval *returnString;

	int keyLen = 0,
		typeLen = 0,
		fromLen = 0,
		noFilter = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|ssb",&key,&keyLen,&type,&typeLen,&from,&fromLen,&noFilter) == FAILURE){
		RETURN_NULL();
	}

	CRequest_Args(key,type,from,noFilter,&returnString TSRMLS_CC);
	RETVAL_ZVAL(returnString,1,1);
}

PHP_METHOD(CController,File)
{
	char	*key;
	int		keyLen = 0;
	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		RETURN_NULL();
	}

	//获取文件
	getFileParams(key,&returnZval);
	RETVAL_ZVAL(returnZval,1,0);
}

PHP_METHOD(CController,createUrl)
{
	char	*c,
			*a,
			*url;

	int		cLen,
			aLen;

	zval	*otherParams,
			*saveThis,
			*waitParams;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss|z",&c,&cLen,&a,&aLen,&otherParams) == FAILURE){
		RETURN_NULL();
	}

	//将c和a新增到otherParams
	if(IS_ARRAY != Z_TYPE_P(otherParams)){
		MAKE_STD_ZVAL(otherParams);
		array_init(otherParams);
	}

	//赋值
	add_assoc_string(otherParams,"c",c,0);
	add_assoc_string(otherParams,"a",a,0);

	CRouteParse_url(otherParams,&url TSRMLS_CC);
	RETVAL_STRING(url,1);
}

PHP_METHOD(CController,getView)
{
	zval	*returnZval,
			*cconfigInstanceZval,
			*useQuickTemplate;

	char	*templateUsed;

	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);

	CView_factory(templateUsed,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}

//对smarty的display方法进行封装
PHP_METHOD(CController,display){
	
	char	*templateName,
			*gettemplateName,
			*layoutName = "",
			*layoutPath,
			layoutNum[10240],
			templateNum[10240],
			*templateUsed;

	zval	*getLayout,
			*viewObjectZval,
			*cacheNum = NULL,
			*pageInfoZval,
			*langInfoZval,
			*cconfigInstanceZval,
			*useQuickTemplate;

	int		templateLen,
			cache = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|bz",&gettemplateName,&templateLen,&cache,&cacheNum) == FAILURE){
		RETURN_NULL();
		return;
	}

	//模板名称
	if(strstr(gettemplateName,".html") == NULL){
		strcat2(&templateName,gettemplateName,".html",NULL);
	}else{
		templateName = estrdup(gettemplateName);
	}

	if(strlen(templateName) <= 0){
		RETVAL_NULL();
		return;
	}

	//布局文件
	getLayout = zend_read_property(CControllerCe,getThis(),"layout",strlen("layout"),1 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(getLayout) && Z_STRLEN_P(getLayout) > 0){
		strcat2(&layoutName,Z_STRVAL_P(getLayout),".html",NULL);
	}else{
		layoutName = "";
	}

	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);

	//清楚模板缓存
	if(strlen(layoutName) > 0){
		strcat2(&layoutPath,"layout/",layoutName,NULL);

		//判断布局文件是否存在
		if(!templateExists(viewObjectZval,layoutPath TSRMLS_CC)){
			char errorMessage[10240];
			sprintf(errorMessage,"%s%s","[ViewException] The view Layout file does not exist ",layoutPath);
			zend_throw_exception(CViewExceptionCe, errorMessage, 12001 TSRMLS_CC);
			RETVAL_NULL();
			return;
		}


	}else{
		layoutPath = "";
	}

	//缓存编号路径
	if(cacheNum != NULL && IS_LONG == Z_TYPE_P(cacheNum)){

		sprintf(layoutNum,"%s%d",layoutName,Z_LVAL_P(cacheNum));
		sprintf(templateNum,"%s%d",templateName,Z_LVAL_P(cacheNum));

	}else if(cacheNum != NULL && IS_STRING == Z_TYPE_P(cacheNum)){

		sprintf(layoutNum,"%s%s",layoutName,Z_STRVAL_P(cacheNum));
		sprintf(templateNum,"%s%s",templateName,Z_STRVAL_P(cacheNum));

	}else{
		sprintf(templateNum,"%s",templateName);
		sprintf(layoutNum,"%s",layoutPath);
	}

	if(cache == 0){	
		clearSmartyCache(viewObjectZval,layoutPath,layoutNum TSRMLS_CC);
		clearSmartyCache(viewObjectZval,templateName,templateNum TSRMLS_CC);
	}

	//判断模板存在
	if(!templateExists(viewObjectZval,templateName TSRMLS_CC)){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ViewException] The view file does not exist ",templateName);
		zend_throw_exception(CViewExceptionCe, errorMessage, 12001 TSRMLS_CC);
		RETVAL_NULL();
		return;
	}

	//pageInfo的值
	pageInfoZval = zend_read_property(CControllerCe,getThis(),ZEND_STRL("pageInfo"),1 TSRMLS_CC);
	smarty_assign_ex(viewObjectZval,"pageInfo",pageInfoZval TSRMLS_CC);

	//set lang info
	langInfoZval = zend_read_property(CControllerCe,getThis(),ZEND_STRL("lang"),1 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(langInfoZval)){
		smarty_assign_ex(viewObjectZval,"lang",langInfoZval TSRMLS_CC);
	}

	//存在布局文件时
	if(strlen(layoutPath) > 0){
		smarty_assign(viewObjectZval,"CONTENT_INSERET_LAYOUT",templateName TSRMLS_CC);
		smarty_display(viewObjectZval,layoutPath,templateNum TSRMLS_CC);
	}else{
		smarty_display(viewObjectZval,templateName,templateNum TSRMLS_CC);
	}

	//触发呈现视图HOOKS_VIEW_SHOW的钩子 常用于Pv统计
	MODULE_BEGIN
		zval	*paramsList[1],
				param1,
				*saveTemplateName;
		paramsList[0] = &param1;
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(saveTemplateName);
		ZVAL_STRING(saveTemplateName,templateName,1);
		ZVAL_ZVAL(paramsList[0],saveTemplateName,1,1);
		CHooks_callHooks("HOOKS_VIEW_SHOW",paramsList,1 TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
	MODULE_END

	efree(templateName);
	if(strlen(layoutName) > 0){
		efree(layoutName);
	}
	if(strlen(layoutPath) > 0){
		efree(layoutPath);
	}

	zval_ptr_dtor(&viewObjectZval);
}

PHP_METHOD(CController,fetch)
{
	char	*gettemplateName,
			*templateName,
			*templateUsed;

	int		templateLen;

	zval	*viewObjectZval,
			*fetchResult,
			*returnZval,
			*cconfigInstanceZval,
			*useQuickTemplate,
			*langInfoZval,
			*pageInfoZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&gettemplateName,&templateLen) == FAILURE){
		RETURN_NULL();
		return;
	}

	//模板名称
	if(strstr(gettemplateName,".html") == NULL){
		strcat2(&templateName,gettemplateName,".html",NULL);
	}else{
		templateName = estrdup(gettemplateName);
	}


	if(strlen(templateName) <= 0){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ViewException] The view file does not exist ",templateName);
		zend_throw_exception(CViewExceptionCe, errorMessage, 12002 TSRMLS_CC);
		efree(templateName);
		RETVAL_NULL();
		return;
	}

	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);

	//判断模板存在
	if(!templateExists(viewObjectZval,templateName TSRMLS_CC)){
		char errorMessage[10240];
		sprintf(errorMessage,"%s%s","[ViewException] The view file does not exist ",templateName);
		zend_throw_exception(CViewExceptionCe, errorMessage, 12003 TSRMLS_CC);
		efree(templateName);
		zval_ptr_dtor(&viewObjectZval);
		RETVAL_NULL();
		return;
	}

	//pageInfo的值
	pageInfoZval = zend_read_property(CControllerCe,getThis(),ZEND_STRL("pageInfo"),1 TSRMLS_CC);
	smarty_assign_ex(viewObjectZval,"pageInfo",pageInfoZval TSRMLS_CC);

	//set lang info
	langInfoZval = zend_read_property(CControllerCe,getThis(),ZEND_STRL("lang"),1 TSRMLS_CC);
	if(IS_ARRAY == Z_TYPE_P(langInfoZval)){
		smarty_assign_ex(viewObjectZval,"lang",langInfoZval TSRMLS_CC);
	}

	//调用fetch方法
	smarty_fetch(viewObjectZval,templateName,&fetchResult TSRMLS_CC);

	ZVAL_ZVAL(return_value,fetchResult,1,0);
	zval_ptr_dtor(&fetchResult);
	zval_ptr_dtor(&viewObjectZval);
	efree(templateName);
}

PHP_METHOD(CController,assign)
{

	char	*key;
	int		keyLen;
	zval	*val,
			*viewObjectZval,
			*cconfigInstanceZval,
			*useQuickTemplate;

	char	*templateUsed;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&key,&keyLen,&val) == FAILURE){
		RETURN_FALSE;
	}

	//读取USE_QUICKTEMPLATE配置
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = estrdup("CQuickTemplate");
	}else{
		templateUsed = estrdup("smarty");
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);


	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);


	//赋值
	smarty_assign_ex(viewObjectZval,key,val TSRMLS_CC);

	zval_ptr_dtor(&viewObjectZval);
	efree(templateUsed);
}

PHP_METHOD(CController,setTitle)
{
	zval	*pageInfo;

	char	*val;
	int		valLen;

	pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pageInfo)){	
		zval *savePage;
		MAKE_STD_ZVAL(savePage);
		array_init(savePage);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),savePage TSRMLS_CC);
		pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
		zval_ptr_dtor(&savePage);
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//设置title
	if(Z_TYPE_P(pageInfo) == IS_ARRAY){
		add_assoc_string(pageInfo,"title",val,1);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),pageInfo TSRMLS_CC);
	}
}

PHP_METHOD(CController,setKeyword)
{
	zval	*pageInfo;

	char	*val;
	int		valLen;

	pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pageInfo)){	
		zval *savePage;
		MAKE_STD_ZVAL(savePage);
		array_init(savePage);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),savePage TSRMLS_CC);
		pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
		zval_ptr_dtor(&savePage);
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//设置title
	if(Z_TYPE_P(pageInfo) == IS_ARRAY){

		add_assoc_string(pageInfo,"keyword",val,1);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),pageInfo TSRMLS_CC);
	}
}

PHP_METHOD(CController,setDescription)
{
	zval	*pageInfo;

	char	*val;
	int		valLen;

	pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pageInfo)){	
		zval *savePage;
		MAKE_STD_ZVAL(savePage);
		array_init(savePage);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),savePage TSRMLS_CC);
		pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
		zval_ptr_dtor(&savePage);
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&val,&valLen) == FAILURE){
		return;
	}

	//设置desc
	if(Z_TYPE_P(pageInfo) == IS_ARRAY){
		add_assoc_string(pageInfo,"desc",val,1);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),pageInfo TSRMLS_CC);
	}
}

PHP_METHOD(CController,setPageData)
{
	zval	*pageInfo;

	char	*val1 = "",
			*val2 = "",
			*val3 = "";
	int		val1Len = 0,
			val2Len = 0,
			val3Len = 0;


	pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pageInfo)){	
		zval *savePage;
		MAKE_STD_ZVAL(savePage);
		array_init(savePage);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),savePage TSRMLS_CC);
		pageInfo = zend_read_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),0 TSRMLS_CC);
		zval_ptr_dtor(&savePage);
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|sss",&val1,&val1Len,&val2,&val2Len,&val3,&val3Len) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	//设置title
	if(Z_TYPE_P(pageInfo) == IS_ARRAY){

		add_assoc_string(pageInfo,"title",val1,1);
		add_assoc_string(pageInfo,"keyword",val2,1);
		add_assoc_string(pageInfo,"desc",val3,1);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("pageInfo"),pageInfo TSRMLS_CC);

	}
	RETVAL_TRUE;
}

PHP_METHOD(CController,__set)
{

	zval	*viewObjectZval,
			*val,
			*cconfigInstanceZval,
			*useQuickTemplate;

	char	*key,
			*templateUsed;
	int		keyLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&key,&keyLen,&val) == FAILURE){
		return;
	}

	//读取USE_QUICKTEMPLATE配置
	templateUsed = "smarty";
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("USE_QUICKTEMPLATE",cconfigInstanceZval,&useQuickTemplate TSRMLS_CC);

	if(IS_LONG == Z_TYPE_P(useQuickTemplate) && 1 == Z_LVAL_P(useQuickTemplate)){
		templateUsed = "CQuickTemplate";
	}

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&useQuickTemplate);

	//视图对象
	CView_factory(templateUsed,&viewObjectZval TSRMLS_CC);

	if(IS_OBJECT == Z_TYPE_P(viewObjectZval) && IS_NULL != Z_TYPE_P(val) ){
		smarty_assign_ex(viewObjectZval,key,val TSRMLS_CC);
		zval_ptr_dtor(&viewObjectZval);
	}

}

void CController_showMessage(long result,zval *nowData,char *errorId,char *thisMessage TSRMLS_DC){

	char	*outputString;

	zval	*endData,
			*errorData;

	MAKE_STD_ZVAL(endData);
	array_init(endData);

	add_assoc_bool(endData,"result",result);
	add_assoc_zval(endData,"data",nowData);

	MAKE_STD_ZVAL(errorData);
	array_init(errorData);
	add_assoc_string(errorData,"id",errorId,1);
	add_assoc_string(errorData,"message",thisMessage,1);

	add_assoc_zval(endData,"error",errorData);

	//clear output
	ob_end_clean();
	json_encode(endData,&outputString);

	php_printf("%s",outputString);

	zval_ptr_dtor(&endData);
	zval_ptr_dtor(&errorData);
	efree(outputString);

	zend_bailout();
}

PHP_METHOD(CController,showMessage)
{
	long	result = 0,
			errorId = 0;

	zval	*data = NULL,
			*nowData;
	
	char	*message,
			*thisMessage,
			errorIdString[32];
	
	int		messageLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b|zls",&result,&data,&errorId,&message,&messageLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[FatalException] Call CController->showMessage(bool offset,array data,int errorId,string message) params error");
		return;
	}

	if(messageLen == 0){
		thisMessage = estrdup("");
	}else{
		thisMessage = estrdup(message);
	}

	if(data == NULL){
		MAKE_STD_ZVAL(nowData);
		array_init(nowData);
	}else{
		MAKE_STD_ZVAL(nowData);
		ZVAL_ZVAL(nowData,data,1,0);
	}

	sprintf(errorIdString,"%d",errorId);
	CController_showMessage(result,nowData,errorIdString,thisMessage TSRMLS_CC);

	zval_ptr_dtor(&nowData);
	efree(thisMessage);
}

PHP_METHOD(CController,showMessageData)
{
	zval	*data;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&data) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[FatalException] Call CController->showMessageData(zval data) params error");
		return;
	}

	CController_showMessage(1,data,"0","" TSRMLS_CC);
}

void CController_showErrorMessage(char *error,char *errorFile TSRMLS_DC)
{
	zval	*cconfigInstanceZval,
			*thisErrorInfo,
			*data;

	char	*showError;

	//check error configs
	CConfig_getInstance(errorFile,&cconfigInstanceZval TSRMLS_CC);

	if(isdigitstr(error)){
		CConfig_loadIntKey(toInt(error),cconfigInstanceZval,&thisErrorInfo TSRMLS_CC);
	}else{
		CConfig_load(error,cconfigInstanceZval,&thisErrorInfo TSRMLS_CC);
	}

	if(IS_STRING == Z_TYPE_P(thisErrorInfo)){
		showError = estrdup(Z_STRVAL_P(thisErrorInfo));
	}else{
		showError = estrdup(error);
	}

	MAKE_STD_ZVAL(data);
	array_init(data);

	CController_showMessage(0,data,error,showError TSRMLS_CC);

	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&thisErrorInfo);
	zval_ptr_dtor(&data);
	efree(showError);
}

PHP_METHOD(CController,showErrorMessage)
{
	zval	*error;

	char	*errorFile,
			*configName;

	int		errorFileLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z|s",&error,&errorFile,&errorFileLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[FatalException] Call CController->showErrorMessage(string/int data,string errorFile) params error");
		return;
	}

	if(IS_STRING != Z_TYPE_P(error) && IS_LONG != Z_TYPE_P(error)){
		php_error_docref(NULL TSRMLS_CC,E_ERROR,"[FatalException] Call CController->showErrorMessage(string/int data,string errorFile) params error");
		return;
	}

	//tostring
	convert_to_string(error);

	//configName
	if(errorFileLen == 0){
		configName = estrdup("errors");
	}else{
		configName = estrdup(errorFile);
	}

	CController_showErrorMessage(Z_STRVAL_P(error),configName TSRMLS_CC);

	efree(configName);
}

PHP_METHOD(CController,setLanguage)
{
	zval	*cconfigInstanceZval,
			*defaultLang,
			*classSetLang,
			*appPath;

	char	*defaultLanguage,
			*nowRequestLang,
			*getLang,
			*postLang,
			*cookieLang,
			*langFilePath;
	
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("DEFAULT_LANG",cconfigInstanceZval,&defaultLang TSRMLS_CC);
	
	classSetLang = zend_read_property(CControllerCe,getThis(), ZEND_STRL("langType"),0 TSRMLS_CC);

	//get from config file
	if(IS_STRING == Z_TYPE_P(defaultLang)){
		defaultLanguage = estrdup(Z_STRVAL_P(defaultLang));
	}else{
		//un use lang
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&defaultLang);
		return;
	}


	if(IS_STRING == Z_TYPE_P(classSetLang) && strlen(Z_STRVAL_P(classSetLang)) > 0){
		nowRequestLang = estrdup(Z_STRVAL_P(classSetLang));
	}else{

		nowRequestLang = estrdup(defaultLanguage);

		//check get have lang
		getGetParams("lang",&getLang);
		if(getLang != NULL){
			efree(nowRequestLang);
			nowRequestLang = getLang;
		}

		//check post have lang
		getPostParams("lang",&postLang);
		if(postLang != NULL){
			efree(nowRequestLang);
			nowRequestLang = postLang;
		}

		//get cookie
		CCookie_get("lang",&cookieLang);
		if(strlen(cookieLang) > 0){
			efree(nowRequestLang);
			nowRequestLang = estrdup(cookieLang);
		}
		efree(cookieLang);
	}

	//check lang file is exists
	appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);
	spprintf(&langFilePath,0,"%s%s%s%s",Z_STRVAL_P(appPath),"/application/languages/",nowRequestLang,".php");
	if(SUCCESS != fileExist(langFilePath)){
		efree(langFilePath);
		spprintf(&langFilePath,0,"%s%s%s%s",Z_STRVAL_P(appPath),"/application/languages/",defaultLanguage,".php");
	}

	if(SUCCESS == fileExist(langFilePath)){
		//load this file
		zval	*langClassContent;
		CLoader_import(langFilePath,langFilePath,&langClassContent TSRMLS_CC);
		zend_update_property(CControllerCe,getThis(), ZEND_STRL("lang"),langClassContent TSRMLS_CC);
		zval_ptr_dtor(&langClassContent);
	}

	efree(nowRequestLang);
	efree(langFilePath);
	efree(defaultLanguage);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&defaultLang);
	
}