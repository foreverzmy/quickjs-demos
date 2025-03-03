CC = gcc
QUICKJS_PATH = ../quickjs
CFLAGS = -I$(QUICKJS_PATH) -Wall
LDFLAGS = $(QUICKJS_PATH)/libquickjs.a
LIBUV_PATH = $(shell pkg-config --cflags --libs libuv)

main: main.c $(QUICKJS_PATH)/libquickjs.a
	$(CC) $(CFLAGS) $(LIBUV_PATH) -lcurl -o main main.c $(LDFLAGS)

run: 
	./main test1.js test2.js test3.js test4.js test5.js test6.js

clean:
	rm -f main
