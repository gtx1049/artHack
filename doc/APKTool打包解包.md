##APKTool打包解包

### 1. 简介
APKTool是一个第三方的关于APK的逆向工具，他的功能很强大，可以将一个已经编译好的APK拆开，获取APK中的DEX文件，XML文件以及资源文件，DEX文件还可以进一步解析成smalli代码。在修改完成后，还可以通过APKTool进行还原，重新打包成APK进行使用。

### 2. 参数

下载好APKTool之后（包括apktool.jar和apktool.bat），在命令行里输入apktool（实际执行的是apktool.bat），可以看到以下输出：

		usage: apktool
		 -advance,--advanced   prints advance information.
		 -version,--version    prints the version then exits
		usage: apktool if|install-framework [options] <framework.apk>
		 -p,--frame-path <dir>   Stores framework files into <dir>.
		 -t,--tag <tag>          Tag frameworks using <tag>.
		usage: apktool d[ecode] [options] <file_apk>
		 -f,--force              Force delete destination directory.
		 -o,--output <dir>       The name of folder that gets written. Default is apk.out
		 -p,--frame-path <dir>   Uses framework files located in <dir>.
		 -r,--no-res             Do not decode resources.
		 -s,--no-src             Do not decode sources.
		 -t,--frame-tag <tag>    Uses framework files tagged by <tag>.
		usage: apktool b[uild] [options] <app_path>
		 -f,--force-all          Skip changes detection and build all files.
		 -o,--output <dir>       The name of apk that gets written. Default is dist/name.apk
		 -p,--frame-path <dir>   Uses framework files located in <dir>.

可以看到加参数d为解包，参数b为打包。

### 3. 实例

对于我们的项目，需要直接修改classes.dex文件，如果smalli重打包会破坏程序结构，所以，在解包时，必须加上参数-s，命令如下：

		apktool d base.apk -p frameworkes -f -o arthack_unpack -s

-p为指定framework的文件夹，在这里需要指定，如果不指定其调用系统默认的framework库会引起错误。-o为指定输出目录。当重打包时，命令如下：

		apktool b arthack_unpack -p frameworkes