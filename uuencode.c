#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *trans_ptr;
/* The two currently defined translation tables.  The first is the
   standard uuencoding, the second is base64 encoding.  */
const char uu_std[64] = {
    '`', '!', '"', '#', '$', '%', '&', '\'', '(',  ')', '*', '+', ',',
    '-', '.', '/', '0', '1', '2', '3', '4',  '5',  '6', '7', '8', '9',
    ':', ';', '<', '=', '>', '?', '@', 'A',  'B',  'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',  'O',  'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[',  '\\', ']', '^', '_'};

const char uu_base64[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

/* ENC is the basic 1 character encoding function to make a char printing.  */
#define ENC(Char) (trans_ptr[(Char)&077])

static inline void try_putchar(c) int c;
{
  if (putchar(c) == EOF) exit(0);
}

/*------------------------------------------------.
| Copy from IN to OUT, encoding as you go along.  |
`------------------------------------------------*/

static void encode() {
  register int n;
  int finishing = 0;
  register char *p;
  char buf[46]; /* 45 should be enough, but one never knows... */

  while (!finishing && (n = fread(buf, 1, 45, stdin)) > 0) {
    if (n < 45) {
      if (feof(stdin))
        finishing = 1;
      else
        exit(0);
    }

    if (trans_ptr == uu_std) putchar(ENC(n));

    for (p = buf; n > 2; n -= 3, p += 3) {
      try_putchar(ENC(*p >> 2));
      try_putchar(ENC(((*p << 4) & 060) | ((p[1] >> 4) & 017)));
      try_putchar(ENC(((p[1] << 2) & 074) | ((p[2] >> 6) & 03)));
      try_putchar(ENC(p[2] & 077));
    }

    if (n > 0) /* encode the last one or two chars */
    {
      char tail = trans_ptr == uu_std ? ENC('\0') : '=';

      if (n == 1) p[1] = '\0';

      try_putchar(ENC(*p >> 2));
      try_putchar(ENC(((*p << 4) & 060) | ((p[1] >> 4) & 017)));
      try_putchar(n == 1 ? tail : ENC((p[1] << 2) & 074));
      try_putchar(tail);
    }

    try_putchar('\n');
  }

  if (ferror(stdin)) exit(0);
  if (fclose(stdin) != 0) exit(0);

  if (trans_ptr == uu_std) {
    try_putchar(ENC('\0'));
    try_putchar('\n');
  }
}

int main(int argc, const char **argv) {
  int mode;
  struct stat sb;
  trans_ptr = uu_std;
  FILE *fp = freopen(argv[1], "rb", stdin);
  if (fp != stdin) exit(1);
  if (fstat(fileno(stdin), &sb) != 0) exit(1);
  mode = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);

  if (printf("begin%s %o %s\n", trans_ptr == uu_std ? "" : "-base64", mode,
             argv[2]) < 0)
    exit(1);
  encode();
  if (ferror(stdout) || printf(trans_ptr == uu_std ? "end\n" : "====\n") < 0 ||
      fclose(stdout) != 0)
    exit(1);
  exit(0);
}
