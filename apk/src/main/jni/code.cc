//
// Created by Administrator on 2015/12/1.
//
#include "com_example_tomek_notepad_NoteActivity.h"
#include <cstdio>
#include <cstdlib>
#include <android/log.h>
#include <sys/mman.h>
#include<string.h>
#include <unistd.h>
//#include "art_method.h"
#include <dlfcn.h>
#include "crypt_aes.h"

#define METHOD_MEM_COUNT 30

#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "ARTHACK", __VA_ARGS__)
//#define LOGI(...)
#define MEMDUMP 1
#define ALIGN_MEM(ptr) ((((unsigned long)ptr)) & 0xfffff000)

#define METHOD_ACCESS_FLAG(m) m + 5
#define METHOD_QUICK_ENTRY(m) m + 11
#define METHOD_DEX_ITEM(m) m + 6

#define QUICK_FLAG 0x80001

#define PAGE_SIZE 0x1000

#define MY_LIB "/data/app/com.tomaszmarzeion.notepad-1/lib/arm/libJniTest.so"
#define MY_CLASS "com/example/tomek/notepad/CameraActivity"

char mypassword[255] = {0};

static void logOut(const char* func,char** method)
{
    for(int i = 0; i < METHOD_MEM_COUNT; i++)
        LOGI("%s : %d offset: %x", func, i, *(method + i));
}

static int countZero(int index, char* bitmap, int bitmap_size)
{
    int i = 0, j;
    int ret = 0;
    int count = 1;
    for(i = 0; i < bitmap_size; i++)
    {
        for(j = 0; j < 8; j++)
        {
            if((bitmap[i] & 0x01) == 0)
                ret++;
            if(count == index)
                return ret;
            count++;

            //LOGI("ret %d bitbyte %02x", ret, bitmap[i]);

            bitmap[i] >>= 1;
        }
    }
}

//index from 1
static void loadMethodbyIndex(int index, FILE* fptr, int* size, const char* classname, codepack* pack)
{
    unsigned char* buf;
    char* loadclassname;
    int len, classnamelen;
    int blocks;
    int nowpointer = 0;
    int codebegin;
    int ori_index = index;

    oatmethodheader oat_method_header;
    int i;

    fseek(fptr, -8, SEEK_END);
    fread(&codebegin, 1, sizeof(codebegin), fptr);
    fseek(fptr, codebegin, SEEK_SET);

    while(true)
    {
        int ret = fread(&len, 1, sizeof(len), fptr);
        if(ret == 0) {
            LOGI("read nothing\n");
            return;
        }
        fread(&classnamelen, 1, sizeof(classnamelen), fptr);

        loadclassname = (char*)malloc(classnamelen + 1);
        fread(loadclassname, 1, classnamelen, fptr);
        loadclassname[classnamelen] = '\0';

        LOGI("the classname : %s\n", loadclassname);
        if(strcmp(loadclassname, classname) == 0)
        {
            free(loadclassname);
            break;
        }
        else
        {
            int dexmethodlen;
            fseek(fptr, len, SEEK_CUR);
            fread(&dexmethodlen, 1, sizeof(dexmethodlen), fptr);
            fseek(fptr, dexmethodlen, SEEK_CUR);
            free(loadclassname);
        }
    }

    LOGI("file len : %d\n", len);

    //get buffer
    blocks = (len % 512) == 0 ? len / 512 : len / 512 + 1;

    buf =(unsigned char*) malloc(sizeof(char) * 512 * blocks);
    memset(buf, 0, sizeof(char) * 512 * blocks);

    fread(buf, 1, len, fptr);

    LOGI("pwd : %s\n", mypassword);

    init_aes((unsigned char*)mypassword);
    TFFS_ENCRYPT(buf, 123, blocks, 0, 0);
    //get buffer

    //deal the bitmap if has
    short type;
    memcpy(&type, buf, sizeof(short));
    LOGI("type : %d\n", type);
    if(type == 0)
    {
        nowpointer += 4;
    }
    else if(type == 1)
    {
        int bitmap_size;
        memcpy(&bitmap_size, buf + 2, sizeof(int));

        LOGI("bitmap_size : %d\n", bitmap_size);

        char* bitmap = (char*)malloc(bitmap_size);
        memcpy(bitmap, buf + 6, bitmap_size);
        nowpointer = nowpointer + 2 + 4 + bitmap_size;
        LOGI("bre index : %d\n", index);
        index = index - countZero(index, bitmap, bitmap_size);
        LOGI("aft index : %d\n", index);
        free(bitmap);
    }

    oat_method_header.code_size_ = 0;
    for(i = 0; i < index; i++)
    {
        //fseek(fptr, oat_method_header.code_size_, SEEK_CUR);
        //fread(&oat_method_header, 1, sizeof(oatmethodheader), fptr);
        nowpointer = nowpointer + oat_method_header.code_size_;
        memcpy(&oat_method_header, buf + nowpointer, sizeof(oatmethodheader));
        nowpointer += sizeof(oatmethodheader);
    }
    //char* p = (char*)malloc(oat_method_header.code_size_ + 0xfff);
    char* code = (char*)malloc(oat_method_header.code_size_);
    //mprotect(code, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);

    //fread(code, 1, oat_method_header.code_size_, fptr);
    memcpy(code, buf + nowpointer, oat_method_header.code_size_ );

    *size = oat_method_header.code_size_;

    free(buf);

    pack->oatcode = code;

    //get dex code
    int dexmethodlen;
    fread(&dexmethodlen, 1, sizeof(dexmethodlen), fptr);

    LOGI("dex method len %d \n", dexmethodlen);

    blocks = (dexmethodlen % 512) == 0 ? dexmethodlen / 512 : dexmethodlen / 512 + 1;

    buf =(unsigned char*) malloc(sizeof(char) * 512 * blocks);
    memset(buf, 0, sizeof(char) * 512 * blocks);
    fread(buf, 1, dexmethodlen, fptr);

    //init_aes((unsigned char*)mypassword);
    TFFS_ENCRYPT(buf, 123, blocks, 0, 0);

    nowpointer = 0;
    while(true)
    {
        int code_size;
        int code_index;
        memcpy(&code_size, buf + nowpointer, sizeof(code_size));
        memcpy(&code_index, buf + nowpointer + 4, sizeof(code_index));

        LOGI("code_size : %d\n", code_size);
        LOGI("code index : %d\n", code_index);

        if(code_index == ori_index)
        {
            char* dexcode = (char*)malloc(code_size);
            memcpy(dexcode, buf + nowpointer + 8, code_size);
            pack->dexcode = dexcode;
            pack->dexcode_size = code_size;
            free(buf);
            break;
        }
        nowpointer = nowpointer + 8 + code_size;
    }

    int headerlen;
    fread(&headerlen, 1, 4, fptr);

    oatmethodheader2 my_oatmethodheader;
    int mapping_size;
    int vmap_size;
    for(i = 0; i < index - 1; i++)
    {
        fread(&my_oatmethodheader, 1, sizeof(oatmethodheader2),fptr);
        fread(&mapping_size, 1, sizeof(int), fptr);
        fseek(fptr, mapping_size, SEEK_CUR);
        fread(&vmap_size, 1, sizeof(int), fptr);
        fseek(fptr, vmap_size, SEEK_CUR);
    }

    fread(&my_oatmethodheader, 1, sizeof(oatmethodheader2),fptr);
    fread(&mapping_size, 1, sizeof(int), fptr);
    pack->maping_size = mapping_size;
    pack->maping_table = (char*)malloc(mapping_size);
    fread(pack->maping_table, 1, mapping_size, fptr);

    LOGI("frame_info_frame_size_inbyte : %d\n", my_oatmethodheader.frame_info_frame_size_inbyte);
    LOGI("frame_info_core_spill_mask : %x\n", my_oatmethodheader.frame_info_core_spill_mask);
    LOGI("mapping size : %d\n", mapping_size);

    fread(&vmap_size, 1, sizeof(int), fptr);
    pack->vmap_size = vmap_size;
    pack->vmap_table = (char*)malloc(vmap_size);
    fread(pack->vmap_table, 1, vmap_size, fptr);
    LOGI("vmap_size : %d\n", vmap_size);
    pack->header2 = my_oatmethodheader;

    return;
}

void* getDexBase(char** ori_method)
{
    char** classptr = (char**)(*(ori_method + 2));
    //logOut("the class", classptr);
    char** cache = (char**)(*(classptr + 4));
    //logOut("the cache", cache);
    char** dex = (char**)(*(cache + 8));
    //logOut("the cache", dex);
    char** base = (char**)(*(dex + 1));
    //offset is 4 * sizeof(int) = 16
    base = base + 4;
    return (void*)base;
}

void dumpMem(const char* filename, void* mem)
{
    FILE* fp;

    if(MEMDUMP)
    {
        fp = fopen(filename,"wb");
        if (fp)
        {
            fwrite(mem, 0x2000, 1, fp);
            fclose(fp);
        }
    }

}

void* searchHeader(char* base)
{
    void* ret;
    while(1)
    {
        base--;
        unsigned int* temp = (unsigned int*)base;
        if((*temp) == 0x00353400 && (*(temp - 1)) == 0x74616f)
        {
            ret = temp - 1;
            break;
        }
    }
    return ret;
}

uint32_t returnImm(uint32_t instr)
{
    uint32_t i = (instr >> 26) & 1;
    uint32_t imm3 = (instr >> 12) & 0x7;
    uint32_t imm8 = instr & 0xFF;
    uint32_t Rn = (instr >> 16) & 0xF;
    uint32_t imm16 = (Rn << 12) | (i << 11) | (imm3 << 8) | imm8;
    return imm16;
}

void dealBootConstant(char* code, int code_size, int offset, uint32_t begin)
{
    unsigned int instr1;
    unsigned int instr2;
    int i;

    uint32_t high = begin >> 16;

    for(i = 0; i < code_size; i++)
    {
        instr1 = *((uint32_t*)code);
        instr2 = *((uint32_t*)(code + 4));

        if(((instr1 >> 20) & 0x1F) == 0x04 && ((instr2 >> 20) & 0x1F) == 0x0C)
        {
            uint32_t now_high = returnImm(instr2);
            uint32_t now_low = returnImm(instr1);
            uint32_t now = (now_high << 16) & now_low;

            if(now_high >= high)
            {
                now += offset;
                uint16_t immhigh = now >> 16;
                uint16_t immlow = now & 0x0000FFFF;
                instr2 = instr2 | ((immhigh >> 12) << 16) | (immhigh & 0xfff);
                instr1 = instr1 | ((immlow >> 12) << 16) | (immlow & 0xfff);
            }
        }
        else
        {
            instr2 = *((uint32_t*)(code + 6));
            if(((instr1 >> 20) & 0x1F) == 0x04 && ((instr2 >> 20) & 0x1F) == 0x0C)
            {
                uint32_t now_high = returnImm(instr2);
                uint32_t now_low = returnImm(instr1);
                uint32_t now = (now_high << 16) & now_low;

                if(now_high >= high)
                {
                    now += offset;
                    uint16_t immhigh = now >> 16;
                    uint16_t immlow = now & 0x0000FFFF;
                    instr2 = instr2 | ((immhigh >> 12) << 16) | (immhigh & 0xfff);
                    instr1 = instr1 | ((immlow >> 12) << 16) | (immlow & 0xfff);
                }
            }
        }

        code++;
    }
}

void recoveryMethod(JNIEnv * env, const char* methodname, const char* sign, int index, const char* classname, const char* shortclassname)
{
    char** method;
    char** ori_method;
    FILE* fptr;
    char* code;
    unsigned int code_size;

    jmethodID methid;
    jmethodID orimethid;
    jclass myclass;

    int size;

    char buf[80];
    getcwd(buf, sizeof(buf));

    fptr = fopen(MY_LIB, "rb");

    myclass = env->FindClass(classname);
    if(myclass == NULL)
    {
        LOGI("Can not find class\n");
        return;
    }

    //orimethid = env->GetMethodID(myclass, "takePhoto", "()V");
    orimethid = env->GetMethodID(myclass, methodname, sign);

    //method = (char**)methid;
    ori_method = (char**)orimethid;

    //memcpy(method, ori_method, 14 * sizeof(int));

    logOut("the Method", ori_method);

    void* base = getDexBase(ori_method);
    void* codeitem = base + (int)(*(METHOD_DEX_ITEM(ori_method)));

    //we need correct boot.art offset
    uint32_t ori_image_begin;
    fseek(fptr, -4, SEEK_END);
    fread(&ori_image_begin, 1, sizeof(int), fptr);
    oatheader* oathead = (oatheader*)searchHeader((char*)base);
    unsigned int iamge_file_location_oat_data_begin = oathead->image_file_location_oat_data_begin_;
    int offset = iamge_file_location_oat_data_begin - ori_image_begin;

    codepack pack;
    //load method code into memory buffer
    loadMethodbyIndex(index, fptr, &size, shortclassname, &pack);

    dealBootConstant(pack.oatcode, size, offset, ori_image_begin);

    code = pack.oatcode;

    dumpMem("/storage/sdcard0/oat_bre", *(METHOD_QUICK_ENTRY(ori_method)));
    //change memory access mode
    mprotect((char *)ALIGN_MEM(*(METHOD_QUICK_ENTRY(ori_method)) - 0x100), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);

    int thumbflag = 0;
    int quick_entry = (int)(*(METHOD_QUICK_ENTRY(ori_method)));
    if(quick_entry & 0x1 == 1)
    {
        thumbflag = 1;
    }
    memcpy(*(METHOD_QUICK_ENTRY(ori_method)) - thumbflag - 4, &size, 4);
    //mark here
    memcpy(*(METHOD_QUICK_ENTRY(ori_method)) - thumbflag, code, size);

    dumpMem("/storage/sdcard0/oat_aft", *(METHOD_QUICK_ENTRY(ori_method)));
    dumpMem("/storage/sdcard0/dexbase_bre", codeitem);

    //recovery dex code
    mprotect((char *)ALIGN_MEM(codeitem), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(codeitem, pack.dexcode, pack.dexcode_size);


    dumpMem("/storage/sdcard0/dexbase_aft", codeitem);

    LOGI("enterr");
    char* entry = *(METHOD_QUICK_ENTRY(ori_method));
    char* temp = entry - sizeof(oatmethodheader) - thumbflag;
    oatmethodheader* header = (oatmethodheader*)temp;
    dumpMem("/storage/sdcard0/header_bre", temp);

    LOGI("d core : %x", header->frame_info_core_spill_mask);
    LOGI("d fp : %x", header->frame_info_fp_spill_mask);
    LOGI("d size : %d", header->frame_info_frame_size_inbyte);
    mprotect((char *)ALIGN_MEM(temp), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    header->frame_info_core_spill_mask = pack.header2.frame_info_core_spill_mask;
    header->frame_info_fp_spill_mask = pack.header2.frame_info_fp_spill_mask;
    header->frame_info_frame_size_inbyte = pack.header2.frame_info_frame_size_inbyte;

    dumpMem("/storage/sdcard0/header_aft", temp);
    LOGI("core : %x", header->frame_info_core_spill_mask);
    LOGI("fp : %x", header->frame_info_fp_spill_mask);
    LOGI("size : %d", header->frame_info_frame_size_inbyte);

    char* mapping_offset = entry - header->mapping_table_offset_ - thumbflag;
    char* vmap_offset = entry - header->vmap_table_offset_ - thumbflag;

    dumpMem("/storage/sdcard0/mapping_bre", mapping_offset);
    mprotect((char *)ALIGN_MEM(mapping_offset), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(mapping_offset, pack.maping_table, pack.maping_size);

    dumpMem("/storage/sdcard0/mapping_aft", mapping_offset);

    dumpMem("/storage/sdcard0/vmap_bre", vmap_offset);
    mprotect((char *)ALIGN_MEM(vmap_offset), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
    memcpy(vmap_offset, pack.vmap_table, pack.vmap_size);

    dumpMem("/storage/sdcard0/vmap_aft", vmap_offset);

    free(pack.maping_table);
    free(pack.vmap_table);
    free(pack.dexcode);
    free(code);

}

JNIEXPORT jstring JNICALL Java_com_example_tomek_notepad_NoteActivity_init(JNIEnv * env, jobject jObj)
{
    //recoveryMethod(env, "initCamera", "()V", 6, "com/example/tomek/notepad/CameraActivity", "CameraActivity;");
    //recoveryMethod(env, "SaveData", "(Landroid/graphics/Bitmap;)V", 2, "com/example/tomek/notepad/CameraActivity", "CameraActivity;");

    recoveryMethod(env, "record", "()V", 10, "com/example/tomek/notepad/RecordAct", "RecordAct;");
    //recoveryMethod(env, "stopRecord", "()V", 11, "com/example/tomek/notepad/RecordAct", "RecordAct;");

    //recoveryMethod(env, "startRecord", "()V", 4, "com/example/tomek/notepad/CameraThread", "CameraThread;");
    //recoveryMethod(env, "stopRecord", "()V", 5, "com/example/tomek/notepad/CameraThread", "CameraThread;");

    return env->NewStringUTF("success!");
}

JNIEXPORT jstring JNICALL Java_com_example_tomek_notepad_NoteActivity_initPwd(JNIEnv * env, jobject jObj, jstring pwd)
{
    //char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(pwd, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0)
    {
        //rtn = (char*)malloc(alen + 1);

        memcpy(mypassword, ba, alen);
        mypassword[alen] = '\0';

    }
    env->ReleaseByteArrayElements(barr, ba, 0);



    return env->NewStringUTF("success!");
}