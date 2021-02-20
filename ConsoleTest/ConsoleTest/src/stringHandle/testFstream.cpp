#include <fstream>
#include <iostream>
#include <string>      // getline
using namespace std;
//stringstream ss;
//ss.str("");   // .str() and .clear() work together to clean the content
//ss.clear();
int readFileStream()
{
	ifstream infile;
	infile.open("D:/1.txt", ios_base::in);
	if (!infile)
	{
		cout << strerror(errno) << endl;

	//	char errmsg[256];
	//	strerror_s(errmsg, errno);
	//	printf("Failed to open result file: %s", errmsg);

		return -1;
	}
	//��ʼ��ȡ
	string linebuf;
	while (getline(infile, linebuf))   // !infile.eof()
	{
		cout << linebuf << endl;
	}

	infile.close();
	return 0;
}

int writeFileStream()
{
	char *filename = "D:/2.txt";
	ofstream outfile;
	outfile.open(filename, ios_base::in);
	if (!outfile)
	{
		cout << "Failed to open " << filename <<": "<< strerror(errno) << endl;
		return -1;
	}
	string linebuf = "���˷���ȥ����ع�����";
	outfile << linebuf;

	outfile.close();
	return 0;
}