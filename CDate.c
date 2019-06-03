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
#include "php_CDate.h"


//zend类方法
zend_function_entry CDate_functions[] = {
	PHP_ME(CDate,setLanguage,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getTimestampToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getTimeareaToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getAllDaysInDates,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getNumToSay,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getTimestampToMinute,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,getBetweenTimeAreaStamp,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,isWeekend,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,parseNginxDate,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,isLeapYear,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,daysInMonth,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CDate,daysInYear,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDate)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CDate",CDate_functions);
	CDateCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_null(CDateCe, ZEND_STRL("lang"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
}

PHP_METHOD(CDate,setLanguage)
{
	zval	*language;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"a",&language) == FAILURE){
		return;
	}
	zend_update_static_property(CDateCe,ZEND_STRL("lang"),language TSRMLS_CC);
}

void CDate_getDefaultSay(char *key,char **returnString)
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

void CDate_getLanguageSay(char *key,char **returnString TSRMLS_DC)
{
	zval	*lang,
			**langeVal;


	lang = zend_read_static_property(CDateCe,ZEND_STRL("lang"),0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(lang)){
		CDate_getDefaultSay(key,returnString);
		return;
	}

	//read language
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(lang),key,strlen(key)+1,(void**)&langeVal)){
		convert_to_string(*langeVal);
		*returnString = estrdup(Z_STRVAL_PP(langeVal));
		return;
	}else{
		char	*getDefault;
		CDate_getDefaultSay(key,&getDefault);
		if(strlen(getDefault) != 0){
			*returnString = estrdup(getDefault);
		}else{
			*returnString = estrdup(key);
		}
		efree(getDefault);
	}
}

void CDate_getTimestampToSay(int timestamp,char **returnString TSRMLS_DC){

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
	CDate_getLanguageSay("year",&year TSRMLS_CC);
	CDate_getLanguageSay("hour",&hour TSRMLS_CC);
	CDate_getLanguageSay("day",&day TSRMLS_CC);
	CDate_getLanguageSay("minute",&min TSRMLS_CC);
	CDate_getLanguageSay("second",&sec TSRMLS_CC);
	CDate_getLanguageSay("now",&now TSRMLS_CC);
	CDate_getLanguageSay("today",&today TSRMLS_CC);
	CDate_getLanguageSay("yesterday",&yesterday TSRMLS_CC);
	CDate_getLanguageSay("ago",&ago TSRMLS_CC);


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

void CDate_getTimeareaToSay(int timestamp,int timestamp2,char **returnString TSRMLS_DC){

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
	CDate_getLanguageSay("year",&year TSRMLS_CC);
	CDate_getLanguageSay("hour",&hour TSRMLS_CC);
	CDate_getLanguageSay("day",&day TSRMLS_CC);
	CDate_getLanguageSay("minute",&min TSRMLS_CC);
	CDate_getLanguageSay("second",&sec TSRMLS_CC);
	CDate_getLanguageSay("now",&now TSRMLS_CC);
	CDate_getLanguageSay("today",&today TSRMLS_CC);
	CDate_getLanguageSay("yesterday",&yesterday TSRMLS_CC);
	CDate_getLanguageSay("ago",&ago TSRMLS_CC);

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

PHP_METHOD(CDate,getTimestampToSay)
{
	long	timestamp;

	char	*endSay;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&timestamp) == FAILURE){
		return;
	}

	CDate_getTimestampToSay(timestamp,&endSay TSRMLS_CC);
	RETVAL_STRING(endSay,0);
}

PHP_METHOD(CDate,getTimeareaToSay)
{
	long	timestamp1,
			timestamp2;

	char	*endSay;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ll",&timestamp1,&timestamp2) == FAILURE){
		return;
	}

	CDate_getTimeareaToSay(timestamp1,timestamp2,&endSay TSRMLS_CC);
	RETVAL_STRING(endSay,0);
}

PHP_METHOD(CDate,getAllDaysInDates)
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

PHP_METHOD(CDate,getNumToSay)
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
		CDate_getLanguageSay("wan",&wan TSRMLS_CC);
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
		CDate_getLanguageSay("yi",&yi TSRMLS_CC);
		sprintf(endString,"%.1f%s",endNum,yi);
		RETVAL_STRING(endString,1);
		efree(yi);
	MODULE_END
}

PHP_METHOD(CDate,getTimestampToMinute)
{

}

PHP_METHOD(CDate,getBetweenTimeAreaStamp)
{

}

PHP_METHOD(CDate,isWeekend)
{
	char	*date,
			*nowDateString;
	int		dateLen = 0,
			nowTimestamp = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&date,&dateLen) == FAILURE){
		return;
	}

	nowTimestamp = php_strtotime(date);

	php_date_ex("w",nowTimestamp,&nowDateString);

	if(strcmp(nowDateString,"6") == 0 || strcmp(nowDateString,"0") == 0){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}

	efree(nowDateString);
}

void CDate_getMonthSay(char *monthString,char **monthIntString){
	
	if(strcmp(monthString,"Jan") == 0){
		*monthIntString = estrdup("01");
	}else if(strcmp(monthString,"Feb") == 0){
		*monthIntString = estrdup("02");
	}else if(strcmp(monthString,"Mar") == 0){
		*monthIntString = estrdup("03");
	}else if(strcmp(monthString,"Apr") == 0){
		*monthIntString = estrdup("04");
	}else if(strcmp(monthString,"May") == 0){
		*monthIntString = estrdup("05");
	}else if(strcmp(monthString,"Jun") == 0){
		*monthIntString = estrdup("06");
	}else if(strcmp(monthString,"Jul") == 0){
		*monthIntString = estrdup("07");
	}else if(strcmp(monthString,"Aug") == 0){
		*monthIntString = estrdup("08");
	}else if(strcmp(monthString,"Sep") == 0){
		*monthIntString = estrdup("09");
	}else if(strcmp(monthString,"Oct") == 0){
		*monthIntString = estrdup("10");
	}else if(strcmp(monthString,"Nov") == 0){
		*monthIntString = estrdup("11");
	}else if(strcmp(monthString,"Dec") == 0){
		*monthIntString = estrdup("12");
	}else{
		*monthIntString = estrdup("00");
	}

}

PHP_METHOD(CDate,parseNginxDate)
{
	char	*string,
			timestring[40],
			*monthNum;
	int		stringLen = 0;
	zval	*match,
			**year,
			**month,
			**days,
			**hour,
			**min,
			**sec;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	if(!preg_match("/(\\d+)\\/([A-Za-z]+)\\/(\\d+):(\\d+):(\\d+):(\\d+)\\s\\+(\\d{4})/",string,&match)){
		zval_ptr_dtor(&match);
		RETURN_FALSE;
	}

	if(zend_hash_num_elements(Z_ARRVAL_P(match)) != 8){
		zval_ptr_dtor(&match);
		RETURN_FALSE;
	}

	zend_hash_index_find(Z_ARRVAL_P(match),3,(void**)&year);
	zend_hash_index_find(Z_ARRVAL_P(match),2,(void**)&month);
	zend_hash_index_find(Z_ARRVAL_P(match),1,(void**)&days);
	zend_hash_index_find(Z_ARRVAL_P(match),4,(void**)&hour);
	zend_hash_index_find(Z_ARRVAL_P(match),5,(void**)&min);
	zend_hash_index_find(Z_ARRVAL_P(match),6,(void**)&sec);

	CDate_getMonthSay(Z_STRVAL_PP(month),&monthNum);
	sprintf(timestring,"%s-%s-%s %s:%s:%s",Z_STRVAL_PP(year),monthNum,Z_STRVAL_PP(days),Z_STRVAL_PP(hour),Z_STRVAL_PP(min),Z_STRVAL_PP(sec));
	zval_ptr_dtor(&match);
	efree(monthNum);
	RETVAL_STRING(timestring,1);
}

int CDate_isLeapYear(long year){
	if(year%100==0){
		if (year%400==0 && year%3200 !=0){
			return 1;
		}else{
			return 0;
		}
		
	}else{
		if(year%4 == 0 && year%100 !=0){
			return 1;
		}else {
			return 0;
		}
	}
}


PHP_METHOD(CDate,isLeapYear)
{
	long	year = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&year) == FAILURE){
		return;
	}

	if(CDate_isLeapYear(year)){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CDate,daysInMonth)
{
	char	*date,
			*year,
			*month;
	int		dateLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&date,&dateLen) == FAILURE){
		return;
	}

	year = strtok(date,"-");
	month = strtok(NULL,"-");

	if(
		strcmp(month,"1") == 0 || 
		strcmp(month,"01") == 0 || 
		strcmp(month,"3") == 0 || 
		strcmp(month,"03") == 0 || 
		strcmp(month,"5") == 0 || 
		strcmp(month,"05") == 0 || 
		strcmp(month,"7") == 0 || 
		strcmp(month,"07") == 0 || 
		strcmp(month,"8") == 0 || 
		strcmp(month,"08") == 0 || 
		strcmp(month,"10") == 0 || 
		strcmp(month,"12") == 0 
	){
		RETVAL_LONG(31);
	}else if(
		strcmp(month,"4") == 0 || 
		strcmp(month,"04") == 0 || 
		strcmp(month,"6") == 0 || 
		strcmp(month,"06") == 0 || 
		strcmp(month,"9") == 0 || 
		strcmp(month,"09") == 0 || 
		strcmp(month,"11") == 0 
	){
		RETVAL_LONG(30);
	}else if(
		strcmp(month,"2") == 0 || 
		strcmp(month,"02") == 0
	){
		//check isLeapYear
		if(CDate_isLeapYear(toInt(year))){
			RETVAL_LONG(29);
		}else{
			RETVAL_LONG(28);
		}
	}else{
		RETVAL_FALSE;
	}
}

PHP_METHOD(CDate,daysInYear)
{
	long	year = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&year) == FAILURE){
		return;
	}

	if(CDate_isLeapYear(year)){
		RETVAL_LONG(366);
	}else{
		RETVAL_LONG(365);
	}
}