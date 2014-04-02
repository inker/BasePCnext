// basepcnext.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "BasePC.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	//unsigned short tst = 40000;
	//cout << (short)tst << endl;
	//unsigned short memarr[] = { 0x0045, 0x0032, 0, 0x4000, 0x6001, 0x3002, 0xf000 };
	//vector<unsigned short> memory(memarr, memarr + sizeof (memarr) / sizeof (short));
	//BasePC bpc(memory, 3);

	map<unsigned short, unsigned short> m;
	//m[0] = 0x0434;
	//m[1] = 0x0111;
	//m[2] = 0x0000;
	//m[3] = 0x4000;
	//m[4] = 0x6001;
	//m[5] = 0x3002;
	m[6] = 0xf000;
	//BasePC bpc = { m, 0 };
	string default_path = "lab1-6short.csv";
	string path;
	/*if (argc > 1) {
		cout << argc << endl;
		ostringstream oss;
		cout << argv[0] << endl;
		oss << argv[0];
		path = oss.str();
		cout << path << endl;
	} else */{
		/*cout << "enter the path to the file or click 'enter' to use the default path (" << default_path << ") - 'Y'" << endl;
		char cpath[256];
		cin.getline(cpath, 256);
		if (cpath == "Y")*/ path = default_path;
	}
	//vector<unsigned short> vvv(3000, 0x1111);
	//BasePC bpc(vvv.begin(), vvv.end());
	//bpc.set_cell(0x01f, 0x4018);
	BasePC bpc(path);
	bpc.run("output.html");
	//bpc.show_memory_not_null();
	//_getch();
	return 0;
}

