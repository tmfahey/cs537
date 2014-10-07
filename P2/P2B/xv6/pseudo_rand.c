#include<stdio.h>
#include<stdint.h>

static uint32_t temper(unsigned int);
uint32_t lcg64_temper(uint64_t *);

int main(void)
{
   uint64_t seed = 100;

	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));
	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));
	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));
	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));
	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));
	printf("\nRand# = %d", (lcg64_temper(&seed) % 100));

}


static uint32_t temper(uint32_t x)
{
    x ^= x>>11;
    x ^= x<<7 & 0x9D2C5680;
    x ^= x<<15 & 0xEFC60000;
    x ^= x>>18;
    return x;
}

uint32_t lcg64_temper(uint64_t *seed)
{
    *seed = 6364136223846793005ULL * *seed + 1;
    return temper(*seed >> 32);
}
