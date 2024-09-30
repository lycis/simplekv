ZIGPATH=
CC=$(ZIGPATH)zig cc -fdiagnostics-color=always
SRC=src/

.PHONY: all

windows-server-test:
	echo "⚙️ Building windows server unit tests"
	$(CC) -target x86_64-windows -DUNIT_TEST -o dist/server-test.exe $(SRC)utilfuns.c $(SRC)server.c $(SRC)kvstore.c $(SRC)server_unit_tests.c -lws2_32
	dist/server-test.exe

windows-server: windows-server-test
	echo "⚙️ Building windows server"
	$(CC) -target x86_64-windows -o dist/server.exe $(SRC)server.c $(SRC)kvstore.c $(SRC)utilfuns.c d-lws2_32

windows-client:
	echo "⚙️ Building windows client"
	$(CC) -target x86_64-windows -o dist/client.exe $(SRC)client.c -lws2_32

windows: windows-server windows-client

all: windows

clean: 
	rm -f dist/