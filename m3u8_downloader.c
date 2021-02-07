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

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize);

int main(int argc, char **argv)
{
  CURL *curl;
  FILE *fp;
  FILE *fp2;
  //char *readptr;
  int result;

  //fp = fopen(argv[2], "wb");
  fp = fopen("output.m3u8", "wb");

  printf("The download operation will start\n");

  curl = curl_easy_init();
  if (curl == NULL)
  {
    printf("Curl initialization error: %d \n", errno);
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  //curl_easy_setopt(curl, CURLOPT_READDATA, readptr);

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
    return result;
  }
  //printf("the data is: %s\n", readptr);

#if 1
  double dl;
  curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dl);
  if (1)
  {
    printf("Downloaded %.0f bytes\n", dl);
  }
#endif

  char *parsedoutput;

  parsedoutput = malloc(sizeof(char) * dl * 3);

  /* Open the command for reading. */
  fp2 = popen("node index.js", "r");
  if (fp2 == NULL)
  {
    printf("Failed to run command\n");
    exit(1);
  }

  int i = 0;
  /* Read the output a line at a time - output it. */
#if 1
  //while (fgets(parsedoutput, sizeof(parsedoutput), fp2) != NULL)
  while (!feof(fp2))
  {
    parsedoutput[i] = getc(fp2);
    i++;
  }

#endif

  char urlbuffer[100];
  int urloffset = 0;
  int memsetcount = 0;
  while (1)
  {

    urloffset = LookForArrayInsideArray(parsedoutput, sizeof(char) * dl * 3, "url", 3);
    if (urloffset < 0)
    {
      break;
    }
    memsetcount += urloffset;
    memset(parsedoutput, 0, urloffset + 5);

    //printf("start search in :%c\n", parsedoutput[urloffset]);
    memset(urlbuffer, 0, 100);
    for (i = 0; i < 100; i++)
    {
      if (parsedoutput[urloffset + 7 + i] != '\"')
      {
        urlbuffer[i] = parsedoutput[urloffset + 7 + i];
      }
      else
      {
        break;
      }
    }

#if 0
    urloffset++;
    urlsearch = strstr(&parsedoutput[urloffset], "url");
    if (urlsearch != NULL)
    {
      for (i = 0; i < 100; i++)
      {
        urloffset++;
        if (urlsearch[7 + i] != '\"')
        {
          urlbuffer[i] = urlsearch[7 + i];
        }
        else
        {
          break;
        }
      }

    printf("urlbuffer: %s\n", urlbuffer);
    printf("urloffset: %d\n", urloffset);
    //printf("parsedoutput-urlbuffer: %lu\n", (long int)urlbuffer - (long int)parsedoutput);
  }

#endif
    printf("urlbuffer: %s\n", urlbuffer);
    printf("urloffset: %d\n", urloffset);
    usleep(10000);
  }

  /* close */
  pclose(fp2);

  fclose(fp);
  curl_easy_cleanup(curl);

  return 0;
}

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize)
{
  int i = 0;
  if (NeedleSize > HaystackSize)
  {
    return -1;
  }
  for (i = 0; i < (HaystackSize - NeedleSize + 1); i++)
  {
    if (strncmp(&cpHaystack[i], cpNeedle, NeedleSize) == 0) // then they are the same, return the index value
    {
      return i;
    }
  }
  return -1;
}