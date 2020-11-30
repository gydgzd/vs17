/*
 * readPartTable.cpp
 *
 *  Created on: Jul 22, 2016
 *      Author: Gyd
 */
#include <mntent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <iostream>
using namespace std;
string searchPartTable(string pvname)
{

	struct mntent *m;
	FILE *f = NULL;
	f = setmntent("/etc/mtab","r");
	if(!f)
		printf("error:%s\n",strerror(errno));
	while((m=getmntent(f)))
	{
		if(m->mnt_fsname == pvname)
		{
			printf("%s,\t %s , %s , %s \n",m->mnt_dir,m->mnt_fsname,m->mnt_type,m->mnt_opts);
			return m->mnt_dir;

		}
	}//cout<<m->mnt_dir <<" "<<m->mnt_fsname <<" "<< m->mnt_type<<endl;

	endmntent(f);

	return "Nothing";
}



