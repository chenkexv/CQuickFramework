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

#ifndef PHP_CQuickFramework_H
#define PHP_CQuickFramework_H

extern zend_module_entry CQuickFramework_module_entry;
#define phpext_CQuickFramework_ptr &CQuickFramework_module_entry

#ifdef PHP_WIN32
#define PHP_CQuickFramework_API __declspec(dllexport)
#else
#define PHP_CQuickFramework_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define MODULE_BEGIN if(1 == 1){
#define MODULE_BEGIN_NO if(1 == 2){
#define MODULE_END }

#define CMYFRAME_STRLEN(str)		(str), (strlen(str)+1)



PHP_MINIT_FUNCTION(CQuickFramework);
PHP_MSHUTDOWN_FUNCTION(CQuickFramework);
PHP_RINIT_FUNCTION(CQuickFramework);
PHP_RSHUTDOWN_FUNCTION(CQuickFramework);
PHP_MINFO_FUNCTION(CQuickFramework);

//定义宏
#define CMYFRAME_REGISTER_CLASS(className)  register_class_##className(module_number TSRMLS_CC)
#define CMYFRAME_REGISTER_CLASS_RUN(className)  int register_class_##className(int module_number TSRMLS_DC)


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
PHP_FUNCTION(CQF_createProject);
PHP_FUNCTION(CQF_createPlugin);
PHP_FUNCTION(CQF_createConsumer);
PHP_FUNCTION(CQF_dumpClassMapForIDE);
PHP_FUNCTION(CQF_help);


#ifdef ZTS
#include "TSRM.h"
#define CQuickFramework_G(v) TSRMG(CQuickFramework_globals_id, zend_CQuickFramework_globals *, v)
#else
#define CQuickFramework_G(v) (CQuickFramework_globals.v)
#endif


#endif