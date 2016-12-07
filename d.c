#include <stdio.h>
#include <stdlib.h>

#define REC_SIZE 0x400
#define RESTORE_EXT ".JPG"
static char ZERO_CLUSTER[4096] = {0};

typedef union {
    char attr[1];
    char attr_name[1];
} ATTR_RES_ATTR;

typedef struct {
  int attr_len;
  short offset_attr;
  char idx_flag;
  char pad1;
  ATTR_RES_ATTR attr;
} ATTR_RESIDENT;

typedef union {
  char data_runs[1];
  char attr_name[1];
} ATTR_NON_RES_DATA;

typedef struct {
/*  0*/  long long start_vcn;
/*  8*/  long long last_vcn;
/* 16*/  short offset_datarun;
/* 18*/  short compress_usize;
/* 20*/  char padn[4];
/* 24*/  long long alloc_size;
/* 32*/  long long real_size;
/* 40*/  long long init_size_stream;
/* 48*/  ATTR_NON_RES_DATA data;
} ATTR_NON_RESIDENT;

typedef union {
  ATTR_RESIDENT resident;
  ATTR_NON_RESIDENT non_resident;
} ATTR_ATTR ;

typedef struct {
/*  0*/  short type;
/*  2*/  char res2[2];
/*  4*/  short len;
/*  6*/  char res1[2];
/*  8*/  char non_res_flag;
/*  9*/  char name_len;
/* 10*/  short offset_name;
/* 12*/  char flags[2];
/* 14*/  short attr_id;
/* 16*/  ATTR_ATTR res;
} ATTR_HEAD, *PATTR_HEAD;

typedef struct {
  char sig[4];
  char res1[16];
  short offset_attr_1;
  short flag;
  char res2[1];
} REC_HEAD, *PREC_HEAD;

typedef struct {
  long long ref_parent;
  char ctime[8];
  char atime[8];
  char mtime[8];
  char rtime[8];
  long long aloc_size;
  long long real_size;
  char flags[4];
  char res1[4];
  unsigned char len;
  char filename_space;
  char filename[1];
} ATTR_FILE, *PATTR_FILE;

typedef union {
  long long len;
  unsigned char buf[8];
} RUNS_LEN;
typedef union {
  long long offset;
  unsigned char buf[8];
} RUNS_OFFSET;

void uni2str(char *uni, int ulen, char *str)
{
  int i;

  for(i=0; i<ulen; i++)
  {
    *str++ = *uni++;
/*    if(*uni != 0)
      *str++ = *uni++; */
    uni++;
  }
  *str = 0;
}

void data_runs(unsigned char *pruns, int limit, FILE *fin, FILE *fout) {
  int runs_len_bytes, runs_offset_bytes;
  RUNS_LEN runs_len;
  RUNS_OFFSET runs_offset;
  unsigned char *end = pruns + limit;
  long long run_1 = 0;

  frewind(fin);

  while(*pruns){
    runs_len_bytes = (*pruns) & 0xF;
    runs_offset_bytes = (((*pruns) >> 4 ) & 0x0F);
printf("runs:%d/%d ", runs_len_bytes, runs_offset_bytes);
    if ( pruns + 1 + runs_len_bytes + runs_offset_bytes > end )
      break;
    if ( runs_len_bytes > sizeof(RUNS_LEN) ||
         runs_offset_bytes > sizeof(RUNS_OFFSET)) {
/*      pruns += 2;
      continue; */
      break;
    }
    memset(runs_offset.buf, 0, sizeof(runs_offset));
    memset(runs_len.buf, 0, sizeof(runs_len));
    memcpy(runs_len.buf, pruns + 1, runs_len_bytes);
    memcpy(runs_offset.buf, pruns + 1 + runs_len_bytes, runs_offset_bytes);
      run_1 = 
    printf("0x%08x+0x%08x ", runs_offset.offset, runs_len.len);
    if(runs_offset.offset) {
      
    } else {
      fwrite(ZERO_CLUSTER, 1, sizeof(ZERO_CLUSTER), fout);
    }
    pruns += 1 + runs_len_bytes + runs_offset_bytes;
  }
  printf("End runs: %02x ", *pruns);
}

void get_resident(ATTR_RESIDENT *resident)
{
  int i;
  unsigned char *ptmp = resident->attr.attr;

  printf("Resident(0x%04x) ", resident->attr_len);
  if(resident->attr_len == 0)
    return;
  printf("data: ");
  for(i=0; i<4; i++) {
    if ( i >= resident->attr_len ) {
      break;
    }
    printf("%02x ", *(ptmp + i));
  }
  printf("... ");
}

void get_non_resident(ATTR_NON_RESIDENT *non_resident, int limit)
{
  printf("Non-resident(0x%08x) data:", non_resident->real_size);
  data_runs( non_resident->data.data_runs, limit);
}

void main()
{
  char buf[REC_SIZE];
  char *pos;
  unsigned char *ptmp;
  int cnt;
  PATTR_HEAD pattr_head;
  PREC_HEAD prec_head = buf;
  char prt_buf[512];
  PATTR_FILE pattr_file;
  long mft_pos = 0;
  int bresident = 0;
  int bname = 0;
  int bdirectory = 0;
  int binuse = 0;
  int i;
  char filename[512];

  while( (cnt = read(0, buf, REC_SIZE)) == REC_SIZE) {
    bdirectory = (prec_head->flag >> 1) & 1;
    binuse = prec_head->flag & 1;
    printf("\n0x%08x:%c:%d ", mft_pos, bdirectory?'D':'F', binuse);
    mft_pos+=cnt;
    if(buf[0] == 0 || (!binuse))
    {
      printf("skip");
      continue;
    }
    filename[0] = 0;
    pos = buf + prec_head->offset_attr_1;
    pattr_head = pos;
    while(pattr_head->type != -1) {
      printf("[%02x:", pattr_head->type);
      bresident = !(pattr_head->non_res_flag);
      bname = (pattr_head->name_len);
      if(bname) {
        uni2str(pos + pattr_head->offset_name, pattr_head->name_len, prt_buf);
        printf("Attr:%s ", prt_buf);
/*        printf("Name(%d/%d) ", pattr_head->name_len, pattr_head->offset_name); */
      }
      switch ( pattr_head->type ) {
        case 0x10:
          break;
        case 0x20:
          break;
        case 0x30:
          if(bresident && (!bname)) {
            pattr_file = pattr_head->res.resident.attr.attr;
/*          printf("attrib_file[0] = 0x%02x\n", ((unsigned char*)pattrib_file)[0]); */
            uni2str(pattr_file->filename, pattr_file->len, filename);
            printf("file: %s, parent=0x%08x ", filename, pattr_file->ref_parent << 10);
          }
          break;
        case 0x50:
          break;
        case 0x60:
          break;
        case 0x70:
          break;
        case 0x80:
/*          if(strcmp("Thumbs.db", filename) == 0)
            break; */
          if(bresident) {
            get_resident(&(pattr_head->res.resident));
          } else {
            get_non_resident(&(pattr_head->res.non_resident), pattr_head->len - pattr_head->res.non_resident.offset_datarun );
            printf("/");
          }
          break;
        case 0x90:
          break;
        case 0xa0:
          break;
        case 0xb0:
          if(bresident) {
            get_resident(&(pattr_head->res.resident));
          } else {
            get_non_resident(&(pattr_head->res.non_resident), pattr_head->len - pattr_head->res.non_resident.offset_datarun );
          }
          break;
        case 0xc0:
          break;
        case 0x100:
          break;
        default:
          printf("Unknown attribute: 0x%04x ", pattr_head->type);
      }
      printf("] ");
      pos += pattr_head->len;
      pattr_head = pos;
    }
  }
}

