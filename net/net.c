#include "net.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void generate_peer_id(unsigned char peer_id[20])
{
   unsigned char buf[12];
   FILE *f = fopen("/dev/urandom", "rb");
   int i;
   size_t n;
    
   for (i = 0; i < peer_id_version_len; i++)
   {
       peer_id[i] = peer_id_version[i];
   }

   if (f != NULL)
   {
       n = fread(buf, 1, 12, f);
       fclose(f);

       for (i = 0; i < (int) n; i++)
       {
           peer_id[i + 8] = charset[buf[i] % charset_len];
       }

       if (n < 12)
       {
           srand(time(NULL));
           for (i = (int) n; i < 12; i++)
           {
               peer_id[8+i] = charset[rand() % charset_len];
           }
       }
   } else
   {
       srand(time(NULL));
       for (i = 8; i < 20; i++)
       {
           peer_id[i] = charset[rand() % charset_len];
       }

   }
   return;
}

void url_encode_hash(const uint8_t *hash_info, char *out)
{
    //OUT MUST BE 61 CHARS LONG
    int i;
    for (i = 0; i < 20; i++)
    {
        sprintf(out + i*3, "%%%02X", hash_info[i]);
    }
    out[60] = '\0';
}
