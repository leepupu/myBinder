#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>

using namespace std;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        cout << "USAGE: appender.exe a, b\nappend content of a to tail of b\n";
        return -1;
    }
    ifstream fin(argv[1], ios::in | ios::binary);
    ofstream fout(argv[2], ios::out | ios::app | ios::ate | ios::binary);
    fin.seekg(0, ios::end);
    int length = fin.tellg();
    fin.seekg(0, ios::beg);
    char buffer[length+1];
    fin.read(buffer, length);
    fout.write(buffer, length);
    fout.write((char*)&length, sizeof(length));
    fin.close();
    fout.close();
}
