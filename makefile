
CFLAGS+=-g -std=c11 -Wall -Wextra -pedantic -Wno-deprecated -Wstack-usage=1024

codec: encoder.c
	gcc -g -std=c11 -Wall -Wextra -pedantic -Wno-deprecated -Wstack-usage=1024 -o codec encoder.c -lm

debug: CFLAGS+=-g
debug: codec

clean:
	-rm codec

