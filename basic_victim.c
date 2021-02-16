#include <stdlib.h>
#include <stdint.h>

int main(int argc, char **argv){
    char __attribute__((aligned(4096))) leak[64*64];
    //char str[32] = "root:$6$792UOYOw$LIepd3icqFmKnOJ";
	  char str[32] = "root:$6$AAAAAAAAAAAAAAAAAAAAAAAA";
    while (1) {
        *(volatile uint64_t*) leak = *((uint64_t*)str);
        asm volatile("mfence\n");
    }

}
