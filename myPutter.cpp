#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>
#include "MyFile.h"
#include "zlib.h"
#include "zconf.h"
#define DEBUG 0

using namespace std;

/**
    Compile with subsystem windows avoid show console
    so stdout can't be seen only if you redirect the stdout to file
    Ex. run cmd: myPutter.exe > log.txt
**/
int main(int argc, char** argv)
{
    char buf[MAX_PATH];
    if (GetTempPath (MAX_PATH, buf) == 0)
    {
        cout << "error\n";
        return -1;
    }
    ifstream fin(argv[0], ios::in | ios::binary);
    int size;
    fin.seekg(-4, fin.end);
    if(DEBUG)
        cout << fin.tellg() << endl;
    fin.read((char*)&size, sizeof(size));
    if(DEBUG)
        cout << "size: " << size << endl;
    // set file pointer to appendent data's head
    fin.seekg(-(size+4), ios::cur);
    if(DEBUG)
        cout << "fin.tellg: " << fin.tellg() << endl;

    MyFile a;
    while(fin >> a)
    {
        if(DEBUG)
        {
            cout << a.filename << endl;
            cout << "compressed length: " << a.length << endl;
        }
        string tmp_file = buf;
        bool exe = false;
        ofstream fout;
        if(a.filename.length() >= 3 && a.filename.substr(a.filename.length() - 3).compare("exe") == 0)
        {
            // is a exe file, write to temp folder
            tmp_file += a.filename;
            exe = true;
            fout.open(tmp_file.c_str(), ios::out | ios::binary);
        }
        else
        {
            fout.open(a.filename.c_str(), ios::out | ios::binary);
        }
        char* dst_buffer = new char[a.origin_length+1];
        int new_length = a.origin_length;

        uncompress((Bytef*)dst_buffer, (uLongf*)&new_length, (Bytef*)a.data, a.length);

        fout.write(dst_buffer, new_length);
        fout.close();
        if(exe)
        {
            if(DEBUG)
                cout << "executing: " << tmp_file << endl;
            HINSTANCE hReturnCode=ShellExecute(NULL, "open", tmp_file.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
    return 0;
}
