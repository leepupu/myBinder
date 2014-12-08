#ifndef MYFILE_H
#define MYFILE_H
#include <fstream>
#include <iostream>
#include <limits.h>

using namespace std;

class MyFile
{
    public:
        MyFile();
        virtual ~MyFile();
        char* data;
        string filename;
        int length;
        int origin_length;
        // Insertion operator
        friend std::ostream& operator<<(std::ostream& os, const MyFile& s)
        {
            // write out individual members of s with an end of line between each one
            os << s.filename << '\n';
            os << s.length << '\n';
            os << s.origin_length << '\n';
            os.write(s.data, s.length);
            return os;
        }

        // Extraction operator
        friend std::istream& operator>>(std::istream& is, MyFile& s)
        {
            getline(is, s.filename);
            // read in individual members of s
            is >> s.length;
            is >> s.origin_length;
            std::cout << s.filename << std::endl;
            is.get();
            if(s.data != 0)
                delete [] s.data;
            s.data = new char[s.length];
            is.read(s.data, s.length);
            return is;
        }
    protected:
    private:
};

#endif // MYFILE_H
