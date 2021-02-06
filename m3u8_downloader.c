#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <curl/curl.h>
int main(int argc, char **argv)
{
    CURL *curl;
    FILE *fp;
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
    curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    result = curl_easy_perform(curl);

    if (result == CURLE_OK)
    {
        printf("Download is successfull\n");
    }
    else
    {
        printf("ERROR: %s\n", curl_easy_strerror(result));
    }

    fclose(fp);
    curl_easy_cleanup(curl);

    return 0;
}