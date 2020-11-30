/*
 * test_popen.cpp
 *
 *  Created on: Aug 15, 2019
 *      Author: root
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <iostream>

int test_popen()
{
    char buffer[256] = { '\0' };
    char cmd[256] = "/home/gyd/gitProject/test/a.sh; /home/gyd/gitProject/test/b.sh";
    FILE *fp;
    fp = popen(cmd , "r");
    if (NULL == fp)
    {
        snprintf(buffer, sizeof(buffer), "popen failed. %s, with errno %d.\n", strerror(errno), errno);
        std::cout << buffer;
        return -1;
    }
    std::cout << " executed: " << cmd;
    int ret_code = pclose(fp);
    if(-1 == ret_code)
    {   // return -1 if wait4(2) returns an error, or some other error is detected.
        // if pclose cannot obtain the child status, errno is set to ECHILD.
        if(ECHILD==errno)
        {
            std::cout << "pclose cannot obtain the child status.\n";
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "Close file failed. %s, with errno %d.\n", strerror(errno), errno);
            std::cout << buffer;
        }
        return -1;
    }

    int status = WEXITSTATUS(ret_code);      // child process exit status.
    if(0 == status)        // success
    {
        std::cout << " execute successful.";
        return 0;
    }
    else                    // failed
    {
        std::cout  << " execute error.";
        return -1;
    }
    return 0;
}


