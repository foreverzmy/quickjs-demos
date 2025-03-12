CC = gcc
QUICKJS_PATH = ../quickjs
CFLAGS = -I$(QUICKJS_PATH) -Wall
LDFLAGS = $(QUICKJS_PATH)/libquickjs.a
LIBUV_PATH = $(shell pkg-config --cflags --libs libuv)

main: main.c $(QUICKJS_PATH)/libquickjs.a
	$(CC) $(CFLAGS) $(LIBUV_PATH) -lcurl -o main main.c $(LDFLAGS)

run: 
	./main 

clean:
	rm -f main
