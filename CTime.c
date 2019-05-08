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
#include "php_CTime.h"


//zend类方法
zend_function_entry CTime_functions[] = {
	PHP_ME(CTime,setLanguage,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getTimestampToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getTimeareaToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getAllDaysInDates,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getNumToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getTimestampToMinute,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CTime,getBetweenTimeAreaStamp,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CTime)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CTime",CTime_functions);
	CTimeCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CTimeCe, ZEND_STRL("lang"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
}

PHP_METHOD(CTime,setLanguage)
{
	zval	*language;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&language) == FAILURE){
		return;
	}
	zend_update_static_property(CTimeCe,ZEND_STRL("lang"),language TSRMLS_CC);
}

void CTime_getDefaultSay(char *key,char **returnString)
{
	if(strcmp(key,"year") == 0){
		base64Decode("5bm0",returnString);
	}else if(strcmp(key,"hour") == 0){
		base64Decode("5bCP5pe2",returnString);
	}else if(strcmp(key,"day") == 0){
		base64Decode("5aSp",returnString);
	}else if(strcmp(key,"minute") == 0){
		base64Decode("5YiG6ZKf",returnString);
	}else if(strcmp(key,"second") == 0){
		base64Decode("56eS",returnString);
	}else if(strcmp(key,"ago") == 0){
		base64Decode("5YmN",returnString);
	}else if(strcmp(key,"now") == 0){
		base64Decode("5Yia5Yia",returnString);
	}else if(strcmp(key,"today") == 0){
		base64Decode("5LuK5aSp",returnString);
	}else if(strcmp(key,"yesterday") == 0){
		base64Decode("5pio5aSp",returnString);
	}else if(strcmp(key,"wan") == 0){
		base64Decode("5LiH",returnString);
	}else if(strcmp(key,"yi") == 0){
		base64Decode("5Lq/",returnString);
	}else{
		*returnString = estrdup("");
	}
}

void CTime_getLanguageSay(char *key,char **returnString TSRMLS_DC)
{
	zval	*lang,
			**langeVal;


	lang = zend_read_static_property(CTimeCe,ZEND_STRL("lang"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(lang)){
		CTime_getDefaultSay(key,returnString);
		return;
	}

	//read language
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(lang),key,strlen(key)+1,(void**)&langeVal)){
		convert_to_string(*langeVal);
		*returnString = estrdup(Z_STRVAL_PP(langeVal));
		return;
	}else{
		char	*getDefault;
		CTime_getDefaultSay(key,&getDefault);
		if(strlen(getDefault) != 0){
			*returnString = estrdup(getDefault);
		}else{
			*returnString = estrdup(key);
		}
		efree(getDefault);
	}
}

void CTime_getTimestampToSay(int timestamp,char **returnString TSRMLS_DC){

	long	timediff = 0;

	zval	*nowTime;

	char	*year,
			*day,
			*hour,
			*min,
			*sec,
			*now,
			*today,
			*yesterday,
			*ago,
			endSay[60],
			*thatDay,
			*nowDay;

	microtime(&nowTime);
	timediff = (int)Z_DVAL_P(nowTime) - timestamp;
	php_date_ex("ymd",timestamp,&thatDay);
	php_date_ex("ymd",(int)Z_DVAL_P(nowTime),&nowDay);
	zval_ptr_dtor(&nowTime);

	//get languages say
	CTime_getLanguageSay("year",&year TSRMLS_CC);
	CTime_getLanguageSay("hour",&hour TSRMLS_CC);
	CTime_getLanguageSay("day",&day TSRMLS_CC);
	CTime_getLanguageSay("minute",&min TSRMLS_CC);
	CTime_getLanguageSay("second",&sec TSRMLS_CC);
	CTime_getLanguageSay("now",&now TSRMLS_CC);
	CTime_getLanguageSay("today",&today TSRMLS_CC);
	CTime_getLanguageSay("yesterday",&yesterday TSRMLS_CC);
	CTime_getLanguageSay("ago",&ago TSRMLS_CC);


	if(timediff >= 0 && timediff < 5){
		sprintf(endSay,"%s",now);
	}else if(timediff >= 5 && timediff < 60){
		sprintf(endSay,"%d%s%s",timediff,sec,ago);
	}else if(timediff >= 60 && timediff < 3600){
		double nowmin = ceil(timediff/60);
		sprintf(endSay,"%d%s%s",(int)nowmin,min,ago);
	}else if(strcmp(thatDay,nowDay) == 0){
		char *thisDate;
		php_date_ex("G:i",timestamp,&thisDate);
		sprintf(endSay,"%s%s",today,thisDate);
		efree(thisDate);
	}else if(timediff > 172800 && timediff < 604800){
		double nowmin = ceil(timediff/86400);
		char *thisDate;
		php_date_ex("G:i",timestamp,&thisDate);
		sprintf(endSay,"%d%s%s %s",(int)nowmin,day,ago,thisDate);
		efree(thisDate);
	}else{
		char *thisDate;
		php_date_ex("m-d",timestamp,&thisDate);
		sprintf(endSay,"%s",thisDate);
		efree(thisDate);
	}

	*returnString = estrdup(endSay);

	efree(year);
	efree(hour);
	efree(day);
	efree(min);
	efree(sec);
	efree(now);
	efree(today);
	efree(yesterday);
	efree(ago);
	efree(thatDay);
	efree(nowDay);
}

void CTime_getTimeareaToSay(int timestamp,int timestamp2,char **returnString TSRMLS_DC){

	long	timediff = 0;

	char	*year,
			*day,
			*hour,
			*min,
			*sec,
			*now,
			*today,
			*yesterday,
			*ago,
			endSay[60],
			*thatDay,
			*nowDay;

	timediff = timestamp2 - timestamp;

	//get languages say
	CTime_getLanguageSay("year",&year TSRMLS_CC);
	CTime_getLanguageSay("hour",&hour TSRMLS_CC);
	CTime_getLanguageSay("day",&day TSRMLS_CC);
	CTime_getLanguageSay("minute",&min TSRMLS_CC);
	CTime_getLanguageSay("second",&sec TSRMLS_CC);
	CTime_getLanguageSay("now",&now TSRMLS_CC);
	CTime_getLanguageSay("today",&today TSRMLS_CC);
	CTime_getLanguageSay("yesterday",&yesterday TSRMLS_CC);
	CTime_getLanguageSay("ago",&ago TSRMLS_CC);

	if(timediff >= 0 && timediff < 60){
		sprintf(endSay,"%d%s",timediff,sec);
	}else if(timediff >= 60 && timediff < 3600){
	    double nowmin = ceil(timediff/60);
		sprintf(endSay,"%d%s",(int)nowmin,min);
	}else if(timediff >= 3600 && timediff < 86400){
		int thisHour = (int)floor(timediff/3600);
		int less = timediff - thisHour*3600;
		if(less > 60){
			int thisMin = (int)ceil(less/60);
			sprintf(endSay,"%d%s%d%s",thisHour,hour,thisMin,min);
		}else{
			sprintf(endSay,"%d%s",thisHour,hour);
		}
	}else if(timediff > 86400){

		int thisDays = (int)floor(timediff/86400);
		int thisHour = (int)floor((timediff - thisDays*86400)/3600);
		if(thisDays > 0){
			if(thisHour > 0){
				sprintf(endSay,"%d%s%d%s",thisDays,day,thisHour,hour);
			}else{
				sprintf(endSay,"%d%s",thisDays,day);
			}
		}else{
			sprintf(endSay,"%d%s",thisHour,hour);
		}
	}else{
		int thisDays = (int)ceil(timediff/86400);
		sprintf(endSay,"%d%s",thisDays,day);
	}


	*returnString = estrdup(endSay);

	efree(year);
	efree(hour);
	efree(day);
	efree(min);
	efree(sec);
	efree(now);
	efree(today);
	efree(yesterday);
	efree(ago);
}

PHP_METHOD(CTime,getTimestampToSay)
{
	long	timestamp;

	char	*endSay;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&timestamp) == FAILURE){
		return;
	}

	CTime_getTimestampToSay(timestamp,&endSay TSRMLS_CC);
	RETVAL_STRING(endSay,0);
}

PHP_METHOD(CTime,getTimeareaToSay)
{
	long	timestamp1,
			timestamp2;

	char	*endSay;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ll",&timestamp1,&timestamp2) == FAILURE){
		return;
	}

	CTime_getTimeareaToSay(timestamp1,timestamp2,&endSay TSRMLS_CC);
	RETVAL_STRING(endSay,0);
}

PHP_METHOD(CTime,getAllDaysInDates)
{
	long	timestamp1,
			timestamp2,
			dateLen1,
			dateLen2;

	char	*date1,
			*date2,
			*saveDate;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&date1,&dateLen1,&date2,&dateLen2) == FAILURE){
		return;
	}

	array_init(return_value);

	timestamp1 = php_strtotime(date1);
	timestamp2 = php_strtotime(date2);

	while(timestamp1 <= timestamp2){
		php_date_ex("Y-m-d",timestamp1,&saveDate);
		add_next_index_string(return_value,saveDate,0);
		timestamp1 = php_strtotime_ex("+1 day",timestamp1);
	}

}

PHP_METHOD(CTime,getNumToSay)
{
	long	num;

	char	*wan,
			*yi;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&num) == FAILURE){
		return;
	}

	if(num <= 9999){
		RETURN_LONG(num);
	}

	if(num < 100000000){
		double	thisNum = num/10000.0;
		int		thisIntNum = (int)(thisNum+0.5);
		char	endString[90];
		CTime_getLanguageSay("wan",&wan TSRMLS_CC);
		sprintf(endString,"%d%s",thisIntNum,wan);
		RETVAL_STRING(endString,1);
		efree(wan);
		return;
	}

	MODULE_BEGIN

		double thisNum = num/1000000.0;
		int thisIntNum = (int)(thisNum+0.5);
		double endNum = thisIntNum/100.0;
		char	endString[90];
		CTime_getLanguageSay("yi",&yi TSRMLS_CC);
		sprintf(endString,"%.1f%s",endNum,yi);
		RETVAL_STRING(endString,1);
		efree(yi);
	MODULE_END
}

PHP_METHOD(CTime,getTimestampToMinute)
{

}

PHP_METHOD(CTime,getBetweenTimeAreaStamp)
{

}