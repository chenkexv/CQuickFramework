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
#include "php_CSecurityCode.h"
#include "php_CException.h"
#include "php_CResponse.h"
#include "php_CWebApp.h"
#include "php_CSession.h"
#include "main/SAPI.h"
#include <math.h>


//zend类方法
zend_function_entry CSecurityCode_functions[] = {
	PHP_ME(CSecurityCode,entry,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CSecurityCode,check,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CSecurityCode)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CSecurityCode",CSecurityCode_functions);
	CSecurityCodeCe = zend_register_internal_class(&funCe TSRMLS_CC);

	zend_declare_property_string(CSecurityCodeCe, ZEND_STRL("seKey"),"fdfds3cxs",ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CSecurityCodeCe, ZEND_STRL("expire"),3000,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CSecurityCodeCe, ZEND_STRL("codeSet"),"23467892ABCDEFGHJKLMPQRTUVWXY",ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CSecurityCodeCe, ZEND_STRL("fontSize"),20,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_long(CSecurityCodeCe, ZEND_STRL("length"),4,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CSecurityCodeCe, ZEND_STRL("bg"),ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}


void CSecurityCode_drawCureve(zval *imageSource,int width,int height,int fontSize,zval *color TSRMLS_DC){

	int A,b,f,T,px1 = 10,px2,i = 0;
	double w,px,py;

	A = php_rand_call(1,(int)(height/2));
	b = php_rand_call(0-(int)(height/4),(int)height/4);
	f = php_rand_call(0-(int)(height/4),(int)height/4);
	T = php_rand_call((int)(height*1.5),(int)(width*2));
	w = (2*3.14159265358) / T;
	px2 = php_rand_call((int)(width/2),(int)(width*0.667));

	for(px = px1;px <= px2;px=px+0.9){
		if(w != 0){
			py = A * sin(w*px+f) + b + height/2;
			i = (int)( (fontSize - 6)/4 );
			while(i > 0 ){

				//draw pixl
				zval	callFun,
						returnFun,
						*paramsList[4];

				MAKE_STD_ZVAL(paramsList[0]);
				MAKE_STD_ZVAL(paramsList[1]);
				MAKE_STD_ZVAL(paramsList[2]);
				MAKE_STD_ZVAL(paramsList[3]);

				ZVAL_ZVAL(paramsList[0],imageSource,1,0);
				ZVAL_DOUBLE(paramsList[1],(double)(px+i));
				ZVAL_DOUBLE(paramsList[2],(double)(py+i));
				ZVAL_ZVAL(paramsList[3],color,1,0);
				INIT_ZVAL(callFun);
				ZVAL_STRING(&callFun,"imagesetpixel", 0);
				call_user_function(EG(function_table), NULL, &callFun, &returnFun, 4, paramsList TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
				zval_ptr_dtor(&paramsList[1]);
				zval_ptr_dtor(&paramsList[2]);
				zval_ptr_dtor(&paramsList[3]);
				zval_dtor(&returnFun);
				i--;
			}
		}
	}
}

void CSecurityCode_drawCureve1(zval *imageSource,int width,int height,int fontSize,zval *color TSRMLS_DC){

	int A,b,f,T,px1 = 10,px2,i = 0;
	double w,px,py;

	A = php_rand_call(1,(int)(height/2));
	b = php_rand_call(0-(int)(height/4),(int)height/4);
	f = php_rand_call(0-(int)(height/4),(int)height/4);
	T = php_rand_call((int)(height*1.5),(int)(width*2));
	w = (2*3.14159265358) / T;
	px2 = php_rand_call((int)(width/2),(int)(width*0.667));

	px1 = php_rand_call(10, (int)(width/2));

	for(px = px1;px <= px2;px=px+0.9){
		if(w != 0){
			py = A * cos(w*px+f) + b + height/2;
			i = (int)( (fontSize - 6)/4 );
			while(i > 0 ){

				//draw pixl
				zval	callFun,
						returnFun,
						*paramsList[4];

				MAKE_STD_ZVAL(paramsList[0]);
				MAKE_STD_ZVAL(paramsList[1]);
				MAKE_STD_ZVAL(paramsList[2]);
				MAKE_STD_ZVAL(paramsList[3]);

				ZVAL_ZVAL(paramsList[0],imageSource,1,0);
				ZVAL_DOUBLE(paramsList[1],(double)(px+i));
				ZVAL_DOUBLE(paramsList[2],(double)(py+i));
				ZVAL_ZVAL(paramsList[3],color,1,0);
				INIT_ZVAL(callFun);
				ZVAL_STRING(&callFun,"imagesetpixel", 0);
				call_user_function(EG(function_table), NULL, &callFun, &returnFun, 4, paramsList TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
				zval_ptr_dtor(&paramsList[1]);
				zval_ptr_dtor(&paramsList[2]);
				zval_ptr_dtor(&paramsList[3]);
				zval_dtor(&returnFun);
				i--;
			}
		}
	}
}

void CSecurityCode_drawCureve2(zval *imageSource,int width,int height,int fontSize,zval *color TSRMLS_DC){

	int A,b,f,T,px1 = 10,px2,i = 0;
	double w,px,py;

	A = php_rand_call(1,(int)(height/2));
	b = php_rand_call(0-(int)(height/4),(int)height/4);
	f = php_rand_call(0-(int)(height/4),(int)height/4);
	T = php_rand_call((int)(height*1.5),(int)(width*2));
	w = (2*3.14159265358) / T;
	px2 = php_rand_call((int)(width/2),(int)(width*0.667));

	for(px = px1;px <= px2;px=px+0.9){
		if(w != 0){
			py = A * tan(w*px+f) + b + height/2;
			i = (int)( (fontSize - 6)/4 );
			while(i > 0 ){

				//draw pixl
				zval	callFun,
						returnFun,
						*paramsList[4];

				MAKE_STD_ZVAL(paramsList[0]);
				MAKE_STD_ZVAL(paramsList[1]);
				MAKE_STD_ZVAL(paramsList[2]);
				MAKE_STD_ZVAL(paramsList[3]);

				ZVAL_ZVAL(paramsList[0],imageSource,1,0);
				ZVAL_DOUBLE(paramsList[1],(double)(px+i));
				ZVAL_DOUBLE(paramsList[2],(double)(py+i));
				ZVAL_ZVAL(paramsList[3],color,1,0);
				INIT_ZVAL(callFun);
				ZVAL_STRING(&callFun,"imagesetpixel", 0);
				call_user_function(EG(function_table), NULL, &callFun, &returnFun, 4, paramsList TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
				zval_ptr_dtor(&paramsList[1]);
				zval_ptr_dtor(&paramsList[2]);
				zval_ptr_dtor(&paramsList[3]);
				zval_dtor(&returnFun);
				i--;
			}
		}
	}
}

PHP_METHOD(CSecurityCode,entry)
{

	long	useGzip = 0;

	zval	*length,
			*fontSize,
			*expire,
			*imageSource,
			*colorSource,
			callCreateReturn,
			callColorRerturn,
			*bg,
			*saveKey;

	char	code[1024];

	int		imageLen = 0,
			imageHeight = 0,
			r=243,g=251,b=254,
			addHeaderStatus;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&useGzip) == FAILURE){
	}

	//read static property
	length = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("length"),0 TSRMLS_CC);
	fontSize = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("fontSize"),0 TSRMLS_CC);
	expire = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("expire"),0 TSRMLS_CC);
	bg = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("bg"),0 TSRMLS_CC);
	saveKey = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("seKey"),0 TSRMLS_CC);


	//set rgb
	if(IS_ARRAY == Z_TYPE_P(bg) && zend_hash_num_elements(Z_ARRVAL_P(bg)) == 3){
		//has a right rgb config
		zval **thisVal;
		if(zend_hash_index_find(Z_ARRVAL_P(bg),0,(void**)&thisVal) && IS_LONG == Z_TYPE_PP(thisVal)){
			r = Z_LVAL_PP(thisVal);
		}
		if(zend_hash_index_find(Z_ARRVAL_P(bg),1,(void**)&thisVal) && IS_LONG == Z_TYPE_PP(thisVal)){
			g = Z_LVAL_PP(thisVal);
		}
		if(zend_hash_index_find(Z_ARRVAL_P(bg),2,(void**)&thisVal) && IS_LONG == Z_TYPE_PP(thisVal)){
			b = Z_LVAL_PP(thisVal);
		}
	}


	//img size
	imageLen = (Z_LVAL_P(length)*Z_LVAL_P(fontSize)*1.5) + (Z_LVAL_P(fontSize)*1.5);
	imageHeight = Z_LVAL_P(fontSize)*2;

	//create image
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[2];
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_LONG(paramsList[0],imageLen);
		MAKE_STD_ZVAL(paramsList[1]);
		ZVAL_LONG(paramsList[1],imageHeight);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"imagecreate", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &callCreateReturn, 2, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		imageSource = &callCreateReturn;
	MODULE_END

	//imagecolorallocate set background
	MODULE_BEGIN
		zval	constructVal,
				constructReturn,
				*paramsList[4];
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);
		ZVAL_ZVAL(paramsList[0],imageSource,1,0);
		ZVAL_LONG(paramsList[1],r);
		ZVAL_LONG(paramsList[2],g);
		ZVAL_LONG(paramsList[3],b);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"imagecolorallocate", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &constructReturn, 4, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		zval_dtor(&constructReturn);
	MODULE_END

	//imagecolorallocate
	MODULE_BEGIN
		zval	constructVal,
				*paramsList[4];
		int		rand1 = 10,
				rand2 = 100,
				rand3 = 50;

		rand1 = php_rand_call(10,120);
		rand2 = php_rand_call(100,150);
		rand3 = php_rand_call(50,150);
		MAKE_STD_ZVAL(paramsList[0]);
		MAKE_STD_ZVAL(paramsList[1]);
		MAKE_STD_ZVAL(paramsList[2]);
		MAKE_STD_ZVAL(paramsList[3]);
		ZVAL_ZVAL(paramsList[0],imageSource,1,0);
		ZVAL_LONG(paramsList[1],rand1);
		ZVAL_LONG(paramsList[2],rand2);
		ZVAL_LONG(paramsList[3],rand3);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"imagecolorallocate", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &callColorRerturn, 4, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_ptr_dtor(&paramsList[1]);
		zval_ptr_dtor(&paramsList[2]);
		zval_ptr_dtor(&paramsList[3]);
		colorSource = &callColorRerturn;
	MODULE_END

	//set Noise
	MODULE_BEGIN
		int		k,h;
		zval	*codeSet;
		codeSet = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("codeSet"),0 TSRMLS_CC);
		for(k = 0 ; k < 20 ; k++){

			zval	constructVal,
					constouctReturn,
					*paramsList[4],
					*thisNoiseColor;
			int		rand1 = 10,
					rand2 = 100,
					rand3 = 50;

			//noise color
			rand1 = php_rand_call(150,225);
			rand2 = php_rand_call(150,225);
			rand3 = php_rand_call(150,225);
			MAKE_STD_ZVAL(paramsList[0]);
			MAKE_STD_ZVAL(paramsList[1]);
			MAKE_STD_ZVAL(paramsList[2]);
			MAKE_STD_ZVAL(paramsList[3]);
			ZVAL_ZVAL(paramsList[0],imageSource,1,0);
			ZVAL_LONG(paramsList[1],rand1);
			ZVAL_LONG(paramsList[2],rand2);
			ZVAL_LONG(paramsList[3],rand3);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal,"imagecolorallocate", 0);
			call_user_function(EG(function_table), NULL, &constructVal, &constouctReturn, 4, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_ptr_dtor(&paramsList[1]);
			zval_ptr_dtor(&paramsList[2]);
			zval_ptr_dtor(&paramsList[3]);
			thisNoiseColor = &constouctReturn;

			for(h = 0 ; h < 5;h++){

				zval	callFun,
						returnFun,
						*callParams[6];
				char	*codeSetString,
						thisCodeWord,
						thisCodeString[1];
				int		rand = 0;

				MAKE_STD_ZVAL(callParams[0]);
				MAKE_STD_ZVAL(callParams[1]);
				MAKE_STD_ZVAL(callParams[2]);
				MAKE_STD_ZVAL(callParams[3]);
				MAKE_STD_ZVAL(callParams[4]);
				MAKE_STD_ZVAL(callParams[5]);
				ZVAL_ZVAL(callParams[0],imageSource,1,0);
				ZVAL_LONG(callParams[1],5);
				ZVAL_LONG(callParams[2],php_rand_call(-10,imageLen));
				ZVAL_LONG(callParams[3],php_rand_call(-10,imageHeight));
				codeSetString = Z_STRVAL_P(codeSet);
				rand = php_rand_call(0,27);
				thisCodeWord = codeSetString[rand];
				sprintf(thisCodeString,"%c",thisCodeWord);
				ZVAL_STRING(callParams[4],thisCodeString,1);
				ZVAL_ZVAL(callParams[5],thisNoiseColor,1,0);
				INIT_ZVAL(callFun);
				ZVAL_STRING(&callFun,"imagestring", 0);
				call_user_function(EG(function_table), NULL, &callFun, &returnFun, 6, callParams TSRMLS_CC);
				zval_ptr_dtor(&callParams[0]);
				zval_ptr_dtor(&callParams[1]);
				zval_ptr_dtor(&callParams[2]);
				zval_ptr_dtor(&callParams[3]);
				zval_ptr_dtor(&callParams[4]);
				zval_ptr_dtor(&callParams[5]);
				zval_dtor(&returnFun);
			}


			zval_dtor(&constouctReturn);
		}
	
	MODULE_END

	//set Curve
	MODULE_BEGIN
		CSecurityCode_drawCureve(imageSource,imageLen,imageHeight,Z_LVAL_P(fontSize),colorSource TSRMLS_CC);
		CSecurityCode_drawCureve1(imageSource,imageLen,imageHeight,Z_LVAL_P(fontSize),colorSource TSRMLS_CC);
		CSecurityCode_drawCureve2(imageSource,imageLen,imageHeight,Z_LVAL_P(fontSize),colorSource TSRMLS_CC);
	MODULE_END

	//set code
	MODULE_BEGIN
		int		i,h,rand,
				codeNx = 2,
				nxRand = 0;
		zval	*codeSet;
		char	*codeSetString,
				thisCodeWord,
				fontPath[1024],
				nodeCode[1];
		zval	*appPath;
		codeSet = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("codeSet"),0 TSRMLS_CC);
		appPath = zend_read_static_property(CWebAppCe,ZEND_STRL("app_path"),0 TSRMLS_CC);
		codeSetString = Z_STRVAL_P(codeSet);
		h = Z_LVAL_P(length);
		code[h] = '\0';
		for(i = 0 ; i < h ; i++){
			rand = php_rand_call(0,27);
			thisCodeWord = codeSetString[rand];
			code[i] = thisCodeWord;
			nxRand = php_rand_call((int)(Z_LVAL_P(fontSize)*1.2),(int)(Z_LVAL_P(fontSize)*1.6));
			codeNx = codeNx + nxRand;

			//get ttf
			sprintf(fontPath,"%s%s%d%s",Z_STRVAL_P(appPath),"/application/classes/fonts/",php_rand_call(1,4),".ttf");
			sprintf(nodeCode,"%c",thisCodeWord);

			//imagettftext
			MODULE_BEGIN
				zval	constructVal,
						constructReturn,
				*paramsList[8];
				MAKE_STD_ZVAL(paramsList[0]);
				MAKE_STD_ZVAL(paramsList[1]);
				MAKE_STD_ZVAL(paramsList[2]);
				MAKE_STD_ZVAL(paramsList[3]);
				MAKE_STD_ZVAL(paramsList[4]);
				MAKE_STD_ZVAL(paramsList[5]);
				MAKE_STD_ZVAL(paramsList[6]);
				MAKE_STD_ZVAL(paramsList[7]);
				ZVAL_ZVAL(paramsList[0],imageSource,1,0);
				ZVAL_ZVAL(paramsList[1],fontSize,1,0);
				ZVAL_LONG(paramsList[2],php_rand_call(-20,20));
				ZVAL_LONG(paramsList[3],codeNx);
				ZVAL_LONG(paramsList[4],Z_LVAL_P(fontSize)*1.5);
				ZVAL_ZVAL(paramsList[5],colorSource,1,0);
				ZVAL_STRING(paramsList[6],fontPath,1);
				ZVAL_STRING(paramsList[7],nodeCode,1);
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal,"imagettftext", 0);
				call_user_function(EG(function_table), NULL, &constructVal, &constructReturn, 8, paramsList TSRMLS_CC);
				zval_ptr_dtor(&paramsList[0]);
				zval_ptr_dtor(&paramsList[1]);
				zval_ptr_dtor(&paramsList[2]);
				zval_ptr_dtor(&paramsList[3]);
				zval_ptr_dtor(&paramsList[4]);
				zval_ptr_dtor(&paramsList[5]);
				zval_ptr_dtor(&paramsList[6]);
				zval_ptr_dtor(&paramsList[7]);
				zval_dtor(&constructReturn);
			MODULE_END
		}
	MODULE_END

	//set code to session
	MODULE_BEGIN
		zval	*saveArray,
				*time,
				**array;

		microtime(&time);
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		add_assoc_string(saveArray,"code",code,1);
		add_assoc_long(saveArray,"time",(int)(Z_DVAL_P(time)));
		if( zend_hash_find(&EG(symbol_table),"_SESSION",sizeof("_SESSION"),(void**)&array) == SUCCESS ){
			if(IS_ARRAY != Z_TYPE_PP(array)){
				array_init(*array);
			}
			zend_hash_update(Z_ARRVAL_PP(array),Z_STRVAL_P(saveKey),strlen(Z_STRVAL_P(saveKey))+1,&saveArray,sizeof(zval*),NULL);
		}
		zval_ptr_dtor(&time);
	MODULE_END
	
	//header
	callHeader("Cache-Control: private, max-age=0, no-store, no-cache, must-revalidate",1 TSRMLS_CC);
	callHeader("Cache-Control: post-check=0, pre-check=0",0 TSRMLS_CC);
	callHeader("Pragma: no-cache",1 TSRMLS_CC);
	callHeader("Content-type: image/png",1 TSRMLS_CC);

	//call imagepng
	MODULE_BEGIN
		zval	constructVal,
				constructReturn,
				*paramsList[1];
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],imageSource,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"imagepng", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&constructReturn);
	MODULE_END


	//call destory
	MODULE_BEGIN
		zval	constructVal,
				constructReturn,
				*paramsList[1];
		MAKE_STD_ZVAL(paramsList[0]);
		ZVAL_ZVAL(paramsList[0],imageSource,1,0);
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal,"imagedestroy", 0);
		call_user_function(EG(function_table), NULL, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
		zval_ptr_dtor(&paramsList[0]);
		zval_dtor(&constructReturn);
	MODULE_END

	//destory
	zval_dtor(&callCreateReturn);
	zval_dtor(&callColorRerturn);
}

PHP_METHOD(CSecurityCode,check)
{

	char	*code;

	zval	**array,
			*saveKey,
			**thisCodeVal,
			**trueCode,
			**timeout,
			*time,
			*expire;

	long	codeLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&code,&codeLen) == FAILURE){
		RETVAL_FALSE;
		return;
	}

	if(codeLen == 0){
		RETVAL_FALSE;
		return;
	}

	//find sessino
	if( zend_hash_find(&EG(symbol_table),"_SESSION",sizeof("_SESSION"),(void**)&array) != SUCCESS ){
		RETVAL_FALSE;
		return;
	}

	//savekey
	saveKey = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("seKey"),0 TSRMLS_CC);
	expire = zend_read_static_property(CSecurityCodeCe,ZEND_STRL("expire"),0 TSRMLS_CC);

	//find seesion val
	if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(array),Z_STRVAL_P(saveKey),strlen(Z_STRVAL_P(saveKey))+1,(void**)&thisCodeVal) && IS_ARRAY == Z_TYPE_PP(thisCodeVal)){
	}else{
		RETVAL_FALSE;
		return;
	}

	//get code and time
	if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisCodeVal),"code",strlen("code")+1,(void**)&trueCode) && IS_STRING == Z_TYPE_PP(trueCode)){
	}else{
		RETVAL_FALSE;
		return;
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_PP(thisCodeVal),"time",strlen("time")+1,(void**)&timeout) && IS_LONG == Z_TYPE_PP(timeout)){
	}else{
		RETVAL_FALSE;
		return;
	}

	//checkExpire
	microtime(&time);
	if((int)(Z_DVAL_P(time)) - Z_LVAL_PP(timeout) > Z_LVAL_P(expire)){
		zend_hash_del(Z_ARRVAL_PP(array),Z_STRVAL_P(saveKey),strlen(Z_STRVAL_P(saveKey))+1);
		zval_ptr_dtor(&time);
		RETVAL_FALSE;
		return;
	}

	//ignore up or lower
	if(strcasecmp(code,Z_STRVAL_PP(trueCode)) == 0){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}


	zval_ptr_dtor(&time);
}