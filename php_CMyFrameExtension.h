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

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

#ifndef PHP_CMYFRAMEEXTENSION_H
#define PHP_CMYFRAMEEXTENSION_H

extern zend_module_entry CMyFrameExtension_module_entry;
#define phpext_CMyFrameExtension_ptr &CMyFrameExtension_module_entry

#ifdef PHP_WIN32
#define PHP_CMYFRAMEEXTENSION_API __declspec(dllexport)
#else
#define PHP_CMYFRAMEEXTENSION_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define MODULE_BEGIN if(1 == 1){
#define MODULE_BEGIN_NO if(1 == 2){
#define MODULE_END }

#define CMYFRAME_STRLEN(str)		(str), (strlen(str)+1)



PHP_MINIT_FUNCTION(CMyFrameExtension);
PHP_MSHUTDOWN_FUNCTION(CMyFrameExtension);
PHP_RINIT_FUNCTION(CMyFrameExtension);
PHP_RSHUTDOWN_FUNCTION(CMyFrameExtension);
PHP_MINFO_FUNCTION(CMyFrameExtension);

//定义宏
#define CMYFRAME_REGISTER_CLASS(className)  register_class_##className(TSRMLS_C)
#define CMYFRAME_REGISTER_CLASS_RUN(className)  int register_class_##className(TSRMLS_C)


#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)) || (PHP_MAJOR_VERSION > 5)
#define CMYFRAME_STORE_EG_ENVIRON() \
	{ \
		zval ** __old_return_value_pp   = EG(return_value_ptr_ptr); \
		zend_op ** __old_opline_ptr  	= EG(opline_ptr); \
		zend_op_array * __old_op_array  = EG(active_op_array);

#define CMYFRAME_RESTORE_EG_ENVIRON() \
		EG(return_value_ptr_ptr) = __old_return_value_pp;\
		EG(opline_ptr)			 = __old_opline_ptr; \
		EG(active_op_array)		 = __old_op_array; \
	}

#else

#define CMYFRAME_STORE_EG_ENVIRON() \
	{ \
		zval ** __old_return_value_pp  		   = EG(return_value_ptr_ptr); \
		zend_op ** __old_opline_ptr 		   = EG(opline_ptr); \
		zend_op_array * __old_op_array 		   = EG(active_op_array); \
		zend_function_state * __old_func_state = EG(function_state_ptr);

#define CMYFRAME_RESTORE_EG_ENVIRON() \
		EG(return_value_ptr_ptr) = __old_return_value_pp;\
		EG(opline_ptr)			 = __old_opline_ptr; \
		EG(active_op_array)		 = __old_op_array; \
		EG(function_state_ptr)	 = __old_func_state; \
	}

#endif

#include "php_CBase.h"
#define CMYFRAME_EMPTYSTRING ""

//全局函数
PHP_FUNCTION(CDump);
PHP_FUNCTION(CMyFrameExtension_createProject);
PHP_FUNCTION(CMyFrameExtension_createPlugin);
PHP_FUNCTION(CMyFrameExtension_createConsumer);
PHP_FUNCTION(CMyFrameExtension_dumpClassMapForIDE);
PHP_FUNCTION(CMyFrameExtension_help);


//全局变量定义
ZEND_BEGIN_MODULE_GLOBALS(CMyFrameExtension)
	long open_trace;
ZEND_END_MODULE_GLOBALS(CMyFrameExtension)
#ifdef ZTS
#include "TSRM.h"
#define CMYFRAMEEXTENSION_G(v) TSRMG(CMyFrameExtension_globals_id, zend_CMyFrameExtension_globals *, v)
#else
#define CMYFRAMEEXTENSION_G(v) (CMyFrameExtension_globals.v)
#endif


#endif