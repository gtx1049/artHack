#ifndef __CRYPT_AES_H__
#define __CRYPT_AES_H__


#define TFFS_ENCRYPT cryptsector

extern int init_aes(char *);

extern void cryptsector(unsigned char *buffer, unsigned int lba, int nsectors, unsigned int key, int encrypt);

#endif
