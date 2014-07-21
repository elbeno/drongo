#include <stdio.h>
void main(int argc, char*argv)
{
  int x, nl = 0, last=0;
  while (!feof (stdin))
  {
    x= fgetc(stdin);
    if (x==10 || x==13)
    {
      if (nl == 0 | x==last)
      {
        nl = 1 - nl;
        last = x;
        fputc ('\n', stdout);
      }
    }
    else
    {
      fputc (x, stdout);
      nl = 0;
    }
  }
}
