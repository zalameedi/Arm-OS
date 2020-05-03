/*******************************************************
*                      t.c file                        *
*******************************************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define TRK 18
#define CYL 36
#define BLK 1024

#include "ext2.h"
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;
GD    *gp;
INODE *ip;

char buf1[BLK], buf2[BLK];
//int color = 0x0A;

int prints(char *s)
{
  while (*s != '\0') 
  {
    putc(*s);
    s++;
  }
}

int gets(char *s) 
{ 
  while ((*s = getc()) != '\r')
  {
    putc(*s);
    s++;
  }
  *s = '\0'; // string termination
}

int getblk(u16 blk, char *buf)
{
  // readfd( (2*blk)/CYL, ( (2*blk)%CYL)/TRK, ((2*blk)%CYL)%TRK, buf);
  readfd( blk/18, ((blk)%18)/9, ( ((blk)%18)%9)<<1, buf);
}

int search(INODE *ip, char *name)
// which searches the DIRectory's data blocks for a name string; 
// return its inode number if found; 0 if not.
{
  int i;
  char *cp; 
  char c; //char dbuf[BLK];
  DIR  *dp;

  for (i=0; i < 12; i++){  // assuming only 12 entries
    //if (ip->i_block[i] == 0)
      //break;

    getblk((u16)ip->i_block[i], buf2);  // get disk block
    
    dp = (DIR *)buf2;
    cp = buf2;
      
    while (cp < buf2 + BLK){
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0; // save last char/byte
      prints(dp->name);

      if (strcmp(dp->name, name) == 0)
        return dp->inode;
      
      dp->name[dp->name_len] = c;  // restore char
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
}

main()
{ 
  u8     ino;
  u32    *up;
  u16    i, iblk;
  char   c, temp[64];

  prints("read block# 2 (GD)\n\r");
  getblk(2, buf1);
  gp = (GD *)buf1;

  iblk = (u16)gp->bg_inode_table;
  prints("inode_block=");
  putc(iblk+'0'); 
  prints("\n\r");
 
  getblk(iblk, buf1);
  ip = (INODE *)buf1 + 1;

  ino = (u16)search(ip,"boot")-1;
  getblk((u16)iblk+(ino/8), buf1);
  ip = (INODE *)buf1 + (ino % 8);

  ino = (u16)search(ip,"mtx")-1;
  getblk((u16)iblk+(ino/8), buf1);
  ip = (INODE *)buf1 + (ino % 8);

  getblk((u16)ip->i_block[12], buf2);

  setes(0x1000);

  for (i=0; i<12; i++){
      getblk((u16)ip->i_block[i], 0);
      putc('*');
      inces();
  }

  if (ip->i_block[12]){
     up = (u32 *)buf2;
     while(*up){
       getblk((u16)*up, 0); putc('.');
        inces();
        up++;
     }
  }
  prints("\n\rR?");
  getc();
}
