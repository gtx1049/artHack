##OAT文件格式

### 1. OAT本质

OAT的本质文件格式是一个ELF文件，ELF文件是linux的标准可执行文件格式，对于ELF的详细解读，可以参考[ELF文件]()。而对于OAT形式的ELF文件，其最主要分成两个部分，第一个部分称为OATDATA，其中存放了原DEX文件以及程序运行时为找到方法和类的相关信息，第二部分称为OATEXEC，存放了程序运行时的Native代码。这两部分在OAT的ELF中，是以两个段存在的，而这两个段的位置索引存放在动态链接符号表中。

所以对于整个OAT文件的解析，首先要读取ELF Header以定位Section Header的位置，通过判定Section Header的类型，可以得知在这一Header中存在的段是什么类型，我们需要的就是动态符号表。继续进行解析，可以获取的动态符号表的Entry。按照OAT生成规则，第一个Entry存放的为OATDATA，第二个Entry存放的为OATEXEC。

### 2. DEX部分

在OATDATA部分，其存放了一份完整的DEX文件，其具体的文件格式可以在[DEX文件]()章节中查看。整个的OAT结构如图。

可以看到OAT Header指向了实际的DEX文件，而DEX文件中又按序存放的了JAVA的所有类，在OAT Dex Header中，同样按序存放了OAT数据中Class的偏移，所有共有三个位置存在着了JAVA类的信息，他们都是按序一一对应的。

### 3. OAT部分

接下来我们对OAT部分进行具体分析。

Oat Header的结构如下： 

可以在此得到*dex\_file\_coount*，即DEX文件的个数，一般来说只有一个DEX文件，是从APK中的classes.dex中得到的。在*key\_value\_store*中，记录着一些生成OAT文件时的记录。

紧接着OAT Header之后，就是DEX文件的OAT Dex File Header，其结构如下：

这一结构非常重要，其中可以得到*dex\_file\_pointer*，为dex文件的偏移位置，而_dex\_file\_location\_checksum_是之后必须修改的数据，因为在OAT启动时，ART运行时会将这个校验码与APK中压缩后dex文件的CRC校验码做对比，验证通过后才能正常运行。*classes\_offsets*是一个长度为dex中所有类数量的重要数组，依次指向oat格式的class。

在跳过一段DEX文件之后，可以索引到OAT Class，这里是由一串Oat Class Header构成的，Oat Class Header的结构如下：

在这一结构中，*type*指定了该类是不是被编译，是所有方法被编译还是部分方法被编译，而*bitmap\_size*和_bitmap_决定了哪些方法被编译。最重要的一项数据为_methods\_offsets_，存放了此类所有方法编译好的Native代码的偏移地址。

被指向的Native代码偏移直接就是Native Code的指令码，但是在程序运行时，需要一些辅助信息，这写辅助信息存放在Native Code的前0x1c位置，作为Oat Quick Method Header存在。其格式如下：

这里我们需要的是*code\_size*，即代码尺寸。

### 4. 搜索过程

搜索过程是以类为导向的，即输入类名，搜素到该类的所有方法信息以及其编译好的原生代码位置。首先通过对Oat Header进行解析，得到DEX文件的位置，接下来对DEX文件进行解析（解析过程说明在[DEX文件]()章节中），获取类的序号，通过序号，可以直接映射到相应Oat Class的序号，对此Oat Class Header进行解析，即可得到相应方法的Native指令码位置，再通过Oat Quick Method Header即可得到指令码的尺寸并将其提取。流程如下： 