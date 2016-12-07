# recover_jpg_ntfs
a.c:
  Reads and resolve MBR from stdin
  Result is the starting sector of MFT

b.c:
  Reads and resolve MFT from stdin
    When compiled with DEBUG:
      Outputs a list of file entries
      <Entry pos>:<F or D>:<1 or 0> .. [30:file: <file name>] [80:<data runs>]
    When compiled with WRITE_LIST:
      Outputs a list of file entries
      <File slice starting sector>,<slice sectors>,<slice pos in the file>,<file size>,<output path name>

e.c:
  Reads the output from b.c with compiler option WRITE_LIST, and recover the files.
