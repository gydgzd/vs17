#include "stdafx.h"
#include <stdlib.h>   // fopen
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
//#include <sstream>
#define BUF_LEN 1024000
using namespace std;

struct statistic_datetime
{
	string datetime;
	int count_per_time = 0;
	int count_same_prov = 0;
	int count_via_BigTa = 0;
	int count_conn_via_BigTa = 0;
	int count_conn_over_BigTa = 0;
	int count_s0 = 0;        // count when src  id is 0
	int count_d0 = 0;        // count when dest id is 0
	int count_sd0 = 0;       // count when srcid and destid is 0
};

struct statistic_minutes
{
	string datetime;
	int count = 0;
	int count_same_prov = 0;
	int count_via_BigTa = 0;
	int count_conn_via_BigTa = 0;
	int count_conn_over_BigTa = 0;
	int count_s0 = 0;
	int count_d0 = 0;
	int count_sd0 = 0;
};
struct statistic_hours
{
	string datetime;
	int count = 0;
	int count_same_prov = 0;
	int count_via_BigTa = 0;
	int count_conn_via_BigTa = 0;
	int count_conn_over_BigTa = 0;
	int count_s0 = 0;
	int count_d0 = 0;
	int count_sd0 = 0;
};
int parse_liveout()
{
	char errmsg[1024] = "";
	string logfile = "E:\\work\\liveout.log";
	string res_file = "E:\\work\\liveout_res.txt";     // file to save the result
	ofstream res_ofs(res_file, ios::out);
	if (res_ofs.fail())
	{
		cerr << "Failed to open result file." << endl;
		return -1;
	}
	FILE *fp;
#ifdef WINVER
	errno_t err;
	if ((err = fopen_s(&fp, logfile.c_str(), "r+")) != NULL)     //判断文件打开
	{
		strerror_s(errmsg, errno);
		printf("Error in open file! %s:%s \n", logfile.c_str(), errmsg);
		return -1;
	}
#endif
#ifdef __LINUX
	if (!(fp = fopen(mstr_logfile.c_str(), "r+")))     //判断文件打开  #include <stdlib.h>
	{
		printf("Error in open file! %s:%s \n", mstr_logfile.c_str(), strerror(errno));
		return -1;
	}
#endif
	char *buffer = new char[BUF_LEN];
	memset(buffer, 0, BUF_LEN);
	int length = 0;
	size_t rsize;
	char * pos = buffer;

	string tmp;
	string line;
	char datetime[20] = "";
	char minute[20] = "";
	char hour[20] = "";
	char sz_isend[2] = "";
	char sz_sprovID[11] = "";
	char sz_dprovID[11] = "";
	int isend;
	int checked = 0;
	int sprov_id = 0;
	int dprov_id = 0;
	// to be statistic
	int count_per_time = 0;
	int count_same_prov = 0;
	int count_via_BigTa = 0;
	//
	char * section_begin;
	char * last_begin = NULL;
	char * end;

	vector<statistic_datetime> v_ss_sec;
	vector<statistic_minutes> v_ss_min;
	vector<statistic_hours>   v_ss_hour;
	set<set<int>> ss_viaBigTa_sec;
	set<set<int>> ss_overBigTa_sec;
	set<set<int>> ss_viaBigTa_min;
	set<set<int>> ss_overBigTa_min;
	set<set<int>> ss_viaBigTa_hour;
	set<set<int>> ss_overBigTa_hour;
	set<int> s_conn;

	statistic_datetime ss_sec;
	statistic_minutes ss_min;
	statistic_hours   ss_hour;
	ss_min.count = 0;
	ss_hour.count = 0;
	int n = 0;
	// read a batch of file, stop when the file is end
	while ( rsize = fread(buffer + length, sizeof(char), BUF_LEN - length - 1, fp))  
	{
		pos = buffer;
		buffer[ length + rsize ] = '\0';
		// 2018-10-18 13:04:43 02031 ms=033 toBIG SetBetweenProvRate: 1275:{
		while (section_begin = strstr(pos, "SetBetweenProvRate:"))
		{
			int tmplength = 0;
			checked = 0;
			last_begin = section_begin ;       
			while (*last_begin != '\n')  // go back to begin of line
			{
				last_begin--;
				if (last_begin == buffer)
					break;
			}
			// get date :  "datetime":"2018-10-18 13:04:30",
			if (pos = strstr(section_begin, "\"datetime\":"))
			{
				if ((end = strchr(pos, '\n')) == NULL)
					break;
				tmplength = strlen("\"datetime\":\"");
				memcpy_s(datetime, sizeof(datetime), pos + tmplength, 19);
				memcpy_s(hour, sizeof(hour), datetime, 13);
				memcpy_s(minute, sizeof(hour), datetime, 16);
			}
			else
				break;
			pos = end + 1;
			//"t_endflag":0,
			if (pos = strstr(pos, "\"t_endflag\":"))
			{
				if ((end = strchr(pos, '\n')) == NULL)
					break;
				tmplength = strlen("\"t_endflag\":");
				memcpy_s(sz_isend, sizeof(sz_isend), pos + tmplength, end - pos - tmplength -1);
				sz_isend[1] = 0;
			}
			else
				break;
			pos = end + 1;
			// "sprov_id":11,
			if (pos = strstr(pos, "\"sprov_id\":"))
			{
				if ((end = strchr(pos, '\n')) == NULL)
					break;
				tmplength = strlen("\"sprov_id\":");
				memcpy_s(sz_sprovID, sizeof(sz_sprovID), pos + tmplength, end - pos - tmplength - 1);
				sz_sprovID[end - pos - tmplength - 1] = 0;
			}
			else
				break;
			pos = end + 1;
			// "dprov_id":11
			if (pos = strstr(pos, "\"dprov_id\":"))
			{
				if ((end = strchr(pos, '\n')) == NULL)
					break;
				tmplength = strlen("\"dprov_id\":");
				memcpy_s(sz_dprovID, sizeof(sz_dprovID), pos + tmplength, end - pos - tmplength - 1);
				sz_dprovID[end - pos - tmplength - 1] = 0;
			}
			else
				break;
			pos = end + 1;
			isend = atoi(sz_isend);
			sprov_id = atoi(sz_sprovID);
			dprov_id = atoi(sz_dprovID);
			s_conn.clear();
			s_conn.insert(sprov_id);
			s_conn.insert(dprov_id);
		//	if (isend == 1)        // get one endflag
			{
				if (!strncmp(datetime, ss_sec.datetime.c_str(), 19))  // same datetime
				{
					ss_sec.count_per_time++;
					if (sprov_id == dprov_id)
						ss_sec.count_same_prov++;
					if (sprov_id == 10 || dprov_id == 10){
						ss_sec.count_via_BigTa++;
						ss_viaBigTa_sec.insert(s_conn);
					}
					else
						ss_overBigTa_sec.insert(s_conn);
					if (sprov_id == 0)
						ss_sec.count_s0++;
					if (dprov_id == 0)
						ss_sec.count_d0++;
					if (sprov_id == 0 && dprov_id == 0)
						ss_sec.count_sd0++;	
				}
				else
				{
					if (ss_sec.datetime != "")
					{
						ss_sec.count_conn_via_BigTa = ss_viaBigTa_sec.size();
						ss_sec.count_conn_over_BigTa = ss_overBigTa_sec.size();
						v_ss_sec.push_back(ss_sec);
						ss_viaBigTa_sec.clear();
						ss_overBigTa_sec.clear();
					}
					// initialize ss_sec
					ss_sec.datetime = datetime;
					ss_sec.count_per_time = 1;
					if (sprov_id == dprov_id) ss_sec.count_same_prov = 1;
					else ss_sec.count_same_prov = 0;
					if (sprov_id == 10 || dprov_id == 10){
						ss_sec.count_via_BigTa = 1; ss_sec.count_conn_via_BigTa = 1; ss_sec.count_conn_over_BigTa = 0;
						ss_viaBigTa_sec.insert(s_conn);
					}
					else {
						ss_overBigTa_sec.insert(s_conn);
						ss_sec.count_via_BigTa = 0; ss_sec.count_conn_via_BigTa = 0; ss_sec.count_conn_over_BigTa = 1;
					}
					if (sprov_id == 0)	ss_sec.count_s0 = 1;
					else ss_sec.count_s0 = 0;
					if (dprov_id == 0)	ss_sec.count_d0 = 1;
					else ss_sec.count_d0 = 0;
					if (sprov_id == 0 && dprov_id == 0)	ss_sec.count_sd0 = 1;
					else ss_sec.count_sd0 = 0;
				}
				if (!strncmp(minute, ss_min.datetime.c_str(), 16))  // same minute
				{
					ss_min.count++;
					if (sprov_id == dprov_id)
						ss_min.count_same_prov++;
					if (sprov_id == 10 || dprov_id == 10) {
						ss_min.count_via_BigTa++;
						ss_viaBigTa_min.insert(s_conn);
					}
					else
						ss_overBigTa_min.insert(s_conn);
					if (sprov_id == 0)
						ss_min.count_s0++;
					if (dprov_id == 0)
						ss_min.count_d0++;
					if (sprov_id == 0 && dprov_id == 0)
						ss_min.count_sd0++;
				}
				else 
				{
					if (ss_min.datetime != "")
					{
						ss_min.count_conn_via_BigTa = ss_viaBigTa_min.size();
						ss_min.count_conn_over_BigTa = ss_overBigTa_min.size();
						v_ss_min.push_back(ss_min);
						ss_viaBigTa_min.clear();
						ss_overBigTa_min.clear();
					}
					ss_min.datetime = minute;
					ss_min.count = 1;
					ss_min.count_same_prov = ss_sec.count_same_prov;
					ss_min.count_via_BigTa = ss_sec.count_via_BigTa;
					ss_min.count_conn_via_BigTa = ss_sec.count_conn_via_BigTa;
					ss_min.count_conn_over_BigTa = ss_sec.count_conn_over_BigTa;
					if (sprov_id == 10 || dprov_id == 10) {
						ss_viaBigTa_min.insert(s_conn);
					}
					else {
						ss_overBigTa_min.insert(s_conn);
					}
					ss_min.count_s0 = ss_sec.count_s0;
					ss_min.count_d0 = ss_sec.count_d0;
					ss_min.count_sd0 = ss_sec.count_sd0;
				}	
				if (!strncmp(hour, ss_hour.datetime.c_str(), 13))  // same hour
				{
					ss_hour.count++;
					if (sprov_id == dprov_id)
						ss_hour.count_same_prov++;
					if (sprov_id == 10 || dprov_id == 10) {
						ss_hour.count_via_BigTa++;
						ss_viaBigTa_hour.insert(s_conn);
					}
					else
						ss_overBigTa_hour.insert(s_conn);
					if (sprov_id == 0)
						ss_hour.count_s0++;
					if (dprov_id == 0)
						ss_hour.count_d0++;
					if (sprov_id == 0 && dprov_id == 0)
						ss_hour.count_sd0++;
				}
				else
				{
					if (ss_hour.datetime != "")
					{
						
						ss_hour.count_conn_via_BigTa = ss_viaBigTa_hour.size();
						ss_hour.count_conn_over_BigTa = ss_overBigTa_hour.size();
						v_ss_hour.push_back(ss_hour);
						ss_viaBigTa_hour.clear();
						ss_overBigTa_hour.clear();
						
					}
						
					ss_hour.datetime = hour;
					ss_hour.count = 1;
					ss_hour.count_same_prov = ss_sec.count_same_prov;
					ss_hour.count_via_BigTa = ss_sec.count_via_BigTa;
					ss_hour.count_conn_via_BigTa = ss_sec.count_conn_via_BigTa;
					ss_hour.count_conn_over_BigTa = ss_sec.count_conn_over_BigTa;
					if (sprov_id == 10 || dprov_id == 10) {
						ss_viaBigTa_hour.insert(s_conn);
					}
					else {
						ss_overBigTa_hour.insert(s_conn);
					}
					ss_hour.count_s0 = ss_sec.count_s0;
					ss_hour.count_d0 = ss_sec.count_d0;
					ss_hour.count_sd0 = ss_sec.count_sd0;
				}		
				checked = 1;
			}
			
		} // end of while
		if (section_begin == NULL)
		{
			length = 0;
			continue;
		}
		if (checked == 0)
		{
			length = strlen(last_begin);
			memcpy_s(buffer, BUF_LEN, last_begin, length + 1);
		}
		else
		{
			length = strlen(pos);
			memcpy_s(buffer, BUF_LEN, pos, length + 1);
		}
	} // end of while
	 
	delete[] buffer;
	fclose(fp);
	// add the last record to the result
	v_ss_sec.push_back(ss_sec);
	v_ss_min.push_back(ss_min);
	v_ss_hour.push_back(ss_hour);
	res_ofs << "statistic of SetBetweenProvRate:" << endl;
	res_ofs << "字段顺序依次为: \n时间" << "\t该时间计数" << "\t同省" << "\t经过大塔" << "\t其它" << "\t连接经过大塔" << "\t连接不经过大塔" << endl;
	// statistic of sec 
	res_ofs << "statistic of sec" << endl;
	for (size_t i = 0; i < v_ss_sec.size(); i++)
	{
		res_ofs << v_ss_sec[i].datetime << " : " << v_ss_sec[i].count_per_time << "\t" << v_ss_sec[i].count_same_prov << "\t" << v_ss_sec[i].count_via_BigTa << "\t";
		res_ofs << v_ss_sec[i].count_per_time - v_ss_sec[i].count_same_prov - v_ss_sec[i].count_via_BigTa << "\t" ;
		res_ofs << v_ss_sec[i].count_conn_via_BigTa << "\t" << v_ss_sec[i].count_conn_over_BigTa << "\t";
	//	res_ofs << endl;
		res_ofs << v_ss_sec[i].count_s0 << "\t" << v_ss_sec[i].count_d0 << "\t" << v_ss_sec[i].count_sd0 << endl;
	}

	// statistic of minutes
	res_ofs << "statistic of minutes" << endl;
	for (size_t i = 0; i < v_ss_min.size(); i++)
	{
		res_ofs << v_ss_min[i].datetime << " : " << v_ss_min[i].count << "\t" << v_ss_min[i].count_same_prov << "\t" << v_ss_min[i].count_via_BigTa << "\t";
		res_ofs << v_ss_min[i].count - v_ss_min[i].count_same_prov - v_ss_min[i].count_via_BigTa << "\t";
		res_ofs << v_ss_min[i].count_conn_via_BigTa << "\t" << v_ss_min[i].count_conn_over_BigTa << "\t";
	//	res_ofs << endl;
		res_ofs << v_ss_min[i].count_s0 << "\t" << v_ss_min[i].count_d0 << "\t" << v_ss_min[i].count_sd0 << endl;
	}

	// statistic of hours
	res_ofs << "statistic of hours" << endl;
	for (size_t i = 0; i < v_ss_hour.size(); i++)
	{
		res_ofs << v_ss_hour[i].datetime << " : " << v_ss_hour[i].count << "\t" << v_ss_hour[i].count_same_prov << "\t" << v_ss_hour[i].count_via_BigTa << "\t";
		res_ofs << v_ss_hour[i].count - v_ss_hour[i].count_same_prov - v_ss_hour[i].count_via_BigTa << "\t";
		res_ofs << v_ss_hour[i].count_conn_via_BigTa << "\t" << v_ss_hour[i].count_conn_over_BigTa << "\t";
	//	res_ofs << endl;
		res_ofs << v_ss_hour[i].count_s0 << "\t" << v_ss_hour[i].count_d0 << "\t" << v_ss_hour[i].count_sd0 << endl;
	}
	res_ofs.close();

	return 0;
}