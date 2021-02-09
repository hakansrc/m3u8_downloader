TARGET = m3u8_downloader

OBJS= m3u8_downloader.o

CC = gcc

CFLAGS = -c -g

LIBS = -lcurl  

$(TARGET) : $(OBJS)
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

m3u8_downloader.o : m3u8_downloader.c
	$(CC) $(CFLAGS) $<
