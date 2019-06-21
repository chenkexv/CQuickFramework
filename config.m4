
 PHP_ARG_WITH(CQuickFramework, for CQuickFramework support,
 Make sure that the comment is aligned:
 [  --with-CQuickFramework             Include CQuickFramework support])


if test "$PHP_CQUICKFRAMEWORK" != "no"; then
  

PHP_NEW_EXTENSION(CQuickFramework,Cache.c CActiveRecord.c CApplication.c CArraySort.c CBase.c CBuilder.c CConfigs.c CController.c CDatabase.c CDbError.c CException.c CExec.c CFunction.c CHooks.c CInitApplication.c CLoader.c CMemcache.c CModel.c CQuickFramework.c CPlugin.c CRedis.c CRequest.c CResponse.c CResult.c CRoute.c CRouteParse.c CSession.c CSmarty.c CView.c CWebApp.c CServer.c CDiContainer.c CLog.c CPagination.c CWatcher.c CRabbit.c CRabbitMessage.c CEncrypt.c CCookie.c CSmtp.c CConsumer.c CMonitor.c CArrayHelper.c CValidate.c CQuickTemplate.c CGuardController.c CRabbitHelper.c CHash.c CSecurityCode.c CDebug.c CCrontabController.c CFtp.c CDate.c CWord.c CImage.c CTree.c CHashTable.c CString.c CFile.c CHttp.c CHttpServer.c,$ext_shared)

fi



if test -z "$PHP_DEBUG"; then   
    AC_ARG_ENABLE(debug,  
    [   --enable-debug          compile with debugging symbols],[  
        PHP_DEBUG=$enableval  
    ],[ PHP_DEBUG=no  
    ])  
fi  
