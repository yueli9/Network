#include <string.h>
#include <stdio.h>

int main()
{
   char str[80] = "$128.10.3.59$57343$";
   const char s[2] = "$";
   char *token;
   
   /* get the first token */
   token = strtok(str, s);
   
   /* walk through other tokens */
   while( token != NULL ) 
   {
      printf( " token is %s\n", token );
      printf("str is %s\n",str );
    
      token = strtok(NULL, s);
            printf( " token is %s\n", token );

   }
   
   return(0);
}