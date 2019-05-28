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
#include "php_COpcode.h"


char *opname(zend_uchar opcode){
    switch(opcode) {
        case ZEND_NOP: return "ZEND_NOP"; break;
        case ZEND_ADD: return "ZEND_ADD"; break;
        case ZEND_SUB: return "ZEND_SUB"; break;
        case ZEND_MUL: return "ZEND_MUL"; break;
        case ZEND_DIV: return "ZEND_DIV"; break;
        case ZEND_MOD: return "ZEND_MOD"; break;
        case ZEND_SL: return "ZEND_SL"; break;
        case ZEND_SR: return "ZEND_SR"; break;
        case ZEND_CONCAT: return "ZEND_CONCAT"; break;
        case ZEND_BW_OR: return "ZEND_BW_OR"; break;
        case ZEND_BW_AND: return "ZEND_BW_AND"; break;
        case ZEND_BW_XOR: return "ZEND_BW_XOR"; break;
        case ZEND_BW_NOT: return "ZEND_BW_NOT"; break;
        default : 
			return "UNKNOW"; 
			break;		
	}
}

//输出opcode
char *format_zval(zval *z)
{
    static char buffer[65535];
    int len;
 
    switch(z->type) {
        case IS_NULL:
            return "NULL";
        case IS_LONG:
        case IS_BOOL:
            snprintf(buffer, 1024, "%d", z->value.lval);
            return buffer;
        case IS_DOUBLE:
            snprintf(buffer, 1024, "%f", z->value.dval);
            return buffer;
        case IS_STRING:
            snprintf(buffer, 1024, "\"%s\"", z->value.str.val);
            return buffer;
        case IS_ARRAY:
        case IS_OBJECT:
        case IS_RESOURCE:
        case IS_CONSTANT:
        case IS_CONSTANT_ARRAY:
            return "";
        default:
            return "unknown";
    }
}
 
char * format_znode(znode *n){
    static char buffer[65535];
 
    switch (n->op_type) {
        case IS_CONST:
            return format_zval(&n->u.constant);
            break;
        case IS_VAR:
            snprintf(buffer, 1024, "$%d",  n->u.var/sizeof(temp_variable));
            return buffer;
            break;
        case IS_TMP_VAR:
            snprintf(buffer, 1024, "~%d",  n->u.var/sizeof(temp_variable));
            return buffer;
            break;
        default:
            return "";
            break;
    }
}


void dump_op(zend_op *op, int num){

    php_printf("%5d  %5d %30s %040s %040s %040s<br>", num, op->lineno,
            opname(op->opcode),
            format_znode(&op->op1),
            format_znode(&op->op2),
            format_znode(&op->result)) ;
}

void dump_op_array(zend_op_array *op_array){
    if(op_array) {
        int i;
        printf("%5s  %5s %30s %040s %040s %040s\n", "opnum", "line", "opcode", "op1", "op2", "result");
        for(i = 0; i < op_array->last; i++) {
            dump_op(&op_array->opcodes[i], i);
        }
    }
}

zend_op *copyZendOpcodes(zend_op *source,int deep){

	zend_op *toData;
	toData = pemalloc(sizeof(zend_op),1);
	memcpy(toData,source,sizeof(zend_op));

	//挨个深拷贝


}


//结构体深拷贝
zend_op_array *copy_op_array_deep(zend_op_array *source){

	zend_op_array *op_array;
	op_array = pemalloc(sizeof(zend_op_array),1);
	memcpy(op_array,source,sizeof(zend_op_array));


	op_array->type = source->type;
	op_array->backpatch_count = source->backpatch_count;
	op_array->size = source->size;
	op_array->last = source->last;
	op_array->T = source->T;
	op_array->size_var = source->size_var; 
	op_array->last_var = source->last_var;
	op_array->last_brk_cont = source->last_brk_cont; 
	op_array->current_brk_cont = source->current_brk_cont; 
	op_array->last_try_catch = source->last_try_catch; 
	op_array->return_reference = source->return_reference; 
	op_array->done_pass_two = source->done_pass_two; 
	op_array->uses_this = source->uses_this; 
	op_array->function_name = pestrdup(source->function_name,1);
	op_array->filename = pestrdup(source->filename,1);
	op_array->doc_comment = pestrdup(source->doc_comment,1);
	op_array->doc_comment_len = source->doc_comment_len; 
	op_array->num_args = source->num_args; 
	op_array->required_num_args = source->required_num_args; 
	op_array->fn_flags = source->fn_flags;  

	//zend_char*
	op_array->refcount = (zend_uint*) pemalloc(sizeof(zend_uint),1);
	memcpy(op_array->refcount,source->refcount,sizeof(zend_uint));

	//zend_op*
	op_array->opcodes = copyZendOpcodes(source->opcodes,1);
return;

	op_array->vars = pemalloc(sizeof(zend_compiled_variable),1);
	memcpy(op_array->vars,source->vars,sizeof(zend_compiled_variable));

	op_array->arg_info = pemalloc(sizeof(zend_arg_info),1);
	memcpy(op_array->arg_info,source->arg_info,sizeof(zend_arg_info));

	op_array->scope = pemalloc(sizeof(zend_class_entry),1);
	memcpy(op_array->scope,source->scope,sizeof(zend_class_entry));

	op_array->brk_cont_array = pemalloc(sizeof(zend_brk_cont_element),1);
	memcpy(op_array->brk_cont_array,source->brk_cont_array,sizeof(zend_brk_cont_element));

	op_array->try_catch_array =  pemalloc(sizeof(zend_try_catch_element),1);
	memcpy(op_array->try_catch_array,source->try_catch_array,sizeof(zend_try_catch_element));

	op_array->static_variables = pemalloc(sizeof(HashTable),1);
	memcpy(op_array->static_variables,source->static_variables,sizeof(HashTable));

	op_array->start_op = pemalloc(sizeof(zend_op),1);
	memcpy(op_array->start_op,source->start_op,sizeof(zend_op));



	return op_array;
}

//结构体拷贝
zend_op_array *copy_op_array(zend_op_array *source,int isPersist){

	if(1 == isPersist){
		return copy_op_array_deep(source);
	}

}

