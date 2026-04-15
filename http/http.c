#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void generate_peer_id(unsigned char peer_id[21])
{
   unsigned char buf[12];
   FILE *f = fopen("/dev/urandom", "rb");
   int i;

   if (f == NULL)
   {
       srand(time(NULL));
       for (i = 8; i < 20; i++)
       {
           peer_id[i] = charset[rand() % charset_len];
       }
       peer_id[20] = '\0';

       return;
   }

   fread(buf, 1, 12, f);
   fclose(f);

   for (i = 8; i < 20; i++)
   {
       peer_id[i] = charset[buf[i - 8] % charset_len];
   }


   for (i = 0; i < peer_id_version_len; i++)
   {
       peer_id[i] = peer_id_version[i];
   }

   peer_id[20] = '\0';
   
   return;
}


