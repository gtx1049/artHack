##Python加载C库

### 1. 使用方法

ctypes是Python的一个外部库，通过这一外部库，可以非常方便的调用由C语言生成的链接库与动态链接库，为在Python中使用，首先使用import ctypes将这一库导入。在需要使用C库时，加载DLL：

	dlllib = ctypes.CDLL("dll_path")

之后可以通过dlllib这一变量调用函数名，以调用C函数。

### 2. 函数原型

通过函数名赋值给变量，可以修改函数原型，代码如下：

	pydealfile = artdeal.dealARTFile
	pydealfile.argtype = [c_char_p]
	pydealfile.restype = POINTER(c_int)

dealARTFile为函数名，将其赋值给变量pydealfile后，可以通过argtype指定参数类型，参数类型是一个列表形式，这里c\_char\_p为C语言中的char*，即字符串。而restype为返回值，是一个单独的变量，这里返回的是POINTER(c\_int)，即一个int类型的指针。

### 3. 指针

指针是通过ctypes提供的指针函数来创建的，有以下两种形式：
	
	i = c_int(10)  
	pi = POINTER(i) 

pi就是一个指向10个int数组的指针

### 4. 数据类型

ctypes数据类型列表如下所示：

	ctypes type		C type									Python Type
	c_char			char									1-character string
	c_wchar			wchar_t									1-character unicode string
	c_byte			char									int/long
	c_ubyte			unsigned char							int/long
	c_bool			bool									bool
	c_short			short									int/long
	c_ushort		unsigned short							int/long
	c_int			int										int/long
	c_uint			unsigned int							int/long
	c_long			long									int/long
	c_ulong			unsigned long							int/long
	c_longlong		__int64 or longlong						int/long
	c_ulonglong		unsigned __int64 or unsigned long long	int/long
	c_float			float									float
	c_double		double									float
	c_longdouble	long double float						float
	c_char_p		char *									string or None
	c_wchar_p		wchar_t *								unicode or None
	c_void_p		void *									int/long or None