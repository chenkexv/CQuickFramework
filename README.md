<h1>框架安装步骤</h1>
<p>CQuickFramework遵循PHP全部的扩展开发标准,对PHP而言是非侵入式的.其安装方法跟其他扩展一样.可按如下步奏安装.</p>

<p>1.git clone https://github.com/chenkexv/CQuickFramework.git</p>
<p>2.cd CQuickFramework</p>
<p>3.PHP_PATH/bin/phpize</p>
<p>4../configure --with-php-config=PHP_PATH/bin/php-config</p>
<p>5.make && make install</p>
<p>6.编辑php.ini文件加入一行 extension=CMyFrameExtension.so</p>
<p>7.重启php-fpm或http服务</p>
<p>PS:其中PHP_PATH为php可执行文件所在目录</p>
<p>安装完成后,进入你的网站目录运行如下命令,将自动生成工程目录和配置模板</p>
<code><br>&nbsp;&nbsp;&nbsp;php -r "CMyFrameExtension_createProject();";&nbsp;&nbsp;&nbsp;<br></code>

<p>&nbsp;</p>
<p>使用浏览器访问 http://localhost

或使用命令行 php index.php base/index

程序将输出 "Hello CQuickFramework" 代表安装成功.





<p>&nbsp;</p>
<p>&nbsp;</p>
<h1>IDE函数自动补全</h1>

由于框架代码及类定义均已封装在so库或dll库中,IDE通常无法对内建类进行自动补全.

执行如下命令,框架将在当前目录创建CQuickFramework.php类库文件,将此文件放入IDE项目中,即可自动补全函数.

<code><br>&nbsp;&nbsp;&nbsp;php -r "CMyFrameExtension_dumpClassMapForIDE();";&nbsp;&nbsp;&nbsp;<br></code>