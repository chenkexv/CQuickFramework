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
#include "php_CDatabase.h"
#include "php_CBuilder.h"
#include "php_CException.h"


//zend类方法
zend_function_entry CDatabase_functions[] = {
	PHP_ME(CDatabase,getInstance,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(CDatabase,__construct,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(CDatabase,getDatabase,NULL,ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

//模块被加载时
CMYFRAME_REGISTER_CLASS_RUN(CDatabase)
{
	zend_class_entry funCe;

	INIT_CLASS_ENTRY(funCe,"CDatabase",CDatabase_functions);
	CDatabaseCe = zend_register_internal_class(&funCe TSRMLS_CC);

	//变量
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("instance"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("objectPdo"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("dataObject"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("cBuilder"),ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(CDatabaseCe, ZEND_STRL("configData"),ZEND_ACC_PRIVATE|ZEND_ACC_STATIC TSRMLS_CC);
	
	return SUCCESS;
}


//删除数据库实例对象
int CDatabase_deleteDatabase(int useMaster,char *dbConf TSRMLS_DC){

	zval	*instanceZval,
			*pdoListZval;
	char	thisConKey[1024];
	int		existObject = 0;

	//静态数据
	instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);

	//配置的键名
	sprintf(thisConKey,"%d%s",useMaster,dbConf);

	//是否存在
	existObject = zend_hash_exists(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);

	//不存在返回移除失败
	if(!existObject){
		return 0;
	}

	//移除此项配置
	zend_hash_del(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);

	//删除objectPdo静态配置
	pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);

	if(IS_ARRAY == Z_TYPE_P(pdoListZval)){
		zval *updateArray;

		if(!zend_hash_exists(Z_ARRVAL_P(pdoListZval),thisConKey,strlen(thisConKey)+1)){
			return 0;
		}

		//执行删除
		zend_hash_del(Z_ARRVAL_P(pdoListZval),thisConKey,strlen(thisConKey)+1);
		if(zend_hash_num_elements(Z_ARRVAL_P(pdoListZval)) > 0){
			zend_update_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),pdoListZval TSRMLS_CC);
		}else{
			zend_update_static_property_null(CDatabaseCe,ZEND_STRL("objectPdo") TSRMLS_CC);
		}
	}

	return 1;
}


//获取单例对象
void CDatabase_getInstance(int useMaster,char *dbConf,zval **returnZval TSRMLS_DC)
{
	zval	*instanceZval,
			*instanceActive,
			**thisKeyZval,
			*thisKeyZvalSave,
			*cBuilderZval;

	char	thisConKey[128];

	int		existObject;

	MAKE_STD_ZVAL(*returnZval);


	//获取单例对象保存表
	instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(instanceZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("instance"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		instanceZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("instance"),0 TSRMLS_CC);
	}

	//配置Key
	sprintf(thisConKey,"%d%s",useMaster,dbConf);

	//判断存在该key的配置
	existObject = zend_hash_exists(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1);


	//不存在则尝试获取新的实例
	if(0 == existObject){

		zend_class_entry	**classCePP,
							*classCe;

		zval			*object,
						*saveObject;

		//查询自身类对象
		zend_hash_find(EG(class_table),"cdatabase",strlen("cdatabase")+1,(void**)&classCePP);
		classCe = *classCePP;
		//实例化该插件
		MAKE_STD_ZVAL(object);
		object_init_ex(object,classCe);


		//执行构造器
		if (classCe->constructor) {
			zval constructReturn;
			zval constructVal;
			zval *params[2],
				 param1,
				 param2;
			params[0] = &param1;
			params[1] = &param2;
			MAKE_STD_ZVAL(params[0]);
			ZVAL_LONG(params[0],useMaster);

			MAKE_STD_ZVAL(params[1]);
			ZVAL_STRING(params[1],dbConf,1);

			INIT_ZVAL(constructVal);
			ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 0);

			call_user_function(NULL, &object, &constructVal, &constructReturn, 2, params TSRMLS_CC);
			zval_ptr_dtor(&params[0]);
			zval_ptr_dtor(&params[1]);
			zval_dtor(&constructReturn);
		}

		//保存对象
		MAKE_STD_ZVAL(saveObject);
		ZVAL_ZVAL(saveObject,object,1,0);
		zend_hash_add(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1,&saveObject,sizeof(zval*),NULL);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("instance"),instanceZval TSRMLS_CC);
		zval_ptr_dtor(&object);
	}

	//取当前静态变量中的对象
	if(zend_hash_find(Z_ARRVAL_P(instanceZval),thisConKey,strlen(thisConKey)+1,(void**)&thisKeyZval) != SUCCESS || IS_OBJECT != Z_TYPE_PP(thisKeyZval) ){
		zend_throw_exception(CDbExceptionCe, "[CMyFrameFatal]A fatal error CMyFrame extension to obtain CDatabase data object", 1007 TSRMLS_CC);
		return;
	}

	//取出CBuilder变量
	MAKE_STD_ZVAL(thisKeyZvalSave);
	ZVAL_ZVAL(thisKeyZvalSave,*thisKeyZval,1,0);


	cBuilderZval = zend_read_property(CDatabaseCe,thisKeyZvalSave,ZEND_STRL("cBuilder"),0 TSRMLS_CC);
	zval_ptr_dtor(&thisKeyZvalSave);

	if(IS_OBJECT == Z_TYPE_P(cBuilderZval)){
		ZVAL_ZVAL(*returnZval,cBuilderZval,1,0);
		return;
	}

	zend_throw_exception(CDbExceptionCe, "[CMyFrameFatal]A fatal error CMyFrame extension to obtain CDatabase data object", 1007 TSRMLS_CC);
}

//类方法:创建应用对象
PHP_METHOD(CDatabase,getInstance)
{
	char	*dbConf = "main";
	int		useMaster = 0,
			dbConfLen = 0;
	zval	*returnZval;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|bs",&useMaster,&dbConf,&dbConfLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getInstance] Parameter error ", 1007 TSRMLS_CC);
		return;
	}

	if(dbConfLen == 0){
		dbConf = "main";
	}


	CDatabase_getInstance(useMaster,dbConf,&returnZval TSRMLS_CC);
	ZVAL_ZVAL(return_value,returnZval,1,0);
	zval_ptr_dtor(&returnZval);
}

//构造函数
PHP_METHOD(CDatabase,__construct)
{
	char	*dbConf,
			useMaterStr[10240],
			tablePreConfKey[1024],
			*tablePre;
	long		useMaster = 1;
	int	dbConfLen = 0;
	zval	*returnZval,
			*object,
			*useMasterZval,
			*cconfigInstanceZval,
			*tablePreZval;

	zend_class_entry	**classCePP,
						*classCe;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ls",&useMaster,&dbConf,&dbConfLen) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getInstance] Parameter error ", 1007 TSRMLS_CC);
		return;
	}

	//尝试获取CBuilder对象
	zend_hash_find(EG(class_table),"cbuilder",strlen("cbuilder")+1,(void**)&classCePP);
	classCe = *classCePP;


	//获取对象
	MAKE_STD_ZVAL(object);
	object_init_ex(object,classCe);

	//执行构造器
	if (classCe->constructor) {
		zval constructReturn;
		zval constructVal;
		INIT_ZVAL(constructVal);
		ZVAL_STRING(&constructVal, classCe->constructor->common.function_name, 1);
		call_user_function(NULL, &object, &constructVal, &constructReturn, 0, NULL TSRMLS_CC);
		zval_dtor(&constructReturn);
	}

	//设置该CBuilder对象中的属性
	sprintf(useMaterStr,"%d",useMaster);
	zend_update_property_string(CBuilderCe,object,"isMaster",strlen("isMaster"), useMaterStr TSRMLS_CC);
	zend_update_property_string(CBuilderCe,object,"configName",strlen("configName"), dbConf TSRMLS_CC);

	//写入表前缀
	if(strcmp(useMaterStr,"1") == 0){
		sprintf(tablePreConfKey,"%s%s%s","DB.",dbConf,".master.tablePrefix");
	}else{
		sprintf(tablePreConfKey,"%s%s%s","DB.",dbConf,".slaves.tablePrefix");
	}

	CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);
	CConfig_load(tablePreConfKey,cconfigInstanceZval,&tablePreZval TSRMLS_CC);
	if(IS_STRING == Z_TYPE_P(tablePreZval)){
		tablePre = estrdup(Z_STRVAL_P(tablePreZval));
	}else{
		tablePre = estrdup("");
	}

	zend_update_property_string(CBuilderCe,object,"tablePre",strlen("tablePre"), tablePre TSRMLS_CC);
	efree(tablePre);
	zval_ptr_dtor(&cconfigInstanceZval);
	zval_ptr_dtor(&tablePreZval);

	//设置自身的CBuilder变量
	zend_update_property(CDatabaseCe,getThis(),ZEND_STRL("cBuilder"),object TSRMLS_CC);
	ZVAL_ZVAL(return_value,object,1,1);
}

//获取PDO数据对象
void CDatabase_getDatabase(char *configName,int useMaster,zval **returnZval TSRMLS_DC)
{
	zval	*pdoListZval,
			*configDataZval,
			*pdoObject;
	char	pdoKey[10240];
	int		hasExist;

	MAKE_STD_ZVAL(*returnZval);
	ZVAL_NULL(*returnZval);

	//读取pdo列表
	pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(pdoListZval)){
		zval *saveArray;
		MAKE_STD_ZVAL(saveArray);
		array_init(saveArray);
		zend_update_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),saveArray TSRMLS_CC);
		zval_ptr_dtor(&saveArray);
		pdoListZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("objectPdo"),0 TSRMLS_CC);
	}

	//PDO保存的key
	sprintf(pdoKey,"%d%s",useMaster,configName);

	//判断是否存在PDO对象
	hasExist = zend_hash_exists(Z_ARRVAL_P(pdoListZval),pdoKey,strlen(pdoKey)+1);
	if(hasExist == 0){
		
		//获取配置文件
		char	*dbName;
		zval	*cconfigInstanceZval,
				*dbConfigZval,
				**masterZval,
				**slaveZval,
				*thisConfZval;

		//配置单例
		CConfig_getInstance("main",&cconfigInstanceZval TSRMLS_CC);

		//配置名称
		strcat2(&dbName,"DB.",configName,NULL);

		//数据库配置
		CConfig_load(dbName,cconfigInstanceZval,&dbConfigZval TSRMLS_CC);
		efree(dbName);

		if(IS_ARRAY != Z_TYPE_P(dbConfigZval)){
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&dbConfigZval);
			zend_throw_exception(CDbExceptionCe, "[MainConfigError] CDatabase::getDataBase() method try for there is no configuration items:[Config->DB]", 1007 TSRMLS_CC);
			return;
		}
		
		//如果没有master的key
		if(!zend_hash_exists(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1)){
			zval_ptr_dtor(&cconfigInstanceZval);
			zval_ptr_dtor(&dbConfigZval);
			zend_throw_exception(CDbExceptionCe, "[MainConfigError] CDatabase::getDataBase() method try for there is no configuration items:[Config->DB->master]", 1007 TSRMLS_CC);
			return;
		}

		//master配置信息
		zend_hash_find(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1,(void**)&masterZval);

		//slave配置信息
		if(!zend_hash_exists(Z_ARRVAL_P(dbConfigZval),"slaves",strlen("slaves")+1)){
			zend_hash_find(Z_ARRVAL_P(dbConfigZval),"master",strlen("master")+1,(void**)&slaveZval);
		}else{
			zend_hash_find(Z_ARRVAL_P(dbConfigZval),"slaves",strlen("slaves")+1,(void**)&slaveZval);
		}

		//获取configData静态变量
		configDataZval = zend_read_static_property(CDatabaseCe,ZEND_STRL("configData"),0 TSRMLS_CC);
		if(IS_NULL == Z_TYPE_P(configDataZval)){
			array_init(configDataZval);
			zend_update_static_property(CDatabaseCe,ZEND_STRL("configData"), configDataZval TSRMLS_CC);

		}

		//此次配置信息
		MAKE_STD_ZVAL(thisConfZval);
		if(useMaster == 1){
			zval *otherConfig,
				 *saveThis;
			char otherKey[1024];
			MAKE_STD_ZVAL(otherConfig);
			ZVAL_ZVAL(otherConfig,*slaveZval,1,0);
			ZVAL_ZVAL(thisConfZval,*masterZval,1,0);

			//保存当前配置
			zend_hash_update(Z_ARRVAL_P(configDataZval),pdoKey, strlen(pdoKey)+1, &thisConfZval, sizeof(zval*),NULL);

			//保存其他配置
			sprintf(otherKey,"%d%s",0,configName);
			zend_hash_update(Z_ARRVAL_P(configDataZval),otherKey, strlen(otherKey)+1, &otherConfig, sizeof(zval*),NULL);
		}else{
			zval *otherConfig,
				 *saveThis;
			char otherKey[1024];

			MAKE_STD_ZVAL(otherConfig);
			ZVAL_ZVAL(otherConfig,*masterZval,1,0);
			ZVAL_ZVAL(thisConfZval,*slaveZval,1,0);

			//保存当前配置
			zend_hash_update(Z_ARRVAL_P(configDataZval),pdoKey, strlen(pdoKey)+1, &thisConfZval, sizeof(zval*),NULL);

			//保存其他配置
			sprintf(otherKey,"%d%s",1,configName);
			zend_hash_update(Z_ARRVAL_P(configDataZval),otherKey, strlen(otherKey)+1, &otherConfig, sizeof(zval*),NULL);
		}


		//尝试获取PDO对象
		MODULE_BEGIN
			zend_class_entry	**pdoPP,
								*pdoP;

			zval	**hostZval,
					**userZval,
					**passZval,
					**masterRead,
					**slavesWrite;

			char	*pdoHost,
					*user,
					*pass;

			//找不到PDO对象
			if(zend_hash_find(EG(class_table),"pdo",strlen("pdo")+1,(void**)&pdoPP) == FAILURE){
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError] CMyFrame couldn't find the PDO object, please make sure the PDO extension is installed", 1001 TSRMLS_CC);
				return;
			}

			//获取各项参数
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"connectionString",strlen("connectionString")+1,(void**)&hostZval);
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"username",strlen("username")+1,(void**)&userZval);
			zend_hash_find(Z_ARRVAL_P(thisConfZval),"password",strlen("password")+1,(void**)&passZval);

			//判断各项属性合法
			if(IS_STRING == Z_TYPE_PP(hostZval) && IS_STRING == Z_TYPE_PP(userZval) && IS_STRING == Z_TYPE_PP(passZval) ){
			}else{
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError]The database configuration [DB] some key type errors", 1001 TSRMLS_CC);
				return;
			}

			if(strlen(Z_STRVAL_PP(hostZval)) <= 0 || strlen(Z_STRVAL_PP(userZval)) <= 0 ){
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_ptr_dtor(&dbConfigZval);
				zend_throw_exception(CDbExceptionCe, "[MainConfigError]Main key database configuration [DB->connectionString] or [DB->username] key cannot be empty", 1001 TSRMLS_CC);
				return;
			}

			//赋值
			pdoHost = estrdup(Z_STRVAL_PP(hostZval));
			user = estrdup(Z_STRVAL_PP(userZval));
			pass = estrdup(Z_STRVAL_PP(passZval));

			//实例化至Zval结构体
			pdoP = *pdoPP;
			MAKE_STD_ZVAL(pdoObject);
			object_init_ex(pdoObject,pdoP);

			//执行其构造器 并传入参数
			if (pdoP->constructor) {
				zval constructReturn;
				zval constructVal,
					 *displayParam[4],
					 *queryParam[1],
					 param1,
					 param2,
					 param3,
					 param4,
					 param5;

				displayParam[0] = &param1;
				displayParam[1] = &param2;
				displayParam[2] = &param3;

				queryParam[0] = &param5;
				
				//构造参数
				MAKE_STD_ZVAL(displayParam[0]);
				MAKE_STD_ZVAL(displayParam[1]);
				MAKE_STD_ZVAL(displayParam[2]);
				ZVAL_STRING(displayParam[0],pdoHost,1);
				ZVAL_STRING(displayParam[1],user,1);
				ZVAL_STRING(displayParam[2],pass,1);

				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, pdoP->constructor->common.function_name, 0);
				call_user_function(NULL, &pdoObject, &constructVal, &constructReturn, 3,displayParam TSRMLS_CC);

				zval_ptr_dtor(&displayParam[0]);
				zval_ptr_dtor(&displayParam[1]);
				zval_ptr_dtor(&displayParam[2]);
				zval_dtor(&constructReturn);


				//检查异常 若为PDO则转为CDbException异常抛出
				MODULE_BEGIN
					if(EG(exception)){

						//确定异常类是否为PDOException
						zend_class_entry *exceptionCe;

						exceptionCe = Z_OBJCE_P(EG(exception));
						if(strcmp(exceptionCe->name,"PDOException") == 0){
							
							//读取其错误信息
							zval *exceptionMessage;
							char *pdoErrStr;
							exceptionMessage = zend_read_property(exceptionCe,EG(exception), "message",strlen("message"),0 TSRMLS_CC);
							strcat2(&pdoErrStr,"[CDbException] ",Z_STRVAL_P(exceptionMessage),", Unable to connect to the database using the specified configuration",NULL);
							zend_clear_exception(TSRMLS_C);

							zval_ptr_dtor(&cconfigInstanceZval);
							zval_ptr_dtor(&dbConfigZval);
							zval_ptr_dtor(&pdoObject);
							efree(user);
							efree(pdoHost);
							efree(pass);

							zend_throw_exception(CDbExceptionCe, pdoErrStr, 1001 TSRMLS_CC);
							return;
						}
					}
				MODULE_END

				//设置字符集
				INIT_ZVAL(constructVal);
				ZVAL_STRING(&constructVal, "query", 0);
				MAKE_STD_ZVAL(queryParam[0]);
				ZVAL_STRING(queryParam[0],"set names utf8",1);
				call_user_function(NULL, &pdoObject, &constructVal, &constructReturn, 1,queryParam TSRMLS_CC);
				zval_ptr_dtor(&queryParam[0]);
				

				//将PDO保存在静态变量中
				zend_hash_update(Z_ARRVAL_P(pdoListZval),pdoKey, strlen(pdoKey)+1, &pdoObject, sizeof(zval*),NULL);

				//返回一个对象
				ZVAL_ZVAL(*returnZval,pdoObject,1,0);
				zval_ptr_dtor(&dbConfigZval);
				zval_ptr_dtor(&cconfigInstanceZval);
				zval_dtor(&constructReturn);
				efree(user);
				efree(pdoHost);
				efree(pass);
				return;
			}


		MODULE_END

		zval_ptr_dtor(&cconfigInstanceZval);
		zval_ptr_dtor(&dbConfigZval);

		//错误报告
		zend_throw_exception(CDbExceptionCe, "[DatabaseConnectionError] Unable to connect to the database using the specified configuration", 1007 TSRMLS_CC);
		return;
	}else{

		//直接返回PDO对象
		zval	**pdoObject;
		zend_hash_find(Z_ARRVAL_P(pdoListZval),pdoKey,strlen(pdoKey)+1,(void**)&pdoObject);
		ZVAL_ZVAL(*returnZval,*pdoObject,1,0);
		return;
	}

}

//获取PDO数据对象
PHP_METHOD(CDatabase,getDatabase)
{
	zval	*returnZval,
			*userMasterZval;
	char	*configName,
			*defaultConfigName = "main";
	int		configNameLen = 0,
			userMaster = 0;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|sz",&configName,&configNameLen,&userMasterZval) == FAILURE){
		zend_throw_exception(CDbExceptionCe, "[CDatabase] Call [CDatabase->getDatabase] Parameter error ", 1007 TSRMLS_CC);
		RETVAL_FALSE;
		return;
	}

	if(userMasterZval != NULL && IS_BOOL == Z_TYPE_P(userMasterZval) && Z_LVAL_P(userMasterZval) == 1 ){
		userMaster = 1;
	}

	CDatabase_getDatabase(defaultConfigName,userMaster,&returnZval TSRMLS_CC);
	RETVAL_ZVAL(returnZval,1,1);
}