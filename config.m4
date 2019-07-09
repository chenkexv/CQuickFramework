
PHP_ARG_WITH(CQuickFramework, for CQuickFramework support,
Make sure that the comment is aligned:
[  --with-CQuickFramework    CQuickFramework配置|启用CQuickFramework扩展框架支持 ])


dnl 接受--enable-qrcode参数 有此参数将编译qrcode类 并检查libqrrencode外部依赖
PHP_ARG_ENABLE(qrcode, wheter to enable qrcode support,
[  --enable-qrcode           CQuickFramework配置|添加此项目将依赖libqrrencode并可生成二维码], no, no)

dnl 输出当前是否接受qrcode
AC_MSG_CHECKING([Include qrcode])
if test "$PHP_QRCODE" = "yes"; then
	AC_DEFINE([CQUICKFRAMEWORK_QRCODE], [1], [Whether qrcode are available])
	AC_MSG_RESULT([yes, qrcode])
else
	AC_MSG_RESULT([no])
fi


dnl 设置扩展所使用的源文件
if test "$PHP_CQUICKFRAMEWORK" != "no"; then

    frameworkSrouceFile="Cache.c \
	    CActiveRecord.c \
	    CApplication.c \
	    CArraySort.c \
	    CBase.c \
	    CBuilder.c \
	    CConfigs.c \
	    CController.c \
	    CDatabase.c \
	    CDbError.c \
	    CException.c \
	    CExec.c \
	    CFunction.c \
	    CHooks.c \
	    CInitApplication.c \
	    CLoader.c \
	    CMemcache.c \
	    CModel.c \
	    CQuickFramework.c \
	    CPlugin.c \
	    CRedis.c \
	    CRequest.c \
	    CResponse.c \
	    CResult.c \
	    CRoute.c \
	    CRouteParse.c \
	    CSession.c \
	    CSmarty.c \
	    CView.c \
	    CWebApp.c \
	    CServer.c \
	    CDiContainer.c \
	    CLog.c \
	    CPagination.c \
	    CWatcher.c \
	    CRabbit.c \
	    CRabbitMessage.c \
	    CEncrypt.c \
	    CCookie.c \
	    CSmtp.c \
	    CConsumer.c \
	    CMonitor.c \
	    CArrayHelper.c \
	    CValidate.c \
	    CQuickTemplate.c \
	    CGuardController.c \
	    CRabbitHelper.c \
	    CHash.c \
	    CSecurityCode.c \
	    CDebug.c \
	    CCrontabController.c \
	    CFtp.c \
	    CDate.c \
	    CWord.c \
	    CImage.c \
	    CTree.c \
	    CHashTable.c \
	    CString.c \
	    CFile.c \
	    CHttp.c \
		CHttpPool.c \
	    CHttpServer.c "

    dnl 检查qrcode所需要的外部libqrencode依赖
    if test "$PHP_QRCODE" = "yes"; then
	    AC_MSG_CHECKING([checking libqrencode support])
	    for i in /usr/local /usr; do
		    if test -r $i/include/qrencode.h; then
			    PHP_ADD_INCLUDE($i/include)
			    PHP_CHECK_LIBRARY(qrencode, QRcode_encodeString,
			    [
				    PHP_ADD_LIBRARY_WITH_PATH(qrencode, $i/$PHP_LIBDIR, CQUICKFRAMEWORK_SHARED_LIBADD)
				    AC_DEFINE(CQUICKFRAMEWORK_USE_QRENCODE, 1, [Have qrencode support])
				    frameworkSrouceFile="$frameworkSrouceFile CQrcode.c"
			    ],[
				    AC_MSG_ERROR([Wrong qrencode version or library not found])
			    ],[
				    -L$i/$PHP_LIBDIR -lm
			    ])
			    break
		    else
			    AC_MSG_RESULT([no, found in $i])
			    AC_MSG_ERROR([use the --enable-qrcode , but can not find qrencode.h . Install the qrencode library or cancel the --enable-qrcode option])
		    fi
	    done
    fi
    
	PHP_ADD_INCLUDE(./include)
	PHP_ADD_LIBRARY_WITH_PATH(hiredis, ./lib, CQUICKFRAMEWORK_SHARED_LIBADD)
    PHP_NEW_EXTENSION(CQuickFramework,$frameworkSrouceFile,$ext_shared)
	PHP_SUBST(CQUICKFRAMEWORK_SHARED_LIBADD)
fi



if test -z "$PHP_DEBUG"; then   
    AC_ARG_ENABLE(debug,  
    [   --enable-debug        CQuickFramework配置|启动此参数将添加DEBUG信息及符号表更便于调试],[  
        PHP_DEBUG=$enableval  
    ],[ PHP_DEBUG=no  
    ])  
fi  