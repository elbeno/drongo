#include <sys/file.h>

void main (int argc, char **argv)
{
  int fh,slen;
  char *cp = argv[1];
  if (argc != 2)
  {
    write(1,"Usage: write \"string\"\n",22) ;
    return ;
  }
  if ((fh = open ("/homes/ajb1011/ubermud1/mud/main/fridge/log",0,O_APPEND))
         == 0)
  {
    write ("Can't open log file\n",20) ;
    return ;
  }
  slen=0;
  while (*cp++ != 0) slen++;
  write (fh, argv[1],slen) ;
  write (1, argv[1],slen) ;
  close (fh);
}
