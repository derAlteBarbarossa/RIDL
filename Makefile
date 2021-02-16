CFLAGS += -march=native -O3

all:
	gcc -o main main.c ${CFLAGS}
	gcc -o victim basic_victim.c ${CFLAGS}

clean:
	rm -f main victim 
