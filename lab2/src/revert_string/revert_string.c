#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "revert_string.h"

void RevertString(char *str)
{
	char buf;
	for(int i = 0; i < strlen(str) / 2; i++)
	{
		buf = str[i];
		str[i] = str[strlen(str) - 1 - i];
		str[strlen(str) - 1 - i] = buf;
	}
}

