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
zend_class_entry	*CDirCe;


//类方法:创建应用
PHP_METHOD(CDir,getDirName);
PHP_METHOD(CDir,getDirFiles);
PHP_METHOD(CDir,getDirAllFiles);
PHP_METHOD(CDir,getDirFilesToTree);
PHP_METHOD(CDir,exists);
PHP_METHOD(CDir,isAbsolute);
PHP_METHOD(CDir,isRoot);
PHP_METHOD(CDir,isReadable);
PHP_METHOD(CDir,delete);
PHP_METHOD(CDir,deleteAllFiles);
PHP_METHOD(CDir,rename);
PHP_METHOD(CDir,getDirSize);

#define PHP_SCANDIR_SORT_ASCENDING 0
#define PHP_SCANDIR_SORT_DESCENDING 1
#define PHP_SCANDIR_SORT_NONE 2




