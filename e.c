#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define FIN "/dev/sdd1"
/* #define BUFFER_CLUS 0x40000 */
#define BUFFER_CLUS 0x40000

typedef struct {
  char filename[256];
  long lread_from;
  int ifilesize;
  int irealwrite;
  int iread_len;
  int iloc;
} RUNS, *PRUNS;

static RUNS runs[28000];
static unsigned char *pbuf;
static long passed = 0;

void make_path(int j)
{
  char *path, *dir;

  path = strdup(runs[j].filename);
  dir = dirname(path);
  if(access(dir, W_OK|X_OK) != 0) {
    mkdir(dir, 0755);
  }
  free(path);
}

int seek_run(PRUNS prun, int fout)
{
  int ret = 0;

  if(-1 == lseek(fout, prun->iloc << 12, SEEK_SET)){
    printf("Failed 1 to seek %d to %s(%d)\n", prun->lread_from, prun->filename, errno);
    ret = -2;
  }
  return ret;
}

int write_run(PRUNS prun, int fout)
{
  int ret = 0;

  if(-1 == write(fout, pbuf + ((prun->lread_from - passed) << 12), prun->irealwrite)) {
    printf("Failed to write %d to %s(%d), errno=%d\n", prun->lread_from, prun->filename, prun->iloc, errno);
    ret = -3;
  } else
    printf("%d bytes from %d written to %s at %d\n", prun->irealwrite, prun->lread_from, prun->filename, prun->iloc);
  return ret;
}

int restore_a_run(PRUNS prun)
{
  int fout;
  int ret = 0;

  if(access(prun->filename, F_OK) == 0){
    fout = open(prun->filename, O_RDWR);
    if(fout == -1) {
      printf("Failed on open for insert %d, %s(%d)\n", prun->lread_from, prun->filename, errno);
      return -1;
    }
    ret = seek_run(prun, fout) << 1;
    if(0 == ret)
      ret = write_run(prun, fout) << 1;
  } else {
    fout = open(prun->filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fout == -1) {
      printf("Failed to create %d to %s(%d)\n", prun->lread_from, prun->filename, errno);
      return -9;
    }
    if(prun->iloc != 0) {
      ret = seek_run(prun, fout);
      if(0 == ret)
        ret = write_run(prun, fout) << 2;
    } else {
      ret = write_run(prun, fout);
    }
  }
  close(fout);
  printf("ret=%d\n", ret);
  return ret;
}

void restore(int len)
{
  FILE *fin;
  int i, i0 = 0, i1 = 0, j = 0;

  fin = fopen(FIN, "r");
  if(fin == 0) {
    printf("Failed to open %s (%d)\n", FIN, errno);
    return;
  }
  pbuf = malloc(BUFFER_CLUS << 12);
  if(pbuf == 0) {
    printf("malloc fail %d\n", errno);
    return;
  }
  for(i=0; i<len; i++) {
    if(runs[i].lread_from + runs[i].iread_len - passed > BUFFER_CLUS ) {
      fread(pbuf, 0x1000, runs[i1].lread_from + runs[i1].iread_len - passed, fin);
      printf("%d clustors read from %d\n", runs[i1].lread_from + runs[i1].iread_len - passed, passed);
      for(j=i0; j<i1; j++) {
        make_path(j);
        restore_a_run(runs + j);
      }
      passed = runs[i].lread_from;
      fseek(fin, passed << 12, SEEK_SET);
      i0 = i;
    }
    i1 = i;
  }
  fclose(fin);
}

void main()
{
  char *rd;
  char *clus, *colon_clus_len, *colon_loc, *colon_filesize, *colon_filename;
  int i = 0;
/*  long lread_from, lfilesize, lrealwrite;
  int iread_len, iloc; */

  while(rd = readline(0)) {
    if(strlen(rd) == 0) {
      free(rd);
      break;
    }
    clus = rd;
    if(colon_clus_len = strchr(rd, ',')) {
      *colon_clus_len++ = 0;
    } else {
      free(rd);
      continue;
    }
    if(colon_loc = strchr(colon_clus_len, ',')) {
      *colon_loc++ = 0;
    } else {
      free(rd);
      continue;
    }
    if(colon_filesize = strchr(colon_loc, ',')) {
      *colon_filesize++ = 0;
    } else {
      free(rd);
      continue;
    }
    if(colon_filename = strchr(colon_filesize, ',')) {
      *colon_filename++ = 0;
    } else {
      free(rd);
      continue;
    }
    runs[i].lread_from = atol(clus);
    runs[i].iread_len = atoi(colon_clus_len);
    runs[i].iloc = atoi(colon_loc);
    runs[i].ifilesize = atol(colon_filesize);
    strcpy(runs[i].filename, colon_filename);
    if ((runs[i].iloc + runs[i].iread_len) << 12 > runs[i].ifilesize )
      runs[i].irealwrite = runs[i].ifilesize - (runs[i].iloc << 12);
    else
      runs[i].irealwrite = runs[i].iread_len << 12;
    printf("%s: %4d=write pos, from clustor %d on disk, %d clustors written, actually %d bytes written\n", runs[i].filename, runs[i].iloc, runs[i].lread_from, runs[i].iread_len, runs[i].irealwrite);
    free(rd);
    i++;
  }
  restore(i);
}

