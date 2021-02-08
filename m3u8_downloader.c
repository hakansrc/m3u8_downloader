#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>

#define SCRIPT_FIRSTPART "const M3U8FileParser = require('m3u8-file-parser');\n\
const fs = require('fs');\n\
const content = fs.readFileSync('" /*master.m3u8*/

#define SCRIPT_SECONDPART "', { encoding: 'utf-8' });\n\
const reader = new M3U8FileParser();\n\
reader.read(content);\n\
console.log(JSON.stringify(reader.getResult(), null, 2));"

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize, int ElementCount);

int main(int argc, char **argv)
{
  CURL *curl;
  FILE *fp;
  FILE *fp2;
  FILE *fp3;
  CURLU *h;
  CURLUcode uc;
  char *host;
  char *path;
  int is_it_dir = 0;
  char dirnamebuffer[100];
  char dircommandbuffer[150];
  char scriptcontent[500];

  char *progindex_content;

  //char highes

  char wholeurl[500];

  h = curl_url(); /* get a handle to work with */
  if (!h)
  {
    return -1;
  }

  /* parse a full URL */
  uc = curl_url_set(h, CURLUPART_URL, argv[1], 0);
  if (uc)
  {
    return -1;
  }
  /* extract host name from the parsed URL */
  uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
  if (!uc)
  {
    printf("Host name: %s\n", host);
  }

  /* extract the path from the parsed URL */
  uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
  if (!uc)
  {
    printf("Path: %s\n", path);
    curl_free(path);
  }

  //char *readptr;
  int result;

  //fp = fopen(argv[2], "wb");
  fp = fopen("master.m3u8", "wb");

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
  int j = 0;
  /* Read the output a line at a time - output it. */
#if 1
  //while (fgets(parsedoutput, sizeof(parsedoutput), fp2) != NULL)
  while (!feof(fp2))
  {
    parsedoutput[i] = getc(fp2);
    i++;
  }
  //fclose(fp2);
  printf("parsedoutput:%s\n", parsedoutput);
  int SearchReturn = 0;
  int ResolutionValue = 0;
  int ResolutionValueMax = 0;
  int MaxResIndex = 0;
  int MaxResUrlIndex = 0;
  char urlbuffer[100];
  char filepathbuffer[200];
  for (i = 0; i < dl * 3; i++)
  {
    SearchReturn = LookForArrayInsideArray(parsedoutput, 3 * dl, "resolution", 10, i + 1);
    if (SearchReturn < 0)
    {
      break;
    }
    //printf("SearchReturn: %d\n", SearchReturn);
    //printf("parsedoutput[search+13]: %c\n", parsedoutput[SearchReturn + 13]);
    ResolutionValue = atoi(&parsedoutput[SearchReturn + 13]);
    //printf("ResolutionValue: %d\n", ResolutionValue);
    if (ResolutionValue > ResolutionValueMax)
    {
      ResolutionValueMax = ResolutionValue;
      MaxResIndex = SearchReturn;
      MaxResUrlIndex = LookForArrayInsideArray(parsedoutput, 3 * dl, "url", 3, i + 1);
    }
  }
  //printf("ResolutionValueMax: %d\n", ResolutionValueMax);
  //printf("MaxResIndex: %d\n", MaxResIndex);
  //printf("MaxResUrlIndex: %d\n", MaxResUrlIndex);

  memset(urlbuffer, 0, 100);
  for (i = 0; i < 100; i++)
  {
    if (parsedoutput[MaxResUrlIndex + 7 + i] != '\"')
    {
      urlbuffer[i] = parsedoutput[MaxResUrlIndex + 7 + i];
    }
    else
    {
      urlbuffer[i] = '\0';
      break;
    }
  }
  printf("urlbuffer: %s\n", urlbuffer);
  printf("the input url is :%s \n", argv[1]);

  uc = curl_url_set(h, CURLUPART_URL, argv[1], 0);
  if (uc)
  {
    return -1;
  }
  uc = curl_url_set(h, CURLUPART_URL, urlbuffer, 0);
  if (uc)
  {
    printf("url get error \n");
    return -1;
  }
  uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
  if (!uc)
  {
    printf("path: %s\n", path);
  }

  memset(wholeurl, 0, 500);
  sprintf(wholeurl, "%s%s%s", "https://", host, path);
  printf("wholeurl: %s\n", wholeurl);
  curl_easy_setopt(curl, CURLOPT_URL, wholeurl);
  for (i = strlen(urlbuffer); i > 0; i--)
  {
    if (urlbuffer[i - 1] == '/') //then we have directories, we have to create those directories
    {
      memset(dirnamebuffer, 0, 100);
      memset(dircommandbuffer, 0, 150);
      strncpy(dirnamebuffer, urlbuffer, i);
      sprintf(dircommandbuffer, "mkdir -p %s", dirnamebuffer);
      printf("the dir name is :%s \n", dirnamebuffer);
      system(dircommandbuffer);
      break;
    }
  }

  fclose(fp);
  fp = fopen(urlbuffer, "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

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
  //fclose(fp);
  sprintf(scriptcontent, "%s%s%s", SCRIPT_FIRSTPART, urlbuffer, SCRIPT_SECONDPART);
  printf("scriptcontent:\n%s\n", scriptcontent);

  fp3 = fopen("highres.js", "wb");
  fwrite(scriptcontent, 1, strlen(scriptcontent), fp3);
  fclose(fp3);

  fp3 = popen("node highres.js", "r");
  if (fp3 == NULL)
  {
    printf("Failed to run command\n");
    exit(1);
  }

#if 0
  while (fgets(path, sizeof(path), fp3) != NULL)
  {
    //usleep(10000);
    printf("%s", path);
  }

#endif

  //double dl;
  curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dl);
  if (1)
  {
    printf("Downloaded %.0f bytes\n", dl);
  }
  progindex_content = malloc(sizeof(char) * dl * 2);
  i = 0;
  while (!feof(fp3))
  {
    progindex_content[i] = getc(fp3);
    i++;
  }
  printf("%s\n", progindex_content);
  pclose(fp3);

  fp = fopen(argv[2], "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  printf("urlbuffer: %s\n", urlbuffer);

  for (i = strlen(urlbuffer); i > 0; i--)
  {
    if (urlbuffer[i - 1] == '/') //then we have directories, we have to create those directories
    {
      memset(dirnamebuffer, 0, 100);
      memset(dircommandbuffer, 0, 150);
      strncpy(dirnamebuffer, urlbuffer, i);
      sprintf(dircommandbuffer, "mkdir -p %s", dirnamebuffer);
      printf("the dir name is :%s \n", dirnamebuffer);
      system(dircommandbuffer);
      break;
    }
  }

  for (i = 0; i < dl * 2; i++)
  {
    //memset(urlbuffer, 0, 100);
    SearchReturn = LookForArrayInsideArray(progindex_content, 2 * dl, "url", 3, i + 1);
    if (SearchReturn < 0)
    {
      break;
    }
#if 1
    for (j = 0; j < 100; j++)
    {
      if (progindex_content[SearchReturn + 7 + j] != '\"')
      {
        urlbuffer[j] = progindex_content[SearchReturn + 7 + j];
      }
      else
      {
        urlbuffer[j] = '\0';
        break;
      }
    }
#endif
    printf("urlbuffer: %s\n", urlbuffer);
    sprintf(filepathbuffer, "%s%s", dirnamebuffer, urlbuffer);
    printf("filepathbuffer: %s\n", filepathbuffer);

    uc = curl_url_set(h, CURLUPART_URL, argv[1], 0);
    if (uc)
    {
      return -1;
    }
    uc = curl_url_set(h, CURLUPART_URL, filepathbuffer, 0);
    if (uc)
    {
      printf("url get error \n");
      return -1;
    }
    uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
    if (!uc)
    {
      printf("path: %s\n", path);
    }

    memset(wholeurl, 0, 500);
    sprintf(wholeurl, "%s%s%s", "https://", host, path);
    printf("dirnamebuffer: %s\n", dirnamebuffer);
    printf("wholeurl: %s\n", wholeurl);
    printf("path: %s\n", path);
    //continue;
    //curl_easy_setopt(curl, CURLOPT_URL, wholeurl);
    curl_easy_setopt(curl, CURLOPT_URL, wholeurl);
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
  }

#endif
#if 0
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

    uc = curl_url_set(h, CURLUPART_URL, argv[1], 0);
    if (uc)
    {
      return -1;
    }

    uc = curl_url_set(h, CURLUPART_URL, urlbuffer, 0);
    if (uc)
    {
      printf("url get error \n");
      return -1;
    }
    uc = curl_url_get(h, CURLUPART_PATH, &path, 0);
    if (!uc)
    {
      printf("Path: %s\n", path);
      memset(wholeurl, 0, 500);
      sprintf(wholeurl, "%s%s%s", "https://", host, path);
      printf("wholeurl: %s\n", wholeurl);

      curl_easy_setopt(curl, CURLOPT_URL, wholeurl);
      for (i = strlen(urlbuffer); i > 0; i--)
      {
        if (urlbuffer[i - 1] == '/') //then we have directories, we have to create those directories
        {
          memset(dirnamebuffer, 0, 100);
          memset(dircommandbuffer, 0, 150);
          strncpy(dirnamebuffer, urlbuffer, i);
          sprintf(dircommandbuffer, "mkdir -p %s", dirnamebuffer);
          printf("the dir name is :%s \n", dirnamebuffer);
          system(dircommandbuffer);
          break;
        }
      }

      fp3 = fopen(urlbuffer, "wb");
      printf("writing into: %s\n", urlbuffer);

#if 0
      if (LookForArrayInsideArray(urlbuffer, strlen(urlbuffer), "/", 1) >= 0)
      {
        printf("yes it is a dir\n");
      }
      else
      {
        printf("not a dir\n");
      }
#endif
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp3);

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
      fclose(fp3);

      curl_free(path);
    }
  }
#endif
  /* close */
  pclose(fp2);

  fclose(fp);
  curl_easy_cleanup(curl);

  curl_free(host);

  return 0;
}

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize, int ElementCount)
{
  int i = 0;
  int counter = 0;
  if (NeedleSize > HaystackSize)
  {
    return -1;
  }
  for (i = 0; i < (HaystackSize - NeedleSize + 1); i++)
  {
    if (strncmp(&cpHaystack[i], cpNeedle, NeedleSize) == 0) // then they are the same, return the index value
    {
      counter++;
    }
    if (counter == ElementCount)
    {
      return i;
    }
  }
  return -1;
}