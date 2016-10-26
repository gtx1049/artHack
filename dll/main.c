#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypt_aes.h"
#include "art.h"

#define ORI_FILE "arthack"

#define SYM_TABLE 11

#define OAT_DATA 1
#define OAT_EXEC 2

#define ERROR -1

//#define ART5_0

/**********************************************************************
*Function Name: getClassCount
*Purpose:       获取DEX文件中有多少类
*Params:
				@FILE* fp 文件指针
				@int oatbegin oat数据部分的起始位置
				@int dex_file_pointer dex部分的起始位置

*Return:		@int 返回dex文件中类的个数
*Limitation:
**********************************************************************/
static int getClassCount(FILE* fp, int oatbegin, int dex_file_pointer)
{

    dexheader dexheader;
    long cur;

    cur = ftell(fp);

    fseek(fp, dex_file_pointer + oatbegin, SEEK_SET);

	//存放在dex的header中

    fread(&dexheader, 1, sizeof(dexheader), fp);

    fseek(fp, cur, SEEK_SET);

    return dexheader.class_defs_size_;
}

/**********************************************************************
*Function Name: countChar
*Purpose:       计算一个char有几个1
*Params:
				@char thebyte 传入的char

*Return:		@int 返回count
*Limitation:
**********************************************************************/
static int countChar(char thebyte)
{
    char one = 0x1;
    int count = 0;
    int i;
    for(i = 0; i < 8; i++)
    {
        if((thebyte & one) == 1)
            count++;
        thebyte >>= 1;
    }
    return count;
}

/**********************************************************************
*Function Name: classParse
*Purpose:       在调用此函数前，文件的指针已经指向oat_class的偏移，之后进行类解析，取得此类的所有方法，包括方法的oat原生代码的偏移与尺寸，并将原生代码及其系列参数存放到文件codefile中
*Params:
				@FILE* fp 文件指针
				@int oatdata_offset oat数据偏移地址
				@unsigned int method_size 这个类中有多少个方法

*Return:		@int* 数组：[偏移][尺寸][偏移][尺寸]...
*Limitation:
**********************************************************************/
static  int* classParse(FILE* fp, int oatdata_offset, unsigned int *method_size)
{
    oatmethodheader oat_method_header;

    FILE* out;
    FILE* bitmapfp;
    FILE* headerinfo;

    char* code;

    int i, j;

    unsigned short status;
    unsigned short type;

    unsigned int bitmap_size;

    unsigned int* methods_offset;

    int* ret;

    char* bitmap;

    fread(&status, 1, sizeof(short), fp);
    fread(&type, 1, sizeof(short), fp);

    printf("class status %d \n", status);
    printf("class type %d \n", type);

    out = fopen("codefile", "wb+");
	//类的类型需要判定，如果类型为1，说明此类不全编译，则按照bitmap进行尺寸判定
    if(type == 1)
    {
        printf("bit map offset : %ld\n", ftell(fp));
        printf("type : %x\n", type);

        fread(&bitmap_size, 1, sizeof(int), fp);
        bitmap = (char*)malloc(bitmap_size);
        //fseek(fp, bitmap_size, SEEK_CUR);
        fread(bitmap, 1, bitmap_size, fp);

        *method_size = 0;
        for(i = 0; i < bitmap_size; i++)
        {
            *method_size += countChar(bitmap[i]);
        }

        fwrite(&type, 1, sizeof(short), out);
        fwrite(&bitmap_size, 1, sizeof(int), out);
        fwrite(bitmap, 1, bitmap_size, out);

        printf("method_size : %d\n", *method_size);
        printf("bitmap_size : %d\n", bitmap_size);

        bitmapfp = fopen("bitmap", "wb+");
        //fprintf(bitmapfp, "%d\n", bitmap_size);

        for(i = 0; i < bitmap_size; i++)
        {
            for(j = 0; j < 8; j++)
            {

                if((bitmap[i] & 0x01) == 1)
                    fprintf(bitmapfp, "%d", 1);
                else
                    fprintf(bitmapfp, "%d", 0);
                bitmap[i] >>= 1;

            }
        }

        fclose(bitmapfp);

        free(bitmap);
        //method_size = bitmap_size;
    }
    else if(type == 0)
    {
        fwrite(&type, 1, sizeof(int), out);
    }

    //创建结果数组
    methods_offset = malloc(sizeof(int) * (*method_size));

    #ifdef ART5_0
    for(i = 0; i < *method_size; i++)
    {
        fread(&methods_offset[i], sizeof(int), 1, fp);
        fread(&j, sizeof(int), 1, fp);
    }
    #else
    fread(methods_offset, sizeof(int), *method_size, fp);
    #endif

    ret = malloc(sizeof(int) * ((*method_size) * 2 + 1));

    //增加了header
    headerinfo = fopen("headerinfo", "wb+");

	//跳转、装载、写入

    for(i = 0; i < *method_size; i++)
    {

        //some code is +1 offset to express it is a 16bit instruction
        if(methods_offset[i] % 2 == 1)
        {
            methods_offset[i] -= 1;
        }

        printf("method_off : %x \n", methods_offset[i] + oatdata_offset);
        ret[i * 2]  = methods_offset[i] + oatdata_offset;

        fseek(fp, methods_offset[i] + oatdata_offset - sizeof(oatmethodheader), SEEK_SET);
        fread(&oat_method_header, 1, sizeof(oatmethodheader), fp);

        printf("code_size : %d \n", oat_method_header.code_size_);
        ret[i * 2 + 1]  = oat_method_header.code_size_;

        fseek(fp, methods_offset[i] + oatdata_offset, SEEK_SET);

        code = malloc(oat_method_header.code_size_ + sizeof(oatmethodheader));

        memcpy(code, &oat_method_header, sizeof(oatmethodheader));
        fread(code + sizeof(oatmethodheader), 1, oat_method_header.code_size_, fp);

        fwrite(code, 1, oat_method_header.code_size_ + sizeof(oatmethodheader), out);

        createMehtodHeader(fp, headerinfo, oatdata_offset, oat_method_header, methods_offset[i]);

        printf("code_index : %d, offset : %lx \n", i, ftell(out));

        free(code);
    }
    fclose(out);
    fclose(headerinfo);

    return ret;
}

/**********************************************************************
*Function Name: classParseNoWrite
*Purpose:       在调用此函数前，文件的指针已经指向oat_class的偏移，之后进行类解析，取得此类的OAT方法数
*Params:
				@FILE* fp 文件指针
				@int oatdata_offset oat数据偏移地址
				@unsigned int method_size 这个类中有多少个方法

*Return:		@void
*Limitation:
**********************************************************************/
static  void classParseNoWrite(FILE* fp, int oatdata_offset, unsigned int *method_size)
{
    int i;

    unsigned short status;
    unsigned short type;

    unsigned int bitmap_size;

    char* bitmap;

    int cur;
    cur = ftell(fp);

    fread(&status, 1, sizeof(short), fp);
    fread(&type, 1, sizeof(short), fp);

	//类的类型需要判定，如果类型为1，说明此类不全编译，则按照bitmap进行尺寸判定
    if(type == 1)
    {
        fread(&bitmap_size, 1, sizeof(int), fp);
        bitmap = (char*)malloc(bitmap_size);
        //fseek(fp, bitmap_size, SEEK_CUR);
        fread(bitmap, 1, sizeof(bitmap_size), fp);

        *method_size = 0;
        for(i = 0; i < bitmap_size; i++)
        {
            *method_size += countChar(bitmap[i]);
        }

        free(bitmap);
        //method_size = bitmap_size;
    }
    fseek(fp, cur, SEEK_SET);
    return;
}

/**********************************************************************
*Function Name: getLength
*Purpose:       获取uleb128格式的数据，并按照其长度推移文件
*Params:
				@FILE* fp 文件指针

*Return:		@int uleb128格式数据
*Limitation:
**********************************************************************/
static unsigned int getLength(FILE* fp)
{
    unsigned char cur_byte;
    unsigned int result;

    fread(&cur_byte, 1, sizeof(char), fp);
    result = cur_byte;

    if (result > 0x7f)
    {
        fread(&cur_byte, 1, sizeof(char), fp);

        result = (result & 0x7f) | ((cur_byte & 0x7f) << 7);


        if (cur_byte > 0x7f)
        {
            fread(&cur_byte, 1, sizeof(char), fp);
            result |= (cur_byte & 0x7f) << 14;
            if (cur_byte > 0x7f)
            {
                fread(&cur_byte, 1, sizeof(char), fp);
                result |= (cur_byte & 0x7f) << 21;
                if (cur_byte > 0x7f)
                {
                    fread(&cur_byte, 1, sizeof(char), fp);
                    result |= cur_byte << 28;
                }
            }
        }
    }

    return result;
}

/**********************************************************************
*Function Name: stridxFromType
*Purpose:       dex格式相关，dex的方法与类关联typeid，并通过typeid关联获取字符串表的id(下标)
*Params:
				@unsigned int type type值
				@dexheader dexheader dex文件的头结构体
				@FILE* fp 文件指针
				@int dex_begin dex起始偏移

*Return:		@unsigned int 字符串id(下标)
*Limitation:
**********************************************************************/
static unsigned int stridxFromType(unsigned int type, dexheader dexheader, FILE *fp, int dex_begin)
{
    unsigned int cur;
    unsigned int stridx;

    cur = ftell(fp);

    fseek(fp, dexheader.type_ids_off_ + dex_begin, SEEK_SET);
    fseek(fp, type * sizeof(int), SEEK_CUR);
    fread(&stridx, 1, sizeof(int), fp);

    fseek(fp, cur, SEEK_SET);
    return stridx;
}

/**********************************************************************
*Function Name: dexClassParse
*Purpose:       解析DEX文件
*Params:
				@FILE* fp 文件指针
				@char* classname 类名
				@int dex_begin dex部分的起始位置
				@unsigned int* method_size 方法的尺寸，传入指针，这个变量在这一函数内赋值

*Return:		@int 返回类数组的标识id，即oatclass_array的下标
*Limitation:
**********************************************************************/
static int dexClassParse(FILE* fp, const char* classname, int dex_begin, unsigned int *method_size)
{
    int class_str_idx;

    char* str;
    char** strlist;

    unsigned int data_off;

    int i, j;
    unsigned int cur;

    int item_size;

    dexheader dexheader;

    classdefitem classdefitem;

    int direct_methods_size;
    int virtual_methods_size;

	//读取dex文件的头

    fseek(fp, dex_begin, SEEK_SET);
    fread(&dexheader, 1, sizeof(dexheader), fp);

    printf("string_ids_size %d \n", dexheader.string_ids_size_);
    printf("string_ids_off %x \n", dexheader.string_ids_off_ + dex_begin);

	//读取dex中所有字符串，放在一个strlist中

    fseek(fp, dexheader.string_ids_off_ + dex_begin, SEEK_SET);

    strlist = malloc(dexheader.string_ids_size_ * sizeof(char**));
    for(i = 0; i < dexheader.string_ids_size_; i++)
    {
        fread(&data_off, 1, sizeof(data_off), fp);
        cur = ftell(fp);
        //printf("data_off %x \n", data_off + dex_begin);

        fseek(fp, data_off + dex_begin, SEEK_SET);
        //fread();
        item_size = getLength(fp);

        str = malloc(item_size + 1);
        fread(str, 1, item_size, fp);

        str[item_size] = '\0';

        strlist[i] = str;
        //printf("idx : %d : %s \n", i, strlist[i]);

        //free(str);
        fseek(fp, cur, SEEK_SET);
    }

	//取得dex文件中的类定义位置，并依次对比字符串，当得到与目标相符的字符串时，返回类的下标

    fseek(fp, dexheader.class_defs_off_ + dex_begin, SEEK_SET);
    for(i = 0; i < dexheader.class_defs_size_; i++)
    {
        fread(&classdefitem, 1, sizeof(classdefitem), fp);

        if(strstr(strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)], classname))
        {
            printf("idx : %d : %s \n", i, strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)]);

            fseek(fp, classdefitem.class_data_off_ + dex_begin, SEEK_SET);
            printf("data_off : %x \n", classdefitem.class_data_off_ + dex_begin);

            printf("\nstatic_fields_size : %d \n", getLength(fp));
            printf("instance_fields_size : %d \n", getLength(fp));
            direct_methods_size = getLength(fp);
            printf("direct_methods_size : %d \n", direct_methods_size);
            virtual_methods_size = getLength(fp);
            printf("virtual_methods_size : %d \n\n", virtual_methods_size);

            *method_size = direct_methods_size + virtual_methods_size;

            //释放所有字符串的内存
            for(j = 0; j < dexheader.string_ids_size_; j++)
            {
                if(strlist[j] != NULL)
                    free(strlist[j]);
            }
            free(strlist);
            strlist = NULL;

            class_str_idx = i;
            return class_str_idx;
        }
        //printf("class def : %s \n", strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)]);
    }

    return ERROR;
}

/**********************************************************************
*Function Name: dexMethodParse
*Purpose:       解析DEX方法，输出方法名
*Params:
				@FILE* fp 文件指针
				@int method_id 方法标识号
				@dexheader dex_header dex文件头
				@char** strlist 字符串列表
                @int dex_begin 区分dex或oat

*Return:		@int 无意义
*Limitation:
**********************************************************************/
static int dexMethodParse(FILE* fp, int method_id, dexheader dex_header, char** strlist, int dex_begin)
{
    int cur;

    methodiditem methoditem;

    FILE* namefile = NULL;
    namefile = fopen("namefile", "w");

    cur = ftell(fp);

    fseek(fp, dex_header.method_ids_off_ + method_id * sizeof(methodiditem) + dex_begin, SEEK_SET);

	//dex相关，根据method_item中存放的类标识和方法名标识输出类名和方法名

    fread(&methoditem, 1, sizeof(methodiditem), fp);

    printf("\nclass_name : %s \n", strlist[stridxFromType(methoditem.class_idx, dex_header, fp, dex_begin)]);
    printf("method_name : %s \n\n", strlist[methoditem.name_idx]);

    fprintf(namefile, "%s\n", strlist[stridxFromType(methoditem.class_idx, dex_header, fp, dex_begin)]);
    fclose(namefile);

    fseek(fp, cur, SEEK_SET);

    return 0;
}

/**********************************************************************
*Function Name: printMethodInfo
*Purpose:       解析DEX方法，获取详细信息，以备清零
*Params:
				@FILE* fp 文件指针
				@dexheader dexheader dex头
				@int static_fields_size 静态域个数
				@int instance_fields_size 实例域个数
				@int direct_methods_size 直接方法个数
				@int virtual_methods_size 虚方法个数
				@char** strlist 字符串列表
                @ine dex_begin 区分dex文件与oat文件

*Return:		@int* 数组：[方法条目偏移][代码尺寸][代码偏移][方法条目偏移][代码尺寸][代码偏移]
*Limitation:
**********************************************************************/
static int* printMethodInfo(FILE* fp, dexheader dexheader, int static_fields_size, int instance_fields_size, int direct_methods_size, int virtual_methods_size, char** strlist, int dex_begin)
{
    int cur;
    int method_index = 1;

    int i;
    int last;
    codeitem code_item;

    int* ret;

    dexcodeitem dex_code_item;

    ret = malloc(sizeof(int) * (direct_methods_size + virtual_methods_size) * 3);

	//跳过前两个域，这里取值都使用uleb128格式

    for(i = 0; i < static_fields_size; i++)
    {
        getLength(fp);
        getLength(fp);
    }
    for(i = 0; i < instance_fields_size; i++)
    {
        getLength(fp);
        getLength(fp);
    }

	//输出方法信息，并记录

    last = 0;
    for(i = 0; i < direct_methods_size; i++)
    {
        ret[(method_index - 1) * 3] = ftell(fp);

        code_item.method_idx_diff = getLength(fp);
        code_item.access_flags = getLength(fp);
        code_item.code_off = getLength(fp);

        printf("\n****************************\n");

        printf("method index %d : \n", method_index);
        printf("direct method\n");


        code_item.method_idx_diff += last;
        last = code_item.method_idx_diff;

        dexMethodParse(fp, code_item.method_idx_diff, dexheader, strlist, dex_begin);

        printf("code_item.method_idx_diff : %d\n", code_item.method_idx_diff);
        printf("code_item.access_flags : %x\n", code_item.access_flags);
        printf("code_item.code_off : %x\n", code_item.code_off);

        cur = ftell(fp);
        fseek(fp, code_item.code_off + dex_begin, SEEK_SET);
        fread(&dex_code_item, 1, sizeof(dexcodeitem), fp);
        fseek(fp, cur, SEEK_SET);

        printf("introduction size : %d \n", dex_code_item.insns_size);
        printf("introduction off : %x \n", code_item.code_off);

        ret[(method_index - 1) * 3 + 1] = dex_code_item.insns_size;
        ret[(method_index - 1) * 3 + 2] = code_item.code_off + sizeof(dex_code_item);
        method_index++;

        printf("****************************\n");
    }

    last = 0;
    for(i = 0; i < virtual_methods_size; i++)
    {
        ret[(method_index - 1) * 3] = ftell(fp);

        code_item.method_idx_diff = getLength(fp);
        code_item.access_flags = getLength(fp);
        code_item.code_off = getLength(fp);

        printf("\n****************************\n");

        printf("method index %d : \n", method_index);
        printf("virtual method\n");

        code_item.method_idx_diff += last;
        last = code_item.method_idx_diff;

        dexMethodParse(fp, code_item.method_idx_diff, dexheader, strlist, dex_begin);

        printf("code_item.method_idx_diff : %d\n", code_item.method_idx_diff);
        printf("code_item.access_flags : %x\n", code_item.access_flags);
        printf("code_item.code_off : %x\n", code_item.code_off);

        cur = ftell(fp);
        fseek(fp, code_item.code_off + dex_begin, SEEK_SET);
        fread(&dex_code_item, 1, sizeof(dexcodeitem), fp);
        fseek(fp, cur, SEEK_SET);

        if(code_item.code_off == 0)
            dex_code_item.insns_size = 0;

        printf("introduction size : %d \n", dex_code_item.insns_size);
        printf("introduction off : %x \n", code_item.code_off);

        ret[(method_index - 1) * 3 + 1] = dex_code_item.insns_size;
        ret[(method_index - 1) * 3 + 2] = code_item.code_off + sizeof(dex_code_item);
        method_index++;

        printf("****************************\n");
    }
    //printf("printMethodInfo %p \n", ret);
    return ret;
}

/**********************************************************************
*Function Name: dexClassParseX
*Purpose:       单纯解析DEX文件，根据类名获取dex中的相关方法数组
*Params:
				@FILE* fp 文件指针
				@char* classname 类名
				@unsigned int* method_size 方法的尺寸，传入指针，这个变量在这一函数内赋值
                @int dex_begin 区分是原dex文件还是oat中dex文件

*Return:		@int* 数组：[方法条目偏移][代码尺寸][代码偏移][方法条目偏移][代码尺寸][代码偏移]
*Limitation:
**********************************************************************/
static int* dexClassParseX(FILE* fp, const char* classname, int *method_size, int dex_begin)
{
    int* ret;

    char* str;
    char** strlist;

    unsigned int data_off;

    int i, j;
    unsigned int cur;

    int item_size;

    dexheader dexheader;

    classdefitem classdefitem;

    int static_fields_size;
    int instance_fields_size;
    int direct_methods_size;
    int virtual_methods_size;

    fread(&dexheader, 1, sizeof(dexheader), fp);

    printf("string_ids_size %d \n", dexheader.string_ids_size_);
    printf("string_ids_off %x \n", dexheader.string_ids_off_);

    fseek(fp, dexheader.string_ids_off_ + dex_begin, SEEK_SET);

	//初始化字符列表

    strlist = malloc(dexheader.string_ids_size_ * sizeof(char**));
    for(i = 0; i < dexheader.string_ids_size_; i++)
    //for(i = 0; i < 400; i++)
    {
        fread(&data_off, 1, sizeof(data_off), fp);
        cur = ftell(fp);
        //printf("data_off %x \n", data_off + dex_begin);

        fseek(fp, data_off + dex_begin, SEEK_SET);
        //fread();
        item_size = getLength(fp);

        str = malloc(item_size + 1);
        fread(str, 1, item_size, fp);

        str[item_size] = '\0';

        strlist[i] = str;
        //printf("idx : %d : %s \n", i, strlist[i]);

        //free(str);
        fseek(fp, cur, SEEK_SET);
    }

    fseek(fp, dexheader.class_defs_off_ + dex_begin, SEEK_SET);
    for(i = 0; i < dexheader.class_defs_size_; i++)
    {
        fread(&classdefitem, 1, sizeof(classdefitem), fp);

		//在字符串列表中进行搜索

        if(strstr(strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)], classname))
        {
            printf("idx : %d : %s \n", i, strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)]);

            fseek(fp, classdefitem.class_data_off_ + dex_begin, SEEK_SET);
            printf("data_off : %x \n", classdefitem.class_data_off_);

            static_fields_size = getLength(fp);
            printf("\nstatic_fields_size : %d \n", static_fields_size);
            instance_fields_size = getLength(fp);
            printf("instance_fields_size : %d \n", instance_fields_size);
            direct_methods_size = getLength(fp);
            printf("direct_methods_size : %d \n", direct_methods_size);
            virtual_methods_size = getLength(fp);
            printf("virtual_methods_size : %d \n\n", virtual_methods_size);

            *method_size = direct_methods_size + virtual_methods_size;

            printf("class def : %s \n", strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, dex_begin)]);

			//搜到时，进入进一步解析

            ret = printMethodInfo(fp, dexheader, static_fields_size, instance_fields_size, direct_methods_size, virtual_methods_size, strlist, dex_begin);

            //释放所有字符串的内存
            for(j = 0; j < dexheader.string_ids_size_; j++)
            {
                //printf("X : %d\n", j);
                free(strlist[j]);
            }

            free(strlist);

            //printf("dexClassParseX %p \n", ret);
            return ret;
        }
        //printf("class def : %s \n", strlist[stridxFromType(classdefitem.class_idx_, dexheader, fp, 0)]);
    }

    return NULL;
}

/**********************************************************************
*Function Name: dealDEXFile
*Purpose:       解析DEX文件
*Params:
				@char* classname 类名

*Return:		@int* 返回一个数组，包含某一类的所有方法的代码偏移与尺寸
				数组：[方法条目偏移][代码尺寸][代码偏移][方法条目偏移][代码尺寸][代码偏移]
*Limitation:
**********************************************************************/
int* dealDEXFile(const char* classname)
{
    int* ret;
    FILE *fp;
    int method_size;

    fp = fopen("ori_classes.dex", "rb");

    ret = dexClassParseX(fp, classname, &method_size, 0);

    printf("dealDEXFile %p \n", ret);
    fclose(fp);
    return ret;
}

/**********************************************************************
*Function Name: dealOATDEXFile
*Purpose:       解析OAT中DEX文件
*Params:
				@char* classname 类名

*Return:
*Limitation:
**********************************************************************/
void dealOATDEXFile(const char* classname, FILE* fp, int dex_begin)
{
    int cur;
    int method_size;

    cur = ftell(fp);
    //fp = fopen("ori_classes.dex", "rb");
    fseek(fp, dex_begin, SEEK_SET);

    dexClassParseX(fp, classname, &method_size, dex_begin);

    fseek(fp, cur, SEEK_SET);

    return;
}

/**********************************************************************
*Function Name: dealARTFile
*Purpose:       解析OAT文件
*Params:
				@char* classname 类名

*Return:		@int* 返回一个数组，包含某一类的所有方法的代码偏移与尺寸，最
				后一位为OAT文件中dex的偏移
				数组：[偏移][尺寸][偏移][尺寸]...[dex起始位置]
*Limitation:
**********************************************************************/
int* dealARTFile(const char* classname)
{
    int i, count;
    long file_offset;

    FILE* pf_base;

    elfheader elf_header;
    sectionheader section_header;
    symentry sym_entry;

    oatheader oat_header;

    unsigned int oatdata_offset;

    /*unsigned int oatdata_size;
    unsigned int oatexec_offset;
    unsigned int oatexec_size;
    unsigned int dex_offset;
    oatdexheader oat_dex_header;*/

    int class_size;

    int *oat_class_array;
    int target_class;

    int dex_file_pointer;
    int dex_length;

    unsigned int dex_begin;

    unsigned int method_size;

    int* ret;

    pf_base = fopen(ORI_FILE, "rb");

    //第一步，因OAT文件为ELF，首先需要解析ELF格式

    fread(&elf_header, 1, sizeof(elf_header), pf_base);

    printf("ehsize %d\n", elf_header.e_shoff);
    printf("section type : %d\n", elf_header.e_shstrndx);

    file_offset = elf_header.e_shoff;
    fseek(pf_base, file_offset, SEEK_SET);

	//判断section_header的类型，取到动态连接块类型，因为 oatdata 和 oatexec 的位置是放在这里的
    for(i = 0; i < elf_header.e_shnum; i++)
    {
        fread(&section_header, 1, sizeof(section_header), pf_base);
        /*printf("section type : %d\n", section_header.sh_type);
        printf("section offset : %x\n", section_header.sh_offset);
        printf("section offset : %x\n", section_header.sh_size);*/
        if(section_header.sh_type == SYM_TABLE)
            break;
    }

	//取得符号表单位，第一个section entry的值为 oatdata 的偏移, 第二个section entry的值为 oatexec 的偏移, 实际上我们并不需要oat_exec，存放的都是运行代码

    fseek(pf_base, section_header.sh_offset, SEEK_SET);
    count = section_header.sh_size / sizeof(sym_entry);
    for(i = 0; i < count; i++)
    {
        fread(&sym_entry, 1, sizeof(sym_entry), pf_base);
        //printf("entry offset : %x\n", sym_entry.st_value);
        //printf("entry str : %x\n", sym_entry.st_name);
        switch (i)
        {
            //1st section is OAT_DATA
            case OAT_DATA:
                oatdata_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
            //2nd seciton is OAT_EXEC
            case OAT_EXEC:
                //oatexec_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
        }
    }

	//开始处理 oatdata, 首先跳过 oatdata部分的头，取得 dex文件的长度和位置

    fseek(pf_base, oatdata_offset, SEEK_SET);
    fread(&oat_header, 1, sizeof(oat_header), pf_base);

    //printf("key_value_store_ %d \n", oat_header.key_value_store_size_);

    fseek(pf_base, oat_header.key_value_store_size_, SEEK_CUR);

	//取得 dex 长度、位置
    fread(&dex_length, 1, sizeof(int), pf_base);
    fseek(pf_base, dex_length + sizeof(int), SEEK_CUR);
    fread(&dex_file_pointer, 1, sizeof(int), pf_base);
    printf("dex_file_pointer %x \n", dex_file_pointer);

	//这里取得的偏移是相对于 oatdata_offset 的
    dex_begin = dex_file_pointer + oatdata_offset;

    //因为类信息存放在oat中的一个类的数组里，首先取得oat的数组长度，并分配内存，读进此数组
    class_size = getClassCount(pf_base, oatdata_offset, dex_file_pointer);

    printf("class_size %d \n", class_size);

    oat_class_array = malloc(class_size * sizeof(int));
    fread(oat_class_array, sizeof(int), class_size, pf_base);
	//oat_class_array中的下标标识了每个类条目的偏移信息，所以需要通过类名得到这一下标，这个下标是存放在dex部分的，所以需要dexClassParse函数进行解析，得到对应的下标值
    target_class = dexClassParse(pf_base, classname, dex_begin, &method_size);

    printf("target_class : %d \n", target_class);

    if(target_class == ERROR)
    {
        printf("class not found \n");
        return NULL;
    }

    //输出方法名
    dealOATDEXFile(classname, pf_base, dex_begin);

	//跳转到此类偏移地址，并使用classParse进行详细代码尺寸与偏移的获取
    fseek(pf_base, oat_class_array[target_class] + oatdata_offset, SEEK_SET);
    ret = classParse(pf_base, oatdata_offset, &method_size);

    free(oat_class_array);
	//将dex_begin放到折返数组的最后，返回
	printf("method size .... %d\n", method_size);
    ret[method_size * 2] = dex_begin;

    fclose(pf_base);

    return ret;
}

/**********************************************************************
*Function Name: getMethodSize
*Purpose:       得到需求类的方法个数
*Params:
				@char* classname 类名

*Return:		@int 返回对应类的方法个数

*Limitation:
**********************************************************************/
int getMethodSize(const char* classname)
{
    int i, count;
    long file_offset;

    FILE* pf_base;

    elfheader elf_header;
    sectionheader section_header;
    symentry sym_entry;

    oatheader oat_header;

    unsigned int oatdata_offset;
    //unsigned int oatdata_size;
    //unsigned int oatexec_offset;
    //unsigned int oatexec_size;

    //unsigned int dex_offset;

    //oatdexheader oat_dex_header;

    int class_size;

    int *oat_class_array;
    int target_class;


    int dex_file_pointer;
    int dex_length;

    unsigned int dex_begin;

    unsigned int method_size;

    pf_base = fopen(ORI_FILE, "rb");

    //at first, we need to analyze the elf file
    //to find oatdata and oatexec

    fread(&elf_header, 1, sizeof(elf_header), pf_base);

    //printf("ehsize %d\n", elf_header.e_shoff);
    //printf("section type : %d\n", elf_header.e_shstrndx);

    file_offset = elf_header.e_shoff;
    fseek(pf_base, file_offset, SEEK_SET);

    for(i = 0; i < elf_header.e_shnum; i++)
    {
        fread(&section_header, 1, sizeof(section_header), pf_base);
        //printf("section type : %d\n", section_header.sh_type);
        //printf("section offset : %x\n", section_header.sh_offset);
        //printf("section offset : %x\n", section_header.sh_size);
        if(section_header.sh_type == SYM_TABLE)
            break;
    }

    fseek(pf_base, section_header.sh_offset, SEEK_SET);
    count = section_header.sh_size / sizeof(sym_entry);
    for(i = 0; i < count; i++)
    {
        fread(&sym_entry, 1, sizeof(sym_entry), pf_base);
        //printf("entry offset : %x\n", sym_entry.st_value);
        //printf("entry str : %x\n", sym_entry.st_name);
        switch (i)
        {
            //1st section is OAT_DATA
            case OAT_DATA:
                oatdata_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
            //2nd seciton is OAT_EXEC
            case OAT_EXEC:
                //oatexec_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
        }
    }

    fseek(pf_base, oatdata_offset, SEEK_SET);
    fread(&oat_header, 1, sizeof(oat_header), pf_base);

    //printf("key_value_store_ %d \n", oat_header.key_value_store_size_);

    fseek(pf_base, oat_header.key_value_store_size_, SEEK_CUR);

    fread(&dex_length, 1, sizeof(int), pf_base);
    fseek(pf_base, dex_length + sizeof(int), SEEK_CUR);
    fread(&dex_file_pointer, 1, sizeof(int), pf_base);
    //printf("dex_file_pointer %x \n", dex_file_pointer);

    dex_begin = dex_file_pointer + oatdata_offset;

    //get class count
    class_size = getClassCount(pf_base, oatdata_offset, dex_file_pointer);

    //printf("class_size %d \n", class_size);

    oat_class_array = malloc(class_size * sizeof(int));
    fread(oat_class_array, sizeof(int), class_size, pf_base);

    target_class = dexClassParse(pf_base, classname, dex_begin, &method_size);

    //printf("target_class : %d \n", target_class);

    if(target_class == ERROR)
    {
        printf("class not found \n");
        return 0;
    }

	//跳转到此类偏移地址，并使用classParse进行详细代码尺寸与偏移的获取
    fseek(pf_base, oat_class_array[target_class] + oatdata_offset, SEEK_SET);
    classParseNoWrite(pf_base, oatdata_offset, &method_size);

	//前同dealARTFile, 最后返回值变为method_size

    free(oat_class_array);
    if(target_class == ERROR)
    {
        printf("class not found \n");
        return -1;
    }

    fclose(pf_base);

    return method_size;
}

/**********************************************************************
*Function Name: getMethodSizeDEX
*Purpose:       得到需求类的DEX方法个数
*Params:
				@char* classname 类名

*Return:		@int 返回对应类的方法个数

*Limitation:
**********************************************************************/
int getMethodSizeDEX(const char* classname)
{
    int i, count;
    long file_offset;

    FILE* pf_base;

    elfheader elf_header;
    sectionheader section_header;
    symentry sym_entry;

    oatheader oat_header;

    unsigned int oatdata_offset;

    int class_size;

    int *oat_class_array;
    int target_class;


    int dex_file_pointer;
    int dex_length;

    unsigned int dex_begin;

    unsigned int method_size;

    pf_base = fopen(ORI_FILE, "rb");

    //at first, we need to analyze the elf file
    //to find oatdata and oatexec

    fread(&elf_header, 1, sizeof(elf_header), pf_base);

    //printf("ehsize %d\n", elf_header.e_shoff);
    //printf("section type : %d\n", elf_header.e_shstrndx);

    file_offset = elf_header.e_shoff;
    fseek(pf_base, file_offset, SEEK_SET);

    for(i = 0; i < elf_header.e_shnum; i++)
    {
        fread(&section_header, 1, sizeof(section_header), pf_base);
        //printf("section type : %d\n", section_header.sh_type);
        //printf("section offset : %x\n", section_header.sh_offset);
        //printf("section offset : %x\n", section_header.sh_size);
        if(section_header.sh_type == SYM_TABLE)
            break;
    }

    fseek(pf_base, section_header.sh_offset, SEEK_SET);
    count = section_header.sh_size / sizeof(sym_entry);
    for(i = 0; i < count; i++)
    {
        fread(&sym_entry, 1, sizeof(sym_entry), pf_base);
        //printf("entry offset : %x\n", sym_entry.st_value);
        //printf("entry str : %x\n", sym_entry.st_name);
        switch (i)
        {
            //1st section is OAT_DATA
            case OAT_DATA:
                oatdata_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
            //2nd seciton is OAT_EXEC
            case OAT_EXEC:
                //oatexec_offset = sym_entry.st_value;
                //oatdata_size = sym_entry.st_size;
                break;
        }
    }

    fseek(pf_base, oatdata_offset, SEEK_SET);
    fread(&oat_header, 1, sizeof(oat_header), pf_base);

    //printf("key_value_store_ %d \n", oat_header.key_value_store_size_);

    fseek(pf_base, oat_header.key_value_store_size_, SEEK_CUR);

    fread(&dex_length, 1, sizeof(int), pf_base);
    fseek(pf_base, dex_length + sizeof(int), SEEK_CUR);
    fread(&dex_file_pointer, 1, sizeof(int), pf_base);
    //printf("dex_file_pointer %x \n", dex_file_pointer);

    dex_begin = dex_file_pointer + oatdata_offset;

    //get class count
    class_size = getClassCount(pf_base, oatdata_offset, dex_file_pointer);

    //printf("class_size %d \n", class_size);

    oat_class_array = malloc(class_size * sizeof(int));
    fread(oat_class_array, sizeof(int), class_size, pf_base);

    target_class = dexClassParse(pf_base, classname, dex_begin, &method_size);

    if(target_class == ERROR)
    {
        printf("class not found \n");
        return 0;
    }

	//前同dealARTFile, 最后返回值变为method_size

    free(oat_class_array);
    fclose(pf_base);

    return method_size;
}

/**********************************************************************
*Function Name: getZipDexCRC
*Purpose:       得到zip文件中classes.dex的crc校验码
*Params:

*Return:		@int 返回zip文件中classes.dex的crc校验码

*Limitation:
**********************************************************************/
unsigned int getZipDexCRC()
{
    const char* filename = "./arthack_unpack/dist/base.apk";
    FILE* fp;
    zipentry entry;
    char* entryname;

    fp = fopen(filename, "rb");

    while(fread(&entry, 1, sizeof(zipentry), fp))
    {
        entryname = malloc(entry.frfilenamelength * sizeof(char));

        fread(entryname, 1, entry.frfilenamelength, fp);

        entryname[entry.frfilenamelength] = '\0';
        printf("%s \n", entryname);

        if(!strcmp(entryname, "classes.dex"))
        {
            fclose(fp);
            printf("crc32 : %x \n", entry.frcrc);
            free(entryname);
            return entry.frcrc;
        }
        else
        {
            free(entryname);
            fseek(fp, entry.frcompressedsize, SEEK_CUR);
        }
    }

    return 0;
}

/**********************************************************************
*Function Name: getOatCrcOffset
*Purpose:       得到oat文件中classes.dex的crc校验码偏移位置
*Params:

*Return:		@int 返回oat文件中classes.dex的crc校验码偏移位置

*Limitation:
**********************************************************************/
unsigned int getOatCrcOffset()
{
    int i, count;
    long file_offset;

    FILE* pf_base;

    elfheader elf_header;
    sectionheader section_header;
    symentry sym_entry;

    oatheader oat_header;

    unsigned int oatdata_offset;
    int dex_length;
    int crc_offset;

    pf_base = fopen(ORI_FILE, "rb");

    //at first, we need to analyze the elf file
    //to find oatdata and oatexec

    fread(&elf_header, 1, sizeof(elf_header), pf_base);

    file_offset = elf_header.e_shoff;
    fseek(pf_base, file_offset, SEEK_SET);

    for(i = 0; i < elf_header.e_shnum; i++)
    {
        fread(&section_header, 1, sizeof(section_header), pf_base);
        if(section_header.sh_type == SYM_TABLE)
            break;
    }

    fseek(pf_base, section_header.sh_offset, SEEK_SET);
    count = section_header.sh_size / sizeof(sym_entry);
    for(i = 0; i < count; i++)
    {
        fread(&sym_entry, 1, sizeof(sym_entry), pf_base);
        switch (i)
        {
            //1st section is OAT_DATA
            case OAT_DATA:
                oatdata_offset = sym_entry.st_value;
                break;
            //2nd seciton is OAT_EXEC
            case OAT_EXEC:
                break;
        }
    }

    fseek(pf_base, oatdata_offset, SEEK_SET);
    fread(&oat_header, 1, sizeof(oat_header), pf_base);

    fseek(pf_base, oat_header.key_value_store_size_, SEEK_CUR);

    fread(&dex_length, 1, sizeof(int), pf_base);
    fseek(pf_base, dex_length, SEEK_CUR);
    crc_offset = ftell(pf_base);
    fclose(pf_base);

    return crc_offset;
}

int countingMaping(FILE* oatfile)
{
    int total_size;
    int pc_to_dex_size;
    int dex_to_pc_size;
    int i;
    int begin, end;

    begin = ftell(oatfile);

    total_size = getLength(oatfile);
    pc_to_dex_size = getLength(oatfile);
    for(i = 0; i < pc_to_dex_size; i++)
    {
        getLength(oatfile);
        getLength(oatfile);
    }

    dex_to_pc_size = total_size - pc_to_dex_size;
    for(i = 0; i < dex_to_pc_size; i++)
    {
        getLength(oatfile);
        getLength(oatfile);
    }
    end = ftell(oatfile);
    fseek(oatfile, begin, SEEK_SET);

    return end - begin;

}

int countingVmap(FILE* oatfile)
{
    int total_size;

    int i;
    int begin, end;

    begin = ftell(oatfile);

    total_size = getLength(oatfile);

    for(i = 0; i < total_size; i++)
    {
        getLength(oatfile);
    }
    end = ftell(oatfile);
    fseek(oatfile, begin, SEEK_SET);

    return end - begin;
}

void createMehtodHeader(FILE* oatfile, FILE* methodhaeder, int oatdata_offset, oatmethodheader oat_method_header, int method_offset)
{
    int file_ptr;
    int mapping_size;
    int vmap_size;

    char* mapping_table;
    char* vmap_table;

    file_ptr = ftell(oatfile);

    fwrite(&(oat_method_header.mapping_table_offset_), 1, sizeof(oat_method_header.mapping_table_offset_), methodhaeder);
    fwrite(&(oat_method_header.vmap_table_offset_), 1, sizeof(oat_method_header.vmap_table_offset_), methodhaeder);
    fwrite(&(oat_method_header.frame_info_frame_size_inbyte), 1, sizeof(oat_method_header.frame_info_frame_size_inbyte), methodhaeder);
    fwrite(&(oat_method_header.frame_info_core_spill_mask), 1, sizeof(oat_method_header.frame_info_core_spill_mask), methodhaeder);
    fwrite(&(oat_method_header.frame_info_fp_spill_mask), 1, sizeof(oat_method_header.frame_info_fp_spill_mask), methodhaeder);


    fseek(oatfile, method_offset + oatdata_offset - oat_method_header.mapping_table_offset_, SEEK_SET);
    mapping_size = countingMaping(oatfile);

    printf("mapping_offset : %x\n", method_offset + oatdata_offset - oat_method_header.mapping_table_offset_);
    printf("mapping_size : %d\n", mapping_size);
    fseek(oatfile, method_offset + oatdata_offset - oat_method_header.mapping_table_offset_, SEEK_SET);
    mapping_table = (char*)malloc(mapping_size);
    fread(mapping_table, 1, mapping_size, oatfile);

    fwrite(&mapping_size, 1, sizeof(int), methodhaeder);
    fwrite(mapping_table, 1, mapping_size, methodhaeder);
    free(mapping_table);

    fseek(oatfile, method_offset + oatdata_offset - oat_method_header.vmap_table_offset_, SEEK_SET);
    vmap_size = countingVmap(oatfile);

    printf("vmap_offset : %x\n", method_offset + oatdata_offset - oat_method_header.vmap_table_offset_);
    printf("vmap_size : %d\n", vmap_size);
    fseek(oatfile, method_offset + oatdata_offset - oat_method_header.vmap_table_offset_, SEEK_SET);
    vmap_table = (char*)malloc(vmap_size);
    fread(vmap_table, 1, vmap_size, oatfile);
    fwrite(&vmap_size, 1, sizeof(int), methodhaeder);
    fwrite(vmap_table, 1, vmap_size, methodhaeder);
    free(vmap_table);

    fseek(oatfile, file_ptr, SEEK_SET);
    printf("=======================================\n");
    return;
}

unsigned int cryptfile(char* pwd, char* file)
{
    unsigned char* buf;
    int len;
    FILE* fp;
    int blocks;

    printf("*****AES Crypt*****\n");

    fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    printf("len %d\n", len);

    blocks = (len % 512) == 0 ? len / 512 : len / 512 + 1;

    printf("blocks : %d\n", blocks);

    buf = (unsigned char*)malloc(sizeof(char) * 512 * blocks);
    memset(buf, 0, sizeof(char) * 512 * blocks);
    //while(1);
    fread(buf, 1, len, fp);

	fclose(fp);

    printf("pwd : %s\n", pwd);
    init_aes(pwd);
    TFFS_ENCRYPT(buf, 123, blocks, 0, 1);

    printf("crypt\n");

    fp = fopen(file, "wb");
    fwrite(buf, 1, sizeof(char) * 512 * blocks, fp);

	fclose(fp);
	free(buf);
	return 0;
}

#ifndef LIBFLAG
int main()
{
    //getMethodSize("FileSearchActivity;");
    dealARTFile("RecordAct;");

    //getMethodSize("RecordAct;");
    //dealDEXFile("RecordAct;");

    return 0;
}
#endif
