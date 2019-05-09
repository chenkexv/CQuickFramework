<h1>框架安装步骤</h1>

安装方法跟其他扩展一样

1.PHP_PATH/bin/phpize

2../configure --with-php-config=PHP_PATH/bin/php-config

3.make && make install

4.编辑php.ini文件加入一行 extension=CMyFrameExtension.so

5.重启php-fpm或http服务

PS:其中PHP_PATH为php可执行文件所在目录

安装完成后,进入你的网站目录运行如下命令,将自动生成文件目录


php -r "CMyFrameExtension_createProject();";


使用浏览器访问 http://localhost/base/index 

或使用命令行 php index.php base/index

程序将输出 "Hello CQuickFramework" 代表安装成功.
