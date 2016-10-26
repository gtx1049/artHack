##代码保护方案

### 1. 原理介绍

当一个Android应用程序启动时，libart运行库会加载相应应用程序的oat和dex文件到内存中，其中dex负责在程序运行时，根据类签名等信息对类进行查找或对方法进行查找，而当运行到相应方法时，在直接执行内存中已经加载好的OAT代码。

我们的方法为同时定义两个JAVA方法，一个实现正常功能的目标方法，另一个为使用JNI定义的空方法。我们的目标是调用JNI空方法后，实际执行真正功能的目标方法。

所以我们调用的JNI方法是未经定义的，如果正常安装使用，程序会报错崩溃，抛出“没有定义此方法”的异常，所以在程序调用此方法之前，需要做一些特殊操作以确保方法可以正确执行。思路如下：
	
1. 找到目标方法内存结构在内存中的位置a
2. 找到空方法内存结构在内存中的位置b
3. 将b的代码位置指向a的代码位置
4. 改变b的访问控制权限

### 2. 源码清零

除以上步骤外，我们不希望黑客或反编译人员看到任何目标方法的相关代码，包括DEX代码和OAT代码，所以我们在外部需要对DEX中的目标代码和OAT中的目标代码进行清零，又不改变原DEX和OAT的结构。这样，清零后，程序在内存中加载的相关指令就是一块全零的区块。

对这块全零的区块，我们也需要将其还原为原来的可用代码，具体为读取文件中的代码内容，并使用内存拷贝函数写入到正确位置，这一部分也放在上文提到的“特殊操作”中。特殊操作使用JNI实现，用C++代码编写，这样才可以保证不影响JAVA运行时又对相关JAVA运行内存进行修改。

### 3. 实现

首先看以下几个宏定义：

	#define METHOD_ACCESS_FLAG(m) m + 5
	#define METHOD_QUICK_ENTRY(m) m + 11
	#define METHOD_DEX_ITEM(m) m + 6

其中m为ART_METHOD结构，ART\_METHOD是ART运行时对METHOD相关信息在内存中的结构，主要负责方法的各类相关信息以及指令码位置。具体信息可以到[ART\_METHOD]()这一章节查看。在这些宏定义中，METHOD\_ACCESS\_FLAG表示取方法的访问标志，METHOD\_QUICK\_ENTRY表示方法的指令码存储位置，而METHOD\_DEX\_ITEM表示ART结构指向的DEX项。

实际操作中，根据方法的索引读取方法的代码到内存中：

    oatmethodheader oat_method_header;
    int i;

    oat_method_header.code_size_ = 0;
    for(i = 0; i < index; i++)
    {
        fseek(fptr, oat_method_header.code_size_, SEEK_CUR);
        fread(&oat_method_header, 1, sizeof(oatmethodheader), fptr);
    }
    char* code = (char*)malloc(oat_method_header.code_size_);
    fread(code, 1, oat_method_header.code_size_, fptr);

将空方法中的访问标志和DEX项替换为目标方法的：

    //change flag and modify method dex item offset
    *(METHOD_ACCESS_FLAG(method)) = (char*)QUICK_FLAG;
    *(METHOD_DEX_ITEM(method)) = *(METHOD_DEX_ITEM(ori_method));

修改内存权限，将读取的代码拷贝到原来的代码清零位置，这样即可将内存中以零区块存在的替换为正常方法的代码（关于内存权限，可以参开[内存权限]()）：

	//change memory access mode
    mprotect((char *)ALIGN_MEM(*(METHOD_QUICK_ENTRY(ori_method))), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(*(METHOD_QUICK_ENTRY(ori_method)), code + 1, size);

最后，修正指令码访问指针：
	
	(METHOD_QUICK_ENTRY(method)) = *(METHOD_QUICK_ENTRY(ori_method));

之后，再调用此JNI方法，实际调用的是目标方法。

### 4. 流程图