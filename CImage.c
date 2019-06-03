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


#include "php_CQuickFramework.h"
#include "php_CImage.h"
#include "php_CException.h"



//zend类方法
zend_function_entry CImage_functions[] = {
	PHP_ME(CImage,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CImage,__destruct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(CImage,getImageInfo,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,show,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,resize,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,setGrayScale,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,setFilter,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,setQuality,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,save,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CImage,setWaterMask,NULL,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CImage)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CImage",CImage_functions);
	CImageCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//set params
	zend_declare_property_string(CImageCe, ZEND_STRL("imagePath"),"",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CImageCe, ZEND_STRL("imageInfo"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CImageCe, ZEND_STRL("image"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CImageCe, ZEND_STRL("waterInfo"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CImageCe, ZEND_STRL("waterImage"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CImageCe, ZEND_STRL("waterPadding"),5,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CImageCe, ZEND_STRL("quality"),-1,ZEND_ACC_PRIVATE TSRMLS_CC);

	return SUCCESS;
}

PHP_METHOD(CImage,__construct)
{

	char	*filePath,
			*fileType;

	int		filePathLen = 0;

	zval	*imageInfo,
			*saveImageInfo,
			*imageRousce,
			**width,
			**height,
			**mime;

	if (!zend_hash_exists(&module_registry, "gd", strlen("gd")+1)) {
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->__construct] need install the [gd] extension ", 10001 TSRMLS_CC);
		return;
	}

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&filePath,&filePathLen) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->__construct] params error ", 10001 TSRMLS_CC);
		return;
	}

	//check file exists
	if(SUCCESS != fileExist(filePath)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->__construct] File not exists : %s",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		return;
	}

	//get file extension name call getimagesize
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],filePath,1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"getimagesize",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(imageInfo);
		ZVAL_ZVAL(imageInfo,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	if(IS_ARRAY != Z_TYPE_P(imageInfo)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->__construct] Read File Error : %s",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		zval_ptr_dtor(&imageInfo);
		return;
	}

	//get file type image_type_to_extension
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn,
				**fileInfoType;
		MAKE_STD_ZVAL(params[0]);
		zend_hash_index_find(Z_ARRVAL_P(imageInfo),2,(void**)&fileInfoType);
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
		sprintf(errMessage,"[CImageException] Call [CImage->__construct] Read File Type error : %s , only can png,jpeg,gif",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		zval_ptr_dtor(&imageInfo);
		efree(fileType);
		return;
	}


	zend_hash_index_find(Z_ARRVAL_P(imageInfo),0,(void**)&width);
	zend_hash_index_find(Z_ARRVAL_P(imageInfo),1,(void**)&height);
	zend_hash_find(Z_ARRVAL_P(imageInfo),"mime",strlen("mime")+1,(void**)&mime);


	//save Image info
	MAKE_STD_ZVAL(saveImageInfo);
	array_init(saveImageInfo);
	add_assoc_long(saveImageInfo,"width",Z_LVAL_PP(width));
	add_assoc_long(saveImageInfo,"height",Z_LVAL_PP(height));
	add_assoc_string(saveImageInfo,"type",fileType,1);
	add_assoc_string(saveImageInfo,"mime",Z_STRVAL_PP(mime),1);
	
	//get image source
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		char	callFunctionName[60];
		sprintf(callFunctionName,"imagecreatefrom%s",fileType);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],filePath,1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,callFunctionName,0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(imageRousce);
		ZVAL_ZVAL(imageRousce,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	if(IS_RESOURCE != Z_TYPE_P(imageRousce)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->__construct] Read File Error : %s",filePath);
		zval_ptr_dtor(&imageInfo);
		zval_ptr_dtor(&imageRousce);
		zval_ptr_dtor(&saveImageInfo);
		efree(fileType);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		return;
	}


	//update to class
	zend_update_property(CImageCe,getThis(),ZEND_STRL("imageInfo"),saveImageInfo TSRMLS_CC);
	zend_update_property(CImageCe,getThis(),ZEND_STRL("image"),imageRousce TSRMLS_CC);

	//destroy
	zval_ptr_dtor(&imageInfo);
	zval_ptr_dtor(&imageRousce);
	zval_ptr_dtor(&saveImageInfo);
	efree(fileType);
}

PHP_METHOD(CImage,__destruct)
{
	zval	*image,
			*waterImage;

	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	waterImage = zend_read_property(CImageCe,getThis(),ZEND_STRL("waterImage"), 0 TSRMLS_CC);

	if(IS_RESOURCE == Z_TYPE_P(image)){
		zval	functionName,
				functionReturn,
				*params[1];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"imagedestroy",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&functionReturn);
	}

	if(IS_RESOURCE == Z_TYPE_P(waterImage)){
		zval	functionName,
				functionReturn,
				*params[1];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"imagedestroy",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],waterImage,1,0);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&functionReturn);
	}
}

PHP_METHOD(CImage,getImageInfo)
{
	zval	*imageInfo;
	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);
	RETVAL_ZVAL(imageInfo,1,0);
}


PHP_METHOD(CImage,show)
{

	zval	*imageInfo,
			*image,
			**type;

	char	imageHeader[1024];

	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(imageInfo)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->show] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(imageInfo),"type",strlen("type")+1,(void**)&type) && IS_STRING == Z_TYPE_PP(type)){
	}else{
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->show] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(image)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->show] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	
	sprintf(imageHeader,"Content-type: image/%s",Z_STRVAL_PP(type));
	callHeader(imageHeader,1 TSRMLS_CC);

	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1];
		char	functionName[90];

		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		INIT_ZVAL(callFunction);
		sprintf(functionName,"image%s",Z_STRVAL_PP(type));
		ZVAL_STRING(&callFunction,functionName,0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	RETURN_ZVAL(getThis(),1,0);
}

PHP_METHOD(CImage,resize)
{

	long	setWidth = 0,
			setHeight = 0,
			x = 0,
			y = 0,
			trueWidth = 0,
			trueHeight = 0;

	zval	*image,
			*imageInfo,
			**type,
			**nowHeight,
			**nowWidth;

	zval	*imageThub,
			*imageColor;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ll|ll",&setWidth,&setHeight,&x,&y) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->resize] params error ", 10001 TSRMLS_CC);
		return;
	}

	if(setWidth == 0 && setHeight == 0){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->resize] must set width or height ", 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	//imageInfo
	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(imageInfo)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	//get file type
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(imageInfo),"type",strlen("type")+1,(void**)&type) && IS_STRING == Z_TYPE_PP(type)){
	}else{
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	//image source
	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(image)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	zend_hash_find(Z_ARRVAL_P(imageInfo),"height",strlen("height")+1,(void**)&nowHeight);
	zend_hash_find(Z_ARRVAL_P(imageInfo),"width",strlen("width")+1,(void**)&nowWidth);


	//get width and height rate
	if(setWidth == 0){
		trueHeight = setHeight;
		trueWidth = (int)ceil((float)setHeight / Z_LVAL_PP(nowHeight)*Z_LVAL_PP(nowWidth));
	}else if(setHeight == 0){
		trueWidth = setWidth;
		trueHeight = (int)ceil((float)setWidth / Z_LVAL_PP(nowWidth)*Z_LVAL_PP(nowHeight));
	}else{
		trueHeight = setHeight;
		trueWidth = setWidth;
	}


	//call imagecreatetruecolor and save to imageThub
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[2],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[0],trueWidth);
		ZVAL_LONG(params[1],trueHeight);
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

	//call imagecolorallocate and save to imageColor
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
		ZVAL_LONG(params[1],255);
		ZVAL_LONG(params[2],255);
		ZVAL_LONG(params[3],255);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecolorallocate",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 4, params TSRMLS_CC);
		saveRetrun = &returnFunction;
		MAKE_STD_ZVAL(imageColor);
		ZVAL_ZVAL(imageColor,saveRetrun,1,0);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_dtor(&returnFunction);
	MODULE_END

	//call imagecolortransparent
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[2],
				*saveRetrun;

		MAKE_STD_ZVAL(params[0]);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_ZVAL(params[0],imageThub,1,0);
		ZVAL_ZVAL(params[1],imageColor,1,0);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagecolortransparent",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&returnFunction);
	MODULE_END

	//call imagefill
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
		ZVAL_ZVAL(params[3],imageColor,1,0);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagefill",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 4, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_ptr_dtor(&params[3]);
		zval_dtor(&returnFunction);
	MODULE_END

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
		ZVAL_ZVAL(params[1],image,1,0);
		ZVAL_LONG(params[2],0);
		ZVAL_LONG(params[3],0);
		ZVAL_LONG(params[4],0);
		ZVAL_LONG(params[5],0);
		ZVAL_LONG(params[6],trueWidth);
		ZVAL_LONG(params[7],trueHeight);
		ZVAL_LONG(params[8],Z_LVAL_PP(nowWidth));
		ZVAL_LONG(params[9],Z_LVAL_PP(nowHeight));
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

	//call imagedestroy old image
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveRetrun;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,"imagedestroy",0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	//save new Image
	zend_update_property(CImageCe,getThis(),ZEND_STRL("image"),imageThub TSRMLS_CC);

	//update width and height
	add_assoc_long(imageInfo,"width",trueWidth);
	add_assoc_long(imageInfo,"height",trueHeight);
	zend_update_property(CImageCe,getThis(),ZEND_STRL("imageInfo"),imageInfo TSRMLS_CC);

	RETVAL_ZVAL(getThis(),1,0);


	//destory
	zval_ptr_dtor(&imageThub);
	zval_ptr_dtor(&imageColor);
}

PHP_METHOD(CImage,setGrayScale)
{
	zval	*image;

	RETVAL_ZVAL(getThis(),1,0);

	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE == Z_TYPE_P(image)){
		//call imagefilter
		zval	functionName,
				functionReturn,
				*params[2];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"imagefilter",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[1],1);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&functionReturn);
	}
}

PHP_METHOD(CImage,setFilter)
{
	zval	*image;

	long	filterType = 0;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&filterType) == FAILURE){
		return;
	}


	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE == Z_TYPE_P(image)){
		//call imagefilter
		zval	functionName,
				functionReturn,
				*params[2];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"imagefilter",0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_LONG(params[1],filterType);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&functionReturn);
	}
}

PHP_METHOD(CImage,setQuality)
{

	zval	*imageInfo,
			**type;

	long	level;

	RETVAL_ZVAL(getThis(),1,0);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&level) == FAILURE){
		return;
	}

	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(imageInfo) && zend_hash_find(Z_ARRVAL_P(imageInfo),"type",strlen("type")+1,(void**)&type) == SUCCESS && IS_STRING == Z_TYPE_PP(type)){
		if(strcmp(Z_STRVAL_PP(type),"png") == 0){
			level = (int)ceil(level/10.0);
		}
	}
	
	zend_update_property_long(CImageCe,getThis(),ZEND_STRL("quality"),level TSRMLS_CC);
}

PHP_METHOD(CImage,save)
{
	char	*savePath,
			func[90];

	long	savePathLen = 0,
			isOver = 1;

	zval	*pathInfo,
			**dir,
			*imageInfo,
			*image,
			*quality,
			**type;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|b",&savePath,&savePathLen,&isOver) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->save] params error ", 10001 TSRMLS_CC);
		return;
	}

	if(isOver && SUCCESS == fileExist(savePath)){
		if(0 != remove(savePath)){
			//delete fail
		}
	}

	if(!isOver && SUCCESS == fileExist(savePath)){
		RETURN_FALSE;
	}

	//imageInfo
	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(imageInfo)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_FALSE;
	}

	//get file type
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(imageInfo),"type",strlen("type")+1,(void**)&type) && IS_STRING == Z_TYPE_PP(type)){
	}else{
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_FALSE
	}

	//image source
	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(image)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->save] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_FALSE;
	}

	//call pathinfo
	MODULE_BEGIN
		zval	functionName,
				functionReturn,
				*params[1],
				*saveInfo;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],savePath,1);
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,"pathinfo",0);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 1, params TSRMLS_CC);
		saveInfo = &functionReturn;
		MAKE_STD_ZVAL(pathInfo);
		ZVAL_ZVAL(pathInfo,saveInfo,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&functionReturn);
	MODULE_END

	if(IS_ARRAY != Z_TYPE_P(pathInfo)){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->save] can not parse the save Path ", 10001 TSRMLS_CC);
		zval_ptr_dtor(&pathInfo);
		RETURN_FALSE;
	}

	//dir
	zend_hash_find(Z_ARRVAL_P(pathInfo),"dirname",strlen("dirname")+1,(void**)&dir);
	if(FAILURE == fileExist(Z_STRVAL_PP(dir))){
		php_mkdir(Z_STRVAL_PP(dir));
	}

	//quality
	quality = zend_read_property(CImageCe,getThis(),ZEND_STRL("quality"), 0 TSRMLS_CC);

	sprintf(func,"image%s",Z_STRVAL_PP(type));

	//select quality
	if(strcmp(Z_STRVAL_PP(type),"gif") == 0 || Z_LVAL_P(quality) == -1){
		//no quality
		zval	functionName,
				functionReturn,
				*params[2];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,func,0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],savePath,1);
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 2, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_dtor(&functionReturn);
	}else{
		zval	functionName,
				functionReturn,
				*params[3];
		INIT_ZVAL(functionName);
		ZVAL_STRING(&functionName,func,0);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_ZVAL(params[0],image,1,0);
		MAKE_STD_ZVAL(params[1]);
		ZVAL_STRING(params[1],savePath,1);
		MAKE_STD_ZVAL(params[2]);
		ZVAL_LONG(params[2],Z_LVAL_P(quality));
		call_user_function(EG(function_table), NULL, &functionName, &functionReturn, 3, params TSRMLS_CC);
		zval_ptr_dtor(&params[0]);
		zval_ptr_dtor(&params[1]);
		zval_ptr_dtor(&params[2]);
		zval_dtor(&functionReturn);
	}

	if(SUCCESS == fileExist(savePath)){
		RETVAL_TRUE;
	}else{
		RETVAL_FALSE;
	}


	//destroy
	zval_ptr_dtor(&pathInfo);
}

int CImage_getPos(zval *object,int imageWidth,int imageHeight,int waterWidth,int waterHeight ,int postType,int getType TSRMLS_DC)
{
	
	zval	*paddingZval;

	int		padding = 5,
			x = 5,
			y=  5;

	paddingZval = zend_read_property(CImageCe,object,ZEND_STRL("waterPadding"), 0 TSRMLS_CC);
	padding = Z_LVAL_P(paddingZval);

	if(postType == 1){
		x = y = 5;
	}

	if(postType == 2){
		x = (imageWidth) - (waterWidth) - padding;
		y = padding;
	}

	if(postType == 3){
		x = padding;
		y = (imageWidth) - (waterWidth) - padding;
	}

	if(postType == 4){
		x = (imageWidth) - (waterWidth) - padding;
		y = (imageHeight) - (waterHeight) - padding;
	}

	if(postType == 5){
		x = (int)ceil(((imageWidth) - (waterWidth))/2.0);
		y = (int)ceil(((imageHeight) - (waterHeight))/2.0);
	}

	if(postType == 6){
		x = php_rand_call(padding,(imageWidth) - (waterWidth));
		y = php_rand_call(padding,(imageHeight) - (waterHeight));
	}

	if(getType == 1){
		return x;
	}else{
		return y;
	}
}

PHP_METHOD(CImage,setWaterMask)
{

	char	*filePath,
			*fileType;

	long	filePathLen = 0,
			pos = 4;

	int		posX = 5,
			posY = 5;

	zval	*waterInfo,
			*saveImageInfo,
			**width,
			**height,
			**mime,
			**sourceWidth,
			**sourceHeight,
			*imageInfo,
			*imageRousce,
			*image;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sl",&filePath,&filePathLen,&pos) == FAILURE){
		zend_throw_exception(CExceptionCe, "[CImageException] Call [CImage->save] params error ", 10001 TSRMLS_CC);
		return;
	}

	//imageInfo
	imageInfo = zend_read_property(CImageCe,getThis(),ZEND_STRL("imageInfo"), 0 TSRMLS_CC);
	if(IS_ARRAY != Z_TYPE_P(imageInfo)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}
	zend_hash_find(Z_ARRVAL_P(imageInfo),"width",strlen("width")+1,(void**)&sourceWidth);
	zend_hash_find(Z_ARRVAL_P(imageInfo),"height",strlen("height")+1,(void**)&sourceHeight);
	
	image = zend_read_property(CImageCe,getThis(),ZEND_STRL("image"), 0 TSRMLS_CC);
	if(IS_RESOURCE != Z_TYPE_P(image)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] must read image first now");
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}


	if(SUCCESS != fileExist(filePath)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] the water image not exists : %s",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],filePath,1);
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
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] Read File Error : %s",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		zval_ptr_dtor(&waterInfo);
		RETURN_ZVAL(getThis(),1,0);
	}

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
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] Read File Type error : %s , only can png,jpeg,gif",filePath);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		zval_ptr_dtor(&waterInfo);
		efree(fileType);
		RETURN_ZVAL(getThis(),1,0);
	}

	zend_hash_index_find(Z_ARRVAL_P(waterInfo),0,(void**)&width);
	zend_hash_index_find(Z_ARRVAL_P(waterInfo),1,(void**)&height);
	zend_hash_find(Z_ARRVAL_P(waterInfo),"mime",strlen("mime")+1,(void**)&mime);

	//get image source
	MODULE_BEGIN
		zval	callFunction,
				returnFunction,
				*params[1],
				*saveReturn;
		char	callFunctionName[60];
		sprintf(callFunctionName,"imagecreatefrom%s",fileType);
		MAKE_STD_ZVAL(params[0]);
		ZVAL_STRING(params[0],filePath,1);
		INIT_ZVAL(callFunction);
		ZVAL_STRING(&callFunction,callFunctionName,0);
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 1, params TSRMLS_CC);
		saveReturn = &returnFunction;
		MAKE_STD_ZVAL(imageRousce);
		ZVAL_ZVAL(imageRousce,saveReturn,1,0);
		zval_ptr_dtor(&params[0]);
		zval_dtor(&returnFunction);
	MODULE_END

	//save Image info
	MAKE_STD_ZVAL(saveImageInfo);
	array_init(saveImageInfo);
	add_assoc_long(saveImageInfo,"width",Z_LVAL_PP(width));
	add_assoc_long(saveImageInfo,"height",Z_LVAL_PP(height));
	add_assoc_string(saveImageInfo,"type",fileType,1);
	add_assoc_string(saveImageInfo,"mime",Z_STRVAL_PP(mime),1);
	
	//update to class
	zend_update_property(CImageCe,getThis(),ZEND_STRL("waterInfo"),saveImageInfo TSRMLS_CC);
	zend_update_property(CImageCe,getThis(),ZEND_STRL("waterImage"),imageRousce TSRMLS_CC);

	//check water is smaller than source image
	if(Z_LVAL_PP(width) > Z_LVAL_PP(sourceWidth) || Z_LVAL_PP(height) > Z_LVAL_PP(sourceHeight)){
		char	errMessage[1024];
		sprintf(errMessage,"[CImageException] Call [CImage->setWaterMask] The water mask image is bigger than source image",filePath);
		zval_ptr_dtor(&waterInfo);
		zval_ptr_dtor(&saveImageInfo);
		zval_ptr_dtor(&imageRousce);
		efree(fileType);
		zend_throw_exception(CExceptionCe, errMessage, 10001 TSRMLS_CC);
		RETURN_ZVAL(getThis(),1,0);
	}

	//get pos
	posX = CImage_getPos(getThis(),Z_LVAL_PP(sourceWidth),Z_LVAL_PP(sourceHeight),Z_LVAL_PP(width),Z_LVAL_PP(height),pos,1 TSRMLS_CC);
	posY = CImage_getPos(getThis(),Z_LVAL_PP(sourceWidth),Z_LVAL_PP(sourceHeight),Z_LVAL_PP(width),Z_LVAL_PP(height),pos,2 TSRMLS_CC);

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
		ZVAL_ZVAL(params[1],imageRousce,1,0);
		ZVAL_LONG(params[2],posX);
		ZVAL_LONG(params[3],posY);
		ZVAL_LONG(params[4],0);
		ZVAL_LONG(params[5],0);
		ZVAL_LONG(params[6],Z_LVAL_PP(width));
		ZVAL_LONG(params[7],Z_LVAL_PP(height));
		call_user_function(EG(function_table), NULL, &callFunction, &returnFunction, 8, params TSRMLS_CC);

		zend_update_property(CImageCe,getThis(),ZEND_STRL("image"),params[0] TSRMLS_CC);

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

	RETVAL_ZVAL(getThis(),1,0);

	//destroy
	zval_ptr_dtor(&waterInfo);
	zval_ptr_dtor(&imageRousce);
	zval_ptr_dtor(&saveImageInfo);
	efree(fileType);
}