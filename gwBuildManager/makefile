TARGET=gwBuildManager
OBJS=main.o

all:
	gcc -o $(TARGET) *.c
clean:
	rm -f $(TARGET)
install:
	cp -f $(TARGET) /bin
uninstall:
	rm -f /bin/$(TARGET)