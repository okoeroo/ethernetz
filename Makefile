RM = /bin/rm
CAT = /bin/cat
CC = gcc

CFLAGS = \
    -Wcast-qual -Wchar-subscripts -Winline \
    -Wmissing-prototypes -Wnested-externs -Wpointer-arith \
    -O3 -mtune=core2 


CPPFLAGS = \
	-I. \

BIN   =  ./bin/ethernetz_$(shell uname -m)

OBJS  = \
	src/main.o


all: $(BIN)
	cd `dirname $(BIN)`; ln -s `basename $(BIN)` ethernetz; cd -; $(BIN) eth0

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(LFLAGS)

# $(OBJS): 
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BIN) $(OBJS) *~

distclean: clean
