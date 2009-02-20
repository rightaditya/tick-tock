#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/reg_m55800.h"

#define NVR_BASE 0x20000000
#define MEM_SIZE 0x200000

void setup();

int main()
{
    int firstrun = 1, error = 0;
    char *ptr = (char*)BASE_ADDRESS;
    char *start, *end;

    setup();

    start = 0x0;
    while (start < BASE_ADDRESS || end > BASE_ADDRESS + MEM_SIZE || start >= end)
    {   
	if (firstrun == 0)
	    printf("Invalid input, please enter the addresses again...\n");
	
	printf("Enter the start address (inclusive) in hexadecimal: ");
	scanf("%x", &start);
	printf("Enter the end address (exclusive) in hexadecimal: ");
	scanf("%x", &end);
	
	firstrun = 0;
    }

    while (start < end)
    {
	char value;
	int i = 0;

	*start = 0x01;
	value = *start;
	
	while (i < 8)
	{
	    if (value != *start)
	    {
		printf("Memory fault at location %p\n", start);
		error = 1;
	    }

	    *start <<= 1;
	    value <<= 1;
	    
	    ++i;
	}
	
	++start;
    }
    
    if (!error)
	printf("Testing completed with no errors found.\n");

    return 0;
}

void setup()
{
    EBI_CSR3 = 0x00000000;
    EBI_CSR3 |= EBI_CSEN;
    EBI_CSR3 |= EBI_BAT_BYTE_WRITE;
    EBI_CSR3 |= EBI_TDF_1;
    EBI_CSR3 |= EBI_PAGES_4M;
    EBI_CSR3 |= EBI_WSE;
    EBI_CSR3 |= EBI_NWS_3;
    EBI_CSR3 |= EBI_DBW_8;
    EBI_CSR3 |= NVR_BASE;
}
