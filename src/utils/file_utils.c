/**
 * @file file_utils.c
 * @author Asaf Levi
 * CREATED FOR IDEA
 */
#include "file_utils.h"

int count_lines_of_file(FILE* f)
{
	int lines = 1; // there is always one lines that does not have \n after it.
	fseek(f, 0, SEEK_SET);
	
	while (!feof(f))
	{
		int ch = fgetc(f);
		if (ch == '\n')
		{
			lines++;
		}
	}
	
	return lines;
}
