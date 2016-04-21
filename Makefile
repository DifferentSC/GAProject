CC = gcc
TARGET = maxcut

.SUFFIXES : .c .o

all : $(TARGET)

$(TARGET) : maxcut.o
	$(CC) -o $@ maxcut.o

clean :
	rm -f maxcut.o maxcut

