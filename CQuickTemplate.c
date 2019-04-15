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
#include "php_CQuickTemplate.h"
#include "php_CException.h"
#include "ext/standard/php_smart_str_public.h"
#include "ext/standard/php_smart_str.h"


void CQuickTemplate__parse_var(zval *object,zval *match1,zval **returnZval TSRMLS_DC);
void CQuickTemplate__parse_modifiers(zval *object,zval **returnZval,zval *modifier TSRMLS_DC);
void CQuickTemplate_getCompileCache(zval *object,char *name,char **filePath TSRMLS_DC);

ZEND_BEGIN_ARG_INFO_EX(CQuickTemplate_call_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

//zend类方法
zend_function_entry CQuickTemplate_functions[] = {
	PHP_ME(CQuickTemplate,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CQuickTemplate,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CQuickTemplate,assign,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,display,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,compileTemplate,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_include_tags,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_syntax_error,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_attrs,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_vars_props,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_expand_quoted_text,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_var,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_smarty_ref,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_push_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_get_plugin_filepath,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_add_plugin,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_plugin_call,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_modifiers,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_dequote,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_pop_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_foreach_start,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_if_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_compiler_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_block_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_push_cacheable_state,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_arg_list,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_pop_cacheable_state,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_custom_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_parse_var_props,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_quote_replace,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,checkNeedCompileTemplate,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,getTemplateTruePath,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,getCompileCache,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,getFileLastChangeTime,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_compile_registered_object_tag,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,fetch,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,clear_cache,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,template_exists,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,register_function,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,register_block,NULL,ZEND_ACC_PUBLIC)
	PHP_ME(CQuickTemplate,_modifier_date_format,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,smarty_make_timestamp,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_modifier_string_format,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_modifier_default,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_modifier_replace,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_run_mod_handler,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,_modifier_cat,NULL,ZEND_ACC_PRIVATE)
	PHP_ME(CQuickTemplate,__call,CQuickTemplate_call_arginfo,ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CQuickTemplate)
{
	zend_class_entry funCe;
	INIT_CLASS_ENTRY(funCe,"CQuickTemplate",CQuickTemplate_functions);
	CQuickTemplateCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//定义变量
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_string(CQuickTemplateCe, ZEND_STRL("_version"),"1.0.0",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("template_dir"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("cacheDir"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("compileDir"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CQuickTemplateCe, ZEND_STRL("left_delimiter"),"<{",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CQuickTemplateCe, ZEND_STRL("right_delimiter"),"}>",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_folded_blocks"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_current_file"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_current_line_no"),1,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_capture_stack"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_plugin_info"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_init_smarty_vars"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_permitted_tokens"),ZEND_ACC_PRIVATE TSRMLS_CC);

	//词法提取标签
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_db_qstr_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_si_qstr_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_qstr_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_func_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_reg_obj_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_var_bracket_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_num_const_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_dvar_guts_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_dvar_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_cvar_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_svar_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_avar_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_mod_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_var_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_parenth_param_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_func_call_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_obj_ext_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_obj_start_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_obj_params_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_obj_call_regexp"),ZEND_ACC_PRIVATE TSRMLS_CC);


	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_cacheable_state"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_cache_attrs_count"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_nocache_count"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_cache_serial"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_cache_include"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("_strip_depth"),0,ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_string(CQuickTemplateCe, ZEND_STRL("_additional_newline"),"\n",ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_tag_stack"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_block"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_function"),ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(CQuickTemplateCe, ZEND_STRL("security"),0,ZEND_ACC_PRIVATE TSRMLS_CC);

	//模板变量
	zend_declare_property_null(CQuickTemplateCe, ZEND_STRL("_tpl_vars"),ZEND_ACC_PUBLIC TSRMLS_CC);

	return SUCCESS;
}

void this_hash_rehash(zval *stack){

	unsigned int k = 0;
	int should_rehash = 0;
	Bucket *p = Z_ARRVAL_P(stack)->pListHead;
	while (p != NULL) {
		if (p->nKeyLength == 0) {
			if (p->h != k) {
				p->h = k++;
				should_rehash = 1;
			} else {
				k++;
			}
		}
		p = p->pListNext;
	}
	Z_ARRVAL_P(stack)->nNextFreeElement = k;
	if(should_rehash){
		zend_hash_rehash(Z_ARRVAL_P(stack));
	}
}

int CQuickTemplate_getInstance(zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
		    *backZval;

	//读取静态instace变量
	instanceZval = zend_read_static_property(CQuickTemplateCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//为空时则实例化自身
	if(IS_NULL == Z_TYPE_P(instanceZval) ){
		
		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//查询自身类对象
		zend_hash_find(EG(class_table),"cquicktemplate",strlen("cquicktemplate")+1,(void**)&classCePP);
		classCe = *classCePP;

		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);

		//执行构造器
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);
			call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
			zval_dtor(&constructReturn);
		}

		//将类对象保存在instance静态变量
		zend_update_static_property(CQuickTemplateCe,ZEND_STRL("instance"),object TSRMLS_CC);
		
		//返回
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_ZVAL(*returnZval,object,1,1);
		return SUCCESS;
	}

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_ZVAL(*returnZval,instanceZval,1,0);
	return SUCCESS;
}

PHP_METHOD(CQuickTemplate,getInstance){
	zval *instanceZval;
	CQuickTemplate_getInstance(&instanceZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,instanceZval,1,1);
}

void CQuickTemplate__expand_quoted_text(zval *object,zval *var_expr,zval **returnZval TSRMLS_DC){

	zval	*_dvar_guts_regexp,
			*_obj_ext_regexp,
			*_match;

	char	*reg1,
			*reg3,
			*returnString,
			*endString;

	MAKE_STD_ZVAL(*returnZval);

	_dvar_guts_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_dvar_guts_regexp"),0 TSRMLS_CC);
	_obj_ext_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_ext_regexp"),0 TSRMLS_CC);
	strcat2(&reg1,"~(?:\\`(?<!\\\\)\\$",Z_STRVAL_P(_dvar_guts_regexp),"(?:",Z_STRVAL_P(_obj_ext_regexp),")*\\`)|(?:(?<!\\\\)\\$\\w+(\\[[a-zA-Z0-9]+\\])*)~",NULL);
	reg3 = estrdup("~\\.\"\"|(?<!\\\\)\"\"\\.~");

	if(preg_match_all(reg1,Z_STRVAL_P(var_expr),&_match)){

		zval	**match,
				*_replace,
				**_var,
				*sendParams,
				*returnParseAttr;
		int		i,n;

		char	*var1,
				*thisNewString,
				*newString;

		zend_hash_index_find(Z_ARRVAL_P(_match),0,(void**)&match);
		
		MAKE_STD_ZVAL(_replace);
		array_init(_replace);

		n = zend_hash_num_elements(Z_ARRVAL_PP(match));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(match));
		for(i = 0 ; i < n; i++){
			zend_hash_get_current_data(Z_ARRVAL_PP(match),(void**)&_var);

			str_replace("`","",Z_STRVAL_PP(_var),&var1);
			MAKE_STD_ZVAL(sendParams);
			ZVAL_STRING(sendParams,var1,1);
			efree(var1);
			CQuickTemplate__parse_var(object,sendParams,&returnParseAttr TSRMLS_CC);
			strcat2(&thisNewString,"\".(",Z_STRVAL_P(returnParseAttr),").\"",NULL);
			zval_ptr_dtor(&returnParseAttr);
			zval_ptr_dtor(&sendParams);
			add_assoc_string(_replace,Z_STRVAL_PP(_var),thisNewString,1);
			efree(thisNewString);
			zend_hash_move_forward(Z_ARRVAL_PP(match));
		}

		php_strtr(Z_STRVAL_P(var_expr),_replace,&newString);
		preg_replace(reg3,"",newString,&returnString);
		zval_ptr_dtor(&_replace);
		efree(newString);
	}else{
		returnString = estrdup(Z_STRVAL_P(var_expr));
	}

	preg_replace("~^\"([\\s\\w]+)\"$~","'\\1'",returnString,&endString);

	ZVAL_STRING(*returnZval,endString,1);
	efree(endString);
	zval_ptr_dtor(&_match);
	efree(returnString);
	efree(reg1);
	efree(reg3);

}

void CQuickTemplate__parse_var_props(zval *object,zval *values,zval **thisParseValue TSRMLS_DC)
{

	char	*val,
			*reg1,
			*reg2,
			*reg3,
			*reg4,
			*reg5;

	zval	*_obj_call_regexp,
			*_dvar_regexp,
			*_mod_regexp,
			*_db_qstr_regexp,
			*_num_const_regexp,
			*_si_qstr_regexp,
			*_cvar_regexp,
			*_svar_regexp,
			*_permitted_tokens,
			*match = NULL,
			*match2 = NULL,
			*match3 = NULL,
			*match4 = NULL;

	//调用trim
	php_trim(Z_STRVAL_P(values)," \\/\r\n",&val);

	MAKE_STD_ZVAL(*thisParseValue);

	//生成提取式
	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_dvar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_dvar_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),0 TSRMLS_CC);
	_db_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_db_qstr_regexp"),0 TSRMLS_CC);
	_num_const_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),0 TSRMLS_CC);
	_si_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_si_qstr_regexp"),0 TSRMLS_CC);
	_cvar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_cvar_regexp"),0 TSRMLS_CC);
	_svar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_svar_regexp"),0 TSRMLS_CC);
	_permitted_tokens = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_permitted_tokens"),0 TSRMLS_CC);

	strcat2(&reg1,"~^(",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_dvar_regexp),")(",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
	strcat2(&reg2,"~^",Z_STRVAL_P(_db_qstr_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
	strcat2(&reg3,"~^(",Z_STRVAL_P(_db_qstr_regexp),")(",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
	strcat2(&reg4,"~^",Z_STRVAL_P(_num_const_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
	strcat2(&reg5,"~^",Z_STRVAL_P(_si_qstr_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);


	if(preg_match(reg1,val,&match)){

		zval	**match1,
				**modifiers = NULL,
				*returnZval;

		zend_hash_index_find(Z_ARRVAL_P(match),1,(void**)&match1);
		zend_hash_index_find(Z_ARRVAL_P(match),2,(void**)&modifiers);

		//解析单个变量
		CQuickTemplate__parse_var(object,*match1,&returnZval TSRMLS_CC);
		if(modifiers != NULL && IS_NULL != Z_TYPE_PP(modifiers)){
			CQuickTemplate__parse_modifiers(object,&returnZval,*modifiers TSRMLS_CC);
		}

		ZVAL_ZVAL(*thisParseValue,returnZval,1,0);
	
		//销毁
		zval_ptr_dtor(&returnZval);
		efree(val);
		zval_ptr_dtor(&match);
		efree(reg1);
		efree(reg2);
		efree(reg3);
		efree(reg4);
		efree(reg5);
		return;
	}else if(preg_match(reg2,val,&match2)){
		zval	*thisMatch,
				**thisMatch1 = NULL,
				**thisMatch2 = NULL,
				*returnZval;

		preg_match(reg3, val, &thisMatch);
		zend_hash_index_find(Z_ARRVAL_P(thisMatch),1,(void**)&thisMatch1);
		CQuickTemplate__expand_quoted_text(object,*thisMatch1,&returnZval TSRMLS_CC);

		zend_hash_index_find(Z_ARRVAL_P(thisMatch),2,(void**)&thisMatch2);
		if(thisMatch2 != NULL && IS_NULL != Z_TYPE_PP(thisMatch2)){
			CQuickTemplate__parse_modifiers(object,&returnZval, *thisMatch2 TSRMLS_CC);
		}

		ZVAL_ZVAL(*thisParseValue,returnZval,1,0);
		zval_ptr_dtor(&returnZval);
		zval_ptr_dtor(&thisMatch);
		efree(val);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match2);
		efree(reg1);
		efree(reg2);
		efree(reg3);
		efree(reg4);
		efree(reg5);
		return;

	}else if(preg_match(reg4,val,&match3)){
		// numerical constant
		char	*reg41;
		zval	*thisMatch41,
				*returnZval = NULL;
		strcat2(&reg41,"~^(",Z_STRVAL_P(_num_const_regexp),")(",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
		
		if(preg_match(reg41, val, &thisMatch41)){
			zval	**thisVal1,
					**thisVal2;
			if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(thisMatch41),2,(void**)&thisVal2) && IS_STRING == Z_TYPE_PP(thisVal2) && strlen(Z_STRVAL_PP(thisVal2)) > 0){
				zend_hash_index_find(Z_ARRVAL_P(thisMatch41),1,(void**)&thisVal1);
				MAKE_STD_ZVAL(returnZval);
				ZVAL_ZVAL(returnZval,*thisVal1,1,0);
				CQuickTemplate__parse_modifiers(object,&returnZval, *thisVal2 TSRMLS_CC);
				ZVAL_ZVAL(*thisParseValue,returnZval,1,0);
				zval_ptr_dtor(&returnZval);
				zval_ptr_dtor(&thisMatch41);
				efree(val);
				zval_ptr_dtor(&match);
				zval_ptr_dtor(&match2);
				zval_ptr_dtor(&match3);
				efree(reg1);
				efree(reg2);
				efree(reg3);
				efree(reg4);
				efree(reg41);
				efree(reg5);
				return;
			}
		}
		zval_ptr_dtor(&thisMatch41);
		efree(reg41);

	}else if(preg_match(reg5,val,&match4)){
			
		// single quoted text
		char	*reg51;
		zval	*thisMatch51,
				*returnZval = NULL;

		strcat2(&reg51,"~^(",Z_STRVAL_P(_si_qstr_regexp),")(",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);
		if( preg_match(reg51, val, &thisMatch51)){

			zval	**thisVal1,
					**thisVal2;
			if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(thisMatch51),2,(void**)&thisVal2) && IS_STRING == Z_TYPE_PP(thisVal2) && strlen(Z_STRVAL_PP(thisVal2)) > 0){
				zend_hash_index_find(Z_ARRVAL_P(thisMatch51),1,(void**)&thisVal1);
				MAKE_STD_ZVAL(returnZval);
				ZVAL_ZVAL(returnZval,*thisVal1,1,0);
				CQuickTemplate__parse_modifiers(object,&returnZval, *thisVal2 TSRMLS_CC);
				ZVAL_ZVAL(*thisParseValue,returnZval,1,0);

				zval_ptr_dtor(&returnZval);
				zval_ptr_dtor(&thisMatch51);
				efree(val);
				zval_ptr_dtor(&match);
				zval_ptr_dtor(&match2);
				zval_ptr_dtor(&match3);
				zval_ptr_dtor(&match4);
				efree(reg1);
				efree(reg2);
				efree(reg3);
				efree(reg4);
				efree(reg5);
				efree(reg51);
				return;
			}
		}
		zval_ptr_dtor(&thisMatch51);
		efree(reg51);
	}


		
	//待销毁
	ZVAL_STRING(*thisParseValue,val,1);
	efree(val);
	if(match != NULL) zval_ptr_dtor(&match);
	if(match2 != NULL) zval_ptr_dtor(&match2);
	if(match3 != NULL) zval_ptr_dtor(&match3);
	if(match4 != NULL) zval_ptr_dtor(&match4);
	efree(reg1);
	efree(reg2);
	efree(reg3);
	efree(reg4);
	efree(reg5);
}

void CQuickTemplate__parse_attrs(zval *object,zval *argsstring,zval **argsArray TSRMLS_DC){

	char	*reg,
			*reg1,
			*reg2,
			*attr_name = NULL,
			*lastToken = NULL,
			*resetToken = NULL,
			*otherKey;

	zval	*_obj_call_regexp,
			*_qstr_regexp,
			*match,
			**tokens,
			**token,
			*_num_const_regexp,
			*_var_regexp,
			*_mod_regexp;

	int		i,n,state;
	ulong	otheriKey;

	MAKE_STD_ZVAL(*argsArray);
	array_init(*argsArray);

	if(IS_STRING != Z_TYPE_P(argsstring)){
		return;
	}


	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_qstr_regexp"),0 TSRMLS_CC);
	_num_const_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),0 TSRMLS_CC);
	_var_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),0 TSRMLS_CC);

	
	//提取参数
	strcat2(&reg,"~(?:",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_qstr_regexp)," | (?>[^\"\'=\\s]+) )+ | [=] ~x",NULL);
	strcat2(&reg1,"~^",Z_STRVAL_P(_num_const_regexp),"|0[xX][0-9a-fA-F]+$~",NULL);
	strcat2(&reg2,"~^",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_var_regexp),"(?:",Z_STRVAL_P(_mod_regexp),")*$~",NULL);


	if(!preg_match_all(reg,Z_STRVAL_P(argsstring),&match)){
		zval_ptr_dtor(&match);
		efree(reg);
		efree(reg1);
		efree(reg2);
		return;
	}
	efree(reg);

	if(IS_ARRAY != Z_TYPE_P(match)){
		zval_ptr_dtor(&match);
		efree(reg1);
		efree(reg2);
		return;
	}

	n = zend_hash_num_elements(Z_ARRVAL_P(match));
	if(n < 1){
		zval_ptr_dtor(&match);
		efree(reg1);
		efree(reg2);
		return;
	}

	//获取tokens
	zend_hash_index_find(Z_ARRVAL_P(match),0,(void**)&tokens);

	//遍历
	state = 0;
	n = zend_hash_num_elements(Z_ARRVAL_PP(tokens));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(tokens));

	for(i = 0 ; i < n; i++){

		zend_hash_get_current_data(Z_ARRVAL_PP(tokens),(void**)&token);

		switch(state){
			case 0:
				MODULE_BEGIN
					zval *matchToken;
					if(preg_match("~^\\w+$~",Z_STRVAL_PP(token),&matchToken)){
						if(attr_name != NULL){
							efree(attr_name);
							attr_name = NULL;
						}
						attr_name = estrdup(Z_STRVAL_PP(token));
						state = 1;
					}else{
						char errorMessage[1024];
						sprintf(errorMessage,"%s%s","[CViewException] invalid attribute name :",Z_STRVAL_PP(token));
						php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
						zval_ptr_dtor(&matchToken);
						if(attr_name != NULL){
							efree(attr_name);
							attr_name = NULL;
						}
						efree(reg1);
						efree(reg2);
						zval_ptr_dtor(&match);
						return;
					}
					zval_ptr_dtor(&matchToken);
				MODULE_END
				break;

			case 1:

				if(strcmp(Z_STRVAL_PP(token),"=") == 0){
					state = 2;
				}else{
					char errorMessage[1024];
					sprintf(errorMessage,"%s%s","[CViewException] expecting '=' after attribute name  :",lastToken);
					php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
					if(attr_name != NULL){
						efree(attr_name);
						attr_name = NULL;
					}
					efree(reg1);
					efree(reg2);
					zval_ptr_dtor(&match);
					return;
				}
				break;

			case 2:

				if(strcmp(Z_STRVAL_PP(token),"=") != 0){
					zval	*matchToken1 = NULL,
							*matchToken2 = NULL,
							*matchToken3 = NULL,
							*matchToken4 = NULL;
					if (preg_match("~^(on|yes|true)$~", Z_STRVAL_PP(token),&matchToken1)) {
						if(resetToken != NULL){
							efree(resetToken);
							resetToken = NULL;
						}
						resetToken = estrdup("true");
					}else if (preg_match("~^(off|no|false)$~", Z_STRVAL_PP(token),&matchToken2)) {
						if(resetToken != NULL){
							efree(resetToken);
							resetToken = NULL;
						}
                        resetToken = estrdup("false");
                    }else if(strcmp(Z_STRVAL_PP(token),"null") == 0){
						if(resetToken != NULL){
							efree(resetToken);
							resetToken = NULL;
						}
						resetToken = estrdup("null");
					}else if(preg_match(reg1,Z_STRVAL_PP(token),&matchToken3)){

					}else if(!preg_match(reg2,Z_STRVAL_PP(token),&matchToken4)){
	
						//添加addslashes
						char	*newTokenString;
						php_addslashes(Z_STRVAL_PP(token),&newTokenString);
						if(resetToken != NULL){
							efree(resetToken);
							resetToken = NULL;
						}
						strcat2(&resetToken,"\"",newTokenString,"\"",NULL);
						efree(newTokenString);
					}
					
					state = 0;
					if(resetToken == NULL){
						add_assoc_string(*argsArray,attr_name,Z_STRVAL_PP(token),1);
					}else{
						add_assoc_string(*argsArray,attr_name,resetToken,1);
					}

					if(matchToken1 != NULL){
						zval_ptr_dtor(&matchToken1);
						matchToken1 = NULL;
					}
					if(matchToken2 != NULL){
						zval_ptr_dtor(&matchToken2);
						matchToken2 = NULL;
					}
					if(matchToken3 != NULL){
						zval_ptr_dtor(&matchToken3);
						matchToken3 = NULL;
					}
					if(matchToken4 != NULL){
						zval_ptr_dtor(&matchToken4);
						matchToken4 = NULL;
					}
				}else{
					char errorMessage[1024];
					sprintf(errorMessage,"%s","[CViewException] \"'=' cannot be an attribute value");
					php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
					if(attr_name != NULL){
						efree(attr_name);
						attr_name = NULL;
					}
					if(resetToken != NULL){
						efree(resetToken);
						resetToken = NULL;
					}
					efree(reg1);
					efree(reg2);
					zval_ptr_dtor(&match);
					return;
				}
				break;
		}

		if(lastToken != NULL){
			efree(lastToken);
		}
		if(resetToken != NULL){
			efree(resetToken);
			resetToken = NULL;
		}
		lastToken = estrdup(Z_STRVAL_PP(token));
		zend_hash_move_forward(Z_ARRVAL_PP(tokens));
	}


	//裁减变量
	MODULE_BEGIN
		zval	**values,
				*thisParseValue;
		n = zend_hash_num_elements(Z_ARRVAL_PP(argsArray));

		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(argsArray));
		for(i=0 ; i < n ; i++){
			zend_hash_get_current_key(Z_ARRVAL_PP(argsArray),&otherKey,&otheriKey,0);
			zend_hash_get_current_data(Z_ARRVAL_PP(argsArray),(void**)&values);
			CQuickTemplate__parse_var_props(object,*values,&thisParseValue TSRMLS_CC);
			add_assoc_zval(*argsArray,otherKey,thisParseValue);
			zend_hash_move_forward(Z_ARRVAL_PP(argsArray));
		}
	MODULE_END


	//需销毁变量
	zval_ptr_dtor(&match);
	if(attr_name != NULL){
		efree(attr_name);
		attr_name = NULL;
	}
	if(resetToken != NULL){
		efree(resetToken);
		resetToken = NULL;
	}
	efree(reg1);
	efree(reg2);
	if(lastToken != NULL){
		efree(lastToken);
	}
}

//初始化类变量
void CQuickTemplate_initParams(zval *object TSRMLS_DC){

	zval		*default_folded_blocks,
				*default_capture_stack,
				*default_plugin_info,
				*default_permitted_tokens,
				*default_tag_stack,
				*default_block,
				*default_function,
				*default_tpl_vars,
				*cconfigInstanceZval,
				*templateDir,
				*cacheDir,
				*compileDir;

	MAKE_STD_ZVAL(default_folded_blocks);
	MAKE_STD_ZVAL(default_capture_stack);
	MAKE_STD_ZVAL(default_plugin_info);
	MAKE_STD_ZVAL(default_permitted_tokens);
	MAKE_STD_ZVAL(default_tag_stack);
	MAKE_STD_ZVAL(default_block);
	MAKE_STD_ZVAL(default_function);
	MAKE_STD_ZVAL(default_tpl_vars);
	array_init(default_folded_blocks);
	array_init(default_capture_stack);
	array_init(default_plugin_info);
	array_init(default_permitted_tokens);
	array_init(default_tag_stack);
	array_init(default_block);
	array_init(default_function);
	array_init(default_tpl_vars);

	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_folded_blocks"),default_folded_blocks TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_capture_stack"),default_capture_stack TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_plugin_info"),default_plugin_info TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_permitted_tokens"),default_permitted_tokens TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),default_tag_stack TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_block"),default_block TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_function"),default_function TSRMLS_CC);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tpl_vars"),default_tpl_vars TSRMLS_CC);

	zval_ptr_dtor(&default_folded_blocks);
	zval_ptr_dtor(&default_capture_stack);
	zval_ptr_dtor(&default_plugin_info);
	zval_ptr_dtor(&default_permitted_tokens);
	zval_ptr_dtor(&default_tag_stack);
	zval_ptr_dtor(&default_block);
	zval_ptr_dtor(&default_function);
	zval_ptr_dtor(&default_tpl_vars);

	//通过配置文件设定各种编译目录等
	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load("TEMPLATE.CQuickTemplate.CONF_INFO.template_dir",cconfigInstanceZval,&templateDir TSRMLS_CC);
	CConfig_load("TEMPLATE.CQuickTemplate.CONF_INFO.cache_dir",cconfigInstanceZval,&cacheDir TSRMLS_CC);
	CConfig_load("TEMPLATE.CQuickTemplate.CONF_INFO.compile_dir",cconfigInstanceZval,&compileDir TSRMLS_CC);


	//编译目录
	if(IS_STRING == Z_TYPE_P(templateDir)){
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("template_dir"),Z_STRVAL_P(templateDir) TSRMLS_CC);
	}
	if(IS_STRING == Z_TYPE_P(cacheDir)){
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("cacheDir"),Z_STRVAL_P(cacheDir) TSRMLS_CC);
	}
	if(IS_STRING == Z_TYPE_P(compileDir)){
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("compileDir"),Z_STRVAL_P(compileDir) TSRMLS_CC);
	}

	//编译标签
	MODULE_BEGIN

		char	*_db_qstr_regexp = "\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"",
				*_si_qstr_regexp = "\'[^\'\\\\]*(?:\\\\.[^\'\\\\]*)*\'",
				_qstr_regexp[1024],
				*_var_bracket_regexp = "\\[\\$?[\\w\\.]+\\]",
				*_num_const_regexp = "(?:\\-?\\d+(?:\\.\\d+)?)",
				*_dvar_math_regexp = "(?:[\\+\\*\\/\\%]|(?:-(?!>)))",
				*_dvar_math_var_regexp = "[\\$\\w\\.\\+\\-\\*\\/\\%\\d\\>\\[\\]]",
				_dvar_guts_regexp[1024],
				_dvar_regexp[1024],
				*_cvar_regexp = "\\#\\w+\\#",
				*_svar_regexp = "\\%\\w+\\.\\w+\\%",
				_avar_regexp[1024],
				_var_regexp[1024],
				_obj_ext_regexp[1024],
				_obj_restricted_param_regexp[10240],
				_obj_single_param_regexp[10240],
				_obj_params_regexp[10240],
				_obj_start_regexp[10240],
				_obj_call_regexp[10240],
				_mod_regexp[10240],
				*_func_regexp = "[a-zA-Z_]\\w*",
				*_reg_obj_regexp = "[a-zA-Z_]\\w*->[a-zA-Z_]\\w*",
				_param_regexp[10240],
				_parenth_param_regexp[20240],
				_func_call_regexp[20240];

		
		sprintf(_qstr_regexp,"%s%s%s%s%s","(?:",_db_qstr_regexp,"|",_si_qstr_regexp,")");
		sprintf(_dvar_guts_regexp,"%s%s%s%s%s%s%s%s%s%s%s","\\w+(?:",_var_bracket_regexp,")*(?:\\.\\$?\\w+(?:",_var_bracket_regexp,")*)*(?:",_dvar_math_regexp,"(?:",_num_const_regexp,"|",_dvar_math_var_regexp,")*)?");
		sprintf(_dvar_regexp,"%s%s","\\$",_dvar_guts_regexp);
		sprintf(_avar_regexp,"%s%s%s%s%s%s%s","(?:",_dvar_regexp,"|",_cvar_regexp,"|",_svar_regexp,")");
		sprintf(_var_regexp,"%s%s%s%s%s","(?:",_avar_regexp,"|",_qstr_regexp,")");
		sprintf(_obj_ext_regexp,"%s%s%s","\\->(?:\\$?",_dvar_guts_regexp,")");
		sprintf(_obj_restricted_param_regexp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s","(?:","(?:",_var_regexp,"|",_num_const_regexp,")(?:",_obj_ext_regexp,"(?:\\((?:(?:",_var_regexp,"|",_num_const_regexp,")","(?:\\s*,\\s*(?:",_var_regexp,"|",_num_const_regexp,"))*)?\\))?)*)");
		sprintf(_obj_single_param_regexp,"%s%s%s%s%s%s","(?:\\w+|",_obj_restricted_param_regexp,"(?:\\s*,\\s*(?:(?:\\w+|",_var_regexp,_obj_restricted_param_regexp,")))*)");
		sprintf(_obj_params_regexp,"%s%s%s%s%s","\\((?:",_obj_single_param_regexp,"(?:\\s*,\\s*",_obj_single_param_regexp,")*)?\\)");
		sprintf(_obj_start_regexp,"%s%s%s%s%s","(?:",_dvar_regexp,"(?:",_obj_ext_regexp,")+)");
		sprintf(_obj_call_regexp,"%s%s%s%s%s%s%s%s%s%s%s","(?:",_obj_start_regexp,"(?:",_obj_params_regexp,")?(?:",_dvar_math_regexp,"(?:",_num_const_regexp,"|",_dvar_math_var_regexp,")*)?)");
		sprintf(_mod_regexp,"%s%s%s%s%s%s%s%s%s","(?:\\|@?\\w+(?::(?:\\w+|",_num_const_regexp,"|",_obj_call_regexp,"|",_avar_regexp,"|",_qstr_regexp,"))*)");
		sprintf(_param_regexp,"%s%s%s%s%s%s%s%s%s","(?:\\s*(?:",_obj_call_regexp,"|",_var_regexp,"|",_num_const_regexp,"|\\w+)(?>",_mod_regexp,"*)\\s*)");	
		sprintf(_parenth_param_regexp,"%s%s%s%s%s","(?:\\((?:\\w+|",_param_regexp,"(?:\\s*,\\s*(?:(?:\\w+|",_param_regexp,")))*)?\\))");
		sprintf(_func_call_regexp,"%s%s%s%s%s","(?:",_func_regexp,"\\s*(?:",_parenth_param_regexp,"))");

		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_db_qstr_regexp"),_db_qstr_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_si_qstr_regexp"),_si_qstr_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_qstr_regexp"),_qstr_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_var_bracket_regexp"),_var_bracket_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),_num_const_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_dvar_math_regexp"),_dvar_math_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_dvar_math_var_regexp"),_dvar_math_var_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_dvar_guts_regexp"),_dvar_guts_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_dvar_regexp"),_dvar_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_cvar_regexp"),_cvar_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_svar_regexp"),_svar_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_avar_regexp"),_avar_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),_var_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_ext_regexp"),_obj_ext_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_restricted_param_regexp"),_obj_restricted_param_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_single_param_regexp"),_obj_single_param_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_params_regexp"),_obj_params_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_start_regexp"),_obj_start_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),_obj_call_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),_mod_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_func_regexp"),_func_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_reg_obj_regexp"),_reg_obj_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_param_regexp"),_param_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_parenth_param_regexp"),_parenth_param_regexp TSRMLS_CC);
		zend_update_property_string(CQuickTemplateCe,object,ZEND_STRL("_func_call_regexp"),_func_call_regexp TSRMLS_CC);
	MODULE_END


	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&templateDir);
	zval_ptr_dtor(&cacheDir);
	zval_ptr_dtor(&compileDir);
}

//初始化类变量
PHP_METHOD(CQuickTemplate,__construct){

	//初始化类变量
	CQuickTemplate_initParams(getThis() TSRMLS_CC);

}

//获取模板的绝对路径
void CQuickTemplate_getTemplateTruePath(zval *object,char *name,char **trueUrl TSRMLS_DC){
	zval	*templateDir;
	templateDir = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("template_dir"),0 TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(templateDir)){
		
		char *endPath;
		strcat2(&endPath,Z_STRVAL_P(templateDir),"/",name,NULL);
		*trueUrl = estrdup(endPath);
		efree(endPath);
	}else{
		*trueUrl = estrdup(name);
	}
}

PHP_METHOD(CQuickTemplate,getTemplateTruePath){
}

void CQuickTemplate_assign(zval *object,char *key,zval *value TSRMLS_DC){
	
	zval	*_tpl_vars,
			*saveZval;

	_tpl_vars = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tpl_vars"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(saveZval);
	ZVAL_ZVAL(saveZval,value,1,0);
	add_assoc_zval(_tpl_vars,key,saveZval);
}

PHP_METHOD(CQuickTemplate,assign){

	char	*key;
	long	keyLen = 0;
	zval	*value;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&key,&keyLen,&value) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call assign params error");
		RETURN_FALSE;
	}

	CQuickTemplate_assign(getThis(),key,value TSRMLS_CC);
	RETURN_TRUE;
}

void CQuickTemplate__compile_smarty_ref(zval *object,zval *_indexes,char **getString TSRMLS_DC){
	
	char	*_ref = NULL,
			*substr01,
			*compiled_ref,
			*fristKey;

	ulong	firstKey;

	zval	**_var,
			*match;

	int		i,n,_max_index = 0;

	n = zend_hash_num_elements(Z_ARRVAL_P(_indexes));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(_indexes));
	for(i = 0 ; i < n; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(_indexes),(void**)&_var);

		if(IS_STRING != Z_TYPE_PP(_var)){
			if(_ref != NULL) efree(_ref);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] template use tags {$smarty} is an invalid reference");
			return;
		}

		//判断是否符合语法
		substr(Z_STRVAL_PP(_var),0,1,&substr01);

		if(strcmp(substr01,".") != 0 && i < 2 || !preg_match("~^(\\.|\\[|->)~",Z_STRVAL_PP(_var),&match)){
			efree(substr01);
			zval_ptr_dtor(&match);
			if(_ref != NULL) efree(_ref);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] template use tags {$smarty} is an invalid reference");
			return;
		}

		if(i == 0){
			zend_hash_get_current_key(Z_ARRVAL_P(_indexes),&fristKey,&firstKey,0);
			substr1(Z_STRVAL_PP(_var),1,&_ref);
		}

		efree(substr01);
		zval_ptr_dtor(&match);
		zend_hash_move_forward(Z_ARRVAL_P(_indexes));
	}


	if(strcmp(_ref,"now") == 0){

		compiled_ref = estrdup("time()");
		_max_index = 1;

	}else if(strcmp(_ref,"get") == 0){

		compiled_ref = estrdup("$_GET");

	}else if(strcmp(_ref,"post") == 0){

		compiled_ref = estrdup("$_POST");

	}else if(strcmp(_ref,"cookies") == 0){

		compiled_ref = estrdup("$_COOKIE");

	}else if(strcmp(_ref,"server") == 0){

		compiled_ref = estrdup("$_SERVER");

	}else if(strcmp(_ref,"session") == 0){

		compiled_ref = estrdup("$_SESSION");

	}else if(strcmp(_ref,"request") == 0){

		compiled_ref = estrdup("$_REQUEST");

	}else{
		if(_ref != NULL) efree(_ref);
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] template use tags {$smarty} is an invalid reference");
		return;
	}

	if(_max_index > 0 && zend_hash_num_elements(Z_ARRVAL_P(_indexes)) > _max_index ){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] template use tags {$smarty} is an invalid reference");
		return;
	}
	

	*getString = estrdup(compiled_ref);

	//移除数组最后1个
	zend_hash_index_del(Z_ARRVAL_P(_indexes),firstKey);
	this_hash_rehash((_indexes));

	efree(compiled_ref);
	if(_ref != NULL) efree(_ref);
}

void CQuickTemplate__parse_var(zval *object,zval *match1,zval **returnZval TSRMLS_DC){

	zval	*_obj_call_regexp,
			*_dvar_regexp,
			*_mod_regexp,
			*_db_qstr_regexp,
			*_num_const_regexp,
			*_si_qstr_regexp,
			*_cvar_regexp,
			*_svar_regexp,
			*_permitted_tokens,
			*_math_vars,
			*_dvar_math_regexp,
			*_qstr_regexp,
			**_math_var,
			*_obj_params_regexp,
			*_var_bracket_regexp;

	int		has_math = 0,
			n = 0,
			i;

	char	*reg1,
			*reg2,
			*reg3,
			*new_var_expr = NULL,
			*_output = NULL,
			*_complete_var = NULL,
			*var_expr,
			*_var_ref;

	MAKE_STD_ZVAL(*returnZval);

	var_expr = estrdup(Z_STRVAL_P(match1));

	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_dvar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_dvar_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),0 TSRMLS_CC);
	_db_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_db_qstr_regexp"),0 TSRMLS_CC);
	_num_const_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),0 TSRMLS_CC);
	_si_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_si_qstr_regexp"),0 TSRMLS_CC);
	_cvar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_cvar_regexp"),0 TSRMLS_CC);
	_svar_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_svar_regexp"),0 TSRMLS_CC);
	_permitted_tokens = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_permitted_tokens"),0 TSRMLS_CC);
	_dvar_math_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_dvar_math_regexp"),0 TSRMLS_CC);
	_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_qstr_regexp"),0 TSRMLS_CC);
	_obj_params_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_params_regexp"),0 TSRMLS_CC);
	_var_bracket_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_bracket_regexp"),0 TSRMLS_CC);

	//生成提取式
	strcat2(&reg1,"~(",Z_STRVAL_P(_dvar_math_regexp),"|",Z_STRVAL_P(_qstr_regexp),")~",NULL);
	strcat2(&reg2,"~^",Z_STRVAL_P(_dvar_math_regexp),"$~",NULL);
	strcat2(&reg3,"~(?:^\\w+)|",Z_STRVAL_P(_obj_params_regexp),"|(?:",Z_STRVAL_P(_var_bracket_regexp),")|->\\$?\\w+|\\.\\$?\\w+|\\S+~",NULL);

	preg_split(reg1,Z_STRVAL_P(match1),-1,2,&_math_vars TSRMLS_CC);
	efree(reg1);

	n = zend_hash_num_elements(Z_ARRVAL_P(_math_vars));
	if(n > 1){


		zend_hash_internal_pointer_reset(Z_ARRVAL_P(_math_vars));
		for(i = 0 ; i < n; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(_math_vars),(void**)&_math_var);

			if(IS_NULL != Z_TYPE_PP(_math_var) || IS_LONG == Z_TYPE_PP(_math_var)){
				
				//提取数学操作符
				zval	*tempMatch;
				char	*getString;
				if(IS_STRING == Z_TYPE_PP(_math_var)){
					getString = estrdup(Z_STRVAL_PP(_math_var));
				}else if(IS_LONG == Z_TYPE_PP(_math_var)){
					char tempInt[12];
					sprintf(tempInt,"%d",Z_LVAL_PP(_math_var));
					getString = estrdup(tempInt);
				}

				if(preg_match(reg2,getString,&tempMatch)) {
					has_math = 1;
					if(_complete_var != NULL || isdigitstr(_complete_var)){
						zval	*callReturn,
								*sendParams;
						char	*tempChar = NULL;
						MAKE_STD_ZVAL(sendParams);
						ZVAL_STRING(sendParams,_complete_var,1);
						CQuickTemplate__parse_var(object,sendParams,&callReturn TSRMLS_CC);
						if(_output != NULL){
							tempChar = estrdup(_output);
							efree(_output);
						}
						if(tempChar != NULL){
							strcat2(&_output,tempChar,Z_STRVAL_P(callReturn),NULL);
						}else{
							strcat2(&_output,Z_STRVAL_P(callReturn),NULL);
						}

						if(tempChar != NULL) efree(tempChar);
						zval_ptr_dtor(&sendParams);
						zval_ptr_dtor(&callReturn);
					}

					if(1){
						char	*tempChar = NULL;
						if(_output != NULL){
							tempChar = estrdup(_output);
							efree(_output);
						}
						if(tempChar == NULL){
							strcat2(&_output,Z_STRVAL_PP(_math_var),NULL);
						}else{
							strcat2(&_output,tempChar,Z_STRVAL_PP(_math_var),NULL);
						}
						if(tempChar != NULL) efree(tempChar);
					}


					if(_complete_var != NULL){
						efree(_complete_var);
						_complete_var = NULL;
					}
				}else{
					char *tempChar = NULL;
					if(_complete_var != NULL){
						tempChar = estrdup(_complete_var);
						efree(_complete_var);
					}
					
					if(tempChar != NULL) {
						strcat2(&_complete_var,tempChar,getString,NULL);
						efree(tempChar);
					}else{
						_complete_var = estrdup(getString);
					}
				}


				efree(getString);
				zval_ptr_dtor(&tempMatch);
			}

			zend_hash_move_forward(Z_ARRVAL_P(_math_vars));
		}

		if(has_math){
			if(_complete_var != NULL || isdigitstr(_complete_var)){
				zval	*callReturn,
						*sendParams;
				char	*tempChar = NULL;
				MAKE_STD_ZVAL(sendParams);
				ZVAL_STRING(sendParams,_complete_var,1);
				CQuickTemplate__parse_var(object,sendParams,&callReturn TSRMLS_CC);
				if(_output != NULL){
					tempChar = estrdup(_output);
					efree(_output);
				}
				strcat2(&_output,tempChar,Z_STRVAL_P(callReturn),NULL);
				if(tempChar != NULL) efree(tempChar);
				zval_ptr_dtor(&sendParams);
				zval_ptr_dtor(&callReturn);
			}
		
			efree(var_expr);
			var_expr = estrdup(_complete_var);
		}

	}

	//substr
	MODULE_BEGIN
		char *fristWord;
		substr(var_expr,0,1,&fristWord);
		if(isdigitstr(fristWord)){
			_var_ref = estrdup(var_expr);
		}else{
			char *tempString;
			substr1(var_expr,1,&tempString);
			_var_ref = estrdup(tempString);
			efree(tempString);
		}
		efree(fristWord);
	MODULE_END


	if(!has_math){
		zval	*matched,
				**_indexes = NULL,
				*indexCopy,
				**_var_name;
		int		needError = 0,
				destroyIndex = 0;
		char	*_var_name_string,
				*var_expr_substring;
		if(preg_match_all(reg3,_var_ref,&matched) && IS_ARRAY == Z_TYPE_P(matched) && SUCCESS == zend_hash_index_find(Z_ARRVAL_P(matched),0,(void**)&_indexes)){
		}else{
			//抛出错误
			zval_ptr_dtor(&matched);
			zval_ptr_dtor(&_math_vars);
			efree(var_expr);
			if(_complete_var != NULL){
				efree(_complete_var);
			}
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] compile error , can not match parse_attr");
			return;
		}
	
		//取第一个
		zend_hash_index_find(Z_ARRVAL_PP(_indexes),0,(void**)&_var_name);
		_var_name_string = estrdup(Z_STRVAL_PP(_var_name));
		zend_hash_index_del(Z_ARRVAL_PP(_indexes),0);
		this_hash_rehash(*_indexes);
		substr(Z_STRVAL_P(match1),0,1,&var_expr_substring);

		if(strcmp(_var_name_string,"smarty") == 0){

			//尝试调用
			char	*getString;
			CQuickTemplate__compile_smarty_ref(object,*_indexes,&getString TSRMLS_CC);
			if(strlen(getString) > 0){
				if(_output != NULL){
					efree(_output);
				}
				_output = estrdup(getString);
			}else{
				zval	**_var_name_1;
				char	*substring;
				zend_hash_index_find(Z_ARRVAL_PP(_indexes),1,(void**)&_var_name_1);
				substr1(Z_STRVAL_PP(_var_name_1),1,&substring);
				if(_output != NULL){
					efree(_output);
				}
				strcat2(&_output,"$this->_smarty_vars['",substring,"']",NULL);
				efree(substring);
				zend_hash_index_del(Z_ARRVAL_PP(_indexes),1);
				this_hash_rehash(*_indexes);
			}
			efree(getString);

		}else if(isdigitstr(_var_name_string) &&  isdigitstr(var_expr_substring)){
			if(zend_hash_num_elements(Z_ARRVAL_PP(_indexes)) > 0){
				zval	*indexImpode;
				char	*tempString;
				php_implode("", *_indexes,&indexImpode);
				destroyIndex = 1;
				tempString = estrdup(_var_name_string);
				efree(_var_name_string);
				strcat2(&_var_name_string,tempString,Z_STRVAL_P(indexImpode),NULL);
				efree(tempString);
				zval_ptr_dtor(&indexImpode);
			}
			if(_output != NULL){
				efree(_output);
			}
			_output = estrdup(_var_name_string);

		}else{
			if(_output != NULL){
				efree(_output);
			}
			strcat2(&_output,"$this->_tpl_vars['",_var_name_string,"']",NULL);
		}

		if(destroyIndex == 0){
			zval	**_index;
			char	*substr01,
					*substr02,
					*substr22,
					*substr21,
					*substr11,
					*substr20,
					*substr101;
			n = zend_hash_num_elements(Z_ARRVAL_PP(_indexes));
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(_indexes));
			for(i = 0 ; i < n; i++){
				zend_hash_get_current_data(Z_ARRVAL_PP(_indexes),(void**)&_index);
				
				substr(Z_STRVAL_PP(_index),0,1,&substr01);
				substr(Z_STRVAL_PP(_index),0,2,&substr02);
				substr(Z_STRVAL_PP(_index),1,-1,&substr101);
				substr(Z_STRVAL_PP(_index),1,1,&substr11);

				if(strcmp(substr01,"[") == 0){

					char	*substr101_01;
					substr(substr101,0,1,&substr101_01);

					if(isdigitstr(substr101)){
						if(_output != NULL){
							char *tempString;
							tempString = estrdup(_output);
							efree(_output);
							strcat2(&_output,tempString,"[",substr101,"]",NULL);
							efree(tempString);
						}else{
							_output = estrdup(Z_STRVAL_PP(_index));
						}
					}else if(strcmp(substr101_01,"$") == 0){

						//判断是否包含.
						if(strstr(substr101,".") != NULL){
							
							zval	*callReturn,
									*callParams;
							MAKE_STD_ZVAL(callParams);
							ZVAL_STRING(callParams,substr101,1);
							CQuickTemplate__parse_var(object,callParams,&callReturn TSRMLS_CC);
							if(_output != NULL){
								char	*tempString;
								tempString = estrdup(_output);
								efree(_output);
								strcat2(&_output,tempString,"[",Z_STRVAL_P(callReturn),"]",NULL);
								efree(tempString);
							}
							zval_ptr_dtor(&callParams);
							zval_ptr_dtor(&callReturn);
						}else{
							char	*tempString,
									*substr10;
							substr1(substr101,1,&substr10);
							tempString = estrdup(_output);
							efree(_output);
							strcat2(&_output,tempString,"[$this->_tpl_vars['",substr10,"']]",NULL);
							efree(tempString);
							efree(substr10);
						}

					}
					efree(substr101_01);

				}else if(strcmp(substr01,".") == 0){
					char	*substr2,
							*substr1_t;
					substr1(Z_STRVAL_PP(_index),2,&substr2);
					substr1(Z_STRVAL_PP(_index),1,&substr1_t);
					if(strcmp(substr11,"$") == 0){
						char	*tempString;
						tempString = estrdup(_output);
						efree(_output);
						strcat2(&_output,tempString,"[$this->_tpl_vars['",substr2,"']]",NULL);
						efree(tempString);
					}else{
						char	*tempString;
						tempString = estrdup(_output);
						efree(_output);
						strcat2(&_output,tempString,"['",substr1_t,"']",NULL);
						efree(tempString);
					}
					efree(substr2);
					efree(substr1_t);

				}else if(strcmp(substr02,"->") == 0){

					char	*substr22,
							*substr21;
					substr(Z_STRVAL_PP(_index),2,2,&substr22);
					substr(Z_STRVAL_PP(_index),2,1,&substr21);
					if(strcmp(substr22,"__") == 0){
						php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call to internal object members is not allowed");
					}else if(strcmp(substr21,"$") == 0){
						char	*substr3,
								*tempString;
						substr1(Z_STRVAL_PP(_index),3,&substr3);
						tempString = estrdup(_output);
						efree(_output);
						strcat2(&_output,tempString,"->{(($_var=$this->_tpl_vars['",substr3,"']) && substr($_var,0,2)!='__') ? $_var : $this->trigger_error(\"cannot access property $_var\")}",NULL);
						efree(tempString);
						efree(substr3);
					}else{
						char	*tempString;
						tempString = estrdup(_output);
						efree(_output);
						strcat2(&_output,tempString,Z_STRVAL_PP(_index),NULL);
						efree(tempString);
					}
					efree(substr22);
					efree(substr21);

				}else if(strcmp(substr01,"(") == 0){

				}else{
					char	*tempString;
					tempString = estrdup(_output);
					efree(_output);
					strcat2(&_output,tempString,Z_STRVAL_PP(_index),NULL);
					efree(tempString);
				}

				efree(substr01);
				efree(substr101);
				efree(substr11);
				efree(substr02);


				zend_hash_move_forward(Z_ARRVAL_PP(_indexes));
			}
		}

		efree(_var_name_string);
		efree(var_expr_substring);
		zval_ptr_dtor(&matched);

	}

	
	ZVAL_STRING(*returnZval,_output,1);

	//待销毁
	zval_ptr_dtor(&_math_vars);
	efree(var_expr);
	if(_complete_var != NULL){
		efree(_complete_var);
	}
	if(_output != NULL){
		efree(_output);
	}

	efree(reg2);
	efree(reg3);
	efree(_var_ref);
}

void CQuickTemplate_compile_plugin_call(zval *object,char *type,char *name,char **returnString TSRMLS_DC){
	strcat2(returnString,"$this->_",type,"_",name,NULL);
}

void CQuickTemplate__parse_modifiers(zval *object,zval **returnZval,zval *modifier TSRMLS_DC){

	zval	*_qstr_regexp,
			*_match,
			**_modifiers,
			**modifier_arg_strings,
			**_modifier_name_zval,
			**_modifier_args_this,
			**_modifier_args,
			*_childMatch;

	char	*reg1,
			*reg2,
			*modifier_string,
			*substr_modname_01,
			*_modifier_name,
			*modString,
			*newReturnString;

	int		i,n,_map_array = 0;

	if(modifier == NULL){
		return;
	}

	if(IS_NULL == Z_TYPE_P(modifier)){
		return;
	}

	if(IS_STRING == Z_TYPE_P(modifier) && strlen(Z_STRVAL_P(modifier)) == 0){
		return;
	}

	_qstr_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_qstr_regexp"),0 TSRMLS_CC);

	strcat2(&modifier_string,"|",Z_STRVAL_P(modifier),NULL);
	strcat2(&reg1,"~\\|(@?\\w+)((?>:(?:",Z_STRVAL_P(_qstr_regexp),"|[^|]+))*)~",NULL);
	strcat2(&reg2,"~:(",Z_STRVAL_P(_qstr_regexp),"|[^:]+)~",NULL);

	if(!preg_match_all(reg1,modifier_string,&_match)){
		efree(modifier_string);
		efree(reg2);
		efree(reg1);
		zval_ptr_dtor(&_match);
		return;
	}
	efree(reg1);

	zend_hash_index_find(Z_ARRVAL_P(_match),1,(void**)&_modifiers);
	zend_hash_index_find(Z_ARRVAL_P(_match),2,(void**)&modifier_arg_strings);

	n = zend_hash_num_elements(Z_ARRVAL_PP(_modifiers));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(_modifiers));

	for(i = 0 ; i < n; i++){
		zend_hash_get_current_data(Z_ARRVAL_PP(_modifiers),(void**)&_modifier_name_zval);

		if(strcmp(Z_STRVAL_PP(_modifier_name_zval),"smarty") == 0){
			zend_hash_move_forward(Z_ARRVAL_PP(_modifiers));
			continue;
		}

		_modifier_name = estrdup(Z_STRVAL_PP(_modifier_name_zval));

		//参数
		zend_hash_index_find(Z_ARRVAL_PP(modifier_arg_strings),i,(void**)&_modifier_args_this);
		preg_match_all(reg2, Z_STRVAL_PP(_modifier_args_this), &_childMatch);
		zend_hash_index_find(Z_ARRVAL_P(_childMatch),1,(void**)&_modifier_args);

		substr(_modifier_name,0,1,&substr_modname_01);
		if(strcmp(substr_modname_01,"@") == 0){
			char	*newName;
			_map_array = 0;
			substr1(_modifier_name,1,&newName);
			efree(_modifier_name);
			_modifier_name = estrdup(newName);
			efree(newName);
		}else{
			_map_array = 1;
		}

		//_parse_vars_props
		MODULE_BEGIN
			zval	**values,
					*thisParseValue;
			char	*otherKey;
			ulong	otheriKey;
			int		j,k;
			k = zend_hash_num_elements(Z_ARRVAL_PP(_modifier_args));
			zend_hash_internal_pointer_reset(Z_ARRVAL_PP(_modifier_args));
			for(j=0 ; j < k ; j++){
				zend_hash_get_current_data(Z_ARRVAL_PP(_modifier_args),(void**)&values);
				CQuickTemplate__parse_var_props(object,*values,&thisParseValue TSRMLS_CC);
				if(HASH_KEY_IS_LONG == zend_hash_get_current_key_type(Z_ARRVAL_PP(_modifier_args))){
					ulong	thisIntKey;
					zend_hash_get_current_key(Z_ARRVAL_PP(_modifier_args), &otherKey, &thisIntKey, 0);
					add_index_zval(*_modifier_args,thisIntKey,thisParseValue);
				}else{
					zend_hash_get_current_key(Z_ARRVAL_PP(_modifier_args),&otherKey,&otheriKey,0);
					add_assoc_zval(*_modifier_args,otherKey,thisParseValue);
				}
				zend_hash_move_forward(Z_ARRVAL_PP(_modifier_args));
			}
		MODULE_END

		if(strcmp(_modifier_name,"default") == 0){
			char	*output01;
			zval	**_modifier_args_0;
			substr(Z_STRVAL_PP(returnZval),0,1,&output01);
			if(strcmp(output01,"$") == 0){
				char *newOutput;
				strcat2(&newOutput,"@",Z_STRVAL_PP(returnZval),NULL);
				zval_ptr_dtor(returnZval);
				MAKE_STD_ZVAL(*returnZval);
				ZVAL_STRING(*returnZval,newOutput,1);
				efree(newOutput);
			}
			efree(output01);

			if(zend_hash_index_find(Z_ARRVAL_PP(_modifier_args),0,(void**)&_modifier_args_0) == SUCCESS && IS_STRING == Z_TYPE_PP(_modifier_args_0)){
				char *substrArgs01;
				substr(Z_STRVAL_PP(_modifier_args_0),0,1,&substrArgs01);
				if(strcmp(substrArgs01,"$") == 0){
					char *newString;
					strcat2(&newString,"@",Z_STRVAL_PP(_modifier_args_0),NULL);
					add_index_string(*_modifier_args,0,newString,1);
					efree(newString);
				}
				efree(substrArgs01);
			}	
		}

		//判断 _modifier_args 大小
		if(zend_hash_num_elements(Z_ARRVAL_PP(_modifier_args)) > 0 ){
			//modString
			zval *tempImplode;
			php_implode(", ",*_modifier_args,&tempImplode);
			strcat2(&modString,", ",Z_STRVAL_P(tempImplode),NULL);
			zval_ptr_dtor(&tempImplode);
		}else{
			modString = estrdup("");
		}

		if(_map_array){
			char	*returnPlugin;
			CQuickTemplate_compile_plugin_call(object,"modifier",_modifier_name,&returnPlugin TSRMLS_CC);
			strcat2(&newReturnString,"((is_array($_tmp=",Z_STRVAL_PP(returnZval),")) ? $this->_run_mod_handler('",_modifier_name,"', true, $_tmp",modString,") : ",returnPlugin,"($_tmp",modString,"))",NULL);
			efree(returnPlugin);
		}else{
			char	*returnPlugin;
			CQuickTemplate_compile_plugin_call(object,"modifier",_modifier_name,&returnPlugin TSRMLS_CC);
			strcat2(&newReturnString,returnPlugin,"(",Z_STRVAL_PP(returnZval),modString,")",NULL);
			efree(returnPlugin);
		}

		zval_ptr_dtor(returnZval);
		MAKE_STD_ZVAL(*returnZval);
		ZVAL_STRING(*returnZval,newReturnString,1);
		zval_ptr_dtor(&_childMatch);
		efree(substr_modname_01);
		efree(_modifier_name);
		efree(modString);
		efree(newReturnString);
		zend_hash_move_forward(Z_ARRVAL_PP(_modifiers));
	}


	efree(modifier_string);
	zval_ptr_dtor(&_match);
	efree(reg2);
}

void QuickTemplate__parse_include_tags(zval *object,char *source_contents,char *ldq,char *rdq,char **returnContent TSRMLS_DC){
	
	char	*matchReg,
			*otherKey,
			*tempTrueTags,
			*modReg,
			*source_content;

	zval	*match,
			**includeTags,
			**template_tags,
			*_num_const_regexp,
			*_obj_call_regexp,
			*_var_regexp,
			*_reg_obj_regexp,
			*_func_regexp,
			*_mod_regexp,
			*matchModer,
			**tag_command,
			**tag_modifier = NULL,
			**tag_args = NULL,
			**orgTags,
			**match0,
			*attr;

	int		i,n,j;
	ulong	otheriKey;

	source_content = estrdup(source_contents);

	//获取修饰符提取正则
	_num_const_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),0 TSRMLS_CC);
	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_var_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),0 TSRMLS_CC);
	_reg_obj_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_reg_obj_regexp"),0 TSRMLS_CC);
	_func_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_func_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),0 TSRMLS_CC);

	strcat2(&modReg,"~^(?:(",Z_STRVAL_P(_num_const_regexp),"|",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_var_regexp), "|\\/?",Z_STRVAL_P(_reg_obj_regexp),"|\\/?",Z_STRVAL_P(_func_regexp),")(",Z_STRVAL_P(_mod_regexp),"*))(?:\\s+(.*))?$~xs",NULL);

	//提取include
	strcat2(&matchReg,"~",ldq,"\\s*include(.*?)\\s*",rdq,"~s",NULL);
	if(preg_match_all(matchReg,source_content,&match)){

		if(IS_ARRAY != Z_TYPE_P(match)){
			zval_ptr_dtor(&match);
			efree(source_content);
			efree(matchReg);
			return;
		}
		zend_hash_index_find(Z_ARRVAL_P(match),1,(void**)&includeTags);
		if(IS_ARRAY != Z_TYPE_PP(includeTags)){
			zval_ptr_dtor(&match);
			efree(source_content);
			efree(matchReg);
			return;
		}

		zend_hash_index_find(Z_ARRVAL_P(match),0,(void**)&match0);

		//遍历此数据
		n = zend_hash_num_elements(Z_ARRVAL_PP(includeTags));
		zend_hash_internal_pointer_reset(Z_ARRVAL_PP(includeTags));
		for(i = 0 ; i < n; i++){

			zend_hash_get_current_key(Z_ARRVAL_PP(includeTags),&otherKey,&otheriKey,0);
			zend_hash_get_current_data(Z_ARRVAL_PP(includeTags),(void**)&template_tags);

			//提取修饰符
			strcat2(&tempTrueTags,"include ",Z_STRVAL_PP(template_tags),NULL);
			
			
			if(!preg_match(modReg,tempTrueTags,&matchModer)){
				char errorMessage[1024];
				sprintf(errorMessage,"%s%s","[CViewException] Compiler error : unrecognized tag:",tempTrueTags);
				php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
				zval_ptr_dtor(&match);
				zval_ptr_dtor(&matchModer);
				efree(source_content);
				efree(modReg);
				efree(matchReg);
				efree(tempTrueTags);
				return;
			}
			efree(tempTrueTags);

			//tag_command
			zend_hash_index_find(Z_ARRVAL_P(matchModer),1,(void**)&tag_command);

			//tag_modifier
			if(zend_hash_num_elements(Z_ARRVAL_P(matchModer)) >= 3){
				zend_hash_index_find(Z_ARRVAL_P(matchModer),2,(void**)&tag_modifier);
			}

			//tag_args
			if(zend_hash_num_elements(Z_ARRVAL_P(matchModer)) >= 4){
				zend_hash_index_find(Z_ARRVAL_P(matchModer),3,(void**)&tag_args);
			}

			//原始的模板标签
			zend_hash_index_find(Z_ARRVAL_PP(match0),otheriKey,(void**)&orgTags);

			//解析参数
			CQuickTemplate__parse_attrs(object,*tag_args,&attr TSRMLS_CC);

			//子模板的名称
			MODULE_BEGIN
				zval	**fileName,
						*replaceFileName,
						*findFileName,
						**templateName,
						*_tpl_vars;

				char	*fileTempName1;
				if(SUCCESS != zend_hash_find(Z_ARRVAL_P(attr),"file",strlen("file")+1,(void**)&fileName)){
					*returnContent = estrdup(source_content);
					zval_ptr_dtor(&match);
					zval_ptr_dtor(&matchModer);
					zval_ptr_dtor(&attr);
					efree(matchReg);
					efree(modReg);
					return;
				}

				MAKE_STD_ZVAL(replaceFileName);
				MAKE_STD_ZVAL(findFileName);
				array_init(replaceFileName);
				array_init(findFileName);

				add_next_index_string(findFileName,"($this->_tpl_vars['",1);
				add_next_index_string(findFileName,"'])",1);
				add_next_index_string(replaceFileName,"",1);
				add_next_index_string(replaceFileName,"",1);

				str_replaceArray(findFileName,replaceFileName,Z_STRVAL_PP(fileName),&fileTempName1);

				zval_ptr_dtor(&replaceFileName);
				zval_ptr_dtor(&findFileName);

				//模板名称
				_tpl_vars = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tpl_vars"),0 TSRMLS_CC);


				if(SUCCESS == zend_hash_find(Z_ARRVAL_P(_tpl_vars),fileTempName1,strlen(fileTempName1)+1,(void**)&templateName) && IS_STRING == Z_TYPE_PP(templateName)){
					
					//获取模板真实路径
					char	*truePath;
					CQuickTemplate_getTemplateTruePath(object,Z_STRVAL_PP(templateName),&truePath TSRMLS_CC);

					if(SUCCESS == fileExist(truePath)){

						char	*newFileContent,
								*replaceEndContent;

						file_get_contents(truePath,&newFileContent);
						str_replace(Z_STRVAL_PP(orgTags),newFileContent,source_content,&replaceEndContent);

						efree(source_content);
						source_content = estrdup(replaceEndContent);
						efree(replaceEndContent);
						efree(newFileContent);
					}else{
						char errorMessage[1024];
						sprintf(errorMessage,"%s%s","[CViewException] Compiler Error : Template not exists :",fileTempName1);
						efree(truePath);
						zval_ptr_dtor(&replaceFileName);
						zval_ptr_dtor(&findFileName);
						efree(fileTempName1);
						zval_ptr_dtor(&matchModer);
						zval_ptr_dtor(&attr);
						zval_ptr_dtor(&match);
						efree(source_content);
						efree(matchReg);
						efree(modReg);
						php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
						return;
					}
					

					efree(truePath);

				}else{
					char errorMessage[1024];
					sprintf(errorMessage,"%s%s","[CViewException] Compiler Error : Template not exists :",fileTempName1);
					zval_ptr_dtor(&replaceFileName);
					zval_ptr_dtor(&findFileName);
					efree(fileTempName1);
					zval_ptr_dtor(&matchModer);
					zval_ptr_dtor(&attr);
					zval_ptr_dtor(&match);
					efree(source_content);
					efree(matchReg);
					efree(modReg);
					php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
					return;
				}

				efree(fileTempName1);

				

			MODULE_END

			zval_ptr_dtor(&matchModer);
			zval_ptr_dtor(&attr);
			zend_hash_move_forward(Z_ARRVAL_PP(includeTags));
		}
	
	}




	*returnContent = estrdup(source_content);

	//销毁数据
	zval_ptr_dtor(&match);
	efree(source_content);
	efree(matchReg);
	efree(modReg);
}

void CQuickTemplate__pop_tag(zval *object,char *tagName,zval **returnString TSRMLS_DC){

	zval	*_tag_stack;
	int		n = 0;
	char	*openTags = NULL,
			*message;

	MAKE_STD_ZVAL(*returnString);

	_tag_stack = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),0 TSRMLS_CC);

	n = zend_hash_num_elements(Z_ARRVAL_P(_tag_stack));

	if(n > 0){
		//取最后一个stack里的tags
		zval	**lastTags = NULL,
				**lastTagName = NULL;
		ulong	thisIntKey;
		char	*otherKey;

		zend_hash_internal_pointer_end(Z_ARRVAL_P(_tag_stack));
		zend_hash_get_current_data(Z_ARRVAL_P(_tag_stack), (void **)&lastTags);
		zend_hash_get_current_key(Z_ARRVAL_P(_tag_stack), &otherKey, &thisIntKey, 0);

		if(lastTags != NULL && IS_ARRAY == Z_TYPE_PP(lastTags)){
			zend_hash_index_find(Z_ARRVAL_PP(lastTags),0,(void**)&lastTagName);
			if(strcmp(tagName,Z_STRVAL_PP(lastTagName)) == 0){
				ZVAL_STRING(*returnString,Z_STRVAL_PP(lastTagName),1);
				zend_hash_index_del(Z_ARRVAL_P(_tag_stack),thisIntKey);
				this_hash_rehash((_tag_stack));
				zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
				return;
			}
		}else{
			char	errorMessage[10240];
			zend_hash_index_del(Z_ARRVAL_P(_tag_stack),thisIntKey);
			this_hash_rehash((_tag_stack));
			zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
			ZVAL_STRING(*returnString,tagName,1);
			sprintf(errorMessage,"%s%s%s%s","[CViewException] compile error : can not parse _tag_stack :",tagName);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			return;
		}
	
		if(strcmp(tagName,"if") == 0 && (strcmp(Z_STRVAL_PP(lastTagName),"else") == 0 ||  strcmp(Z_STRVAL_PP(lastTagName),"elseif") == 0 ) ){
			//递归
			zval *returnZval;
			zend_hash_index_del(Z_ARRVAL_P(_tag_stack),thisIntKey);
			this_hash_rehash((_tag_stack));
			zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
			CQuickTemplate__pop_tag(object,tagName,&returnZval TSRMLS_CC);
			ZVAL_STRING(*returnString,Z_STRVAL_P(returnZval),1);
			zval_ptr_dtor(&returnZval);
			return;
		}

		if(strcmp(tagName,"foreach") == 0 && strcmp(Z_STRVAL_PP(lastTagName),"foreachelse") == 0  ){
			//递归
			zval *returnZval;
			zend_hash_index_del(Z_ARRVAL_P(_tag_stack),thisIntKey);
			this_hash_rehash((_tag_stack));
			zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
			CQuickTemplate__pop_tag(object,tagName,&returnZval TSRMLS_CC);
			ZVAL_STRING(*returnString,Z_STRVAL_PP(lastTagName),1);
			zval_ptr_dtor(&returnZval);
			return;
		}

		if(strcmp(Z_STRVAL_PP(lastTagName),"else") == 0 || strcmp(Z_STRVAL_PP(lastTagName),"elseif") == 0){
			
			if(openTags != NULL){
				efree(openTags);
				openTags = NULL;
			}
			openTags = estrdup("if");

		}else if( strcmp(Z_STRVAL_PP(lastTagName),"sectionelse") == 0){
			
			if(openTags != NULL){
				efree(openTags);
				openTags = NULL;
			}
			openTags = estrdup("section");

		}else if(strcmp(Z_STRVAL_PP(lastTagName),"foreachelse") == 0){
	
			if(openTags != NULL){
				efree(openTags);
				openTags = NULL;
			}
			openTags = estrdup("foreach");
		}else{

			if(openTags != NULL){
				efree(openTags);
				openTags = NULL;
			}
			openTags = estrdup(Z_STRVAL_PP(lastTagName));
		}
		
		strcat2(message,"expected {",openTags,"}",NULL);
		zend_hash_index_del(Z_ARRVAL_P(_tag_stack),thisIntKey);
		this_hash_rehash((_tag_stack));
		zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
	}

	MODULE_BEGIN
		char	errorMessage[10240];
		sprintf(errorMessage,"%s%s%s%s","[CViewException] mismatched tag {",tagName,"}",message);
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
	MODULE_END

	if(openTags != NULL){
		efree(openTags);
		openTags = NULL;
	}
	if(message != NULL){
		efree(message);
	}
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
}

void CQuickTemplate__push_tag(zval *object,char *tagName TSRMLS_DC){

	zval	*_current_line_no,
			*thisArray,
			*_tag_stack,
			*copyLine;

	_current_line_no = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_current_line_no"),0 TSRMLS_CC);
	_tag_stack = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(thisArray);
	array_init(thisArray);
	MAKE_STD_ZVAL(copyLine);
	ZVAL_ZVAL(copyLine,_current_line_no,1,0);

	add_next_index_string(thisArray,tagName,1);
	add_next_index_zval(thisArray,copyLine);

	//插入标签堆栈
	add_next_index_zval(_tag_stack,thisArray);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),_tag_stack TSRMLS_CC);
}

void CQuickTemplate__compile_if_tag(zval *object,zval *tag_args,int iseslseif,char **ifSciprString TSRMLS_DC){


	zval	*_obj_call_regexp,
			*_mod_regexp,
			*_var_regexp,
			*match,
			**tokens,
			**token,
			*token_count,
			*is_arg_stack,
			*implodeString;

	char	*reg1,
			*reg2,
			token_params[20240],
			*tokenLower;

	int		i,n;


	if(IS_STRING != Z_TYPE_P(tag_args)){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] Compiler Error : tag {if} must have a params");
		return;
	}

	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),0 TSRMLS_CC);
	_var_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),0 TSRMLS_CC);
	
	strcat2(&reg1,"~(?> ",Z_STRVAL_P(_obj_call_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)? | ",Z_STRVAL_P(_var_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)?    | \\-?0[xX][0-9a-fA-F]+|\\-?\\d+(?:\\.\\d+)?|\\.\\d+|!==|===|==|!=|<>|<<|>>|<=|>=|\\&\\&|\\|\\||\\(|\\)|,|\\!|\\^|=|\\&|\\~|<|>|\\||\\%|\\+|\\-|\\/|\\*|\\@    | \\b\\w+\\b | \\S+ )~x",NULL);
	strcat2(&reg2,"~^",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_var_regexp),"(?:",Z_STRVAL_P(_mod_regexp),"*)$~",NULL);

	if(!preg_match_all(reg1,Z_STRVAL_P(tag_args),&match)){
		char	errorMessage[10240];
		sprintf(errorMessage,"%s%s","[CViewException] Compiler Error : can not parse tags :",Z_STRVAL_P(tag_args));
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		efree(reg1);
		efree(reg2);
		zval_ptr_dtor(&match);
		return;
	}

	if(SUCCESS != zend_hash_index_find(Z_ARRVAL_P(match),0,(void**)&tokens)){
		char	errorMessage[10240];
		sprintf(errorMessage,"%s%s","[CViewException] Compiler Error : can not parse if or elseif :",Z_STRVAL_P(tag_args));
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		efree(reg1);
		efree(reg2);
		zval_ptr_dtor(&match);
		return;
	}

	array_count_values(*tokens,&token_count);

	//如果存在isset($token_count['(']) && $token_count['('] != $token_count[')']
	if(zend_hash_exists(Z_ARRVAL_P(token_count),"(",strlen(")")+1)){
		zval	**leftKuo = NULL,
				**rightKuo = NULL;
		int		needError = 0;
		zend_hash_find(Z_ARRVAL_P(token_count),"(",strlen("(")+1,(void**)&leftKuo);
		zend_hash_find(Z_ARRVAL_P(token_count),")",strlen(")")+1,(void**)&rightKuo);

		if(leftKuo == NULL && rightKuo == NULL){
			needError = 1;
		}

		if(IS_STRING == Z_TYPE_PP(leftKuo) && IS_STRING == Z_TYPE_PP(rightKuo) && strcmp(Z_STRVAL_PP(leftKuo),Z_STRVAL_PP(rightKuo)) == 0 ){
			needError = 1;
		}

		if(needError){
			char	errorMessage[1024];
			sprintf(errorMessage,"%s","[CViewException] unbalanced parenthesis in if statement");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			zval_ptr_dtor(&token_count);
			efree(reg1);
			efree(reg2);
			zval_ptr_dtor(&match);
			return;
		}

	}

	MAKE_STD_ZVAL(is_arg_stack);
	array_init(is_arg_stack);

	//遍历toknes
	n = zend_hash_num_elements(Z_ARRVAL_PP(tokens));
	zend_hash_internal_pointer_reset(Z_ARRVAL_PP(tokens));
	for(i = 0 ; i < n; i++){

		zend_hash_get_current_data(Z_ARRVAL_PP(tokens),(void**)&token);

		//转为小写
		tokenLower = estrdup(Z_STRVAL_PP(token));
		php_strtolower(tokenLower,strlen(tokenLower)+1);

		sprintf(token_params,"%s",Z_STRVAL_PP(token));

		//将特定符号转为常规符号
		if(strcmp(tokenLower,"eq") == 0){
			sprintf(token_params,"%s","==");
		}else if(strcmp(tokenLower,"neq") == 0){
			sprintf(token_params,"%s","!=");
		}else if(strcmp(tokenLower,"lt") == 0){
			sprintf(token_params,"%s","<");
		}else if(strcmp(tokenLower,"le") == 0){
			sprintf(token_params,"%s","<=");
		}else if(strcmp(tokenLower,"lte") == 0){
			sprintf(token_params,"%s","<=");
		}else if(strcmp(tokenLower,"gt") == 0){
			sprintf(token_params,"%s",">");
		}else if(strcmp(tokenLower,"gte") == 0){
			sprintf(token_params,"%s",">=");
		}else if(strcmp(tokenLower,"and") == 0){
			sprintf(token_params,"%s","&&");
		}else if(strcmp(tokenLower,"or") == 0){
			sprintf(token_params,"%s","||");
		}else if(strcmp(tokenLower,"not") == 0){
			sprintf(token_params,"%s","!");
		}else if(strcmp(tokenLower,"mod") == 0){
			sprintf(token_params,"%s","%");
		}else if(strcmp(tokenLower,"(") == 0){
		
			//插入is_arg_stack
			add_next_index_long(is_arg_stack,i);

		}else if(strcmp(tokenLower,"is") == 0){

			zval_ptr_dtor(&token_count);
			zval_ptr_dtor(&is_arg_stack);
			efree(tokenLower);
			efree(reg1);
			efree(reg2);
			zval_ptr_dtor(&match);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] please use tags[=] to replace tags[is] ");
			return;

		}else{
			zval	*tempReturn;
			if(preg_match(reg2,Z_STRVAL_PP(token),&tempReturn)){
				zval	*returnZval;
				CQuickTemplate__parse_var_props(object,*token,&returnZval TSRMLS_CC);
		
				if(IS_STRING != Z_TYPE_P(returnZval)){
					php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] cant not parse tag {if} %s",Z_STRVAL_PP(token));
					return;
				}
				sprintf(token_params,"%s",Z_STRVAL_P(returnZval));
				zval_ptr_dtor(&returnZval);
			}
			zval_ptr_dtor(&tempReturn);
		}

		//重新给数组复制
		add_index_string(*tokens,i,token_params,1);
		
		//需销毁
		efree(tokenLower);

		zend_hash_move_forward(Z_ARRVAL_PP(tokens));

	}


	php_implode(" ",*tokens,&implodeString);
	if(iseslseif){
		strcat2(ifSciprString,"<?php elseif (",Z_STRVAL_P(implodeString),"): ?>",NULL);
	}else{
		strcat2(ifSciprString,"<?php if (",Z_STRVAL_P(implodeString),"): ?>",NULL);
	}


	//待销毁变量
	zval_ptr_dtor(&token_count);
	zval_ptr_dtor(&is_arg_stack);
	zval_ptr_dtor(&implodeString);
	efree(reg1);
	efree(reg2);
	zval_ptr_dtor(&match);
}

void CQuickTemplate__dequote(zval *object,char *string,char **itemString TSRMLS_DC){

	char	*substr01,
			*substr_1;

	substr(string,0,1,&substr01);
	substr1(string,-1,&substr_1);

	if((strcmp(substr01,"'") == 0 || strcmp(substr01,"\"") == 0) && strcmp(substr_1,substr01) == 0 ){

		char	*substr1_1;
		substr(string,1,-1,&substr1_1);
		*itemString = estrdup(substr1_1);
		efree(substr1_1);
	}else{
		*itemString = estrdup(string);
	}

	efree(substr01);
	efree(substr_1);
}

void CQuickTemplate__compile_foreach_start(zval *object,zval *tag_args,char **sciprString TSRMLS_DC){
	
	zval	*attr,
			*arg_list,
			**from = NULL,
			**item = NULL,
			**key = NULL,
			**nameZval = NULL,
			*checkMatch;

	char	*itemString,
			*keyString,
			*nameString,
			*reg1;

	smart_str output = {0};


	CQuickTemplate__parse_attrs(object,tag_args,&attr TSRMLS_CC);


	//无from
	if(zend_hash_find(Z_ARRVAL_P(attr),"from",strlen("from")+1,(void**)&from) == SUCCESS){
	}else{
		zval_ptr_dtor(&attr);
		*sciprString = estrdup("");
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] foreach: missing 'from' attribute");
		return;
	}

	//无item
	if(zend_hash_find(Z_ARRVAL_P(attr),"item",strlen("item")+1,(void**)&item) == SUCCESS){
	}else{
		zval_ptr_dtor(&attr);
		*sciprString = estrdup("");
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] foreach: missing 'item' attribute");
		return;
	}

	//处理item
	CQuickTemplate__dequote(object,Z_STRVAL_PP(item),&itemString TSRMLS_CC);

	reg1 = estrdup("~^\\w+$~");
	if(!preg_match(reg1,itemString,&checkMatch)){
		zval_ptr_dtor(&attr);
		efree(itemString);
		zval_ptr_dtor(&checkMatch);
		*sciprString = estrdup("");
		efree(reg1);
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] foreach: 'item' must be a variable name (literal string)");
		return;
	}
	

	zval_ptr_dtor(&checkMatch);

	//存在key
	if(zend_hash_find(Z_ARRVAL_P(attr),"key",strlen("key")+1,(void**)&key) == SUCCESS){
		char	*keyStringTemp;
		CQuickTemplate__dequote(object,Z_STRVAL_PP(key),&keyStringTemp TSRMLS_CC);
		if(!preg_match(reg1,keyStringTemp,&checkMatch)){
			zval_ptr_dtor(&attr);
			efree(itemString);
			efree(keyStringTemp);
			efree(reg1);
			zval_ptr_dtor(&checkMatch);
			*sciprString = estrdup("");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] foreach: 'item' must be a variable name (literal string)");
			return;
		}
		zval_ptr_dtor(&checkMatch);
		strcat2(&keyString,"$this->_tpl_vars['",keyStringTemp,"'] => ",NULL);
		efree(keyStringTemp);
	}else{
		keyString = estrdup("");
	}
	efree(reg1);

	//判断存在name
	if( zend_hash_find(Z_ARRVAL_P(attr),"name",strlen("name")+1,(void**)&nameZval) == SUCCESS && IS_STRING == Z_TYPE_PP(nameZval) ){
		nameString = estrdup(Z_STRVAL_PP(nameZval));
	}else{
		nameString = estrdup("");
	}

	//开始拼接
	smart_str_appends(&output,"<?php ");
	smart_str_appends(&output,"$_from = ");
	smart_str_appends(&output,Z_STRVAL_PP(from));
	smart_str_appends(&output,"; if (!is_array($_from) && !is_object($_from)) { settype($_from, 'array'); }");
	
	if(strlen(nameString) > 0){
		char	*foreach_props;
		strcat2(&foreach_props,"$this->_foreach[",nameString,"]",NULL);

		smart_str_appends(&output,foreach_props);
		smart_str_appends(&output," = array('total' => count($_from), 'iteration' => 0);\n");
		smart_str_appends(&output,"if (");
		smart_str_appends(&output,foreach_props);
		smart_str_appends(&output,"['total'] > 0):\n");
		smart_str_appends(&output,"    foreach ($_from as ");
		smart_str_appends(&output,keyString);
		smart_str_appends(&output,"$this->_tpl_vars['");
		smart_str_appends(&output,itemString);
		smart_str_appends(&output,"']):\n");
		smart_str_appends(&output,"        ");
		smart_str_appends(&output,foreach_props);
		smart_str_appends(&output,"['iteration']++;\n");
		efree(foreach_props);
	}else{

		smart_str_appends(&output,"if (count($_from)):\n");
		smart_str_appends(&output,"    foreach ($_from as ");
		smart_str_appends(&output,keyString);
		smart_str_appends(&output,"$this->_tpl_vars['");
		smart_str_appends(&output,itemString);
		smart_str_appends(&output,"']):\n");
	}

	smart_str_appends(&output,"?>");
	smart_str_0(&output);

	*sciprString = estrdup(output.c);

	//待销毁
	smart_str_free(&output);
	zval_ptr_dtor(&attr);
	efree(itemString);
	efree(keyString);
	efree(nameString);
}

int CQuickTemplate__compile_compiler_tag(zval *object,char *tag_command,char *tag_modifier,zval *tag_args,char **returnString TSRMLS_DC){

	//识别标签  assign
	if(strcmp(tag_command,"assign") == 0){
		zval	*attr,
				**var,
				**value;
		CQuickTemplate__parse_attrs(object,tag_args,&attr TSRMLS_CC);
		if(	SUCCESS  == zend_hash_find(Z_ARRVAL_P(attr),"var",strlen("var")+1,(void**)&var) && SUCCESS  == zend_hash_find(Z_ARRVAL_P(attr),"value",strlen("value")+1,(void**)&value) && IS_STRING == Z_TYPE_PP(var) && IS_STRING == Z_TYPE_PP(value)){
			smart_str	assignString = {0};
			smart_str_appends(&assignString,"<?php $this->assign(");
			smart_str_appends(&assignString,Z_STRVAL_PP(var));
			smart_str_appends(&assignString,",");
			smart_str_appends(&assignString,Z_STRVAL_PP(value));
			smart_str_appends(&assignString,"); ?>");
			smart_str_0(&assignString);
			*returnString = estrdup(assignString.c);
			smart_str_free(&assignString);
			zval_ptr_dtor(&attr);
			return 1;
		}else{
			zval_ptr_dtor(&attr);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] Template call function assign params error ");
			return 0;
		}
	}


	return 0;
}

void CQuickTemplate__compile_arg_list(zval *object,char *type,char *name,zval *attr,zval **returnZval TSRMLS_DC){

	zval	**_var,
			*newZval;

	int		i,n;

	char	*otherKey;

	ulong	otheriKey;

	MAKE_STD_ZVAL(*returnZval);
	array_init(*returnZval);

	n = zend_hash_num_elements(Z_ARRVAL_P(attr));
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(attr));
	for(i = 0 ; i < n; i++){

		zend_hash_get_current_data(Z_ARRVAL_P(attr),(void**)&_var);
		zend_hash_get_current_key(Z_ARRVAL_P(attr),&otherKey,&otheriKey,0);
		MAKE_STD_ZVAL(newZval);

		if(IS_BOOL == Z_TYPE_PP(_var)){
			if(1 == Z_BVAL_PP(_var)){
				ZVAL_STRING(newZval,"true",1);
			}else{
				ZVAL_STRING(newZval,"false",1);
			}
		}else if(IS_NULL == Z_TYPE_PP(_var)){
			ZVAL_STRING(newZval,"null",1);
		}else if(IS_STRING == Z_TYPE_PP(_var)){
			char *thisString;
			strcat2(&thisString,"'",otherKey,"' => ",Z_STRVAL_PP(_var),NULL);
			ZVAL_STRING(newZval,thisString,1);
			efree(thisString);
		}else{
			char errorMessage[128];
			zval_ptr_dtor(returnZval);
			zval_ptr_dtor(&newZval);
			sprintf(errorMessage,"%s%s","[CViewException] Compile error : call block function params error: ",name);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			return;
		}

		add_next_index_zval(*returnZval,newZval);
		zend_hash_move_forward(Z_ARRVAL_P(attr));
	}

}

void CQuickTemplate__compile_plugin_call(zval *object,char *type,char *tagName,zval *callInfo,char **callInfoString TSRMLS_DC){
	zval	**callClass,
			**callMethod;

	if(IS_ARRAY == Z_TYPE_P(callInfo) && zend_hash_num_elements(Z_ARRVAL_P(callInfo)) == 2){
	
		zend_hash_index_find(Z_ARRVAL_P(callInfo),0,(void**)&callClass);
		zend_hash_index_find(Z_ARRVAL_P(callInfo),1,(void**)&callMethod);

		//根据类型判断
		if(IS_STRING == Z_TYPE_PP(callClass) && IS_STRING == Z_TYPE_PP(callMethod)){
			strcat2(callInfoString,Z_STRVAL_PP(callClass),"::",Z_STRVAL_PP(callMethod),NULL);
		}else{
			char errorMessage[128];
			sprintf(errorMessage,"%s%s","[CViewException] Compile error : register block function params error , the function must be static :",tagName);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			return;
		}



	}else{
		strcat2(callInfoString,"$this->_",type,"_",tagName,NULL);
	}

}

int CQuickTemplate__compile_block_tag(zval *object,char *tag_command,char *tag_modifier,zval *tag_args,char **returnString TSRMLS_DC){

	
	int		start_tag = 0,
			found = 0,
			have_function = 0;

	char	*substr_command_01,
			*tagName,
			*message = NULL,
			*callInfoString;

	zval	*_block,
			**callInfo;

	smart_str	output = {0};

	tagName = estrdup(tag_command);



	//确定是开始还是结束标签
	substr(tag_command,0,1,&substr_command_01);
	if(strcmp(substr_command_01,"/") == 0){
		char	*tagTrueName;
		start_tag = 0;
		substr1(tagName,1,&tagTrueName);
		efree(tagName);
		tagName = estrdup(tagTrueName);
		efree(tagTrueName);
	}else{
		start_tag = 1;
	}
	efree(substr_command_01);

	
	//判断是否存在已经注册的block
	_block = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_block"),0 TSRMLS_CC);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(_block),tagName,strlen(tagName)+1,(void**)&callInfo) && IS_ARRAY == Z_TYPE_PP(callInfo) && zend_hash_num_elements(Z_ARRVAL_PP(callInfo)) == 2 ){
		int canCall = 0;
		found = 1;
		canCall = is_callable(*callInfo);
		if(!canCall){
			if(message != NULL) efree(message);
			strcat2(&message,"[CViewExpcetion] parse block function {",tagName,"} is not implemented",NULL);
            have_function = 0;
		}else{
			have_function = 1;
		}
	}else{
		found = 0;
	}


	//未找到
	if(!found){
		efree(tagName);
		if(message != NULL) efree(message);
		return 0;
	}else if(!have_function){
		efree(tagName);
		if(message != NULL) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,message);
			efree(message);
		}
		return 0;
	}


	if(start_tag){
		CQuickTemplate__push_tag(object,tagName TSRMLS_CC);
	}else{
		zval *returnStringTemp;
		CQuickTemplate__pop_tag(object,tagName,&returnStringTemp TSRMLS_CC);
		zval_ptr_dtor(&returnStringTemp);
	}

	if(start_tag){
		zval	*attr,
				*argsList,
				*argsListString;

		//生成组件调用代码
		CQuickTemplate__compile_plugin_call(object,"block",tagName,*callInfo,&callInfoString TSRMLS_CC);

		CQuickTemplate__parse_attrs(object,tag_args,&attr TSRMLS_CC);
		CQuickTemplate__compile_arg_list(object,"block",tagName,attr,&argsList TSRMLS_CC);
		php_implode(",",argsList,&argsListString);

		smart_str_appends(&output,"<?php $this->_tag_stack[] = array('");
		smart_str_appends(&output,tagName);
		smart_str_appends(&output,"', array(");
		smart_str_appends(&output,Z_STRVAL_P(argsListString));
		smart_str_appends(&output,")); ");
		smart_str_appends(&output,"$_block_repeat=true;");
		smart_str_appends(&output,callInfoString);
		smart_str_appends(&output,"($this->_tag_stack[count($this->_tag_stack)-1][1], null, $this, $_block_repeat);while ($_block_repeat) { ob_start(); ?>");
		smart_str_0(&output);

		zval_ptr_dtor(&attr);
		zval_ptr_dtor(&argsList);
		zval_ptr_dtor(&argsListString);
		efree(callInfoString);
	}else{
	
		char	*_out_tag_text;
		CQuickTemplate__compile_plugin_call(object,"block",tagName,*callInfo,&callInfoString TSRMLS_CC);

		strcat2(&_out_tag_text,callInfoString,"($this->_tag_stack[count($this->_tag_stack)-1][1], $_block_content, $this, $_block_repeat)",NULL);	
		smart_str_appends(&output,"<?php $_block_content = ob_get_contents(); ob_end_clean(); ");
		smart_str_appends(&output,"$_block_repeat=false;echo ");
		smart_str_appends(&output,_out_tag_text);
		smart_str_appends(&output,"; } ");
		smart_str_appends(&output," array_pop($this->_tag_stack); ?>");
		smart_str_0(&output);
		efree(_out_tag_text);
		efree(callInfoString);
	}

	*returnString = estrdup(output.c);

	efree(tagName);
	smart_str_free(&output);

	if(message != NULL) efree(message);

	return 1;
}

int CQuickTemplate__compile_custom_tag(zval *object,char *tag_command,char *tag_modifier,zval *tag_args,char **returnString TSRMLS_DC){

	int		start_tag = 0,
			found = 0,
			have_function = 0;

	char	*substr_command_01,
			*tagName,
			*message = NULL,
			*callInfoString;

	zval	*_block,
			**callInfo;

	smart_str	output = {0},
				output1 = {0};

	tagName = estrdup(tag_command);


	//确定是开始还是结束标签
	substr(tag_command,0,1,&substr_command_01);
	if(strcmp(substr_command_01,"/") == 0){
		char	*tagTrueName;
		start_tag = 0;
		substr1(tagName,1,&tagTrueName);
		efree(tagName);
		tagName = estrdup(tagTrueName);
		efree(tagTrueName);
	}else{
		start_tag = 1;
	}
	efree(substr_command_01);

	
	//判断是否存在已经注册的block
	_block = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_function"),0 TSRMLS_CC);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(_block),tagName,strlen(tagName)+1,(void**)&callInfo) && IS_ARRAY == Z_TYPE_PP(callInfo) && zend_hash_num_elements(Z_ARRVAL_PP(callInfo)) == 2 ){
		int canCall = 0;
		found = 1;
		canCall = is_callable(*callInfo);
		if(!canCall){
			if(message != NULL) efree(message);
			strcat2(&message,"[CViewExpcetion] custom function {",tagName,"} is not implemented",NULL);
            have_function = 0;
		}else{
			have_function = 1;
		}
	}else{
		found = 0;
	}


	//未找到
	if(!found){
		efree(tagName);
		if(message != NULL) efree(message);
		return 0;
	}else if(!have_function){
		efree(tagName);
		if(message != NULL) {
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,message);
			efree(message);
		}
		return 0;
	}


	MODULE_BEGIN

		zval	*attr,
				*argsList,
				*argsListString;

		//生成组件调用代码
		CQuickTemplate__compile_plugin_call(object,"block",tagName,*callInfo,&callInfoString TSRMLS_CC);
		CQuickTemplate__parse_attrs(object,tag_args,&attr TSRMLS_CC);
		CQuickTemplate__compile_arg_list(object,"block",tagName,attr,&argsList TSRMLS_CC);
		php_implode(",",argsList,&argsListString);
		smart_str_appends(&output,callInfoString);
		smart_str_appends(&output,"(array(");
		smart_str_appends(&output,Z_STRVAL_P(argsListString));
		smart_str_appends(&output,"), $this)");
		smart_str_0(&output);
		smart_str_appends(&output1,"<?php echo ");
		smart_str_appends(&output1,output.c);
		smart_str_appends(&output1,";?>\n");
		smart_str_0(&output1);
		*returnString = estrdup(output1.c);
		smart_str_free(&output);
		smart_str_free(&output1);
		zval_ptr_dtor(&attr);
		zval_ptr_dtor(&argsList);
		zval_ptr_dtor(&argsListString);
		efree(callInfoString);
	MODULE_END

	efree(tagName);
	if(message != NULL) efree(message);

	return 1;
}

void CQuickTemplate_compile_tag(zval *object,zval *template_tag,zval **returnTagsInfo TSRMLS_DC){
	

	zval	*match,
			**includeTags,
			**template_tags,
			*_num_const_regexp,
			*_obj_call_regexp,
			*_var_regexp,
			*_reg_obj_regexp,
			*_func_regexp,
			*_mod_regexp,
			**tag_command,
			**tag_modifier = NULL,
			*tag_args,
			**tag_args_try,
			*match1,
			*_open_tag;

	char	*reg1,
			*reg2;

	int		num = 0;

	MAKE_STD_ZVAL(*returnTagsInfo);

	_num_const_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_num_const_regexp"),0 TSRMLS_CC);
	_obj_call_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_obj_call_regexp"),0 TSRMLS_CC);
	_var_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_var_regexp"),0 TSRMLS_CC);
	_reg_obj_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_reg_obj_regexp"),0 TSRMLS_CC);
	_func_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_func_regexp"),0 TSRMLS_CC);
	_mod_regexp = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_mod_regexp"),0 TSRMLS_CC);

	//提取表达式
	strcat2(&reg1,"~^(?:(",Z_STRVAL_P(_num_const_regexp),"|",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_var_regexp),"|\\/?",Z_STRVAL_P(_reg_obj_regexp),"|\\/?",Z_STRVAL_P(_func_regexp),")(",Z_STRVAL_P(_mod_regexp),"*)) (?:\\s+(.*))?$ ~xs",NULL);
	if(!preg_match(reg1,Z_STRVAL_P(template_tag),&match)){
		char errorMessage[1024];
		sprintf(errorMessage,"%s","[CQuickTemplate] Compiler error : unrecognized tag: ",Z_STRVAL_P(template_tag));
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		zval_ptr_dtor(&match);
		efree(reg1);
		ZVAL_NULL(*returnTagsInfo);
		return;
	}
	efree(reg1);

	num = zend_hash_num_elements(Z_ARRVAL_P(match));

	//标签命令
	zend_hash_index_find(Z_ARRVAL_P(match),1,(void**)&tag_command);


	//标签修饰符
	zend_hash_index_find(Z_ARRVAL_P(match),2,(void**)&tag_modifier);

	//标签参数
	if(SUCCESS == zend_hash_index_find(Z_ARRVAL_P(match),3,(void**)&tag_args_try) && IS_STRING == Z_TYPE_PP(tag_args_try)){
		MAKE_STD_ZVAL(tag_args);
		ZVAL_ZVAL(tag_args,*tag_args_try,1,0);
	}else{
		MAKE_STD_ZVAL(tag_args);
		ZVAL_STRING(tag_args,"",1);
	}

	strcat2(&reg2,"~^",Z_STRVAL_P(_num_const_regexp),"|",Z_STRVAL_P(_obj_call_regexp),"|",Z_STRVAL_P(_var_regexp),"$~",NULL);
	if(preg_match(reg2,Z_STRVAL_PP(tag_command),&match1)){
		char	*paramsAppend,
				*returnString;
		zval	*varRetruen,
				*sendZval;

		if(IS_STRING == Z_TYPE_PP(tag_modifier)){
			strcat2(&paramsAppend,Z_STRVAL_PP(tag_command),Z_STRVAL_PP(tag_modifier),NULL);
		}else{
			paramsAppend = estrdup(Z_STRVAL_PP(tag_command));
		}


		MAKE_STD_ZVAL(sendZval);
		ZVAL_STRING(sendZval,paramsAppend,1);
		CQuickTemplate__parse_var_props(object,sendZval,&varRetruen TSRMLS_CC);
		strcat2(&returnString,"<?php echo htmlspecialchars(",Z_STRVAL_P(varRetruen),",ENT_QUOTES); ?>","\n",NULL);
		ZVAL_STRING(*returnTagsInfo,returnString,1);

		efree(paramsAppend);
		zval_ptr_dtor(&sendZval);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&varRetruen);
		efree(returnString);
		efree(reg2);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&tag_args);
		return;
	} 
	efree(reg2);


	if(strcmp(Z_STRVAL_PP(tag_command),"if") == 0){
		char	*ifSciprString;
		CQuickTemplate__push_tag(object,"if" TSRMLS_CC);
		CQuickTemplate__compile_if_tag(object,tag_args,0,&ifSciprString TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,ifSciprString,1);
		efree(ifSciprString);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;

	}else if(strcmp(Z_STRVAL_PP(tag_command),"else") == 0){
		//取出_tag_stack最后一个标签判断是否为if 或者 ifelse
		int		length = 0,
				needError = 0;
		zval	**lastTags = NULL,
				**lastTagName = NULL,
				*_tag_stack;
		_tag_stack = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),0 TSRMLS_CC);
		zend_hash_internal_pointer_end(Z_ARRVAL_P(_tag_stack));
		zend_hash_get_current_data(Z_ARRVAL_P(_tag_stack),(void**)&lastTags);

		if(lastTags == NULL){
			needError = 1;
		}else{
			zend_hash_index_find(Z_ARRVAL_PP(lastTags),0,(void**)&lastTagName);
		}
		if(lastTagName != NULL && strcmp(Z_STRVAL_PP(lastTagName),"if") != 0 && strcmp(Z_STRVAL_PP(lastTagName),"elseif") != 0){
			needError = 1;
		}

		if(lastTagName == NULL || needError){
			//报错
			char errorMessage[1024];
			sprintf(errorMessage,"%s","[CQuickTemplate] Compiler error : unexpected {else}");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			ZVAL_NULL(*returnTagsInfo);
			zval_ptr_dtor(&tag_args);
			return;
		}

		CQuickTemplate__push_tag(object,"else" TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,"<?php else: ?>",1);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;

	}else if(strcmp(Z_STRVAL_PP(tag_command),"elseif") == 0){
		//取出_tag_stack最后一个标签判断是否为if 或者 ifelse
		int		needError = 0;
		zval	**lastTags = NULL,
				*_tag_stack,
				**lastTagName = NULL;
		char	*scriptstring;
		_tag_stack = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),0 TSRMLS_CC);
		zend_hash_internal_pointer_end(Z_ARRVAL_P(_tag_stack));
		zend_hash_get_current_data(Z_ARRVAL_P(_tag_stack),(void**)&lastTags);

		if(lastTags == NULL){
			needError = 1;
		}else{
			zend_hash_index_find(Z_ARRVAL_PP(lastTags),0,(void**)&lastTagName);
		}
		if(lastTagName != NULL && strcmp(Z_STRVAL_PP(lastTagName),"if") != 0 && strcmp(Z_STRVAL_PP(lastTagName),"elseif") != 0){
			needError = 1;
		}

		if(lastTagName == NULL){
			//报错
			char errorMessage[1024];
			ZVAL_NULL(*returnTagsInfo);
			sprintf(errorMessage,"%s","[CQuickTemplate] Compiler error : unexpected {elseif}");
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			zval_ptr_dtor(&tag_args);
			return;
		}

		if(strcmp(Z_STRVAL_PP(lastTagName),"if") == 0){
			CQuickTemplate__push_tag(object,"elseif" TSRMLS_CC);
		}

		//编译elseif标签
		CQuickTemplate__compile_if_tag(object,tag_args,1,&scriptstring TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,scriptstring,1);
		efree(scriptstring);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;

	}else if(strcmp(Z_STRVAL_PP(tag_command),"/if") == 0){

		zval *returnString;
		CQuickTemplate__pop_tag(object,"if",&returnString TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,"<?php endif; ?>",1);
		zval_ptr_dtor(&returnString);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;

	}else if(strcmp(Z_STRVAL_PP(tag_command),"foreach") == 0){

		char	*sciprString;
		CQuickTemplate__push_tag(object,"foreach" TSRMLS_CC);
		CQuickTemplate__compile_foreach_start(object,tag_args,&sciprString TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,sciprString,1);
		efree(sciprString);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;

	}else if(strcmp(Z_STRVAL_PP(tag_command),"/foreach") == 0){
		zval	*returnString;
		CQuickTemplate__pop_tag(object,"foreach",&returnString TSRMLS_CC);
		ZVAL_STRING(*returnTagsInfo,"<?php endforeach; endif; ?>",1);
		zval_ptr_dtor(&returnString);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
		zval_ptr_dtor(&tag_args);
		return;
	}else{
		
		char	*returnString1,
				*returnString2,
				*returnString3;

		//寻找自定义函数
		if(CQuickTemplate__compile_compiler_tag(object,Z_STRVAL_PP(tag_command),Z_STRVAL_PP(tag_modifier),tag_args,&returnString1 TSRMLS_CC)){
			ZVAL_STRING(*returnTagsInfo,returnString1,1);
			efree(returnString1);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			zval_ptr_dtor(&tag_args);
			return;

		}else if(CQuickTemplate__compile_block_tag(object,Z_STRVAL_PP(tag_command),Z_STRVAL_PP(tag_modifier),tag_args,&returnString2 TSRMLS_CC)){
			ZVAL_STRING(*returnTagsInfo,returnString2,1);
			efree(returnString2);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			zval_ptr_dtor(&tag_args);
			return;

		}else if(CQuickTemplate__compile_custom_tag(object,Z_STRVAL_PP(tag_command),Z_STRVAL_PP(tag_modifier),tag_args,&returnString3 TSRMLS_CC)){
			ZVAL_STRING(*returnTagsInfo,returnString3,1);
			efree(returnString3);
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			zval_ptr_dtor(&tag_args);
			return;
		}else{
			char	errorMessage[1024];
			zval_ptr_dtor(&match);
			zval_ptr_dtor(&match1);
			sprintf(errorMessage,"%s%s","[CQuickTemplate] Compiler error : unrecognized tag : ",Z_STRVAL_P(template_tag));
			ZVAL_NULL(*returnTagsInfo);
			zval_ptr_dtor(&tag_args);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			return;
		}
	}

	MODULE_BEGIN
		char	errorMessage[1024];
		ZVAL_NULL(*returnTagsInfo);
		zval_ptr_dtor(&tag_args);
		sprintf(errorMessage,"%s%s","[CQuickTemplate] Compiler error : unrecognized tag : ",Z_STRVAL_P(template_tag));
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		zval_ptr_dtor(&match);
		zval_ptr_dtor(&match1);
	MODULE_END
}

//执行编译模板
int CQuickTemplate_compileTemplate(zval *object,char *name TSRMLS_DC){
	
	char	*templatePath,
			*source_content,
			*ldq,
			*rdq,
			*reg0,
			reg1[10240],
			reg2[10240],
			*tag_guard,
			*compileContent,
			*templateHeader,
			*compilePath;

	zval	*left_delimiter,
			*right_delimiter,
			*_match,
			**template_tags,
			*text_blocks,
			*compiled_tags,
			*_tag_stack;

	int		n;

	smart_str	compiled_content = {0};

	CQuickTemplate_getTemplateTruePath(object,name,&templatePath TSRMLS_CC);

	//读取文件内容
	file_get_contents(templatePath,&source_content);

	//确定左右标签
	left_delimiter = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("left_delimiter"),0 TSRMLS_CC);
	right_delimiter = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("right_delimiter"),0 TSRMLS_CC);
	preg_quote(Z_STRVAL_P(left_delimiter),"~",&ldq);
	preg_quote(Z_STRVAL_P(right_delimiter),"~",&rdq);

	//提取include标签
	MODULE_BEGIN
		char *newContent;
		QuickTemplate__parse_include_tags(object,source_content,ldq,rdq,&newContent TSRMLS_CC);
		efree(source_content);
		source_content = estrdup(newContent);
		efree(newContent);
	MODULE_END


	//replace special blocks by "{php}"
	strcat2(&reg0,"~",ldq,"\\*(.*?)\\*",rdq,"|",ldq,"\\s*literal\\s*",rdq,"(.*?)",ldq,"\\s*/literal\\s*",rdq,"|",ldq,"\\s*php\\s*",rdq,"(.*?)",ldq,"\\s*/php\\s*",rdq,"~se",NULL);
	
	MODULE_BEGIN
		char	*tempContent;
		preg_replace(reg0,"'<{php' . str_repeat(" ", substr_count('\0', " ")) .'}>'",source_content,&tempContent);
		efree(source_content);
		source_content = estrdup(tempContent);
		efree(tempContent);
		efree(reg0);
	MODULE_END


	//提取全部模板标签
	sprintf(reg1,"%s%s%s%s%s","~",ldq,"\\s*(.*?)\\s*",rdq,"~s");
	preg_match_all(reg1, source_content,&_match);

	zend_hash_index_find(Z_ARRVAL_P(_match),1,(void**)&template_tags);

	//分割标签包裹的文本
	sprintf(reg2,"%s%s%s%s%s","~",ldq,".*?",rdq,"~s");
	preg_split1(reg2, source_content,&text_blocks);


	//将模板标签编译成php代码
	MAKE_STD_ZVAL(compiled_tags);
	array_init(compiled_tags);

	MODULE_BEGIN
		int		i,n;
		zval	**nowTags,
				*returnTagsInfo;
		n = zend_hash_num_elements(Z_ARRVAL_PP(template_tags));
		for(i = 0;i < n ; i++){
			zend_hash_index_find(Z_ARRVAL_PP(template_tags),i,(void**)&nowTags);
			CQuickTemplate_compile_tag(object,*nowTags,&returnTagsInfo TSRMLS_CC);
			if(IS_NULL == Z_TYPE_P(returnTagsInfo)){
				char errorMessage[1024];
				sprintf(errorMessage,"%s%s","[CQuickTemplate] Compiler error : can not compile tag:",Z_STRVAL_PP(nowTags));
				php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
				efree(templatePath);
				efree(source_content);
				efree(ldq);
				efree(rdq);
				zval_ptr_dtor(&_match);
				zval_ptr_dtor(&text_blocks);
				zval_ptr_dtor(&compiled_tags);
				return;
			}

			add_next_index_zval(compiled_tags,returnTagsInfo);
			zend_hash_move_forward(Z_ARRVAL_PP(template_tags));
		}
	MODULE_END

	//判断调用堆栈
	_tag_stack = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tag_stack"),0 TSRMLS_CC);
	n = zend_hash_num_elements(Z_ARRVAL_P(_tag_stack));
	if(n > 0){
		zval	**lastTask;
		char	*tagName = NULL,
				errorMessage[10240];

		zend_hash_internal_pointer_end(Z_ARRVAL_P(_tag_stack));
		if(SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(_tag_stack), (void **)&lastTask) && IS_ARRAY == Z_TYPE_PP(lastTask)){
			zval	**tagsInfo;
			if(SUCCESS == zend_hash_index_find(Z_ARRVAL_PP(lastTask),0,(void**)&tagsInfo) && IS_STRING == Z_TYPE_PP(tagsInfo) ){
				tagName = estrdup(Z_STRVAL_PP(tagsInfo));
			}
		}
		if(tagName == NULL){
			tagName = estrdup("unknow");
		}

		sprintf(errorMessage,"%s%s","[CQuickTemplate] Compiler error : unclosed tag:",tagName);
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		efree(templatePath);
		efree(source_content);
		efree(ldq);
		efree(rdq);
		zval_ptr_dtor(&_match);
		zval_ptr_dtor(&text_blocks);
		zval_ptr_dtor(&compiled_tags);
		efree(tagName);
		return;
	}

	//生成模板标记
	MODULE_BEGIN
		char	*md5String,
				randString[32];
		int		randNum;
		zval	*nowTime;

		microtimeTrue(&nowTime);
		srand(time(NULL));
		randNum = rand() % 100;
		sprintf(randString,"%f%d",Z_DVAL_P(nowTime),randNum);
		zval_ptr_dtor(&nowTime);
		md5(randString,&md5String);
		strcat2(&tag_guard,"|QuickTemplate",md5String,"|",NULL);
		efree(md5String);
	MODULE_END

	//开始替换标签
	MODULE_BEGIN
		int		i,k;
		zval	**thisVal;
		k = zend_hash_num_elements(Z_ARRVAL_P(compiled_tags));
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(compiled_tags));
		for(i = 0 ; i < k ; i++){
			zend_hash_get_current_data(Z_ARRVAL_P(compiled_tags),(void**)&thisVal);

			if(Z_STRLEN_PP(thisVal) == 0){
				zval	**text_block_i1;
				char	*replaceNewString;
				zend_hash_index_find(Z_ARRVAL_P(text_blocks),i+1,(void**)&text_block_i1);
				preg_replace("~^(\\r\\n|\\r|\\n)~", "", Z_STRVAL_PP(text_block_i1),&replaceNewString);
				add_index_string(text_blocks,i+1,replaceNewString,1);
				efree(replaceNewString);
			}

			//替换text_blocks
			if(1){
				char	*replaceString;
				zval	**text_block_thisVal;
				zend_hash_index_find(Z_ARRVAL_P(text_blocks),i,(void**)&text_block_thisVal);
				str_replace("<?",tag_guard,Z_STRVAL_PP(text_block_thisVal),&replaceString);
				add_index_string(text_blocks,i,replaceString,1);
				efree(replaceString);
			}

		
			//替换compiled_tags
			if(1){
				char	*replaceString;
				zval	**text_block_thisVal;
				zend_hash_index_find(Z_ARRVAL_P(compiled_tags),i,(void**)&text_block_thisVal);
				str_replace("<?",tag_guard,Z_STRVAL_PP(text_block_thisVal),&replaceString);
				add_index_string(compiled_tags,i,replaceString,1);
				efree(replaceString);
			}
		
			if(1){
				zval	**thisTextVal,
						**thisCompileVal;
				zend_hash_index_find(Z_ARRVAL_P(compiled_tags),i,(void**)&thisCompileVal);
				zend_hash_index_find(Z_ARRVAL_P(text_blocks),i,(void**)&thisTextVal);
				smart_str_appends(&compiled_content,Z_STRVAL_PP(thisTextVal));
				smart_str_appends(&compiled_content,Z_STRVAL_PP(thisCompileVal));
			}

			zend_hash_move_forward(Z_ARRVAL_P(compiled_tags));
		}

		if(1){
			zval	**thisTextVal;
			char	*replaceString;
			zend_hash_index_find(Z_ARRVAL_P(text_blocks),i,(void**)&thisTextVal);
			str_replace("<?",tag_guard,Z_STRVAL_PP(thisTextVal),&replaceString);
			smart_str_appends(&compiled_content,replaceString);
			efree(replaceString);
		}

		smart_str_0(&compiled_content);

		if(1){
			char	*com_replace_1,
					*com_replace_2,
					*com_replace_3;
			str_replace("<?","<?php echo '<?' ?>\n",compiled_content.c,&com_replace_1);
			preg_replace("~(?<!')language\\s*=\\s*[\"\']?\\s*php\\s*[\"\']?~", "<?php echo 'language=php' ?>\n",com_replace_1,&com_replace_2);
			str_replace(tag_guard,"<?",com_replace_2,&com_replace_3);
			compileContent = estrdup(com_replace_3);
			efree(com_replace_1);
			efree(com_replace_2);
			efree(com_replace_3);
		}

		//释放中间变量
		smart_str_free(&compiled_content);

		//移除compileContent末尾换行
		if(strlen(compileContent) > 0){

			char *substr_1;
			substr1(compileContent,-1,&substr_1);
			if(strcmp(substr_1,"\n") == 0){
				char *substr0_1;
				substr(compileContent,0,-1,&substr0_1);
				efree(compileContent);
				compileContent = estrdup(substr0_1);
				efree(substr0_1);
			}
			efree(substr_1);
		}
	MODULE_END


	//设置模板头
	MODULE_BEGIN
		char	*dateString;
		zval	*_version;
		php_date("Y-m-d H:i:s",&dateString);
		_version = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_version"),0 TSRMLS_CC);
		strcat2(&templateHeader,"<?php /* QuickTemplate version ",Z_STRVAL_P(_version),", created on ",dateString," */ ?>\n",NULL);
		efree(dateString);
	MODULE_END
	
	//判断是否需要清理已有的编译缓存
	CQuickTemplate_getCompileCache(object,name,&compilePath TSRMLS_CC);
	if(SUCCESS == fileExist(compilePath)){
		if(0 != remove(compilePath)){
			//删除文件失败
		}
	}


	//写入编译缓存
	MODULE_BEGIN
		smart_str cacheFile = {0};
		smart_str_appends(&cacheFile,templateHeader);
		smart_str_appends(&cacheFile,compileContent);
		smart_str_0(&cacheFile);
		file_put_contents(compilePath,cacheFile.c);
		smart_str_free(&cacheFile);
	MODULE_END

	//销毁资源
	efree(templatePath);
	efree(source_content);
	efree(ldq);
	efree(rdq);
	zval_ptr_dtor(&_match);
	zval_ptr_dtor(&text_blocks);
	zval_ptr_dtor(&compiled_tags);
	efree(tag_guard);
	efree(compileContent);
	efree(templateHeader);

	if(SUCCESS == fileExist(compilePath)){
		efree(compilePath);
		return 1;
	}else{
		efree(compilePath);
		return 0;
	}
}

PHP_METHOD(CQuickTemplate,compileTemplate){
}

PHP_METHOD(CQuickTemplate,_parse_include_tags){}
PHP_METHOD(CQuickTemplate,_compile_tag){}
PHP_METHOD(CQuickTemplate,_syntax_error){}
PHP_METHOD(CQuickTemplate,_parse_attrs){}
PHP_METHOD(CQuickTemplate,_parse_vars_props){}
PHP_METHOD(CQuickTemplate,_expand_quoted_text){}
PHP_METHOD(CQuickTemplate,_parse_var){}
PHP_METHOD(CQuickTemplate,_compile_smarty_ref){}
PHP_METHOD(CQuickTemplate,_push_tag){}
PHP_METHOD(CQuickTemplate,_get_plugin_filepath){}
PHP_METHOD(CQuickTemplate,_add_plugin){}
PHP_METHOD(CQuickTemplate,_compile_plugin_call){}
PHP_METHOD(CQuickTemplate,_parse_modifiers){}
PHP_METHOD(CQuickTemplate,_dequote){}
PHP_METHOD(CQuickTemplate,_pop_tag){}
PHP_METHOD(CQuickTemplate,_compile_foreach_start){}
PHP_METHOD(CQuickTemplate,_compile_if_tag){}
PHP_METHOD(CQuickTemplate,_compile_compiler_tag){}
PHP_METHOD(CQuickTemplate,_compile_block_tag){}
PHP_METHOD(CQuickTemplate,_push_cacheable_state){}
PHP_METHOD(CQuickTemplate,_compile_arg_list){}
PHP_METHOD(CQuickTemplate,_pop_cacheable_state){}
PHP_METHOD(CQuickTemplate,_compile_custom_tag){}
PHP_METHOD(CQuickTemplate,_parse_var_props){}
PHP_METHOD(CQuickTemplate,_quote_replace){}

void CQuickTemplate_display(zval *object,char *name TSRMLS_DC){

	int		exists = 0,
			needCompile = 1;

	char	*loPath;


	//判断模板存在
	exists = CQuickTemplate_template_exists(object,name TSRMLS_CC);

	if(0 == exists){
		char errorMessage[1024];
		sprintf(errorMessage,"%s%s","[CQuickTemplate] The template is not exist :",name);
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
		return;
	}

	//检查是否需要重新编译模板
	needCompile = CQuickTemplate_checkNeedCompileTemplate(object,name TSRMLS_CC);

	if(1 == needCompile){

		//重新编译模板
		int		compileStatus = 0;

		compileStatus = CQuickTemplate_compileTemplate(object,name TSRMLS_CC);

		if(!compileStatus){
			char errorMessage[1024];
			sprintf(errorMessage,"%s%s","[CQuickTemplate] Compiler error , can not complete template :",name);
			php_error_docref(NULL TSRMLS_CC, E_ERROR ,errorMessage);
			return;
		}
	}

	//引用模板
	CQuickTemplate_getCompileCache(object,name,&loPath TSRMLS_CC);
	CLoader_loadFile(loPath);
	efree(loPath);
}

PHP_METHOD(CQuickTemplate,display){

	char	*key,
			*cacheId;
	long	keyLen = 0,
			cacheIdLen = 0,
			num;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|sl",&key,&keyLen,&cacheId,&cacheIdLen,&num) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call display params error");
		RETURN_FALSE;
	}

	CQuickTemplate_display(getThis(),key TSRMLS_CC);
}


//获取编译目录
void CQuickTemplate_getCompileCache(zval *object,char *name,char **filePath TSRMLS_DC){

	zval	*compileDir,
			*_tpl_vars,
			**content_insert_layout;
			
	char	compileTruePath[1024],
			returnPath[1024],
			*md5string,
			*layoutName;

	compileDir = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("compileDir"),0 TSRMLS_CC);
	_tpl_vars = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tpl_vars"),0 TSRMLS_CC);
	
	if(IS_STRING != Z_TYPE_P(compileDir)){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] can not read compile dir");
		return;
	}

	//获取布局文件名称
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(_tpl_vars),"CONTENT_INSERET_LAYOUT",strlen("CONTENT_INSERET_LAYOUT")+1,(void**)&content_insert_layout) && IS_STRING == Z_TYPE_PP(content_insert_layout)){
		layoutName = estrdup(Z_STRVAL_PP(content_insert_layout));
	}else{
		layoutName = estrdup("");
	}


	//编译文件路径
	sprintf(compileTruePath,"%s%s%s%s%s",Z_STRVAL_P(compileDir),"/",name,"/",layoutName);

	//获取md5
	md5(compileTruePath,&md5string);
	sprintf(returnPath,"%s%s%s%s",Z_STRVAL_P(compileDir),"/",md5string,".lo");

	*filePath = estrdup(returnPath);
	efree(md5string);
	efree(layoutName);
}

PHP_METHOD(CQuickTemplate,getCompileCache){
}


PHP_METHOD(CQuickTemplate,getFileLastChangeTime){
}
PHP_METHOD(CQuickTemplate,_compile_registered_object_tag){}

PHP_METHOD(CQuickTemplate,fetch){

	char	*key,
			*cacheId,
			*contents;
	long	keyLen = 0,
			cacheIdLen = 0,
			num;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|sl",&key,&keyLen,&cacheId,&cacheIdLen,&num) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call display params error");
		RETURN_FALSE;
	}

	ob_start();
	CQuickTemplate_display(getThis(),key TSRMLS_CC);
	ob_get_contents(&contents);
	ob_end_clean();

	RETVAL_STRING(contents,1);
	efree(contents);
}

PHP_METHOD(CQuickTemplate,clear_cache){}

//判断模板存在
int CQuickTemplate_template_exists(zval *object,char *name TSRMLS_DC){

	char	*templateTruePath;
	int		exists = 0;

	CQuickTemplate_getTemplateTruePath(object,name,&templateTruePath TSRMLS_CC);

	//判断文件存在
	if(SUCCESS == fileExist(templateTruePath)){
		exists = 1;
	}
	
	efree(templateTruePath);
	return exists;
}

PHP_METHOD(CQuickTemplate,template_exists){

	char	*key;
	long	keyLen = 0,
			exists = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&key,&keyLen) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call template_exists params error");
		RETURN_FALSE;
	}

	exists = CQuickTemplate_template_exists(getThis(),key TSRMLS_CC);
	RETVAL_BOOL(exists);
}

//注册函数
int CQuickTemplate_register_function(zval *object,char *name,zval *callInfo TSRMLS_DC){

	zval	*_function,
			*saveCallInfo;

	_function = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_function"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(saveCallInfo);
	ZVAL_ZVAL(saveCallInfo,callInfo,1,0);
	add_assoc_zval(_function,name,saveCallInfo);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_function"),_function TSRMLS_CC);
	return 1;

}

PHP_METHOD(CQuickTemplate,register_function){

	char	*name;
	int		nameLen = 0,
			cached = 0,
			status = 1;
	zval	*callInfo,
			*cacheAttr = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|bz",&name,&nameLen,&callInfo,&cached,&cacheAttr) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call register_block params error");
		RETURN_FALSE;
	}

	status = CQuickTemplate_register_function(getThis(),name,callInfo TSRMLS_CC);
	RETVAL_LONG(status);
}

int CQuickTemplate_register_block(zval *object,char *name,zval *callInfo TSRMLS_DC){

	zval	*_block,
			*saveCallInfo;

	_block = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_block"),0 TSRMLS_CC);

	MAKE_STD_ZVAL(saveCallInfo);
	ZVAL_ZVAL(saveCallInfo,callInfo,1,0);
	add_assoc_zval(_block,name,saveCallInfo);
	zend_update_property(CQuickTemplateCe,object,ZEND_STRL("_block"),_block TSRMLS_CC);
	return 1;
}

PHP_METHOD(CQuickTemplate,register_block){
	
	char	*name;
	int		nameLen = 0,
			cached = 0,
			status = 1;
	zval	*callInfo,
			*cacheAttr = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz|bz",&name,&nameLen,&callInfo,&cached,&cacheAttr) == FAILURE){
		php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] call register_block params error");
		RETURN_FALSE;
	}

	status = CQuickTemplate_register_block(getThis(),name,callInfo TSRMLS_CC);
	RETVAL_LONG(status);
}

void CQuickTemplate_smarty_make_timestamp(zval *object,char *string,char **returnString TSRMLS_DC){

	zval	*match1;

	//返回当前时间
	if(strlen(string) == 0){

		zval	*nowTime;
		char	tempString[16];
		microtime(&nowTime);
		sprintf(tempString,"%d",(int)Z_DVAL_P(nowTime));
		zval_ptr_dtor(&nowTime);
		*returnString = estrdup(tempString);
		return;

	}else if(preg_match("/^\\d{14}$/", string,&match1)){

		char	*substr82,
				*substr102,
				*substr122,
				*substr42,
				*substr62,
				*substr04;

		zval	*returnZval;

		int		s1,s2,s3,s4,s5,s6;

		substr(string,8,2,&substr82);
		substr(string,10,2,&substr102);
		substr(string,12,2,&substr122);
		substr(string,4,2,&substr42);
		substr(string,6,2,&substr62);
		substr(string,0,4,&substr04);
		s1 = toInt(substr82);
		s2 = toInt(substr102);
		s3 = toInt(substr122);
		s4 = toInt(substr42);
		s5 = toInt(substr62);
		s6 = toInt(substr04);
		php_t_mktime(s1,s2,s3,s4,s5,s6,&returnZval);
		if(IS_STRING == Z_TYPE_P(returnZval)){
			*returnString = estrdup(Z_STRVAL_P(returnZval));
		}else if(IS_LONG == Z_TYPE_P(returnZval)){
			char temp[64];
			sprintf(temp,"%d",Z_LVAL_P(returnZval));
			*returnString = estrdup(temp);
		}else{
			*returnString = estrdup(string);
		}
		efree(substr82);
		efree(substr102);
		efree(substr122);
		efree(substr42);
		efree(substr62);
		efree(substr04);
		zval_ptr_dtor(&returnZval);
		zval_ptr_dtor(&match1);
		return;

	}else if(isdigitstr(string)){

		*returnString = estrdup(string);
		zval_ptr_dtor(&match1);
		return;

	}else{
		char temp[14];
		int timestamp ;
		timestamp = php_strtotime(string);

		if(timestamp == 0){
			char *thisReturn;
			CQuickTemplate_smarty_make_timestamp(object,"",&thisReturn TSRMLS_CC);
			*returnString = estrdup(thisReturn);
			efree(thisReturn);
			zval_ptr_dtor(&match1);
			return;
		}

		sprintf(temp,"%d",timestamp);
		*returnString = estrdup(temp);
		zval_ptr_dtor(&match1);
		return;
	}


	*returnString = estrdup(string);
	zval_ptr_dtor(&match1);
}

PHP_METHOD(CQuickTemplate,_modifier_date_format){

	char	*string,
			*format = "%Y-%m-%d %H:%M:%S",
			*default_date = "",
			*returnString;

	int		stringLen = 0,
			formatLen = 0,
			default_dateLen = 0;

	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|ss",&string,&stringLen,&format,&formatLen,default_date,default_dateLen) == FAILURE){
		return;
	}

	if(strlen(string) != 0){

		CQuickTemplate_smarty_make_timestamp(getThis(),string,&returnString TSRMLS_CC);

	}else if(default_dateLen > 0){

		CQuickTemplate_smarty_make_timestamp(getThis(),default_date,&returnString TSRMLS_CC);

	}else{

		return;
	}
	
	php_t_strftime(format,returnString,&returnZval);

	RETVAL_ZVAL(returnZval,1,1);
	efree(returnString);
}


PHP_METHOD(CQuickTemplate,smarty_make_timestamp)
{
	char	*string,
			*returnString;
	int		stringLen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&stringLen) == FAILURE){
		return;
	}

	CQuickTemplate_smarty_make_timestamp(getThis(),string,&returnString TSRMLS_CC);

}


PHP_METHOD(CQuickTemplate,_modifier_string_format){

	char	*string,
			*defaultString,
			*returnString;

	int		stringLen = 0,
			defaultStringLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&string,&stringLen,&defaultString,&defaultStringLen) == FAILURE){
		return;
	}

	php_t_sprintf(defaultString,string,&returnString);

	RETVAL_STRING(returnString,1);
	efree(returnString);
}

PHP_METHOD(CQuickTemplate,_modifier_default){

	char	*string,
			*defaultString;

	int		stringLen = 0,
			defaultStringLen = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&string,&stringLen,&defaultString,&defaultStringLen) == FAILURE){
		return;
	}

	if(stringLen >0){
		RETVAL_STRING(string,1);
	}else{
		RETVAL_STRING(defaultString,1); 
	}
}

PHP_METHOD(CQuickTemplate,_modifier_replace){

	char	*string1,
			*string2,
			*string3,
			*returnString;

	int		stringLen1 = 0,
			stringLen2 = 0,
			stringLen3 = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sss",&string1,&stringLen1,&string2,&stringLen2,&string3,&stringLen3) == FAILURE){
		return;
	}

	str_replace(string2,string3,string1,&returnString);

	RETVAL_STRING(returnString,1);
	efree(returnString);
}

PHP_METHOD(CQuickTemplate,_run_mod_handler){
	

	char	*string1;

	int		stringLen1,
			isRepeat;

	zval	*value1,
			*value2;


	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sbzz",&string1,&stringLen1,&isRepeat,&value1,&value2) == FAILURE){
		return;
	}
}

PHP_METHOD(CQuickTemplate,_modifier_cat){

	char	*string1,
			*string2,
			*returnString;

	int		stringLen1 = 0,
			stringLen2 = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ss",&string1,&stringLen1,&string2,&stringLen2) == FAILURE){
		return;
	}

	strcat2(&returnString,string1,string2,NULL);
	
	RETVAL_STRING(returnString,1);
	efree(returnString);
}

PHP_METHOD(CQuickTemplate,__call){

	char	*val;

	zval	*args,
			*paramsList[64],
			param,
			**thisVal,
			*returnZval;

	int		valLen,
			num,
			i;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"sz",&val,&valLen,&args) == FAILURE){
		return;
	}

	//包含
	if(strstr(val,"_modifier") != NULL){

		//判断是否存在本地函数
		char *trueFunName;
		str_replace("_modifier_","",val,&trueFunName);

	
		//判断函数存在
		if(function_exists(trueFunName)){

			num = zend_hash_num_elements(Z_ARRVAL_P(args));

			if(num >= 64){
				php_error_docref(NULL TSRMLS_CC, E_ERROR ,"[CViewException] CQuickTemplate execution method parameter number too much");
				return;
			}

			zend_hash_internal_pointer_reset(Z_ARRVAL_P(args));
			for(i = 0 ; i < num ; i++){
				paramsList[i] = &param;
				MAKE_STD_ZVAL(paramsList[i]);
				zend_hash_get_current_data(Z_ARRVAL_P(args),(void**)&thisVal);
				ZVAL_ZVAL(paramsList[i],*thisVal,1,0);
				zend_hash_move_forward(Z_ARRVAL_P(args));
			}

			MODULE_BEGIN
				zval	constructReturn,
						constructVal,
						*saveZval;
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal,trueFunName, 0);
				call_user_function(EG(function_table), NULL, &constructVal, &constructReturn, num, paramsList TSRMLS_CC);
				returnZval = &constructReturn;

				for(i = 0 ; i < num ; i++){
					zval_ptr_dtor(&paramsList[i]);
				}

				if(EG(exception)){
					char errMessage[1024];
					sprintf(errMessage,"%s%s%s","[CViewException] CQuickTempate call function error: ",trueFunName,"()");
					zend_clear_exception(TSRMLS_C);
					php_error_docref(NULL TSRMLS_CC, E_ERROR ,errMessage);
					return;
				}

				ZVAL_ZVAL(return_value,returnZval,1,0);
				zval_dtor(&constructReturn);
			MODULE_END

		}

		efree(trueFunName);
	}


}


//确定模板是否需要重新编译
int CQuickTemplate_checkNeedCompileTemplate(zval *object,char *name TSRMLS_DC){
	
	char	*compileFile,
			*templateFile,
			*contentFile;

	long	compileFileTime = 0,
			templateFileTime = 0,
			contentFileTime = 0;

	int		needReCompile = 1;

	zval	**content_insert_layout,
			*_tpl_vars;

	CQuickTemplate_getCompileCache(object,name,&compileFile TSRMLS_CC);

	//如果编译缓存不存在 
	if(SUCCESS != fileExist(compileFile)){
		efree(compileFile);
		return 1;
	}

	//读取content_insert_layout变更时间
	_tpl_vars = zend_read_property(CQuickTemplateCe,object,ZEND_STRL("_tpl_vars"),0 TSRMLS_CC);
	if(SUCCESS == zend_hash_find(Z_ARRVAL_P(_tpl_vars),"CONTENT_INSERET_LAYOUT",strlen("CONTENT_INSERET_LAYOUT")+1,(void**)&content_insert_layout) && IS_STRING == Z_TYPE_PP(content_insert_layout) ){
		char	*contentFileTruePath;
		contentFile = estrdup(Z_STRVAL_PP(content_insert_layout));
		CQuickTemplate_getTemplateTruePath(object,contentFile,&contentFileTruePath TSRMLS_CC);
		contentFileTime = filemtime(contentFileTruePath);
		efree(contentFile);
		efree(contentFileTruePath);
	}

	//对比模板的最后编辑时间和编译缓存的最后编辑时间
	CQuickTemplate_getTemplateTruePath(object,name,&templateFile TSRMLS_CC);
	templateFileTime = filemtime(templateFile);
	compileFileTime = filemtime(compileFile);


	if(templateFileTime > compileFileTime || contentFileTime > compileFileTime){
		needReCompile = 1;
	}else{
		needReCompile = 0;
	}

	efree(compileFile);
	efree(templateFile);

	return needReCompile;
}

PHP_METHOD(CQuickTemplate,checkNeedCompileTemplate){
}