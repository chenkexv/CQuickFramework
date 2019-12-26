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

#if CQUICKFRAMEWORK_USE_QRENCODE
#include <qrencode.h>
#define PNG_SKIP_SETJMP_CHECK 1
#include <png.h>
#endif

#include "php_CQuickFramework.h"
#include "php_CException.h"
#include "php_CWebApp.h"
#include "php_CQrcode.h"


//zend类方法
zend_function_entry CQrcode_functions[] = {
	PHP_ME(CQrcode,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CQrcode,setContent,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQrcode,getInstance,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(CQrcode,show,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQrcode,setIcon,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQrcode,getFilePath,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQrcode,create,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CQrcode)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CQrcode",CQrcode_functions);
	CQrcodeCe = zend_register_internal_class(&funCe TSRMLS_CC);
	zend_declare_property_null(CQrcodeCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CQrcodeCe, ZEND_STRL("content"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CQrcodeCe, ZEND_STRL("iconPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CQrcodeCe, ZEND_STRL("tempPath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQrcodeCe, ZEND_STRL("iconSize"),50,ZEND_ACC_PRIVATE TSRMLS_CC);
}

PHP_METHOD(CQrcode,__construct){

	char	*content;
	int		contentLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&content,&contentLen) == FAILURE){
		return;
	}

	if(contentLen > 0){
		zend_update_property_string(CQrcodeCe,getThis(),ZEND_STRL("content"),content TSRMLS_CC);
	}
}

PHP_METHOD(CQrcode,setContent){

	char	*content;
	int		contentLen = 0;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&content,&contentLen) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->setContent] params error", 1 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CQrcodeCe,getThis(),ZEND_STRL("content"),content TSRMLS_CC);
}

void CQrcode_getInstance(char *groupName,zval **returnZval TSRMLS_DC){

	zval	*instanceZval,
			**instaceSaveZval;

	int hasExistConfig;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CQrcodeCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//如果为NULL则更新为ZvalHashtable
	if(IS_ARRAY != Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CQrcodeCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CQrcodeCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//判断单列对象中存在config的key
	hasExistConfig = zend_hash_exists(Z_ARRVAL_P(instanceZval), groupName, strlen(groupName)+1);

	//为空时则实例化自身
	if(0 == hasExistConfig ){
		

		zval			*object,
						*saveObject;


		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,CQrcodeCe);

		//执行构造器
		if (CQrcodeCe->constructor) {
			zval constructReturn;
			zval constructVal,
				 params1;
			zval *paramsList[1];
			paramsList[0] = &params1;
			MAKE_STD_ZVAL(paramsList[0]);
			ZVAL_STRING(paramsList[0],groupName,1);
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, CQrcodeCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 1, paramsList TSRMLS_CC);
			zval_ptr_dtor(&paramsList[0]);
			zval_dtor(&constructReturn);
		}


		//将类对象保存在instance静态变量
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);

		zend_hash_add(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CQrcodeCe, ZEND_STRL("instance"),instanceZval TSRMLS_CC);

		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,0);
		zval_ptr_dtor(&object);
		return;
	}

	//直接取instace静态变量中的返回值
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(instanceZval),groupName,strlen(groupName)+1,(void**)&instaceSaveZval) ){
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,*instaceSaveZval,1,0);
		return;
	}
}

PHP_METHOD(CQrcode,getInstance)
{
	char	*groupName,
			*dgroupName;
	int		groupNameLen = 0;
	zval	*object;

	//判断是否指定序列
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|s",&dgroupName,&groupNameLen) == FAILURE){
		return;
	}

	if(groupNameLen == 0){
		groupName = estrdup("default");
	}else{
		groupName = estrdup(dgroupName);
	}
	
	CQrcode_getInstance(groupName,&object TSRMLS_CC);
	ZVAL_ZVAL(return_value,object,1,1);
	efree(groupName);
}


//copy from libqrencode
#if CQUICKFRAMEWORK_USE_QRENCODE

enum imageType {
	PNG_TYPE,
	PNG32_TYPE,
	EPS_TYPE,
	SVG_TYPE,
	XPM_TYPE,
	ANSI_TYPE,
	ANSI256_TYPE,
	ASCII_TYPE,
	ASCIIi_TYPE,
	UTF8_TYPE,
	ANSIUTF8_TYPE,
	ANSI256UTF8_TYPE,
	UTF8i_TYPE,
	ANSIUTF8i_TYPE
};


#define INCHES_PER_METER (100.0/2.54)

static void fillRow(unsigned char *row, int num, const unsigned char color[])
{
	int i;

	for(i = 0; i < num; i++) {
		memcpy(row, color, 4);
		row += 4;
	}
}

static int writePNG(const QRcode *qrcode, const char *outfile, enum imageType type,int margin,int size)
{
	static FILE *fp; 
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;
	png_byte alpha_values[2];
	unsigned char *row = NULL, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;
	int casesensitive = 1;
	int eightbit = 0;
	int version = 0;
	int dpi = 72;
	int structured = 0;
	int rle = 0;
	int svg_path = 0;
	int micro = 0;
	int inline_svg = 0;
	unsigned char fg_color[4] = {0, 0, 0, 255};
	unsigned char bg_color[4] = {255, 255, 255, 255};
	
	realwidth = (qrcode->width + margin * 2) * size;
	if(type == PNG_TYPE) {
		row = (unsigned char *)emalloc((size_t)((realwidth + 7) / 8));
	} else if(type == PNG32_TYPE) {
		row = (unsigned char *)emalloc((size_t)realwidth * 4);
	} else {
		return -1;
	}
	if(row == NULL) {
		return -2;
	}

	if(outfile[0] == '-' && outfile[1] == '\0') {
		efree(row);
		return -1;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			efree(row);
			return -3;
		}
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		efree(row);
		fclose(fp);
		return -4;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		efree(row);
		fclose(fp);
		return -5;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		efree(row);
		fclose(fp);
		return -6;
	}

	if(type == PNG_TYPE) {
		palette = (png_colorp) emalloc(sizeof(png_color) * 2);
		if(palette == NULL) {
			efree(row);
			fclose(fp);
			return -7;
		}
		palette[0].red   = fg_color[0];
		palette[0].green = fg_color[1];
		palette[0].blue  = fg_color[2];
		palette[1].red   = bg_color[0];
		palette[1].green = bg_color[1];
		palette[1].blue  = bg_color[2];
		alpha_values[0] = fg_color[3];
		alpha_values[1] = bg_color[3];
		png_set_PLTE(png_ptr, info_ptr, palette, 2);
		png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);
	}

	png_init_io(png_ptr, fp);
	if(type == PNG_TYPE) {
		png_set_IHDR(png_ptr, info_ptr,
				(unsigned int)realwidth, (unsigned int)realwidth,
				1,
				PNG_COLOR_TYPE_PALETTE,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	} else {
		png_set_IHDR(png_ptr, info_ptr,
				(unsigned int)realwidth, (unsigned int)realwidth,
				8,
				PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	}
	png_set_pHYs(png_ptr, info_ptr,
			dpi * INCHES_PER_METER,
			dpi * INCHES_PER_METER,
			PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	if(type == PNG_TYPE) {
	/* top margin */
		memset(row, 0xff, (size_t)((realwidth + 7) / 8));
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			memset(row, 0xff, (size_t)((realwidth + 7) / 8));
			q = row;
			q += margin * size / 8;
			bit = 7 - (margin * size % 8);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					*q ^= (*p & 1) << bit;
					bit--;
					if(bit < 0) {
						q++;
						bit = 7;
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		memset(row, 0xff, (size_t)((realwidth + 7) / 8));
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	} else {
	/* top margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			fillRow(row, realwidth, bg_color);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					if(*p & 1) {
						memcpy(&row[((margin + x) * size + xx) * 4], fg_color, 4);
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	efree(row);
	if(palette != NULL) efree(palette);

	return 0;
}
#endif


void CQrcode_setQrIcon(zval *object,char *filePath TSRMLS_DC){

	zval	*iconFilePath,
			*waterInfo,
			*iconSize,
			**width,
			**height,
			**mime,
			*imageRousce,
			*image,
			*tempFilePath,
			*endImage,
			*imageThub,
			*imageColor,
			*sourceImageSize,
			**nowHeight,
			**nowWidth,
			*iconBackcolor;

	char	*fileType;

	int		posX = 25,
			posY = 25;

	iconFilePath = zend_read_property(CQrcodeCe,object,ZEND_STRL("iconPath"),0 TSRMLS_CC);
	iconSize = zend_read_property(CQrcodeCe,object,ZEND_STRL("iconSize"),0 TSRMLS_CC);
	tempFilePath = zend_read_property(CQrcodeCe,object,ZEND_STRL("tempPath"),0 TSRMLS_CC);

	if(strlen(Z_STRVAL_P(iconFilePath)) <= 0){
		return;
	}

	if(SUCCESS != fileExist(Z_STRVAL_P(iconFilePath))){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] set icon file, but the file not exist", 1 TSRMLS_CC);
		return;
	}

	//read file
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_P(iconFilePath),1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getimagesize",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(waterInfo);
		ZVAL_ZVAL(waterInfo,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	if(IS_ARRAY != Z_TYPE_P(waterInfo)){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] set icon file, but the file not exist", 1 TSRMLS_CC);
		zval_ptr_dtor(&waterInfo);
		return;
	}

	//get source image
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;

		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_P(tempFilePath),1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecreatefrompng",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(image);
		ZVAL_ZVAL(image,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	//get source file size
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_P(tempFilePath),1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getimagesize",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(sourceImageSize);
		ZVAL_ZVAL(sourceImageSize,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	if(IS_ARRAY != Z_TYPE_P(sourceImageSize)){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] set icon file, but can not get qrcode temp img", 1 TSRMLS_CC);
		zval_ptr_dtor(&waterInfo);
		zval_ptr_dtor(&sourceImageSize);
		zval_ptr_dtor(&image);
		return;
	}

	zend_hash_index_find(Z_ARRVAL_P(sourceImageSize),1,(void**)&nowHeight);
	zend_hash_index_find(Z_ARRVAL_P(sourceImageSize),0,(void**)&nowWidth);


	//get file type image_type_to_extension
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn,
				**fileInfoType;
		MAKE_STD_ZVAL(params[0]);
		zend_hash_index_find(Z_ARRVAL_P(waterInfo),2,(void**)&fileInfoType);
		ZVAL_LONG(params[0],Z_LVAL_PP(fileInfoType));
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"image_type_to_extension",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		if(IS_STRING == Z_TYPE_P(saveReturn)){
			substr(Z_STRVAL_P(saveReturn),1,strlen(Z_STRVAL_P(saveReturn))-1,&fileType);
		}else{
			fileType = estrdup("");
		}
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	php_strtolower(fileType,strlen(fileType)+1);

	if(strcmp(fileType,"png") != 0 && strcmp(fileType,"jpeg") != 0 && strcmp(fileType,"gif") != 0){
		char	errMessage[1024];
		sprintf(errMessage,"[CQrcodeException] Call [CQrcode->setIcon] Read File Type error : %s , only can png,jpeg,gif",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		zval_ptr_dtor(&waterInfo);
		efree(fileType);
		zval_ptr_dtor(&image);
		zval_ptr_dtor(&sourceImageSize);
		return;
	}

	zend_hash_index_find(Z_ARRVAL_P(waterInfo),0,(void**)&width);
	zend_hash_index_find(Z_ARRVAL_P(waterInfo),1,(void**)&height);
	zend_hash_find(Z_ARRVAL_P(waterInfo),"mime",strlen("mime")+1,(void**)&mime);

	posX = (int)((Z_LVAL_PP(nowWidth) - Z_LVAL_P(iconSize) - 6) / 2);
	posY = (int)((Z_LVAL_PP(nowHeight) - Z_LVAL_P(iconSize) - 6) / 2);

	//get water source
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		char	callFunctionName[60];
		sprintf(callFunctionName,"imagecreatefrom%s",fileType);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_P(iconFilePath),1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,callFunctionName,0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(imageRousce);
		ZVAL_ZVAL(imageRousce,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	//cut the icon file size
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[2],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[0],Z_LVAL_P(iconSize) + 6);
		ZVAL_LONG(params[1],Z_LVAL_P(iconSize) + 6);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecreatetruecolor",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 2, params TSRMLS_CC);
		saveRetrun = &returnFunction;
		MAKE_STD_ZVAL(imageThub);
		ZVAL_ZVAL(imageThub,saveRetrun,1,0);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&returnFunction);
	MODULE_END

	//get color
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[4],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		MAKE_STD_ZVAL(params[3]);
		ZVAL_ZVAL(params[0],imageThub,1,0);
		ZVAL_LONG(params[1],204);
		ZVAL_LONG(params[2],204);
		ZVAL_LONG(params[3],204);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecolorallocate",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 4, params TSRMLS_CC);
		saveRetrun = &returnFunction;
		MAKE_STD_ZVAL(iconBackcolor);
		ZVAL_ZVAL(iconBackcolor,saveRetrun,1,0);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_dtor(&returnFunction);
	MODULE_END

	//fill background color
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[4],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		MAKE_STD_ZVAL(params[3]);

		ZVAL_ZVAL(params[0],imageThub,1,0);
		ZVAL_LONG(params[1],0);
		ZVAL_LONG(params[2],0);
		ZVAL_ZVAL(params[3],iconBackcolor,1,0);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagefill",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 4, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_dtor(&returnFunction);
	MODULE_END
	zval_ptr_dtor(&iconBackcolor);


	//call imagecopyresampled
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[10],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		MAKE_STD_ZVAL(params[3]);
		MAKE_STD_ZVAL(params[4]);
		MAKE_STD_ZVAL(params[5]);
		MAKE_STD_ZVAL(params[6]);
		MAKE_STD_ZVAL(params[7]);
		MAKE_STD_ZVAL(params[8]);
		MAKE_STD_ZVAL(params[9]);
		ZVAL_ZVAL(params[0],imageThub,1,0);
		ZVAL_ZVAL(params[1],imageRousce,1,0);
		ZVAL_LONG(params[2],3);
		ZVAL_LONG(params[3],3);
		ZVAL_LONG(params[4],0);
		ZVAL_LONG(params[5],0);
		ZVAL_LONG(params[6],Z_LVAL_P(iconSize));
		ZVAL_LONG(params[7],Z_LVAL_P(iconSize));
		ZVAL_LONG(params[8],Z_LVAL_PP(width));
		ZVAL_LONG(params[9],Z_LVAL_PP(height));
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecopyresampled",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 10, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_ptr_dtor(&params[4]);
		zval_ptr_dtor(&params[5]);
		zval_ptr_dtor(&params[6]);
		zval_ptr_dtor(&params[7]);
		zval_ptr_dtor(&params[8]);
		zval_ptr_dtor(&params[9]);
		zval_dtor(&returnFunction);
	MODULE_END


	//call imagecopy
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[8];
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecopy",0);
		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		MAKE_STD_ZVAL(params[2]);
		MAKE_STD_ZVAL(params[3]);
		MAKE_STD_ZVAL(params[4]);
		MAKE_STD_ZVAL(params[5]);
		MAKE_STD_ZVAL(params[6]);
		MAKE_STD_ZVAL(params[7]);
		ZVAL_ZVAL(params[0],image,1,0);
		ZVAL_ZVAL(params[1],imageThub,1,0);
		ZVAL_LONG(params[2],posX);
		ZVAL_LONG(params[3],posY);
		ZVAL_LONG(params[4],0);
		ZVAL_LONG(params[5],0);
		ZVAL_LONG(params[6],Z_LVAL_P(iconSize)+6);
		ZVAL_LONG(params[7],Z_LVAL_P(iconSize)+6);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 8, params TSRMLS_CC);

		MAKE_STD_ZVAL(endImage);
		ZVAL_ZVAL(endImage,params[0],1,0);

		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_ptr_dtor(&params[4]);
		zval_ptr_dtor(&params[5]);
		zval_ptr_dtor(&params[6]);
		zval_ptr_dtor(&params[7]);
		zval_dtor(&returnFunction);
	MODULE_END


	MODULE_BEGIN
		zval	functionName,
				functionReturn,
				*params[3];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"imagepng",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],endImage,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],Z_STRVAL_P(tempFilePath),1);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&functionReturn);
	MODULE_END


	//destroy
	zval_ptr_dtor(&sourceImageSize);
	zval_ptr_dtor(&waterInfo);
	zval_ptr_dtor(&endImage);
	zval_ptr_dtor(&imageRousce);
	efree(fileType);
	zval_ptr_dtor(&image);
	zval_ptr_dtor(&imageThub);
}


PHP_METHOD(CQrcode,create){
	
#if CQUICKFRAMEWORK_USE_QRENCODE

	QRcode				*pQRC = NULL;
	int					realwidth,
						status = 0,
						border = 1;
	long				size = 6;
	unsigned char		*row;
	char				*saveTempPath,
						*saveTempName,
						*fileNameMd5;

	zval				*content,
						*appPath;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&size) == FAILURE){
	}

	content = zend_read_property(CQrcodeCe,getThis(),ZEND_STRL("content"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(content) && strlen(Z_STRVAL_P(content)) > 0){
	}else{
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->create] must set content frist", 1 TSRMLS_CC);
		RETURN_FALSE;
	}


	pQRC = QRcode_encodeString(Z_STRVAL_P(content), 0, QR_ECLEVEL_H, QR_MODE_8, 0);

	if(pQRC == NULL){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->create] call libqrencode to create a qrcode fail", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	//create a tempPath
	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);
	spprintf(&saveTempPath,0,"%s%s",Z_STRVAL_P(appPath),"/data/qrcodeTmp");
	if(FAILURE == fileExist(saveTempPath)){
		//尝试创建文件夹
		php_mkdir(saveTempPath);
	}
	md5(Z_STRVAL_P(content),&fileNameMd5);
	spprintf(&saveTempName,0,"%s%s%s%s",saveTempPath,"/",fileNameMd5,".png");
	efree(fileNameMd5);
	efree(saveTempPath);

	status = writePNG(pQRC, saveTempName, PNG_TYPE,1,size);

	if(status < 0 || FAILURE == fileExist(saveTempName)){
		char error[1024];
		QRcode_free(pQRC);
		efree(saveTempName);
		sprintf(error,"%s%d","[CQrcodeException] call [CQrcode->create] save to png fail : code ",status);
		zend_throw_exception(CExceptionCe, error, 1 TSRMLS_CC);
		return;
	}

	zend_update_property_string(CQrcodeCe,getThis(),ZEND_STRL("tempPath"),saveTempName TSRMLS_CC);

	//check use to saveIcon
	CQrcode_setQrIcon(getThis(),saveTempName TSRMLS_CC);


	efree(saveTempName);
	QRcode_free(pQRC);

#else
	RETVAL_ZVAL(getThis(),1,0);
#endif
}


PHP_METHOD(CQrcode,getFilePath){

	long	returnAllPath = 0;
	zval	*tempPath,
			*appPath;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|l",&returnAllPath) == FAILURE){
	}

	tempPath = zend_read_property(CQrcodeCe,getThis(),ZEND_STRL("tempPath"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tempPath) && strlen(Z_STRVAL_P(tempPath)) > 0 ){
	}else{
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->getFilePath] the qrcode has not created , must call create function frist", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	appPath = zend_read_static_property(CWebAppCe, ZEND_STRL("app_path"), 0 TSRMLS_CC);

	if(0 == returnAllPath){
		char	*repalceAppPath;
		str_replace(Z_STRVAL_P(appPath),"",Z_STRVAL_P(tempPath),&repalceAppPath);
		RETURN_STRING(repalceAppPath,0);
	}

	RETURN_STRING(Z_STRVAL_P(tempPath),1);
}

PHP_METHOD(CQrcode,show){

	zval	*tempPath,
			*imageRousce;


	RETVAL_ZVAL(getThis(),1,0);

	tempPath = zend_read_property(CQrcodeCe,getThis(),ZEND_STRL("tempPath"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tempPath) && strlen(Z_STRVAL_P(tempPath)) > 0 ){
	}else{
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->show] the qrcode has not created , must call create function frist", 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	//no gdb
	if (!zend_hash_exists(&module_registry, "gd", strlen("gd")+1)) {
		zend_throw_exception(CExceptionCe, "[CQrcodeException] Call [CQrcode->show] need install the [gd] extension ", 10001 TSRMLS_CC);
		return;
	}

	callHeader("Content-type: image/png",1 TSRMLS_CC);

	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],Z_STRVAL_P(tempPath),1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecreatefrompng",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(imageRousce);
		ZVAL_ZVAL(imageRousce,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1];
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],imageRousce,1,0);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagepng",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END


	zval_ptr_dtor(&imageRousce);
}

PHP_METHOD(CQrcode,setIcon){

	char	*content;
	int		contentLen = 0;
	long	size = 50;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|l",&content,&contentLen,&size) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CQrcodeException] call [CQrcode->setIcon] params error", 1 TSRMLS_CC);
		return;
	}

	if(FAILURE == fileExist(content)){
		zend_throw_exception(CClassNotFoundExceptionCe, "[CQrcodeException] call [CQrcode->setIcon] the icon not exists", 1 TSRMLS_CC);
		return;
	}

	zend_update_property_long(CQrcodeCe,getThis(),ZEND_STRL("iconSize"),size TSRMLS_CC);
	zend_update_property_string(CQrcodeCe,getThis(),ZEND_STRL("iconPath"),content TSRMLS_CC);
}
