##Python流程

###1. 卸载原有APK

首先需保证手机并无原有的APK影响，因为同一包名的APK安装后会生成不同的缓存名，为使Python自动脚本正常工作，首先需要卸载原有APK，这一部分的实现函数在installApk()中。

###2. 安装完整APK包

通过ADB命令，将APK安装在手机上。

###3. 提取OAT文件、更名复制

针对于LG手机，其[OAT文件]()存在与DATA文件夹下的davilk-cache中，在这一文件夹中以包名为分类存放着各个应用编译好的OAT文件，我们使用脚本运行以下命令，以提取OAT文件。

	commandOne = 'adb shell su -c "cp /data/dalvik-cache/arm/%s /storage/sdcard0/"' %targetfile
	commandTwo = 'adb pull /storage/sdcard0/%s' %targetfile
	commandThree = 'copy %s arthack' %targetfile

通过**adb shell su -c**的方式可以以root权限在手机的shell上执行命令，这里首先将其拷贝到手机的通用存储中，在通过adb命令下载到电脑中，将其更名为**arthack**，以供C程序处理（C程序中处理文件名固定为arthack）。

###4. 解析更名复制的OAT文件

通过编译好的C库**artTool.dll**来对拷贝出来的OAT文件arthack进行解析，这一步的关键在于使用python中的**ctype**库来加载dll文件，并调用其中的函数。关于具体的解析过程，可以查看[解析过程]()这一章节。输入类名后，通过解析，打印出方法信息，并将此类所有方法的native代码存放在新文件codefile中，返回的结果为某一类全部方法的信息，包括方法的代码偏移地址与代码尺寸，存放在数组中，两个一组。数组的最后一项，存放着dex在oat中存放的起始位置。

###5. 修改原OAT文件

通过上一步得到的数组结构，已经得知了某一类全部方法的代码偏移与尺寸，根据显示的信息，也可以确定方法的index索引值，在这一步中，输入index索引值，即可清除保留源文件名的OAT，找到位置去掉OAT生成的native代码。并返回一个dex在oat中的起始位置。

###6. 提取base.apk文件

在Android手机安装APK文件后，会留取一份APK作为资源读取等工作的载体。我们的保护方法需要对这一文件也进行修改，以防破解者在这里找到方法的源代码。base.apk通过以下命令提取：

	cmd = 'adb shell su -c "cp /data/app/%s/base.apk /storage/sdcard0/ "' %pakage
	cmd = 'adb pull /storage/sdcard0/base.apk'

和之前的提取工作一样，将其拷贝到sdcard0目录下并提取。

###7. 解包得到classes.dex、更名复制

因为dex代码存在于**classes.dex**中，所以我们需要修改的是classes.dex文件，所以需要对APK进行解包，以获取其中的classes.dex，这一工作是使用**apkTool**进行的，命令如下：

	cmd = 'apktool d base.apk -p frameworkes -f -o arthack_unpack -s'
	cmd = 'copy arthack_unpack\classes.dex ori_classes.dex'
	cmd = 'copy arthack_unpack\classes.dex classes.dex'

其中这里需要注意的是，apkTool命令如果直接调用，会直接将classes.dex解析为**smali**文件，但是我们不需要改变dex文件中方法的其他内容，只需要对dex码方法位置清0，所以不需要smali文件，所以此处需加上参数-s以保证classes.dex文件被完整生成。其中第二步为复制一个副本，以供C程序进行解析，而classes.dex则供修改以备打包。

###8. 解析更名复制的DEX文件

这一步的解析也是通过C库完成的，通过**ctypes**加载artTool.dll，并调用相关函数，这个函数用来解析dex文件，具体内容可以在[dex文件]()章节中找到。解析完成后，会返回一个数组，3个一组，依次存放dex文件中的方法条目偏移、代码尺寸、代码偏移。

###9. 修改原DEX文件

根据上一步得到的数组，对未改过名称的dex文件中的相关指令码进行清0，以防止代码被反编译。于此同时，根据OAT文件中的DEX起始位置和相关数组，清空OAT文件中存留的指令码。

###10. 替换修改后的DEX重打包

**classes.dex**修改成功后，替换到原APK解包的文件夹中，并继续使用apkTool进行打包工作，命令如下：

	cmd = 'copy classes.dex arthack_unpack\classes.dex'
	cmd = 'apktool b arthack_unpack -p frameworkes'

###11. 修改OAT中的校验码

在程序启动时，会对APK中classes.dex文件的CRC校验码进行验证，以保证APK完整性，APK是一种ZIP格式文件，在这种格式的文件中会保存每个文件节点的压缩数据与CRC校验码，这一校验码会在APK安装时写入OAT文件中的某一位置。在脚本中首先通过C程序解析得到APK文件中classes.dex的CRC值，并使用C程序解析得到OAT文件中classes.dex的CRC偏移，将其修正。

###12. 将OAT文件push到手机内

完成以上工作后，将修改好的OAT文件上传到手机的**data/dalvik-cache**目录下。

	commandSix = 'adb push %s /storage/sdcard0/' %targetfile
	commandSeven = 'adb shell su -c "cp /storage/sdcard0/%s /data/dalvik-cache/arm/ "' %targetfile

###13. 将重打包apk上传到手机内

然后将重打包的base.apk文件上传到**/data/app**中。命令如下：
	
	cmd = 'adb push arthack_unpack/dist/base.apk /storage/sdcard0/'
	cmd = 'adb shell su -c "cp /storage/sdcard0/base.apk /data/app/%s/ "' %package

###14. 上传代码片段

在第四步得到的**codefile**，将其上传到手机的sd目录中以供程序运行时加载使用。