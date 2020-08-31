#include <stdio.h>

void testVolatile()
{
	int i = 10;
	int a = i;
	printf("i= %d\n", a);
	//下面汇编语句的作用就是改变内存中i的值，但是又不让编译器知道
	__asm {
		mov dword ptr[ebp - 4], 20h
	}

	int b = i;
	printf("i= %d\n", b);
}