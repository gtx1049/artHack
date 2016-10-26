##ULEB128数据结构

### 1. 简介

ULEB128的原文为“Little-Endian Base 128”，是一种基于128的小印第安序编码格式，其是一个可变长度编码，可以表示任意的有符号数和无符号数。

### 2. C代码

转换细节略过，这里直接给出C程序的ULEB128读取：
		
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