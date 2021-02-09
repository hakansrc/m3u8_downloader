m3u8_downloader.o: m3u8_downloader.h m3u8_downloader.c
    gcc -o m3u8_downloader.o  m3u8_downloader.c -lcurl