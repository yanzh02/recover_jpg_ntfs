#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
/*#include <wchar.h> */

#define REC_SIZE 0x400
#define CLUSSIZE 0x1000
#define RESTORE_EXT ".JPG"
/*
.avi
.3gp
.doc
.wav
.mov
.vob
.ppt
.zip
.mpg
.mp4
.pdf
*/
#define DISKSIZE 0x10000000000L
#define OUTDIR "restored"

static char *EXTS[] = {".avi", ".3gp", ".doc", ".wav", ".mov", ".ppt", ".zip", ".mpg", ".mp4", ".pdf"};

static char ZERO_CLUSTER[CLUSSIZE] = {0};
static FILE *fin;

typedef struct {
  char rec[REC_SIZE];
/*  char filename[256]; */
  wchar_t filename[256];
  int parent;
  int filesize;
} MFTREC, *PMFTREC;

typedef struct{
  PMFTREC rec;
  long run_loc_in_file;
  int run_len;
} BITMAP, *PBITMAP;
/* static BITMAP bitmap[0x10000000]; */

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

void uni2str(short *uni, int ulen, char *str)
{
  short *uend = uni + ulen;

  while(uni < uend)
  {
    if(*uni == '_' || *uni == '.' || *uni == '~' || *uni == '$' || *uni == '-' || *uni == ' ' ||
       '0' <= *uni && *uni <= '9' ||
       'A' <= *uni && *uni <= 'Z' ||
       'a' <= *uni && *uni <= 'z')
      *str++ = *uni++;
    else {
      sprintf(str, "%04x", *uni++);
      str += 4;
    }
  }
  *str = 0;
}

void data_runs(unsigned char *pruns, int limit, PMFTREC pmftrec) {
  int runs_len_bytes, runs_offset_bytes;
  RUNS_LEN runs_len;
  RUNS_OFFSET runs_offset;
  unsigned char *end = pruns + limit;
  long long run_1 = 0L;
  RUNS_OFFSET runs_inc;
  int loc = 0;
  char out_file[512];
  FILE *fout;

/*  rewind(fin); */

  sprintf(out_file, "%s/%x/%s", OUTDIR, pmftrec->parent, pmftrec->filename);
/*  fout = fopen(out_file, "w"); */
  while(*pruns){
    runs_len_bytes = (*pruns) & 0xF;
    runs_offset_bytes = (((*pruns) >> 4 ) & 0x0F);
/* printf("runs:%d/%d ", runs_len_bytes, runs_offset_bytes); */
    if ( pruns + 1 + runs_len_bytes + runs_offset_bytes > end )
      break;
    if ( runs_len_bytes > sizeof(RUNS_LEN) ||
         runs_offset_bytes > sizeof(RUNS_OFFSET)) {
/*      pruns += 2;
      continue; */
      break;
    }
    memset(runs_offset.buf, 0, sizeof(runs_offset));
    memset(runs_inc.buf, 0, sizeof(runs_inc));
    memset(runs_len.buf, 0, sizeof(runs_len));
    memcpy(runs_len.buf, pruns + 1, runs_len_bytes);
    memcpy(runs_offset.buf, pruns + 1 + runs_len_bytes, runs_offset_bytes);
    if(run_1) {
      if(runs_offset_bytes) {
        if((runs_offset.buf[runs_offset_bytes - 1] >> 7 ) & 1 ) {
          runs_inc.buf[runs_offset_bytes] = 1;
          run_1 += runs_offset.offset - runs_inc.offset;
/*          bitmap[run_1].rec = rec;
          bitmap[run_1].run_loc_in_file= loc;
          bitmap[run_1].run_len = runs_len.len; */
#ifdef WRITE_LIST
          printf("%010d,%d,%d,%d,%s\n", run_1, runs_len.len, loc, pmftrec->filesize, out_file);
#endif
          loc += runs_len.len;
        } else {
          run_1 += runs_offset.offset;
/*          bitmap[run_1].rec = rec;
          bitmap[run_1].run_loc_in_file= loc;
          bitmap[run_1].run_len = runs_len.len; */
#ifdef WRITE_LIST
          printf("%010d,%d,%d,%d,%s\n", run_1, runs_len.len, loc, pmftrec->filesize, out_file);
#endif
          loc += runs_len.len;
        }
      }
    } else {
      run_1 = runs_offset.offset;
#ifdef WRITE_LIST
      printf("%010d,%d,%d,%d,%s\n", run_1, runs_len.len, loc, pmftrec->filesize, out_file);
#endif
/*      frewind(fin);
      fseek(run_1);
      fread(buf, REC_SIZE, runs_len.len, fin); */
 /*     bitmap[run_1].rec = rec;
      bitmap[run_1].run_loc_in_file = 0;
      bitmap[run_1].run_len = runs_len.len; */
      loc = runs_len.len;
    }
/*    printf("clus:0x%x(0x%x-%x):0x%x+0x%x ", run_1, bitmap[run_1].run_loc_in_file, bitmap[run_1].run_len, run_1, runs_len.len); */
#ifdef DEBUG
    printf("0x%x+0x%x ", run_1, runs_len.len);
#endif
    if(runs_offset.offset) {
      
    } else {
/*      fwrite(ZERO_CLUSTER, 1, sizeof(ZERO_CLUSTER), fout); */
    }
    pruns += 1 + runs_len_bytes + runs_offset_bytes;
  }
#ifdef DEBUG
  printf("End runs: %02x ", *pruns);
#endif
/*  fclose(fout); */
}

void get_resident(ATTR_RESIDENT *resident)
{
  int i;
  unsigned char *ptmp = resident->attr.attr;

#ifdef DEBUG
  printf("Resident(0x%04x) ", resident->attr_len);
#endif
  if(resident->attr_len == 0)
    return;
#ifdef DEBUG
  printf("data: ");
#endif
  for(i=0; i<4; i++) {
    if ( i >= resident->attr_len ) {
      break;
    }
#ifdef DEBUG
    printf("%02x ", *(ptmp + i));
#endif
  }
#ifdef DEBUG
  printf("... ");
#endif
}

void get_non_resident(ATTR_NON_RESIDENT *non_resident, int limit, PMFTREC rec)
{
  rec->filesize = non_resident->real_size;
#ifdef DEBUG
  printf("Non-resident(0x%08x) data:", non_resident->real_size);
#endif
  data_runs( non_resident->data.data_runs, limit, rec);
}

void main()
{
/*  char buf[REC_SIZE]; */
  char *pos;
  unsigned char *ptmp;
  int cnt;
  PATTR_HEAD pattr_head;
  PREC_HEAD prec_head;
  char prt_buf[512];
  PATTR_FILE pattr_file;
  long mft_pos = 0;
  int bresident = 0;
  int bname = 0;
  int bdirectory = 0;
  int binuse = 0;
  int i;
  FILE *fout;
/*  char filename[1024]; */
  PMFTREC pmftrec;

/*  memset(bitmap, 0, sizeof(bitmap)); */
/*  fin = fopen(FIN, "r"); */
  while(pmftrec = malloc( sizeof(MFTREC) )) {
    memset(pmftrec, 0, sizeof(MFTREC));
    prec_head = pmftrec->rec;
    if ( (cnt = read(0, pmftrec->rec, REC_SIZE)) != REC_SIZE)
      break;
    bdirectory = (prec_head->flag >> 1) & 1;
    binuse = prec_head->flag & 1;
#ifdef DEBUG
    printf("\n0x%08x:%c:%d ", mft_pos, bdirectory?'D':'F', binuse);
#endif
    mft_pos+=cnt;
    if(pmftrec->rec[0] == 0 || (!binuse))
    {
#ifdef DEBUG
      printf("skip");
#endif
      free(pmftrec);
      continue;
    }
    pmftrec->filename[0] = 0;
    pos = pmftrec->rec + prec_head->offset_attr_1;
    pattr_head = pos;
    while(pattr_head->type != -1) {
#ifdef DEBUG
      printf("[%02x:", pattr_head->type);
#endif
      bresident = !(pattr_head->non_res_flag);
      bname = (pattr_head->name_len);
      if(bname) {
/*          uni2str((char*)(pos + pattr_head->offset_name), pattr_head->name_len, prt_buf);
          wprintf(L"Attr:%ls ", prt_buf); */
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
            pmftrec->parent = pattr_file->ref_parent;
/*          printf("attrib_file[0] = 0x%02x\n", ((unsigned char*)pattrib_file)[0]); */
            uni2str(pattr_file->filename, pattr_file->len, pmftrec->filename);
#ifdef DEBUG
            printf("file: %s, parent=0x%08x ", pmftrec->filename, pattr_file->ref_parent << 10);
#endif
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
            for(i=0; i<(sizeof(EXTS) / sizeof(void*)); i++){
              if(strcasestr(pmftrec->filename, EXTS[i]))
                break;
            }
            if(sizeof(EXTS) / sizeof(void*) != i){
/*            if(strcasestr(pmftrec->filename, RESTORE_EXT)) { */
#ifdef ENABLE_WRITE
              sprintf(out_file, "%s/%x", OUTDIR, pmftrec->parent);
              if(access(out_file, W_OK|X_OK) != 0)
                mkdir(out_file, 0755);
              strcat(out_file, "/");
              strcat(out_file, pmftrec->filename);
/*                if(access(out_file, F_OK) != 0 ){ */
#endif
                get_non_resident(&(pattr_head->res.non_resident), pattr_head->len - pattr_head->res.non_resident.offset_datarun, pmftrec );
#ifdef ENABLE_WRITE
/*                } */
#endif
            }
          }
          break;
        case 0x90:
          break;
        case 0xa0:
          break;
        case 0xb0:
/*
            if(bresident) {
              get_resident(&(pattr_head->res.resident));
            } else {
              get_non_resident(&(pattr_head->res.non_resident), pattr_head->len - pattr_head->res.non_resident.offset_datarun );
            }
*/
          break;
        case 0xc0:
          break;
        case 0x100:
          break;
        default:
#ifdef DEBUG
          printf("Unknown attribute: 0x%04x ", pattr_head->type);
#endif
          break;
      }
#ifdef DEBUG
      printf("] ");
#endif
      pos += pattr_head->len;
      pattr_head = pos;
    }
    free(pmftrec);
  }
/*  fclose(fin); */
}

