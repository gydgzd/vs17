/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alan Antonuk are Copyright (c) 2012-2013
 * Alan Antonuk. All Rights Reserved.
 *
 * Portions created by VMware are Copyright (c) 2007-2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Portions created by Tony Garnock-Jones are Copyright (c) 2009-2010
 * VMware, Inc. and Tony Garnock-Jones. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */
#include "stdafx.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <stdint.h>

#include "utils.h"
#include "Mylog.h"

char * g_logFile = "log/rabbitmq.log";
std::mutex g_logMutex;
void die(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(1);
}
int log(const char *fmt, ...) {
	string tmpmsg ;
	tmpmsg = getLocalTime();
#ifdef WINVER
	_mkdir("./log");
#endif
	FILE *pfile;
	errno_t err;
	if ((err = fopen_s(&pfile, g_logFile, "a+")) != NULL)     //判断文件打开
	{
		char szLog[1280] = "";
		char szError[512] = "";
		printf_s(szLog, "Couldn't open %s!\n\n %s", g_logFile, szError);
		exit(1);    // #include <stdlib.h>                     
	}

	va_list ap;
	va_start(ap, fmt);
	{
		std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
		fprintf(pfile, "%s  Error while ", tmpmsg.c_str());
		vfprintf(pfile, fmt, ap);
	}
	va_end(ap);
	fprintf(pfile, " .\n");
	fclose(pfile);
	return 1;
}

void die_on_error(int x, char const *context) {
  if (x < 0) {
    fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x));
    exit(1);
  }
}
int log_on_error(int x, char const *context) {
	char tmpmsg[1024] = "";
	if (x < 0) {
		sprintf_s(tmpmsg, "%s: %s.", context, amqp_error_string2(x));
		Mylog tmplog("log/rabbitmq.log");
		std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
		tmplog.logException(tmpmsg);
		return 1;
	}
	return 0;
}

void die_on_amqp_error(amqp_rpc_reply_t x, char const *context) {
  string date;
  date = getLocalTime();
  switch (x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
		return;

    case AMQP_RESPONSE_NONE:
      fprintf(stderr, "%s  %s: missing RPC reply type!\n", date.c_str(), context);
      break;

    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      fprintf(stderr, "%s  %s: %s\n", date.c_str(), context, amqp_error_string2(x.library_error));
      break;

    case AMQP_RESPONSE_SERVER_EXCEPTION:
      switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
          amqp_connection_close_t *m =
              (amqp_connection_close_t *)x.reply.decoded;
          fprintf(stderr, "%s  %s: server connection error %uh, message: %.*s\n", date.c_str(),
                  context, m->reply_code, (int)m->reply_text.len,
                  (char *)m->reply_text.bytes);
          break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
          amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
          fprintf(stderr, "%s  %s: server channel error %uh, message: %.*s\n", date.c_str(),
                  context, m->reply_code, (int)m->reply_text.len,
                  (char *)m->reply_text.bytes);
          break;
        }
        default:
          fprintf(stderr, "%s  %s: unknown server error, method id 0x%08X\n", date.c_str(),
                  context, x.reply.id);
          break;
      }
      break;
  }// end of switch
  exit(1);
}

int log_on_amqp_error(amqp_rpc_reply_t x, char const * context)
{
	string date;
	date = getLocalTime();
	FILE *pfile;
	errno_t err;
	if ((err = fopen_s(&pfile, g_logFile, "a+")) != NULL)     //判断文件打开
	{
		char szLog[1280] = "";
		char szError[512] = "";
		fprintf(stderr, "Couldn't open %s!\n\n %s", g_logFile, szError);
		exit(1);    // #include <stdlib.h>                     
	}
	switch (x.reply_type) {
	case AMQP_RESPONSE_NORMAL:
	{
		fclose(pfile);
		return 0;
	}
	case AMQP_RESPONSE_NONE:
	{	
		std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
		fprintf(pfile, "%s  %s: missing RPC reply type!\n", date.c_str(), context);
		break;
	}
	case AMQP_RESPONSE_LIBRARY_EXCEPTION:
	{
		std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
		fprintf(pfile, "%s  %s: %s\n", date.c_str(), context, amqp_error_string2(x.library_error));
		break;
	}
	case AMQP_RESPONSE_SERVER_EXCEPTION:
		switch (x.reply.id) {
		case AMQP_CONNECTION_CLOSE_METHOD: {
			amqp_connection_close_t *m = (amqp_connection_close_t *)x.reply.decoded;
			std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
			fprintf(pfile, "%s  %s: server connection error %uh, message: %.*s\n", date.c_str(),
				context, m->reply_code, (int)m->reply_text.len,
				(char *)m->reply_text.bytes);
			break;
		}
		case AMQP_CHANNEL_CLOSE_METHOD: {
			amqp_channel_close_t *m = (amqp_channel_close_t *)x.reply.decoded;
			std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
			fprintf(pfile, "%s  %s: server channel error %uh, message: %.*s\n", date.c_str(),
				context, m->reply_code, (int)m->reply_text.len,
				(char *)m->reply_text.bytes);
			break;
		}
		default:
		{	
			std::lock_guard<std::mutex> guard(g_logMutex);   // 加锁，保证下面语句原子执行
			fprintf(pfile, "%s  %s: unknown server error, method id 0x%08X\n", date.c_str(),
			context, x.reply.id);
			break;
		}
		}
		break;
	}
	fclose(pfile);
	return 1;
}

static void dump_row(long count, int numinrow, int *chs) {
  int i;

  printf("%08lX:", count - numinrow);

  if (numinrow > 0) {
    for (i = 0; i < numinrow; i++) {
      if (i == 8) {
        printf(" :");
      }
      printf(" %02X", chs[i]);
    }
    for (i = numinrow; i < 16; i++) {
      if (i == 8) {
        printf(" :");
      }
      printf("   ");
    }
    printf("  ");
    for (i = 0; i < numinrow; i++) {
      if (isprint(chs[i])) {
        printf("%c", chs[i]);
      } else {
        printf(".");
      }
    }
  }
  printf("\n");
}

static int rows_eq(int *a, int *b) {
  int i;

  for (i = 0; i < 16; i++)
    if (a[i] != b[i]) {
      return 0;
    }

  return 1;
}

void amqp_dump(void const *buffer, size_t len) {
  unsigned char *buf = (unsigned char *)buffer;
  long count = 0;
  int numinrow = 0;
  int chs[16];
  int oldchs[16] = {0};
  int showed_dots = 0;
  size_t i;

  for (i = 0; i < len; i++) {
    int ch = buf[i];

    if (numinrow == 16) {
      int j;

      if (rows_eq(oldchs, chs)) {
        if (!showed_dots) {
          showed_dots = 1;
          printf(
              "          .. .. .. .. .. .. .. .. : .. .. .. .. .. .. .. ..\n");
        }
      } else {
        showed_dots = 0;
        dump_row(count, numinrow, chs);
      }

      for (j = 0; j < 16; j++) {
        oldchs[j] = chs[j];
      }

      numinrow = 0;
    }

    count++;
    chs[numinrow++] = ch;
  }

  dump_row(count, numinrow, chs);

  if (numinrow != 0) {
    printf("%08lX:\n", count);
  }
}
