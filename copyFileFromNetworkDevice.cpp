#include<Windows.h>
#include <iostream>
using namespace std;
int main()
{
	SHFILEOPSTRUCT s = { 0 };
	s.wFunc = FO_COPY;
	s.pTo = L"E:\\mohanraj\\networkshare";
	s.pFrom = L"\\\\DC-WIN8-64-2\\Users\\test\\Documents\\*\0";
	if (!SHFileOperation(&s))
	{
		wcout << "\nFOLDER HAS BEEN COPIED\n\n";
		wcout << "SOURCE FOLDER PATH : " << s.pFrom << "\n\n";
		wcout << "DESTINATION PATH : " << s.pTo << "\n\n";
	}
	else
		wcout << "ERROR WHILE COPYING A FILE FROM NETWORK DEVICE \tERROR CODE " << GetLastError();
	return 0;
}

