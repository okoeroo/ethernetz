RM = /bin/rm
CAT = /bin/cat
CC = gcc

CFLAGS = \
    -Wcast-qual -Wchar-subscripts -Winline \
    -Wmissing-prototypes -Wnested-externs -Wpointer-arith \
    -O3 -mtune=core2 


CPPFLAGS = \
	-I. \

BIN   =  ./bin/ethernetz

OBJS  = \
	src/main.o


all: $(BIN)
	$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(LFLAGS)

# $(OBJS): 
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BIN) $(OBJS) *~

distclean: clean
