#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define FIRSTPART "const M3U8FileParser = require('m3u8-file-parser');\n\
const fs = require('fs');\n\
const content = fs.readFileSync('" /*master.m3u8*/

#define SECONDPART "', { encoding: 'utf-8' });\n\
const reader = new M3U8FileParser();\n\
reader.read(content);\n\
console.log(JSON.stringify(reader.getResult(), null, 2));"

int main(void)
{
    FILE *fp;
    char path[1035];

    char content[500];
    sprintf(content, "%s%s%s", FIRSTPART, "v9/prog_index.m3u8", SECONDPART);
    printf("content:\n%s\n", content);

    fp = fopen("falann.js", "wb");
    fwrite(content, 1, strlen(content), fp);
    pclose(fp);

    /* Open the command for reading. */
    fp = popen("node falann.js", "r");
    if (fp == NULL)
    {
        printf("Failed to run command\n");
        exit(1);
    }
    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL)
    {
        //usleep(10000);
        printf("%s", path);
    }
    /* close */
    pclose(fp);

    return 0;
}