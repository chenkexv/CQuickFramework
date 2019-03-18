dnl $Id$
dnl config.m4 for extension CMyFrameExtension

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

 PHP_ARG_WITH(CMyFrameExtension, for CMyFrameExtension support,
 Make sure that the comment is aligned:
 [  --with-CMyFrameExtension             Include CMyFrameExtension support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(CMyFrameExtension, whether to enable CMyFrameExtension support,
dnl Make sure that the comment is aligned:
dnl [  --enable-CMyFrameExtension           Enable CMyFrameExtension support])

if test "$PHP_CMYFRAMEEXTENSION" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-CMyFrameExtension -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/CMyFrameExtension.h"  # you most likely want to change this
  dnl if test -r $PHP_CMYFRAMEEXTENSION/$SEARCH_FOR; then # path given as parameter
  dnl   CMYFRAMEEXTENSION_DIR=$PHP_CMYFRAMEEXTENSION
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for CMyFrameExtension files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CMYFRAMEEXTENSION_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CMYFRAMEEXTENSION_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the CMyFrameExtension distribution])
  dnl fi

  dnl # --with-CMyFrameExtension -> add include path
  dnl PHP_ADD_INCLUDE($CMYFRAMEEXTENSION_DIR/include)

  dnl # --with-CMyFrameExtension -> check for lib and symbol presence
  dnl LIBNAME=CMyFrameExtension # you may want to change this
  dnl LIBSYMBOL=CMyFrameExtension # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CMYFRAMEEXTENSION_DIR/lib, CMYFRAMEEXTENSION_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CMYFRAMEEXTENSIONLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong CMyFrameExtension lib version or lib not found])
  dnl ],[
  dnl   -L$CMYFRAMEEXTENSION_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(CMYFRAMEEXTENSION_SHARED_LIBADD)


PHP_NEW_EXTENSION(CMyFrameExtension,Cache.c CActiveRecord.c CApplication.c CArraySort.c CBase.c CBuilder.c CConfigs.c CController.c CDatabase.c CDbError.c CException.c CExec.c CFunction.c CHooks.c CInitApplication.c CLoader.c CMemcache.c CModel.c CMyFrameExtension.c CPlugin.c CRedis.c CRequest.c CResponse.c CResult.c CRoute.c CRouteParse.c CSession.c CSmarty.c CView.c CWebApp.c CServer.c CDiContainer.c CLog.c CThread.c CPagination.c CWatcher.c CRabbit.c CRabbitMessage.c CEncrypt.c CCookie.c CSmtp.c CConsumer.c CMonitor.c CArrayHelper.c CValidate.c CQuickTemplate.c CGuardController.c CRabbitHelper.c CHash.c,$ext_shared)

fi



if test -z "$PHP_DEBUG"; then   
    AC_ARG_ENABLE(debug,  
    [   --enable-debug          compile with debugging symbols],[  
        PHP_DEBUG=$enableval  
    ],[ PHP_DEBUG=no  
    ])  
fi  
