#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
   time_t now = time(NULL);
   unsigned char *buffer;
   int i;
   
   unsigned char *ENV[] = 
   {
      "SCRIPT_NAME",
      "SERVER_NAME",
      "SERVER_ADMIN",
      "HTTP_ACCEPT_ENCODING",
      "HTTP_CONNECTION",
      "REQUEST_METHOD",
      "HTTP_ACCEPT",
      "SCRIPT_FILENAME",
      "SERVER_SOFTWARE",
      "QUERY_STRING",
      "REMOTE_PORT",
      "HTTP_USER_AGENT",
      "SERVER_SIGNATURE",
      "SERVER_PORT",
      "HTTP_ACCEPT_LANGUAGE",
      "REMOTE_ADDR",
      "SERVER_PROTOCOL",
      "PATH",
      "REQUEST_URI",
      "GATEWAY_INTERFACE",
      "SERVER_ADDR",
      "DOCUMENT_ROOT",
      "HTTP_HOST",
      NULL
   };
   
   printf("HTTP/1.1 200 OK\n");
   printf("Content-type: text/html\n\n");
   printf("<html>\n");
   printf("<head><title>CGI TEST PROG</title></head>\n");
   printf("<body>\n");
   printf("<h1>Welcome to the world of dynamic html!</h1>\n");
   printf("<hr><h4>Current Time is %s</h4><hr><br>\n", ctime(&now));
   
   for (i = 0; ENV[i]; ++i)
   {
      buffer = getenv(ENV[i]);
      printf("%s=%s<br>\n", ENV[i], buffer);
   }
   
   printf("</body>\n");
   printf("</html>\n\n");

   return 0;
}