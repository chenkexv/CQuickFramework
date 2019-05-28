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

//zend类对象
zend_class_entry	*CQuickTemplateCe;


//类方法:创建应用
PHP_METHOD(CQuickTemplate,__construct);
PHP_METHOD(CQuickTemplate,getInstance);
PHP_METHOD(CQuickTemplate,__construct);
PHP_METHOD(CQuickTemplate,assign);
PHP_METHOD(CQuickTemplate,display);
PHP_METHOD(CQuickTemplate,displayHTML);
PHP_METHOD(CQuickTemplate,compileTemplate);
PHP_METHOD(CQuickTemplate,_parse_include_tags);
PHP_METHOD(CQuickTemplate,_compile_tag);
PHP_METHOD(CQuickTemplate,_syntax_error);
PHP_METHOD(CQuickTemplate,_parse_attrs);
PHP_METHOD(CQuickTemplate,_parse_vars_props);
PHP_METHOD(CQuickTemplate,_expand_quoted_text);
PHP_METHOD(CQuickTemplate,_parse_var);
PHP_METHOD(CQuickTemplate,_compile_smarty_ref);
PHP_METHOD(CQuickTemplate,_push_tag);
PHP_METHOD(CQuickTemplate,_get_plugin_filepath);
PHP_METHOD(CQuickTemplate,_add_plugin);
PHP_METHOD(CQuickTemplate,_compile_plugin_call);
PHP_METHOD(CQuickTemplate,_parse_modifiers);
PHP_METHOD(CQuickTemplate,_dequote);
PHP_METHOD(CQuickTemplate,_pop_tag);
PHP_METHOD(CQuickTemplate,_compile_foreach_start);
PHP_METHOD(CQuickTemplate,_compile_if_tag);
PHP_METHOD(CQuickTemplate,_compile_compiler_tag);
PHP_METHOD(CQuickTemplate,_compile_block_tag);
PHP_METHOD(CQuickTemplate,_push_cacheable_state);
PHP_METHOD(CQuickTemplate,_compile_arg_list);
PHP_METHOD(CQuickTemplate,_pop_cacheable_state);
PHP_METHOD(CQuickTemplate,_compile_custom_tag);
PHP_METHOD(CQuickTemplate,_parse_var_props);
PHP_METHOD(CQuickTemplate,_quote_replace);
PHP_METHOD(CQuickTemplate,checkNeedCompileTemplate);
PHP_METHOD(CQuickTemplate,getTemplateTruePath);
PHP_METHOD(CQuickTemplate,getCompileCache);
PHP_METHOD(CQuickTemplate,getFileLastChangeTime);
PHP_METHOD(CQuickTemplate,_compile_registered_object_tag);
PHP_METHOD(CQuickTemplate,fetch);
PHP_METHOD(CQuickTemplate,clear_cache);
PHP_METHOD(CQuickTemplate,template_exists);
PHP_METHOD(CQuickTemplate,register_function);
PHP_METHOD(CQuickTemplate,register_block);
PHP_METHOD(CQuickTemplate,_modifier_date_format);
PHP_METHOD(CQuickTemplate,smarty_make_timestamp);
PHP_METHOD(CQuickTemplate,_modifier_string_format);
PHP_METHOD(CQuickTemplate,_modifier_default);
PHP_METHOD(CQuickTemplate,_modifier_replace);
PHP_METHOD(CQuickTemplate,_modifier_isWeekend);
PHP_METHOD(CQuickTemplate,_run_mod_handler);
PHP_METHOD(CQuickTemplate,_modifier_cat);
PHP_METHOD(CQuickTemplate,__call);

