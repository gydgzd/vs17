
#include "stdafx.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string>
using namespace std;

char buffer[256];

int log(const char * fmt, ...) {
	va_list ap = nullptr;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
    fflush(stdout);
    vsnprintf(buffer, 255, fmt, ap);

	fprintf(stdout, " first arg: %d \n", va_arg(ap, int) );  // va_arg to get a parameter 
	fprintf(stdout, " second arg: %d \n", va_arg(ap, int));  
	va_end(ap);
	return 1;
}

void printf_t(FILE *m_file, const char *fmt ...)
{
	va_list vp;
	va_start(vp, fmt);
	//vsprintf(buffer,fmt,vp);  
	vfprintf(m_file, fmt, vp);
	va_end(vp);
	fflush(m_file);
}
extern string getLocalTime(const char *format);
void testValist()
{
	string time = getLocalTime("%Y-%m-%d %H:%M:%S");
	char *b = "D:\\print_t.txt";
	FILE *m_filel;
	m_filel = fopen(b, "ab+");
	printf_t(m_filel, "%s  wenjiande weizhi is %s. %d%d%d\n", time.c_str(), b, 1, 2, 3);
	fflush(m_filel);
	fclose(m_filel);

	log("nihao %d %d\n",5,9);
	return;
}