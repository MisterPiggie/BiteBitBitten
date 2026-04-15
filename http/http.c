#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_peer_id(unsigned char peer_id[20])
{
   int charset_len = sizeof(charset) - 1; 
   unsigned char buf[12];
   FILE *f = fopen("/dev/urandom", "rb");
   int i;

   if (f == NULL)
   {
       srand(time(NULL));
       for (i = 0; i < 12; i++)
       {
           peer_id[i] = charset[rand() % charset_len];
       }
       peer_id[12] = '\0';

       return;
   }

   fread(buf, 1, 12, f);
   fclose(f);

   for (i = 0; i < 12; i++)
   {
       peer_id[i] = charset[buf[i] % charset_len];
   }

   peer_id[12] = '\0';
   
   return;
}


