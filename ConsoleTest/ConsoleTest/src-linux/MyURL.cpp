/*
 * myURL.cpp
 *
 *  Created on: Sep 15, 2017
 *      Author: gyd
 */

#include "MyURL.h"

struct LinkInfo
{
    char traceId[32];
    char startTime[32];
    char spanId[32];
    char parentId[32];
    char duration[8];
    char serviceName[64];
    char spanName[64];
    char host[32];
    int status_code;
    int error;
    LinkInfo()
    {
        memset(this, 0, sizeof(*this));
        status_code = 0;
        error = 0;
    }
};


struct MemoryStruct {
	char *memory;
	size_t size;
};
/*call back function, that write response to memory*/
static size_t WriteMemoryCallback(void *readData, size_t size, size_t nmemb, void *userp)
{
	size_t readsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char *)realloc(mem->memory, mem->size + readsize + 1);
	if (mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	memcpy(&(mem->memory[mem->size]), readData, readsize);
	mem->size += readsize;
	mem->memory[mem->size] = 0;
	// get someline
	char *pos = mem->memory;
	while((pos = strchr(pos, '\n')) != NULL)
	{
	    *pos = 0;
	    printf("%s\n", mem->memory);
	    *pos = '\n';
	}
	return readsize;
}
int getError(char *tag, int& statusCode, int& error)
{
    char *pos;
    pos = strstr(tag, "http.status_code=");
    if(pos != NULL)
    {
        pos += strlen("http.status_code=");
        char status[8] = "";
        int idx = 0;
        while( *pos >= 48 && *pos <=57)
        {
            status[idx] = *pos;
            idx++;
            pos++;
        }
        statusCode = atoi(status);
    }
    //
    pos = strstr(tag, "error=");
    if(pos != NULL)
    {
        pos += strlen("error=");
        char err[8] = "";
        int idx = 0;
        while( *pos >= 48 && *pos <=57)
        {
            err[idx] = *pos;
            idx++;
            pos++;
        }
        error = atoi(err);
    }
    return 0;
}
/*call back function, analysis memory*/
static size_t handleMem(void *readData, size_t size, size_t nmemb, void *userp)
{
    size_t readsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = (char *)realloc(mem->memory, mem->size + readsize + 1);
    if (mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy(&(mem->memory[mem->size]), readData, readsize);
    mem->size += readsize;
    mem->memory[mem->size] = 0;
    // get someline
    char *posBegin = mem->memory;
    char *posEnd = posBegin;
    size_t nSize = 0;
    LinkInfo info;
    char tag[512];
    while((posEnd = strchr(posBegin, '\n')) != NULL)
    {
        *posEnd = 0;
        // analysize one line
     //   printf("%s\n", posBegin);
        char *pos1 = posBegin;
        char *pos2 = pos1;
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.traceId, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.startTime, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.spanId, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.parentId, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.duration, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.serviceName, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.spanName, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }
        pos2 = strchr(pos1, '|');
        if( pos2 != NULL)
        {
            strncpy(info.host, pos1, pos2-pos1);
            pos1 = pos2 + 1;
        }

        strncpy(tag, pos1, posEnd-pos1);

        // get status amd error from tag
        getError( tag, info.status_code, info.error);
        if(info.status_code != 200 && info.status_code != 0)
            printf("%s\n", posBegin);
        //
        *posEnd = '\n';
        nSize += posEnd - posBegin + 1;
        posBegin = posEnd + 1;
    }
    memset(mem->memory, 0, nSize);
    memmove(mem->memory, posBegin, mem->size - nSize);
    mem->size -= nSize;
    return readsize;
}

MyURL::MyURL():mstr_response_file("./response.html") {
	m_taskID = 0;
	m_batchID = 0;
	m_log.mstr_logfile = "log/download.log";
	curl_global_init(CURL_GLOBAL_ALL);
}

MyURL::~MyURL() {
	curl_global_cleanup();
}
/*
 * get the response and save to mstr_response_file
 * in: url
 * out: mstr_response_file
 */
int MyURL::downloadFile(const char *url, const char * savePath)
{
	CURLcode code;
	struct curl_slist *headers = NULL;
	MemoryStruct sz_response;
	sz_response.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
	sz_response.size = 0;            /* no data at this point */

	headers = curl_slist_append(headers, "Content-Type: text/xml;charset=UTF-8");
	headers = curl_slist_append(headers, "Accept-Encoding: gzip,deflate");

	CURL *easycurl=curl_easy_init();
	curl_easy_setopt(easycurl, CURLOPT_URL, url);
	curl_easy_setopt(easycurl, CURLOPT_HTTPHEADER, headers);// 改协议头

	curl_easy_setopt(easycurl, CURLOPT_WRITEFUNCTION, handleMem);
	code = curl_easy_setopt(easycurl, CURLOPT_WRITEDATA, (void *)&sz_response); // 保存返回结果到文件
	if (code != CURLE_OK)
	{
		char logmsg[256] = "";
		snprintf(logmsg, sizeof(logmsg),"ERR: Failed to curl_easy_setopt: %s.", strerror(errno));
		m_log.logException(logmsg);
		return -1;
	}
	code = curl_easy_perform(easycurl);
	if (code != CURLE_OK)
	{
		char logmsg[256] = "";
		sprintf(logmsg, "ERR: Cannot get download URL, failed to curl_easy_perform, %s.", curl_easy_strerror(code));
		m_log.logException(logmsg);
		free(sz_response.memory);
		curl_slist_free_all(headers);
		curl_easy_cleanup(easycurl);
		curl_global_cleanup();
		return -1;
	}

	free(sz_response.memory);
	curl_slist_free_all(headers);
	curl_easy_cleanup(easycurl);
	curl_global_cleanup();
	return 0;
}



int MyURL::saveURL(const char *url,const char *savefile)
{
    if( 0 == strlen(url))
        return -1;
    // open file for write
    FILE *fp;
#ifdef WINVER
    int ret = 0;
    if ((ret = fopen_s(&fp, savefile, "wb")) != 0)
    {
        return -1;
    }
#endif
#ifdef __linux
    if((fp=fopen(savefile,"wb"))==NULL)
    {
        return -1;
    }
#endif
    CURL *curl;
    CURLcode code;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    code = curl_easy_perform(curl);
    if (code != CURLE_OK)
    {
        char logmsg[256] = "";
        sprintf(logmsg, "ERR: Failed to get %s.", savefile);
        m_log.logException(logmsg);
        sprintf(logmsg, "ERR: Failed to curl_easy_perform, %s, the url is %s.", curl_easy_strerror(code), url);
        m_log.logException(logmsg);
        curl_easy_cleanup(curl);
        fclose(fp);
        return -1;
    }
    curl_easy_cleanup(curl);
    fclose(fp);
    return 0;
}

/*
 * set task status
 */
int MyURL::setTaskStatus( int status, long taskID, long batchID)
{
	string dbname = "websiteman";
    for(int i=0; i<3; i++)
    {
    	try
		{
    	    Csql_conn_cpp  ss;
    	    ss.sql_connect("10.1.122.140");
    	    ss.setDBName(dbname);
			ss.m_prepstmt = ss.m_con->prepareStatement("update t_haproxy_task set `ExeStatus`=? where `ID`=? and `batchId`=?");
			ss.m_prepstmt->setInt(1, status);
			ss.m_prepstmt->setInt(2, taskID);
			ss.m_prepstmt->setInt(3, batchID);
			ss.m_prepstmt->executeUpdate();

			char logmsg[256] = "";
			sprintf(logmsg, "INFO: task %ld batch %ld setstatus=%d succeed.", taskID, batchID, status );
			m_log.logException(logmsg);
			break;
		}
		catch(sql::SQLException &e)
		{
			m_log.logException("ERR: 更新task状态时发生异常！");
			m_log.logException(e,__FILE__ , __FUNCTION__, __LINE__);
		}
    }
	return 0;
}

/*
 *
 */
int MyURL::mvToDest(string pathdes)
{
	char cmd[512] = "";
/*
	unsigned long long filesize = 0;
	struct stat buf;
	if((stat(mstr_downPath.c_str(),&buf)!=-1))
	{
		filesize = buf.st_size;
	}
	else
	{
		m_log.logException("Failed to get the status of the temp file.");
		return -1;
	}
	if(filesize == 0 )
	{
		m_log.logException("The size of temp file is 0, stop moving.");
		return -1;
	}
	*/
	sprintf(cmd, "mv %s %s", mstr_downPath.c_str(), pathdes.c_str());
//	system(cmd);
	int stat_ret = 0;
	string retmsg;
//	myexec(cmd, stat_ret ,retmsg);
	m_log.logException(retmsg);
	if(stat_ret != 0)
		return -1;
	else
		return 0;
}
