#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>

int LookForArrayInsideArray(char *cpHaystack, int HaystackSize, char *cpNeedle, int NeedleSize, int ElementCount);

int main(int argc, char **argv)
{
  CURL *curl;
  FILE *fp1;
  CURLU *curlu_handle;
  CURLUcode curlu_code;
  int result;
  char *host;
  char *path;
  char DirNameBuffer[100];    // to keep the directory names
  char DirCommandBuffer[150]; // to keep the command for creation of directiories (with mkdir)
  int i = 0;                  //index variable;
  int j = 0;                  //index variable;

  int NewlineSecond = 0; //  to parse the ts files
  int NewlineThird = 0;  //  to parse the ts files

  char RelativePathBuffer[50]; // the path of the files & folders are kept here

  int Index_StartOfPath = 0; // to track the beginning and the end of the path
  int Index_EndOfPath = 0;   // to track the beginning and the end of the path

  char *MasterPlaylistContent; // the content of the master.m3u8 is held here
  char *ProgIndexContent;      // the content of the prog_index.m3u8 is held here

  char WholeUrl[500]; // the modified url operations are done using this buffer

  double DownloadSize; // to get the sizes of the downloads

  int SearchReturn = 0;       // used for tracking the index values
  int ResolutionValue = 0;    // obtaining the resolution value
  int ResolutionValueMax = 0; // save the max resolutiuon value
  int MaxResIndex = 0;        // save the index of max resolution for later use
  char FilePathBuffer[200];   // file path operations are done using this buffer

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
  else
  {
    printf("curl_url_get error\n");
    return -1;
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
  else
  {
    return -1;
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

  MasterPlaylistContent = malloc(sizeof(char) * DownloadSize);

  fclose(fp1);
  fp1 = fopen("master.m3u8", "r"); // open the master playlist for reading, we will parse it

  i = 0;
  while (!feof(fp1))
  {
    MasterPlaylistContent[i] = getc(fp1);
    i++;
  }

  for (i = 0; i < DownloadSize; i++) // find the greatest resolution playlist
  {
    SearchReturn = LookForArrayInsideArray(MasterPlaylistContent, DownloadSize, "RESOLUTION", strlen("RESOLUTION"), i + 1);
    if (SearchReturn < 0)
    {
      break;
    }
    else
    {
      ResolutionValue = atoi((MasterPlaylistContent + SearchReturn + 11)); // save the greatest resolution & its place
      if (ResolutionValue > ResolutionValueMax)
      {
        ResolutionValueMax = ResolutionValue;
        MaxResIndex = SearchReturn;
      }
    }
  }

  SearchReturn = LookForArrayInsideArray(&MasterPlaylistContent[MaxResIndex], DownloadSize, "\n", 1, 1); // find the start of the highest resolution file path
  if (SearchReturn < 0)
  {
    return -1;
  }
  else
  {
    Index_StartOfPath = SearchReturn;
#ifdef DEBUG
    printf("Index_StartOfPath: %d\n", Index_StartOfPath);
#endif
  }

  SearchReturn = LookForArrayInsideArray(&MasterPlaylistContent[MaxResIndex], DownloadSize, "\n", 1, 2); // find the end of the highest resolution file path
  if (SearchReturn < 0)
  {
    return -1;
  }
  else
  {
    Index_EndOfPath = SearchReturn;
#ifdef DEBUG
    printf("Index_EndOfPath: %d\n", Index_EndOfPath);
#endif
  }

  memcpy(RelativePathBuffer, &MasterPlaylistContent[MaxResIndex + Index_StartOfPath + 1], (Index_EndOfPath - Index_StartOfPath - 1)); // get the the highest resolution file path
  RelativePathBuffer[Index_EndOfPath - Index_StartOfPath - 1] = '\0';
#ifdef DEBUG
  printf("RelativePathBuffer: %s\n", RelativePathBuffer);
#endif
  curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, argv[1], 0);
  if (curlu_code)
  {
    return -1;
  }
  curlu_code = curl_url_set(curlu_handle, CURLUPART_URL, RelativePathBuffer, 0); // set the the highest resolution file path
  if (curlu_code)
  {
    printf("url get error \n");
    return -1;
  }
  curlu_code = curl_url_get(curlu_handle, CURLUPART_PATH, &path, 0);
  if (!curlu_code)
  {
#ifdef DEBUG
    printf("path: %s\n", path);
#endif
  }
  else
  {
    return -1;
  }

  memset(WholeUrl, 0, 500);
  sprintf(WholeUrl, "%s%s%s", "https://", host, path); // construct the url to be downloaded
#ifdef DEBUG
  printf("WholeUrl: %s\n", WholeUrl);
#endif

  curl_easy_setopt(curl, CURLOPT_URL, WholeUrl);
  for (i = strlen(RelativePathBuffer); i > 0; i--) // start looking for the directories from behind, we will create directories recursively using "mkdir -p"
  {
    if (RelativePathBuffer[i - 1] == '/') //then we have directories, we have to create those directories
    {
      memset(DirNameBuffer, 0, 100);
      memset(DirCommandBuffer, 0, 150);
      strncpy(DirNameBuffer, RelativePathBuffer, i);
      sprintf(DirCommandBuffer, "mkdir -p %s", DirNameBuffer);
#ifdef DEBUG
      printf("the directory name is :%s \n", DirNameBuffer);
      printf("the command is :%s \n", DirCommandBuffer);
#endif
      system(DirCommandBuffer);
      break;
    }
  }

  fclose(fp1);
  fp1 = fopen(RelativePathBuffer, "wb");
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

  result = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &DownloadSize); // get the download size for allocation
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

  ProgIndexContent = malloc(DownloadSize * sizeof(char));

  fclose(fp1);
  fp1 = fopen(RelativePathBuffer, "r"); // open the file for parse operations

  i = 0;
  while (!feof(fp1))
  {
    ProgIndexContent[i] = getc(fp1);
    i++;
  }

#ifdef DEBUG
  printf("%s", ProgIndexContent);
#endif

  memset(RelativePathBuffer, 0, 50);

  fp1 = fopen(argv[2], "wb");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp1);

  for (i = 0; i < DownloadSize; i++) // find the ts file names & construct the urls for download & perform download
  {
    SearchReturn = LookForArrayInsideArray(ProgIndexContent, DownloadSize, "#EXTINF", 7, i + 1); // look for each resolution information
    if (SearchReturn < 0)
    {
      break;
    }

    NewlineSecond = LookForArrayInsideArray(&ProgIndexContent[SearchReturn], DownloadSize - SearchReturn, "\n", 1, 2);
    NewlineThird = LookForArrayInsideArray(&ProgIndexContent[SearchReturn], DownloadSize - SearchReturn, "\n", 1, 3);

    memcpy(RelativePathBuffer, (ProgIndexContent + SearchReturn + NewlineSecond + 1), NewlineThird - NewlineSecond - 1);
    sprintf(FilePathBuffer, "%s%s", DirNameBuffer, RelativePathBuffer);
#ifdef DEBUG
    printf("RelativePathBuffer: %s\n", RelativePathBuffer);
    printf("DirNameBuffer %s\n", DirNameBuffer);
    printf("FilePathBuffer %s\n", FilePathBuffer);
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
#ifdef DEBUG
      printf("path: %s\n", path);
#endif
    }
    else
    {
      return -1;
    }
    memset(WholeUrl, 0, 500);
    sprintf(WholeUrl, "%s%s%s", "https://", host, path);
#ifdef DEBUG
    printf("WholeUrl: %s\n", WholeUrl);
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

  fclose(fp1);
  curl_easy_cleanup(curl);
  curl_free(host);
  free(MasterPlaylistContent);
  free(ProgIndexContent);

  printf("The program is finished successfully\n");
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