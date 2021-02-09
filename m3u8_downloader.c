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
  FILE *fp1;
  FILE *fp2;
  FILE *fp3;
  CURLU *curlu_handle;
  CURLUcode curlu_code;
  int result;
  char *host;
  char *path;
  char DirNameBuffer[100];    // to keep the directory names
  char DirCommandBuffer[150]; // to keep the command for creation of directiories (with mkdir)
  char ScriptContent[500];    // we are creating the js scripty manually, therefore the script content is buffered here
  int i = 0;                  //index variable;
  int j = 0;                  //index variable;

  char *ptrParsedOutput;      // parsed content of the master playlist is buffered here
  char *ptrProgIndex_content; // parsed content of the highest resolution playlist is buffered here

  char WholeUrl[500]; // the modified url operations are done using this buffer

  double DownloadSize; // to get the sizes of the downloads

  int SearchReturn = 0;        // used for tracking the index values
  int ResolutionValue = 0;     // obtaining the resolution value
  int ResolutionValueMax = 0;  // save the max resolutiuon value
  int MaxResIndex = 0;         // save the index of max resolution for later use
  int MaxResUrlIndex = 0;      // save the url index of max resulution for later use
  char RelativeUrlBuffer[100]; // url operations are done using this buffer
  char FilePathBuffer[200];    // file path operations are done using this buffer

  curlu_handle = curl_url(); /* get a handle to work with */
  if (!curlu_handle)
  {
    return -1;
  }

  /* parse a full URL */
  curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, argv[1], 0);
  if (curlu_code)
  {
    return -1;
  }
  /* extract host name from the parsed URL */
  curlu_code = curl_url_get(curlu_handle, CURLUPART_HOST, &host, 0);
  if (!curlu_code)
  {
    printf("Host name: %s\n", host);
  }

  /* extract the path from the parsed URL */
  curlu_code = curl_url_get(curlu_handle, CURLUPART_PATH, &path, 0);
  if (!curlu_code)
  {
#ifdef INFO
    printf("Path: %s\n", path);
#endif
    curl_free(path);
  }

  fp1 = fopen("master.m3u8", "wb");

  curl = curl_easy_init();
  if (curl == NULL)
  {
    printf("Curl initialization error: %d \n", errno);
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, argv[1]);   // get the url
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp1); // save the data in the fp1

  curl_easy_setopt(curl, CURLOPT_CONV_FROM_UTF8_FUNCTION, 1L);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

  result = curl_easy_perform(curl); // perform the curl operation

  if (result == CURLE_OK)
  {
    printf("Download of the master playlist is successful\n");
  }
  else
  {
    printf("ERROR: %s\n", curl_easy_strerror(result));
    return result;
  }

  result = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &DownloadSize); // get the download size
  if (result == CURLE_OK)
  {
#ifdef DEBUG
    printf("Downloaded %.0f bytes\n", DownloadSize);
#endif
  }
  else
  {
    return result;
  }

  ptrParsedOutput = malloc(sizeof(char) * DownloadSize * 3); // allocate memory for parsing output

  /* run the javascript for parsing of the master playlist. */
  fp2 = popen("node index.js", "r");
  if (fp2 == NULL)
  {
    printf("Failed to run command\n");
    exit(1);
  }

  // get the content of the parsed output
  while (!feof(fp2))
  {
    ptrParsedOutput[i] = getc(fp2);
    i++;
  }
  //fclose(fp2);
#ifdef DEBUG
  printf("ptrParsedOutput:%s\n", ptrParsedOutput);
#endif

  // parse the data inside the ptrParsedOutput
  for (i = 0; i < DownloadSize * 3; i++)
  {
    SearchReturn = LookForArrayInsideArray(ptrParsedOutput, 3 * DownloadSize, "resolution", 10, i + 1); // look for each resolution information
    if (SearchReturn < 0)
    {
      break;
    }
    ResolutionValue = atoi(&ptrParsedOutput[SearchReturn + 13]); // get the resolution value, 13 is the offset value of the number value from the tag
#ifdef DEBUG
    printf("SearchReturn: %d\n", SearchReturn);
    printf("ptrParsedOutput[search+13]: %c\n", ptrParsedOutput[SearchReturn + 13]);
    printf("ResolutionValue: %d\n", ResolutionValue);
#endif
    if (ResolutionValue > ResolutionValueMax) // save the information about the maximum resolution for later use
    {
      ResolutionValueMax = ResolutionValue;
      MaxResIndex = SearchReturn;
      MaxResUrlIndex = LookForArrayInsideArray(ptrParsedOutput, 3 * DownloadSize, "url", 3, i + 1);
    }
  }
#ifdef DEBUG
  printf("ResolutionValueMax: %d\n", ResolutionValueMax);
  printf("MaxResIndex: %d\n", MaxResIndex);
  printf("MaxResUrlIndex: %d\n", MaxResUrlIndex);
#endif
  memset(RelativeUrlBuffer, 0, 100);
  for (i = 0; i < 100; i++) // save the relative url information for later use (i.e. download operation)
  {
    if (ptrParsedOutput[MaxResUrlIndex + 7 + i] != '\"')
    {
      RelativeUrlBuffer[i] = ptrParsedOutput[MaxResUrlIndex + 7 + i];
    }
    else
    {
      RelativeUrlBuffer[i] = '\0';
      break;
    }
  }
#ifdef DEBUG
  printf("RelativeUrlBuffer: %s\n", RelativeUrlBuffer);
  printf("the input url is :%s \n", argv[1]);
#endif
  curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, argv[1], 0);
  if (curlu_code)
  {
    return -1;
  }
  curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, RelativeUrlBuffer, 0);
  if (curlu_code)
  {
    printf("url get error \n");
    return -1;
  }
  curlu_code = curl_url_get(curlu_handle, CURLUPART_PATH, &path, 0);
  if (!curlu_code)
  {
#ifdef INFO
    printf("path: %s\n", path);
#endif
  }

  memset(WholeUrl, 0, 500);
  sprintf(WholeUrl, "%s%s%s", "https://", host, path); // construct the url to be downloaded
#ifdef DEBUG
  printf("WholeUrl: %s\n", WholeUrl);
#endif
  curl_easy_setopt(curl, CURLOPT_URL, WholeUrl);
  for (i = strlen(RelativeUrlBuffer); i > 0; i--) // start looking for the directories from behind, we will create directiries recursively using "mkdir -p"
  {
    if (RelativeUrlBuffer[i - 1] == '/') //then we have directories, we have to create those directories
    {
      memset(DirNameBuffer, 0, 100);
      memset(DirCommandBuffer, 0, 150);
      strncpy(DirNameBuffer, RelativeUrlBuffer, i);
      sprintf(DirCommandBuffer, "mkdir -p %s", DirNameBuffer);
#ifdef DEBUG
      printf("the directory name is :%s \n", DirNameBuffer);
#endif
      system(DirCommandBuffer);
      break;
    }
  }

  fclose(fp1);
  fp1 = fopen(RelativeUrlBuffer, "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp1);

  result = curl_easy_perform(curl); // we have the information about the highest resolution variant link, perform the operation
  if (result == CURLE_OK)
  {
    printf("Download of highest resolution variant link is successful\n");
  }
  else
  {
    printf("ERROR: %s\n", curl_easy_strerror(result));
    return result;
  }

  /*construct the javascript for the parse operations of the highest resolution variant link*/
  sprintf(ScriptContent, "%s%s%s", SCRIPT_FIRSTPART, RelativeUrlBuffer, SCRIPT_SECONDPART);

#ifdef DEBUG
  printf("ScriptContent:\n%s\n", ScriptContent);
  printf("RelativeUrlBuffer:\n%s\n", RelativeUrlBuffer);
#endif

  /*create the javascript that is constructed for parsing of highest resolution variant link*/
  fp3 = fopen("highres.js", "wb");
  fwrite(ScriptContent, 1, strlen(ScriptContent), fp3);
  fclose(fp3);

  /*call the javascript that is constructed for parsing of highest resolution variant link*/
  fp3 = popen("node highres.js", "r");
  if (fp3 == NULL)
  {
    printf("Failed to run command\n");
    exit(1);
  }

  /*get the size for creating a buffer for parsed output*/
  result = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &DownloadSize);
  if (result == CURLE_OK)
  {
#ifdef DEBUG
    printf("Downloaded %.0f bytes\n", DownloadSize);
#endif
  }
  else
  {
    return result;
  }

  // construct buffer for parsed output
  ptrProgIndex_content = malloc(sizeof(char) * DownloadSize * 3);
  memset(ptrProgIndex_content, 0, sizeof(sizeof(char) * DownloadSize * 3));

  i = 0;
  while (!feof(fp3))
  {
    // get the parsed output of highest resolution variant link
    ptrProgIndex_content[i] = getc(fp3);
    i++;
  }
#ifdef DEBUG
  printf("%s\n", ptrProgIndex_content);
#endif
  pclose(fp3);

  fp1 = fopen(argv[2], "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp1);

#ifdef DEBUG
  printf("RelativeUrlBuffer: %s\n", RelativeUrlBuffer);
#endif

  for (i = strlen(RelativeUrlBuffer); i > 0; i--)
  {
    if (RelativeUrlBuffer[i - 1] == '/') //then we have directories, we have to create those directories recursively
    {
      memset(DirNameBuffer, 0, 100);
      memset(DirCommandBuffer, 0, 150);
      strncpy(DirNameBuffer, RelativeUrlBuffer, i);
      sprintf(DirCommandBuffer, "mkdir -p %s", DirNameBuffer);
#ifdef DEBUG
      printf("the dir name is :%s \n", DirNameBuffer);
#endif
      system(DirCommandBuffer);
      break;
    }
  }

  // now everything is done, we can start downloading of the ts files
  for (i = 0; i < DownloadSize * 3; i++)
  {
    SearchReturn = LookForArrayInsideArray(ptrProgIndex_content, 2 * DownloadSize, "url", 3, i + 1);
    if (SearchReturn < 0)
    {
      break;
    }
    for (j = 0; j < 100; j++)
    {
      if (ptrProgIndex_content[SearchReturn + 7 + j] != '\"')
      {
        RelativeUrlBuffer[j] = ptrProgIndex_content[SearchReturn + 7 + j];
      }
      else
      {
        RelativeUrlBuffer[j] = '\0';
        break;
      }
    }
    sprintf(FilePathBuffer, "%s%s", DirNameBuffer, RelativeUrlBuffer);
#ifdef DEBUG
    printf("RelativeUrlBuffer: %s\n", RelativeUrlBuffer);
    printf("FilePathBuffer: %s\n", FilePathBuffer);
#endif
    curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, argv[1], 0);
    if (curlu_code)
    {
      return -1;
    }
    curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, FilePathBuffer, 0);
    if (curlu_code)
    {
      printf("url get error \n");
      return -1;
    }
    curlu_code = curl_url_get(curlu_handle, CURLUPART_PATH, &path, 0);
    if (!curlu_code)
    {
#ifdef INFO
      printf("path: %s\n", path);
#endif
    }

    memset(WholeUrl, 0, 500);
    sprintf(WholeUrl, "%s%s%s", "https://", host, path);
#ifdef DEBUG
    printf("DirNameBuffer: %s\n", DirNameBuffer);
    printf("WholeUrl: %s\n", WholeUrl);
    printf("path: %s\n", path);
#endif
    curl_easy_setopt(curl, CURLOPT_URL, WholeUrl);
    result = curl_easy_perform(curl);
    if (result == CURLE_OK)
    {
      printf("Download is successful from: %s\n", WholeUrl);
    }
    else
    {
      printf("ERROR: %s\n", curl_easy_strerror(result));
      return result;
    }
  }

  /* close */
  pclose(fp2);

  fclose(fp1);
  curl_easy_cleanup(curl);

  curl_free(host);

  free(ptrParsedOutput);
  free(ptrProgIndex_content);

  printf("Terminating: Operation is successful \n");
  return 0;
}

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize, int ElementCount)
{
  /*Look for the elements, return its index if it exists.
  Indexes of repeating elements can be found if ElementCount>1*/
  int i = 0;
  int counter = 0;
  if (NeedleSize > HaystackSize)
  {
    return -1;
  }
  for (i = 0; i < (HaystackSize - NeedleSize + 1); i++)
  {
    if (strncmp(&cpHaystack[i], cpNeedle, NeedleSize) == 0) // then they are the same, increment the counter
    {
      counter++;
    }
    if (counter == ElementCount) //if element count is reach, return the index value
    {
      return i;
    }
  }
  return -1; // the element does not exists, notify the user
}