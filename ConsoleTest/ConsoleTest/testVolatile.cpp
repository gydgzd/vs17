#include <stdio.h>

void testVolatile()
{
	int i = 10;
	int a = i;
	printf("i= %d\n", a);
	//�������������þ��Ǹı��ڴ���i��ֵ�������ֲ��ñ�����֪��
	__asm {
		mov dword ptr[ebp - 4], 20h
	}

	int b = i;
	printf("i= %d\n", b);
}