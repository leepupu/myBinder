#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <windows.h>
#include "MyFile.h"
#include <olectl.h>
#include <fcntl.h>
#include <io.h>
// Source http://www.zlib.net/
// Comiled by myself
#include "zlib.h"
#include "zconf.h"
#define DEBUG 0 // isolate appendent data for debug

using namespace std;

#pragma pack(push, 2)
typedef struct {
  WORD Reserved1;       // reserved, must be 0
  WORD ResourceType;    // type is 1 for icons
  WORD ImageCount;      // number of icons in structure (1)
  BYTE Width;           // icon width (32)
  BYTE Height;          // icon height (32)
  BYTE Colors;          // colors (0 means more than 8 bits per pixel)
  BYTE Reserved2;       // reserved, must be 0
  WORD Planes;          // color planes
  WORD BitsPerPixel;    // bit depth
  DWORD ImageSize;      // size of structure
  WORD ResourceID;      // resource ID
} GROUPICON;
#pragma pack(pop)


// Source http://stackoverflow.com/questions/27202883/updateresource-call-for-setting-windows-icon-ico-not-working-consistently
void setIcon(char* IconFile, char* ExecutableFile)
{
   HANDLE hWhere = BeginUpdateResource(ExecutableFile, FALSE);

   char *buffer;    // buffer to store raw icon data
   long buffersize; // length of buffer
   int hFile;       // file handle

   hFile = open(IconFile, O_RDONLY | O_BINARY);
   if (hFile == -1)
      return; // if file doesn't exist, can't be opened etc.

   // calculate buffer length and load file into buffer
   buffersize = filelength(hFile);
   buffer = (char *)malloc(buffersize);
   read(hFile, buffer, buffersize);
   close(hFile);

   UpdateResource(
      hWhere,  // Handle to executable
      RT_ICON, // Resource type - icon
      MAKEINTRESOURCE(1), // Make the id 1
      MAKELANGID(LANG_ENGLISH,
                 SUBLANG_DEFAULT), // Default language
      (buffer+22),
      // skip the first 22 bytes because this is the
      // icon header&directory entry (if the file
      // contains multiple images the directory entries
      // will be larger than 22 bytes
      buffersize-22  // length of buffer
     );

   // Again, we use this structure for educational purposes.
   // The icon header and directory entries can be read from
   // the file.
   GROUPICON grData;

   // This is the header
   grData.Reserved1 = 0;     // reserved, must be 0
   grData.ResourceType = 1;  // type is 1 for icons
   grData.ImageCount = 1;    // number of icons in structure (1)

   // This is the directory entry
   grData.Width = 32;        // icon width (32)
   grData.Height = 32;       // icon height (32)
   grData.Colors = 0;        // colors (256)
   grData.Reserved2 = 0;     // reserved, must be 0
   grData.Planes = 2;        // color planes
   grData.BitsPerPixel = 32; // bit depth
   grData.ImageSize = buffersize - 22; // size of image
   grData.ResourceID = 1;       // resource ID is 1

   UpdateResource(
      hWhere,
      RT_GROUP_ICON,
      // RT_GROUP_ICON resources contain information
      // about stored icons
      "MAINICON",
      // MAINICON contains information about the
      // application's displayed icon
      MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
      &grData,
      // Pointer to this structure
      sizeof(GROUPICON)
     );

   delete buffer; // free memory

   // Perform the update, don't discard changes
   EndUpdateResource(hWhere, FALSE);
}


int saveIcon(TCHAR* filename, TCHAR* iconFile);

BOOL SaveIcon3(TCHAR *szIconFile, HICON hIcon[], int nNumIcons);

bool extract_putter(char* self_path, string dst_file)
{
    // extract myPutter appended by appender.exe to dst_file
    ifstream fin(self_path, ios::in | ios::binary);
    int size;
    fin.seekg(-4, fin.end);
    // cout << fin.tellg() << endl;
    fin.read((char*)&size, sizeof(size));
    cout << "size of  myPutter: " << size << endl;
    fin.seekg(-(size+4), ios::cur); // move file pointer back to myputter's head
    cout << "fin.tellg: " << fin.tellg() << endl;

    ofstream fout(dst_file.c_str(), ios::out | ios::binary);  // writer of myputter
    char* buffer = new char[size+1];

    fin.read(buffer, size); // simple read and write
    fout.write(buffer, size);

    fout.close();
    fin.close();
    delete [] buffer;
}

string get_temp_folder() // Use windows API to get %Temp% absolutly path
{
    char buf[MAX_PATH];
    if (GetTempPath (MAX_PATH, buf) == 0)
    {
        cout << "error\n";
        return "";
    }
    string tmp_folder = buf;
    return tmp_folder;
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        cout << "too few args\n";
        return -1;
    }
    string icon_path;
    string target = argv[1]; // host file path
    target += "\\";
    target += argv[3];
    int IconCount = ExtractIconEx(target.c_str(), -1, NULL,NULL, 0);
    if(IconCount >= 1)
    {
        HICON* hIcons = new HICON[1];

        ExtractIconEx(target.c_str(), 0, hIcons, NULL, 1);
        cout << hIcons[0] << endl;
        icon_path = get_temp_folder();
        if(icon_path.length() == 0)
            return -1;
        icon_path += "\\";
        icon_path += argv[3];
        icon_path += ".ico";
        // Extract host file icon to temp folder
        SaveIcon3((char*)icon_path.c_str(), hIcons, 1);
    }
    cout << "count: " << IconCount << endl;
    char* src_folder = argv[1];
    string dst_folder = argv[2];
    string dst_file = dst_folder + "\\" + argv[3];

    extract_putter(argv[0], dst_file); // Step1

    // Set icon in temp folder which just extracted to myPutter
    setIcon((char*)icon_path.c_str(), (char*)dst_file.c_str());

    // Emulator for all files in src folder
    DIR *dir;
    struct dirent *ent;
    ofstream fout(dst_file.c_str(), ios::out | ios::app | ios::ate | ios::binary);
    ofstream fout2;
    if(DEBUG)
      fout2.open("output.dat", ios::out | ios::binary);
    int output_size = fout.tellp();
    if ((dir = opendir (src_folder)) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL) {
        string fp(src_folder);
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) // is self folder or parent folder?
            continue;
        fp += "\\";
        fp += ent->d_name;

        // Reading file
        ifstream fin(fp.c_str(), ios::in | ios::binary);
        MyFile a;

        // Set filename
        a.filename = new char[strlen(ent->d_name+1)];
        a.filename = (ent->d_name);

        // Get file size
        fin.seekg(0, fin.end);
        int length = fin.tellg();
        fin.seekg(0, fin.beg);

        // Prepare buffer to compress file content
        char* source_buffer = new char[length+1];
        char* dst_buffer = new char[length+1];

        cout << "before compress: " << length << endl;

        fin.read(source_buffer, length);

        int new_length = length;

        if(compress((Bytef*)dst_buffer, (uLongf*)&new_length, (Bytef*)source_buffer, (uLongf)length) != Z_OK)
        {
          cout << "error while compress file: " << a.filename << endl;
          break;
        }

        cout << "after compress: " << new_length << endl;

        // set myfile data with comporess data
        a.data = dst_buffer;
        a.length = new_length;
        a.origin_length = length;

        fout << a;
        if(DEBUG)
          fout2 << a;

        // Release memory
        delete [] source_buffer;
        delete [] dst_buffer;


        fin.close();
      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("could not open directory");
      return -1;
    }
    output_size = fout.tellp() - output_size;
    cout <<"appendent data size: " << output_size << endl;

    fout.write((char*)&output_size, sizeof(output_size));
    if(DEBUG)
      fout2.write((char*)&output_size, sizeof(output_size));

    fout.close();
    if(DEBUG)
      fout2.close();
    return 0;
}

//
// ICONS (.ICO type 1) are structured like this:
//
// ICONHEADER (just 1)
// ICONDIR [1...n] (an array, 1 for each image)
// [BITMAPINFOHEADER+COLOR_BITS+MASK_BITS] [1...n] (1 after the other, for each image)
//
// CURSORS (.ICO type 2) are identical in structure, but use
// two monochrome bitmaps (real XOR and AND masks, this time).
//

typedef struct
{
    WORD idReserved; // must be 0
    WORD idType; // 1 = ICON, 2 = CURSOR
    WORD idCount; // number of images (and ICONDIRs)

    // ICONDIR [1...n]
    // ICONIMAGE [1...n]

} ICONHEADER;

//
// An array of ICONDIRs immediately follow the ICONHEADER
//
typedef struct
{
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes; // for cursors, this field = wXHotSpot
    WORD wBitCount; // for cursors, this field = wYHotSpot
    DWORD dwBytesInRes;
    DWORD dwImageOffset; // file-offset to the start of ICONIMAGE

} ICONDIR2;

//
// After the ICONDIRs follow the ICONIMAGE structures -
// consisting of a BITMAPINFOHEADER, (optional) RGBQUAD array, then
// the color and mask bitmap bits (all packed together
//
typedef struct
{
    BITMAPINFOHEADER biHeader; // header for color bitmap (no mask header)
    //RGBQUAD rgbColors[1...n];
    //BYTE bXOR[1]; // DIB bits for color bitmap
    //BYTE bAND[1]; // DIB bits for mask bitmap

} ICONIMAGE;

//
// Write the ICO header to disk
//
static UINT WriteIconHeader(HANDLE hFile, int nImages)
{
    ICONHEADER iconheader;
    DWORD nWritten;

    // Setup the icon header
    iconheader.idReserved = 0; // Must be 0
    iconheader.idType = 1; // Type 1 = ICON (type 2 = CURSOR)
    iconheader.idCount = nImages; // number of ICONDIRs

    // Write the header to disk
    WriteFile( hFile, &iconheader, sizeof(iconheader), &nWritten, 0);

    // following ICONHEADER is a series of ICONDIR structures (idCount of them, in fact)
    return nWritten;
}

//
// Return the number of BYTES the bitmap will take ON DISK
//
static UINT NumBitmapBytes(BITMAP *pBitmap)
{
    int nWidthBytes = pBitmap->bmWidthBytes;

    // bitmap scanlines MUST be a multiple of 4 bytes when stored
    // inside a bitmap resource, so round up if necessary
    if(nWidthBytes & 3)
        nWidthBytes = (nWidthBytes + 4) & ~3;

    return nWidthBytes * pBitmap->bmHeight;
}

//
// Return number of bytes written
//
static UINT WriteIconImageHeader(HANDLE hFile, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
    BITMAPINFOHEADER biHeader;
    DWORD nWritten;
    UINT nImageBytes;

    // calculate how much space the COLOR and MASK bitmaps take
    nImageBytes = NumBitmapBytes(pbmpColor) + NumBitmapBytes(pbmpMask);

    // write the ICONIMAGE to disk (first the BITMAPINFOHEADER)
    ZeroMemory(&biHeader, sizeof(biHeader));

    // Fill in only those fields that are necessary
    biHeader.biSize = sizeof(biHeader);
    biHeader.biWidth = pbmpColor->bmWidth;
    biHeader.biHeight = pbmpColor->bmHeight * 2; // height of color+mono
    biHeader.biPlanes = pbmpColor->bmPlanes;
    biHeader.biBitCount = pbmpColor->bmBitsPixel;
    biHeader.biSizeImage = nImageBytes;

    // write the BITMAPINFOHEADER
    WriteFile(hFile, &biHeader, sizeof(biHeader), &nWritten, 0);

    // write the RGBQUAD color table (for 16 and 256 colour icons)
    if(pbmpColor->bmBitsPixel == 2 || pbmpColor->bmBitsPixel == 8)
    {

    }

    return nWritten;
}

//
// Wrapper around GetIconInfo and GetObject(BITMAP)
//
static BOOL GetIconBitmapInfo(HICON hIcon, ICONINFO *pIconInfo, BITMAP *pbmpColor, BITMAP *pbmpMask)
{
    if(!GetIconInfo(hIcon, pIconInfo))
        return FALSE;

    if(!GetObject(pIconInfo->hbmColor, sizeof(BITMAP), pbmpColor))
        return FALSE;

    if(!GetObject(pIconInfo->hbmMask, sizeof(BITMAP), pbmpMask))
        return FALSE;

    return TRUE;
}

//
// Write one icon directory entry - specify the index of the image
//
static UINT WriteIconDirectoryEntry(HANDLE hFile, int nIdx, HICON hIcon, UINT nImageOffset)
{
    ICONINFO iconInfo;
    ICONDIR2 iconDir;

    BITMAP bmpColor;
    BITMAP bmpMask;

    DWORD nWritten;
    UINT nColorCount;
    UINT nImageBytes;

    GetIconBitmapInfo(hIcon, &iconInfo, &bmpColor, &bmpMask);

    nImageBytes = NumBitmapBytes(&bmpColor) + NumBitmapBytes(&bmpMask);

    if(bmpColor.bmBitsPixel >= 8)
        nColorCount = 0;
    else
        nColorCount = 1 << (bmpColor.bmBitsPixel * bmpColor.bmPlanes);

    // Create the ICONDIR structure
    iconDir.bWidth = (BYTE)bmpColor.bmWidth;
    iconDir.bHeight = (BYTE)bmpColor.bmHeight;
    iconDir.bColorCount = nColorCount;
    iconDir.bReserved = 0;
    iconDir.wPlanes = bmpColor.bmPlanes;
    iconDir.wBitCount = bmpColor.bmBitsPixel;
    iconDir.dwBytesInRes = sizeof(BITMAPINFOHEADER) + nImageBytes;
    iconDir.dwImageOffset = nImageOffset;

    // Write to disk
    WriteFile(hFile, &iconDir, sizeof(iconDir), &nWritten, 0);

    // Free resources
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return nWritten;
}

static UINT WriteIconData(HANDLE hFile, HBITMAP hBitmap)
{
    BITMAP bmp;
    int i;
    BYTE * pIconData;

    UINT nBitmapBytes;
    DWORD nWritten;

    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    nBitmapBytes = NumBitmapBytes(&bmp);

    pIconData = (BYTE *)malloc(nBitmapBytes);

    GetBitmapBits(hBitmap, nBitmapBytes, pIconData);

    // bitmaps are stored inverted (vertically) when on disk..
    // so write out each line in turn, starting at the bottom + working
    // towards the top of the bitmap. Also, the bitmaps are stored in packed
    // in memory - scanlines are NOT 32bit aligned, just 1-after-the-other
    for(i = bmp.bmHeight - 1; i >= 0; i--)
    {
        // Write the bitmap scanline
        WriteFile(
            hFile,
            pIconData + (i * bmp.bmWidthBytes), // calculate offset to the line
            bmp.bmWidthBytes, // 1 line of BYTES
            &nWritten,
            0);

        // extend to a 32bit boundary (in the file) if necessary
        if(bmp.bmWidthBytes & 3)
        {
            DWORD padding = 0;
            WriteFile(hFile, &padding, 4 - bmp.bmWidthBytes, &nWritten, 0);
        }
    }

    free(pIconData);

    return nBitmapBytes;
}

// Source http://www.cnblogs.com/devc/p/3423750.html
//
// Create a .ICO file, using the specified array of HICON images
//
BOOL SaveIcon3(TCHAR *szIconFile, HICON hIcon[], int nNumIcons)
{
    HANDLE hFile;
    int i;
    int * pImageOffset;

    if(hIcon == 0 || nNumIcons < 1)
        return FALSE;

    // Save icon to disk:
    hFile = CreateFile(szIconFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if(hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    //
    // Write the iconheader first of all
    //
    WriteIconHeader(hFile, nNumIcons);

    //
    // Leave space for the IconDir entries
    //
    SetFilePointer(hFile, sizeof(ICONDIR2) * nNumIcons, 0, FILE_CURRENT);

    pImageOffset = (int *)malloc(nNumIcons * sizeof(int));

    //
    // Now write the actual icon images!
    //
    for(i = 0; i < nNumIcons; i++)
    {
        ICONINFO iconInfo;
        BITMAP bmpColor, bmpMask;

        GetIconBitmapInfo(hIcon[i], &iconInfo, &bmpColor, &bmpMask);

        // record the file-offset of the icon image for when we write the icon directories
        pImageOffset[i] = SetFilePointer(hFile, 0, 0, FILE_CURRENT);

        // bitmapinfoheader + colortable
        WriteIconImageHeader(hFile, &bmpColor, &bmpMask);

        // color and mask bitmaps
        WriteIconData(hFile, iconInfo.hbmColor);
        WriteIconData(hFile, iconInfo.hbmMask);

        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
    }

    //
    // Lastly, skip back and write the icon directories.
    //
    SetFilePointer(hFile, sizeof(ICONHEADER), 0, FILE_BEGIN);

    for(i = 0; i < nNumIcons; i++)
    {
        WriteIconDirectoryEntry(hFile, i, hIcon[i], pImageOffset[i]);
    }

    free(pImageOffset);

    // finished!
    CloseHandle(hFile);

    return TRUE;
}
