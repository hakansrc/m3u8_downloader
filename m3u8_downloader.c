#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>

#if 0
struct my_info
{
  int shoesize;
  char *secret;
};

static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
  struct my_info *i = (struct my_info *)userdata;

  /* now this callback can access the my_info struct */

  return nitems * size;
}

struct my_info my = {10, "the cookies are in the cupboard"};
#endif

int main(int argc, char **argv)
{
  CURL *curl;
  FILE *fp;
  char *readptr;
  int result;

  fp = fopen(argv[2], "wb");

  printf("selam dunya\n");

  curl = curl_easy_init();
  if (curl == NULL)
  {
    printf("Curl initialization error: %d \n", errno);
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_READDATA, readptr);

  curl_easy_setopt(curl, CURLOPT_CONV_FROM_UTF8_FUNCTION, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

  //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
  //curl_easy_setopt(curl, CURLOPT_HEADERDATA, &my);

  result = curl_easy_perform(curl);

  if (result == CURLE_OK)
  {
    printf("Download is successfull\n");
  }
  else
  {
    printf("ERROR: %s\n", curl_easy_strerror(result));
  }
  printf("%s\n", readptr);

#if 0
  double dl;
  curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dl);
  if (1)
  {
    printf("Downloaded %.0f bytes\n", cl);
  }
#endif
#if 0
  long req;
  curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &req);
  if (1)
    printf("Request size: %ld bytes\n", req);
#endif
  fclose(fp);
  curl_easy_cleanup(curl);

  return 0;
}