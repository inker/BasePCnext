// basepcnext.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include "BasePC.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	//unsigned short tst = 40000;
	//cout << (short)tst << endl;
	//unsigned short memarr[] = { 0x0045, 0x0032, 0, 0x4000, 0x6001, 0x3002, 0xf000 };
	//vector<unsigned short> memory(memarr, memarr + sizeof (memarr) / sizeof (short));
	//BasePC bpc(memory, 3);
	BasePC bpc("lab1-6.txt");
	bpc.run();
	bpc.show_memory(0x17, 0x26);
	system("pause");
	return 0;
}

