CC := gcc

all: xpp
	true

lua.a:
	cd luaobj; $(CC) -c ../luasrc/*.c
	ar rcs lua.a luaobj/*.o

lib.a:
	cd libobj; $(CC) -c ../libsrc/*.c
	ar rcs lib.a libobj/*.o

%.o: %.c
	$(CC) -c $< -o $@

xpp += main.o
xpp += patlist.o strtest.o output.o tokenizer.o luab.o parser.c
xpp += lib.a lua.a

xpp: $(xpp)
	$(CC) $(xpp) -lm -o xpp

clean:
	rm luaobj/*.o
	rm libobj/*.o
	rm *.o
	rm *.a

