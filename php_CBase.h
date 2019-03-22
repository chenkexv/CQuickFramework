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

char *contact(char *s1,char *s2);
void getServerParam(char *key,char **getStr TSRMLS_DC);

#define HOOK_ROUTE_START "HOOKS_ROUTE_START"
#define HOOKS_ROUTE_END "HOOKS_ROUTE_END"
#define HOOKS_ROUTE_ERROR "HOOKS_ROUTE_ERROR"
#define HOOKS_CONTROLLER_INIT "HOOKS_CONTROLLER_INIT"
#define HOOKS_ACTION_INIT "HOOKS_ACTION_INIT"
#define HOOKS_EXECUTE_BEFORE "HOOKS_EXECUTE_BEFORE"
#define HOOKS_EXECUTE_END "HOOKS_EXECUTE_END"
#define HOOKS_EXECUTE_ERROR "HOOKS_EXECUTE_ERROR"
#define HOOKS_ERROR_HAPPEN "HOOKS_ERROR_HAPPEN"
#define HOOKS_EXCEPTION_HAPPEN "HOOKS_EXCEPTION_HAPPEN"
#define HOOKS_SYSTEM_SHUTDOWN "HOOKS_SYSTEM_SHUTDOWN"
#define HOOKS_CACHE_SET "HOOKS_CACHE_SET"
#define HOOKS_CACHE_GET "HOOKS_CACHE_GET"
#define HOOKS_LOADER_START "HOOKS_LOADER_START"
#define HOOKS_VIEW_GET "HOOKS_VIEW_GET"
#define HOOKS_VIEW_SHOW "HOOKS_VIEW_SHOW"
#define HOOKS_URL_CREATE "HOOKS_URL_CREATE"
#define HOOKS_AUTO_LOAD "HOOKS_AUTO_LOAD"
#define HOOKS_MAIL_BEFORE "HOOKS_MAIL_BEFORE"
#define HOOKS_MONITOR_END "HOOKS_MONITOR_END"