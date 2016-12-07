#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
/*
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>
*/
#define INFILE "vol"
#define NEWCLUS 0x01d82400

/* #define OPWRITE */

typedef struct
{
  unsigned char jmp[3];
  char oem[8];
  unsigned char byte_per_sec[2];
  unsigned char sec_per_clus;
  unsigned char reserved1[7];
  unsigned char media_desc;
  unsigned char reserved2[18];
  unsigned char total_sec[8];
  unsigned char lclus_mft[8];
  unsigned char lclus_mft2[8];
  unsigned char bytes_per_mft;
  unsigned char reserved3[3];
  unsigned char clus_per_idx_buf;
} BOOT_SECT;

typedef union {
  int ivalue;
  unsigned char buf[4];
} CLUSU;

void main()
{
  BOOT_SECT boot_sect;
  unsigned char *p1;
  int ret;
  unsigned char buf[255];
  long long lvalue, *plong;
  plong = &lvalue;
  short shvalue, *pshort;
  pshort = &shvalue;
  p1 = (unsigned char*) &boot_sect;
  char m;
  int n;
  int i;
  FILE *f;
  long pos;
  int ivalue;
  CLUSU clusu;
/*  off64_t pos; */

  ivalue = NEWCLUS;

/*  pos = boot_sect.lclus_mft - (unsigned char*)&boot_sect; */
  pos = 0x138L + (0x000c0000L << 12);
/*  pos = 0x000c0000L << 12;*/
  printf("pos is 0x%016x\n", pos);
#ifdef OPWRITE
  f = fopen(INFILE, "r+");
#else
  f = fopen(INFILE, "r");
#endif
  if(f){
    if(0 == fseek(f, pos, 0) ) {
#ifdef OPWRITE
/*      printf("Writing %02x %02x %02x %02x\n", clusu.buf[0], clusu.buf[1], clusu.buf[2], clusu.buf[3]); */
      fwrite(&ivalue, 4, 1, f);
#else
      n = fread((unsigned char*)&lvalue, 8, 1, f);
      printf("data at pos 0x%016x is 0x%08x\n", pos, lvalue);
#endif
    } else {
      printf("fseek error %d\n", errno);
    }
    fclose(f);
  }

}

