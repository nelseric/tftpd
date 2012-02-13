CC=gcc
CFLAGS=-std=c99 -ggdb -Wall
LDLIBS=
LDFLAGS=-ggdb

SOURCES=tftp.c tftpd.c

OBJECTS=$(SOURCES:.c=.o)

TARGET=tftpd


.PHONY : clean run package erase

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(LDLIBS) $(OBJECTS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.depend: depend 


depend: $(SOURCES)
	ctags -R
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;
include .depend

run: all
	./$(TARGET) 
erase:
	rm $(TARGET) $(TARGET).tar.gz
clean:
	rm $(OBJECTS)
