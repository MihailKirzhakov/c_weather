CC      ?= cc
CFLAGS  ?= -O2
CFLAGS  += -std=c11 -Wall -Wextra -Wpedantic
LDLIBS  = -lcurl -lcjson

SRC = src/main.c src/http.c src/weather.c
OBJ = $(SRC:.c=.o)

weather: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

src/%.o: src/%.c src/http.h src/weather.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f weather $(OBJ)

.PHONY: clean
