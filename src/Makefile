CFLAGS=	-fPIC
all:  directoryfd libcwrap.so ldpreload run
libcwrap.so: interceptOpen.o  interceptAccess.o cwrap.o
	$(CC) $(CFLAGS) -shared -o libcwrap.so interceptOpen.o interceptAccess.o  cwrap.o
interceptOpen.so: interceptOpen.c
	$(CC) -shared -fPIC -o interceptOpen.so interceptOpen.c 
interceptAccess.so: interceptAccess.
	$(CC) -shared -fPIC -o interceptAccess.so interceptAccess.c 
ldpreload: ldpreload.c
	$(CC) -o ldpreload ldpreload.c
directoryfd: directoryfd.c cwrap.c
	$(CC) -o directoryfd directoryfd.c  cwrap.c
run: libcwrap.so directoryfd
	LD_PRELOAD=./libcwrap.so ./directoryfd
clean:
	rm -f *.o *.so ldpreload directoryfd