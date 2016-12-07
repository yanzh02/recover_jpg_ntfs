#include <stdio.h>
#include <stdlib.h>

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

void main()
{
  BOOT_SECT boot_sect;
  unsigned char *p1;
  int ret;
  char buf[255];
  long long lvalue, *plong;
  plong = &lvalue;
  short shvalue, *pshort;
  pshort = &shvalue;
  p1 = (unsigned char*) &boot_sect;
  char m;
  int n;
  int i;

  memset(p1, 0, sizeof(BOOT_SECT));
  ret = read(0, p1, sizeof(BOOT_SECT));

  printf("read %d bytes\n", ret);
  memcpy(buf, boot_sect.oem, 8);
  buf[8] = 0;
  printf("oem:				%s\n", buf);
  memcpy(pshort, boot_sect.byte_per_sec, sizeof(short));
  printf("bytes per sector:		0x%04x (%d)\n", shvalue, shvalue);
  printf("sectors per cluster:		0x%02x (%d)\n", boot_sect.sec_per_clus, boot_sect.sec_per_clus);
  printf("media descriptor:		0x%02x\n", boot_sect.media_desc);
  memcpy(plong, boot_sect.total_sec, sizeof(long long));
  printf("Total sectors:			0x%08x (%d)\n", lvalue, lvalue);
  memcpy(plong, boot_sect.lclus_mft, sizeof(long long));
  printf("Location clustion for $mft:	0x%08x (%d)\n", lvalue, lvalue);
  m = boot_sect.bytes_per_mft;
  if(m < 0)
  {
    n = 1;
    for(i=0; i<-m; i++)
      n = n * 2;
  } else
    n = m;
  printf("bytes per $mft record:		0x%02x (%d)\n", boot_sect.bytes_per_mft, n);
  m = boot_sect.clus_per_idx_buf;
  if(m < 0)
  {
    n = 1;
    for(i=0; i<-m; i++)
      n = n * 2;
  } else
    n = m;
  printf("Clusters per index buffer:	0x%02x (%d)\n", boot_sect.clus_per_idx_buf, n);

}

