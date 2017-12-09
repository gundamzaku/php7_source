`configure: error: xml2-config not found. Please check your libxml2 installation.`

重新安装libxml2和libxml2-devel包  
yum install libxml2  
yum install libxml2-devel -y  


缺少zend_config.h是因为要对php进行configure


当配置PHP时出现  make: *** [ext/fileinfo/libmagic/apprentice.lo] Error 1 时  
是因为服务器内存不足1G。  
只需要在配置命令中添加 --disable-fileinfo即可   
