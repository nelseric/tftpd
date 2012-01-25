CC=gcc
CFLAGS=-std=c99 -ggdb 
LDLIBS=
LDFLAGS=-ggdb

SOURCES=tftpd.c main.c
HEADERS=tftpd.h

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
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;
include .depend

run: all
	./$(TARGET) 
erase:
	rm $(TARGET) $(TARGET).tar.gz
clean:
	rm $(OBJECTS)
package:
	tar -czvf $(TARGET).tar.gz $(SOURCES) $(HEADERS) Makefile
