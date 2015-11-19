
CFLAGS+=-g -std=c11 -Wall -Wextra -pedantic -Wno-deprecated -Wstack-usage=1024

decoder: decoder.c
	gcc $(CFLAGS) -o decoder decoder.c -lm

encoder: encoder.c
	gcc $(CFLAGS) -o encoder encoder.c -lm

debug: CFLAGS+=-g
debug: encoder, decoder

clean:
	-rm decoder
	-rm encoder

