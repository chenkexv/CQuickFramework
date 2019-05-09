<style>
.contentTitle{font-size:14px;}
</style>

<div class="contentTitle">框架安装步骤</div>
<div class="contentNote">安装方法跟其他扩展一样</div>
<p>1.PHP_PATH/bin/phpize</p>
<p>2../configure --with-php-config=PHP_PATH/bin/php-config</p>
<p>3.make && make install</p>
<p>4.编辑php.ini文件加入一行 extension=CMyFrameExtension.</p>so
<p>5.重启php-fpm或http服务</p>
<p>PS:其中PHP_PATH为php可执行文件所在目录</p>
<p>安装完成后,进入你的网站目录运行如下命令,将自动生成文件目录</p>
<code><br>&nbsp;&nbsp;&nbsp;php -r "CMyFrameExtension_createProject();";&nbsp;&nbsp;&nbsp;<br></code>

<p>使用浏览器访问 http://localhost/base/index </p>
<p>或使用命令行 php index.php base/index </p>
<p>程序将输出 "Hello CQuickFramework" 代表安装成功.</p>
