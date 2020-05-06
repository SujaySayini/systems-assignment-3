all: test

WTF: WTF.c
    gcc -lssl -lcrypto -o WTF WTF.c

WTFserver: WTFserver.c
    gcc -pthread -o WTFserver WTFserver.c

WTFtest: WTF WTFserver WTFserver.c
    gcc -pthread -o WTFtest WTFtest.c

clean:
    rm -rf WTF
    rm -rf WTFserver
    rm -rf WTFtest