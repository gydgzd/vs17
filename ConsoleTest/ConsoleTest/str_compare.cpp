#include "stdafx.h"
#include <stdio.h>
int str_compare(char *str1, char *str2)
{
	int ret = 0;
	while (*str1 != 0 && *str2 != 0)
	{
		ret = *str1++ - *str2++;
		if (ret != 0)
			return ret;
	}
	if (*str1 == 0 && *str2 == 0)
		return 0;
	else if (*str1 == 0)
		return *str2;
	else
		return *str1;
}