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
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


//zend类方法
zend_function_entry CWord_functions[] = {
	PHP_ME(CWord,getStringFirstChar,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CWord)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CWord",CWord_functions);
	CWordCe = zend_register_internal_class(&funCe TSRMLS_CC);
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

