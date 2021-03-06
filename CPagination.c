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
#include "php_CPagination.h"
#include "php_CException.h"
#include "php_CRoute.h"
#include "php_CRequest.h"
#include "php_CRouteParse.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


zend_function_entry CPagination_functions[] = {
	PHP_ME(CPagination,fpage,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPagination,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CPagination,setUri,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPagination,setRoute,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPagination,setTotalRows,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CPagination,setPageRows,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};


CMYFRAME_REGISTER_CLASS_RUN(CPagination)
{

	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CPagination",CPagination_functions);
	CPaginationCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_long(CPaginationCe, ZEND_STRL("totalRows"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPaginationCe, ZEND_STRL("nowPage"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPaginationCe, ZEND_STRL("pageRows"),20,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CPaginationCe, ZEND_STRL("uri"),"/",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPaginationCe, ZEND_STRL("totalPage"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPaginationCe, ZEND_STRL("configs"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPaginationCe, ZEND_STRL("routeArray"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CPaginationCe, ZEND_STRL("listNum"),8,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CPaginationCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}

void CPagination_getFirstString(int nowPage,zval *configZval,zval *object,char **returnString TSRMLS_DC){

	zval	**firstConfigs;
	char	*firstSay;

	
	zend_hash_find(Z_ARRVAL_P(configZval),"first",strlen("first")+1,(void**)&firstConfigs);


	if(nowPage == 1){
		strcat2(returnString,"<a href=\"javascript:void(0)\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
	}else{

		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",1);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(returnString,"<a href=\"",hrefUrl,"\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}
}

void CPagination_getPrevString(int nowPage,zval *configZval,zval *object,char **returnString TSRMLS_DC){

	zval	**firstConfigs;
	char	*firstSay;

	zend_hash_find(Z_ARRVAL_P(configZval),"prev",strlen("prev")+1,(void**)&firstConfigs);

	if(nowPage == 1){
		strcat2(returnString,"<a href=\"javascript:void(0)\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
	}else{

		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;

		//璇诲彇褰撳墠URI
		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPage-1);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(returnString,"<a href=\"",hrefUrl,"\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}
}

void CPagination_getNextPageString(int nowPage,int totalPage,zval *configZval,zval *object,char **returnString TSRMLS_DC){

	zval	**firstConfigs;
	char	*firstSay;

	zend_hash_find(Z_ARRVAL_P(configZval),"next",strlen("next")+1,(void**)&firstConfigs);


	if(nowPage >= totalPage){
		strcat2(returnString,"<a href=\"javascript:void(0)\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
	}else{

		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;


		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPage+1);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(returnString,"<a href=\"",hrefUrl,"\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}
}

void CPagination_getLastPageString(int nowPage,int totalPage,zval *configZval,zval *object,char **returnString TSRMLS_DC){

	zval	**firstConfigs;
	char	*firstSay;


	zend_hash_find(Z_ARRVAL_P(configZval),"last",strlen("last")+1,(void**)&firstConfigs);

	if(nowPage >= totalPage){
		strcat2(returnString,"<a href=\"javascript:void(0)\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
	}else{

		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;

		//璇诲彇褰撳墠URI
		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",totalPage);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(returnString,"<a href=\"",hrefUrl,"\">",Z_STRVAL_PP(firstConfigs),"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}
}

void CPagination_getPageListString(int nowPage,zval *configZval,zval *object,char **returnString TSRMLS_DC){

	char	*before1 = "",
			*before2 = "",
			*before3 = "",
			*before4 = "",
			*now = "",
			*after1 = "",
			*after2 = "",
			*after3 = "",
			*after4 = "",
			*connectSymbol;

	zval	*nowUri,
			*totalPage;

	totalPage = zend_read_property(CPaginationCe,object,ZEND_STRL("totalPage"),0 TSRMLS_CC);

	if(nowPage - 1 >= 1){

		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage-1;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&before1,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage - 2 >= 1){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage-2;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&before2,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage - 3 >= 1){
			char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage-3;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&before3,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage - 4 >= 1){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage-4;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&before4,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		if(IS_ARRAY != Z_TYPE_P(thisArr)){
			array_init(thisArr);
		}
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&now,"<a id=\"PageOn\" class=\"PageOn\" href=\"javascript:void(0)\">",nowPageString,"</a>",NULL);
		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage + 1 <= Z_LVAL_P(totalPage)){
	
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage+1;
		sprintf(nowPageString,"%d",nowPageInt);


		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&after1,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);

		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage + 2 <= Z_LVAL_P(totalPage)){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage+2;
		sprintf(nowPageString,"%d",nowPageInt);


		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		//生成URL
		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&after2,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);

		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage + 3 <= Z_LVAL_P(totalPage)){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage+3;
		sprintf(nowPageString,"%d",nowPageInt);


		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);


		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&after3,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);

		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}

	if(nowPage + 4 <= Z_LVAL_P(totalPage)){
		char nowPageString[32];
		zval	*nowUriArr,
			    *thisArr;

		char	*hrefUrl;
		int		nowPageInt = 1;

		nowPageInt = nowPage+4;
		sprintf(nowPageString,"%d",nowPageInt);

		nowUriArr = zend_read_property(CPaginationCe,object,ZEND_STRL("routeArray"),0 TSRMLS_CC);
		MAKE_STD_ZVAL(thisArr);
		ZVAL_ZVAL(thisArr,nowUriArr,1,0);
		add_assoc_long(thisArr,"page",nowPageInt);

		CRouteParse_url(thisArr,&hrefUrl TSRMLS_CC);
		strcat2(&after4,"<a href=\"",hrefUrl,"\">",nowPageString,"</a>",NULL);

		efree(hrefUrl);
		zval_ptr_dtor(&thisArr);
	}


	strcat2(returnString,before4,before3,before2,before1,now,after1,after2,after3,after4,NULL);

	if(strlen(before1) > 0){
		efree(before1);
	}
	if(strlen(before2) > 0){
		efree(before2);
	}
	if(strlen(before3) > 0){
		efree(before3);
	}
	if(strlen(before4) > 0){
		efree(before4);
	}
	if(strlen(after1) > 0){
		efree(after1);
	}
	if(strlen(after2) > 0){
		efree(after2);
	}
	if(strlen(after3) > 0){
		efree(after3);
	}
	if(strlen(after4) > 0){
		efree(after4);
	}
	if(strlen(now) > 0){
		efree(now);
	}

}

PHP_METHOD(CPagination,fpage)
{
	zval	*showKeys = NULL,
			*showValues,
			*totalPage,
			*configZval,
			*nowPageZval,
			*uriZval,
			*totalRows;

	zval	**header,
			**prev,
			**next,
			**first,
			**last;

	char	*line0,
			*line1,
			*line2,
			*line3,
			*line4,
			*line5,
			*line6,
			*line7,
			*line8,
			totalRowsString[32];


	int	 needClear = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&showKeys) == FAILURE){
	}

	//璇诲彇鍒嗛〉淇℃伅
	totalPage = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("totalPage"),0 TSRMLS_CC);
	if(Z_LVAL_P(totalPage) <= 1){
		RETVAL_STRING("",1);
		return;
	}


	//璇诲彇閰嶇疆
	totalRows = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("totalRows"),0 TSRMLS_CC);
	configZval = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("configs"),0 TSRMLS_CC);
	nowPageZval = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("nowPage"),0 TSRMLS_CC);
	uriZval = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("uri"),0 TSRMLS_CC);


	zend_hash_find(Z_ARRVAL_P(configZval),"header",strlen("header")+1,(void**)&header);

	if(showKeys == NULL || IS_ARRAY != Z_TYPE_P(showKeys) ){
		needClear = 1;
		MAKE_STD_ZVAL(showKeys);
		array_init(showKeys);
		add_next_index_long(showKeys,3);
		add_next_index_long(showKeys,4);
		add_next_index_long(showKeys,5);
		add_next_index_long(showKeys,6);
		add_next_index_long(showKeys,7);
	}


	MAKE_STD_ZVAL(showValues);
	array_init(showValues);


	sprintf(totalRowsString,"%d",Z_LVAL_P(totalRows));
	strcat2(&line0,"<div class=\"page_left\">&nbsp;&nbsp;Total<b>,totalRowsString,</b>",Z_STRVAL_PP(header),"&nbsp;&nbsp;",NULL);


	strcat2(&line1,"&nbsp;&nbsp;now Page show <b></b> rows",NULL);


	strcat2(&line2,"&nbsp;&nbsp;<b></b> page",NULL);


	CPagination_getFirstString(Z_LVAL_P(nowPageZval),configZval,getThis(),&line3 TSRMLS_CC);


	CPagination_getPrevString(Z_LVAL_P(nowPageZval),configZval,getThis(),&line4 TSRMLS_CC);

	//PageList
	CPagination_getPageListString(Z_LVAL_P(nowPageZval),configZval,getThis(),&line5 TSRMLS_CC);


	CPagination_getNextPageString(Z_LVAL_P(nowPageZval),Z_LVAL_P(totalPage),configZval,getThis(),&line6 TSRMLS_CC);


	CPagination_getLastPageString(Z_LVAL_P(nowPageZval),Z_LVAL_P(totalPage),configZval,getThis(),&line7 TSRMLS_CC);

	add_index_string(showValues,0,line0,1);
	add_index_string(showValues,1,line1,1);
	add_index_string(showValues,2,line2,1);
	add_index_string(showValues,3,line3,1);
	add_index_string(showValues,4,line4,1);
	add_index_string(showValues,5,line5,1);
	add_index_string(showValues,6,line6,1);
	add_index_string(showValues,7,line7,1);
	add_index_string(showValues,8,"",1);


	MODULE_BEGIN
		int		i,j;
		zval	**showKeysAttr,
				**thisKeyAttrHtml;
		smart_str pageHtml = {0};

		j = zend_hash_num_elements(Z_ARRVAL_P(showKeys));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(showKeys));
		for(i = 0 ; i < j ; i++){
		
			zend_hash_get_current_data(Z_ARRVAL_P(showKeys),(void**)&showKeysAttr);

			if(IS_LONG != Z_TYPE_PP(showKeysAttr)){
				zend_hash_move_forward(Z_ARRVAL_P(showKeys));
				continue;
			}


			if(SUCCESS == zend_hash_index_find( Z_ARRVAL_P(showValues),Z_LVAL_PP(showKeysAttr),(void**)&thisKeyAttrHtml) ){
				smart_str_appends(&pageHtml,Z_STRVAL_PP(thisKeyAttrHtml));
			}
			zend_hash_move_forward(Z_ARRVAL_P(showKeys));
		}
		smart_str_0(&pageHtml);
		RETVAL_STRING(pageHtml.c,1);
		smart_str_free(&pageHtml);

	MODULE_END

	efree(line0);
	efree(line1);
	efree(line2);
	efree(line3);
	efree(line4);
	efree(line5);
	efree(line6);
	efree(line7);
	if(needClear == 1){
		zval_ptr_dtor(&showKeys);
	}
	zval_ptr_dtor(&showValues);
}


int CPagination_getNowPage(){

	char *getReturn;
	int page = 1;

	getGetParams("page",&getReturn);
	if(getReturn == NULL){
		getPostParams("page",&getReturn);
	}

	if(getReturn == NULL){
		return 1;
	}

	if(isdigitstr(getReturn)){
		page = toInt(getReturn);
	}else{
		page = 1; 
	}
	efree(getReturn);
	return page;
}


PHP_METHOD(CPagination,__construct)
{
	long	totalRows = 0,
			pageRows = 20;
			
	int		nowPage = 0,
			totalPage = 0;

	zval	*requsetUri,
			*configsZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|ll",&totalRows,&pageRows) == FAILURE){
	}


	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("totalRows"),totalRows TSRMLS_CC);
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("pageRows"),pageRows TSRMLS_CC);


	nowPage = CPagination_getNowPage();
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("nowPage"),nowPage TSRMLS_CC);


	totalPage = (int)ceil((float)totalRows/pageRows);
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("totalPage"),totalPage TSRMLS_CC);

	//read now route Info
	MODULE_BEGIN
		zval	*controller,
				*action,
				*getParams,
				*saveParams,
				*cconfigInstanceZval,
				*actionPreFix;
		char	*trueAction;

		controller = zend_read_static_property(CRouteCe,ZEND_STRL("thisController"),0 TSRMLS_CC);
		action = zend_read_static_property(CRouteCe,ZEND_STRL("thisAction"),0 TSRMLS_CC);

		//get actionPre
		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
		CConfig_load("ACTION_PREFIX",cconfigInstanceZval,&actionPreFix TSRMLS_CC);
		convert_to_string(actionPreFix);
		str_replace(Z_STRVAL_P(actionPreFix),"",Z_STRVAL_P(action),&trueAction);

		//read all request Params
		getParams = PG(http_globals)[TRACK_VARS_GET];
		
		MAKE_STD_ZVAL(saveParams);
		ZVAL_ZVAL(saveParams,getParams,1,0);
		add_assoc_string(saveParams,"c",Z_STRVAL_P(controller),1);
		add_assoc_string(saveParams,"a",trueAction,1);

		zend_update_property(CPaginationCe,getThis(),ZEND_STRL("routeArray"),saveParams TSRMLS_CC);
		zval_ptr_dtor(&saveParams);
		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&actionPreFix);
		efree(trueAction);

	MODULE_END


	requsetUri = zend_read_static_property(CRouteCe,ZEND_STRL("requsetUri"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(requsetUri)){

		zval	*matchResult;

		int		findPage = 0;

		char	*unsetPageString;

		findPage = preg_match_all("/[?|&]page=\\d+/",Z_STRVAL_P(requsetUri),&matchResult);
		if(findPage != 0){

			int i;
			zval *replaceArray,
				 **findArray;

			zend_hash_index_find(Z_ARRVAL_P(matchResult),0,(void**)&findArray);

			MAKE_STD_ZVAL(replaceArray);
			array_init(replaceArray);
			for(i = 0 ; i < findPage ; i++){
				add_next_index_string(replaceArray,"",1);
			}
			str_replaceArray(*findArray,replaceArray,Z_STRVAL_P(requsetUri),&unsetPageString);

			zend_update_property_string(CPaginationCe,getThis(),ZEND_STRL("uri"),unsetPageString TSRMLS_CC);

			efree(unsetPageString);
			zval_ptr_dtor(&replaceArray);

		}else{
			zend_update_property_string(CPaginationCe,getThis(),ZEND_STRL("uri"),Z_STRVAL_P(requsetUri) TSRMLS_CC);
		}

		zval_ptr_dtor(&matchResult);
	
	}


	MODULE_BEGIN

		char	*prevPage,
				*nextPage,
				*firstPage,
				*lastPage;

		MAKE_STD_ZVAL(configsZval);
		array_init(configsZval);

		//prev
		base64Decode("5LiK5LiA6aG1",&prevPage);
		base64Decode("5LiL5LiA6aG1",&nextPage);
		base64Decode("6aaWIOmhtQ==",&firstPage);
		base64Decode("5bC+IOmhtQ==",&lastPage);


		add_assoc_string(configsZval,"header","rows",1);
		add_assoc_string(configsZval,"prev",prevPage,1);
		add_assoc_string(configsZval,"next",nextPage,1);
		add_assoc_string(configsZval,"first",firstPage,1);
		add_assoc_string(configsZval,"last",lastPage,1);

		efree(prevPage);
		efree(nextPage);
		efree(firstPage);
		efree(lastPage);

	MODULE_END

	zend_update_property(CPaginationCe,getThis(),ZEND_STRL("configs"),configsZval TSRMLS_CC);
	zval_ptr_dtor(&configsZval);
}

PHP_METHOD(CPagination,setUri)
{

}

PHP_METHOD(CPagination,setPageRows)
{
	long	totalRows = 0,
			totalPage = 0,
			pageRows = 0;

	zval	*nowRows;

	int		nowPage;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&pageRows) == FAILURE){
		RETURN_FALSE;
	}
	
	//读取当前的每页记录数
	nowRows = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("totalRows"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(nowRows)){
		totalRows = Z_LVAL_P(nowRows);
	}

	totalPage = (int)ceil((float)totalRows/pageRows);

	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("pageRows"),pageRows TSRMLS_CC);
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("totalPage"),totalPage TSRMLS_CC);

	nowPage = CPagination_getNowPage();
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("nowPage"),nowPage TSRMLS_CC);

	ZVAL_ZVAL(return_value,getThis(),1,0);
}

PHP_METHOD(CPagination,setTotalRows)
{
	long	totalRows = 0,
			totalPage = 0,
			pageRows = 0;

	zval	*nowPageRows;


	int		nowPage;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&totalRows) == FAILURE){
		RETURN_FALSE;
	}
	
	//读取当前的每页记录数
	nowPageRows = zend_read_property(CPaginationCe,getThis(),ZEND_STRL("pageRows"),0 TSRMLS_CC);
	if(IS_LONG == Z_TYPE_P(nowPageRows)){
		pageRows = Z_LVAL_P(nowPageRows);
	}

	totalPage = (int)ceil((float)totalRows/pageRows);

	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("totalRows"),totalRows TSRMLS_CC);
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("totalPage"),totalPage TSRMLS_CC);

	nowPage = CPagination_getNowPage();
	zend_update_property_long(CPaginationCe,getThis(),ZEND_STRL("nowPage"),nowPage TSRMLS_CC);

	ZVAL_ZVAL(return_value,getThis(),1,0);
}

PHP_METHOD(CPagination,setRoute)
{
	zval	*routeArray = NULL,
			*saveRoute = NULL;
	
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&routeArray) == FAILURE){
		RETURN_FALSE;
	}

	if(routeArray == NULL){
		RETURN_FALSE;
	}

	if(IS_ARRAY != Z_TYPE_P(routeArray)){
		RETURN_FALSE;
	}


	//澶嶅埗
	MAKE_STD_ZVAL(saveRoute);
	ZVAL_ZVAL(saveRoute,routeArray,1,0);
	zend_update_property(CPaginationCe,getThis(),ZEND_STRL("routeArray"),saveRoute TSRMLS_CC);
	zval_ptr_dtor(&saveRoute);

	ZVAL_ZVAL(return_value,getThis(),1,0);
}


//获取CResponse单例对象
void CPagination_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CPaginationCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CPaginationCe);

		//执行构造器
		if (CPaginationCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CPaginationCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_update_static_property(CPaginationCe,ZEND_STRL("instance"),saveObject TSRMLS_CC);
		zval_ptr_dtor(&saveObject);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return;
}