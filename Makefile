# change these to reflect your Lua installation
V = 5.3
INSTALL_TOP= /usr/local
INSTALL_BIN= $(INSTALL_TOP)/bin
INSTALL_INC= $(INSTALL_TOP)/include
INSTALL_LIB= $(INSTALL_TOP)/lib
INSTALL_MAN= $(INSTALL_TOP)/man/man1
INSTALL_LMOD= $(INSTALL_TOP)/share/lua/$V
INSTALL_CMOD= $(INSTALL_TOP)/lib/lua/$V

CC = gcc
CFLAGS = -O2 -Wall -std=c99 -fpic -pedantic -shared $(INCLUDES)
AR= ar rcu
RANLIB= ranlib

INCLUDES = -I .
OBJS = ltime.o datetime.o datetime_format.o epoch.o
LIB = ltime.so
LIBA = liblua_ltime.a

make: $(LIB) $(LIBA)

$(LIB) $(LIBA): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(LIB)
	$(AR) $(LIBA) $(OBJS)
	$(RANLIB) $(LIBA)

$(OBJS): ltime.h

.PHONY : clean
clean:
	rm -f $(OBJS) $(LIB) $(LIBA)

test: make
	lua test.lua

re: clean make

install: make
	install -D -s $(LIB) $(INSTALL_CMOD)/$(LIB)
	install -p   $(LIBA) $(INSTALL_LIB)/$(LIBA)

