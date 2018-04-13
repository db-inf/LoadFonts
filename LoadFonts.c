//Description - Load fonts bypassing registry and  Windows\Fonts directory 
//Usage : loadfonts [ path-to-fontfile | [-R] directory | -L list-of-fonts-and-dirs ]...
// fontfile : tested with *.ttf, *.ttc (not all work), .otf, *.pfb (with associated .pfm and .mmm, not all work)

/* To compile with Microsoft Visual C 2005 (Command line) :
 * X:\...\> g:\PROGRAMS\MSVC2005\SETVCENV.BAT
 * X:\...\msvc\LaadFonts> cl /MT LoadFonts.c gdi32.lib user32.lib
 * 		/MT link with LIBCMT.LIB (no need to install SDK DLLs)
 *
 * DO NOT compile with /DUNICODE : to go from Linux UTF-8 to Windows UTF-16, a lot of work is needed
 * 		//int wmain(int argc, wchar_t* argv[], wchar_t *envp[])
 */
//#define _CRT_SECURE_NO_DEPRECATE //for strcat and thelikes

//probably something wrong in my age old msvc config that makes even an empty file with only #include windows.h
//spit out  C4005 macro redefinition warnings. All from this couple :
// G:\PROGRAMS\MSVC2005\WINSDK\Include\specstrings.h(334) : warning C4005: '__reserved' : macro redefinition
//        G:\PROGRAMS\MSVC2005\VC\INCLUDE\sal.h(702) : see previous definition of '__reserved'
# pragma warning(disable:4005)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winbase.h>

/*Enforces that the argument is terminated by a \ or / path separator, adding one if needed */
void enforcePathSeparator(char *s)
{//Windows isn't forgiving about 2 consecutive path separators
	int last = lstrlen(s) - 1;
	if (s[last] == '\\' || s[last] == '/')
		return;
	lstrcat(s, "\\");
}

void loadFont(int i, char * s)
{
	/* https://msdn.microsoft.com/ja-jp/windows/dd183326(v=vs.80)
	int AddFontResource(  _In_ LPCTSTR lpszFilename);
	  Parameters:
	lpszFilename [in]
		A pointer to a null-terminated character string that contains a valid font file name. This parameter can specify any of the following files.   
			Type	Meaning
			.fon	Font resource file.
			.fnt    Raw bitmap font file.
			.ttf    Raw TrueType file.
			.ttc    East Asian Windows: TrueType font collection.
			.fot    TrueType resource file.
			.otf    PostScript OpenType font.
			.mmm    Multiple master Type1 font resource file. It must be used with .pfm and .pfb files.
			.pfb    Type 1 font bits file. It is used with a .pfm file.
			.pfm    Type 1 font metrics file. It is used with a .pfb file.
		To add a font whose information comes from several resource files, have lpszFileName point to a string with the file names separated by a "|" For example, "abcxxxxx.pfm|abcxxxxx.pfb".
	If AddFontResource succeeds, the return value specifies the number of fonts added.
	If AddFontResource fails, the return value is zero. No extended error information is available

	AddFontResource docu also says nothing how to get a multiple master instance, that is composed of a PFM file and a PostScript Printer Stub (PSS) file.
	*/
	int iResult;
	if (lstrcmpi(".pfb", s + lstrlen(s) - 4) == 0)
	{//Type 1 font bits, add like-named .pfm (font metrics), and if it exists .mmm (multiple master metrics) file
		WIN32_FIND_DATA ffd;
		HANDLE hFind;
		char *sMMM;
		char *sPSFont = malloc(1 + sizeof(*sPSFont) * (3 * lstrlen(s) + 2));//up to 3 copies and 2 x "|"
		if (sPSFont == NULL)
		{
			printf("ERROR out-of-memory handling %s for arg[%i].\r\n", s, i);
			return;
		}
		//NOTE : the order for mult-file fonts is not documented but important: pfm|pfb or pfm|pfb|mmm.
		lstrcpy(sPSFont, s);
		sPSFont[lstrlen(sPSFont) - 1] += ('m' - 'b');//change .pfb to .pfm
		lstrcat(sPSFont, "|");
		lstrcat(sPSFont, s);//the .pfb
		lstrcat(sPSFont, "|");
		sMMM = sPSFont + strlen(sPSFont);
		lstrcat(sPSFont, s);
		sPSFont[lstrlen(sPSFont) - 3] += ('m' - 'p');//change .pfb to .mmm
		sPSFont[lstrlen(sPSFont) - 2] += ('m' - 'f');
		sPSFont[lstrlen(sPSFont) - 1] += ('m' - 'b');
		hFind = FindFirstFile(sMMM, &ffd);//search for .mmm file
		if (hFind == INVALID_HANDLE_VALUE) 
			*(sMMM - 1) = 0;//truncate the .mmm
		else
			FindClose(hFind);
		//printf("Trying fonts from %s\r\n", sPSFont);
		iResult = AddFontResource(sPSFont);
		free(sPSFont);
	}
	else
		iResult = AddFontResource(s);
	if (iResult)
		printf("%i fonts for arg[%i] in %s\r\n", iResult, i, s);
	else
		printf("ERROR no fonts for arg[%i] in %s\r\n", i, s);
}

void loadDirectory(int i, char *s)
{
	//extensions of font files that AddFontResource can handle; PFB handles like-named .PFM and .MMM as well
	const char *sExts[] = {"FON", "FNT", "TTF", "TTC", "FOT", "OTF", "PFB"};
	const char *sWildcard = "*.";
	int j;
	for (j = sizeof(sExts) / sizeof(sExts[0]); j-- > 0;)
	{
		WIN32_FIND_DATA ffd;
		HANDLE hFind;
		const char *sType = sExts[j];
		char *sPattern = malloc(1 + sizeof(*sPattern) * (lstrlen(s) + 1 + lstrlen(sWildcard) + lstrlen(sType)));
		//printf("type %s\r\n", sType);
		if (sPattern == 0)
		{
			printf("ERROR out-of-memory handling type %s in %s for arg[%i].\r\n", sType, s, i);
			continue;
		}
		lstrcpy(sPattern, s);
		enforcePathSeparator(sPattern);
		lstrcat(sPattern, sWildcard);
		lstrcat(sPattern, sType);
		hFind = FindFirstFile(sPattern, &ffd);
		free(sPattern);					
		if (hFind == INVALID_HANDLE_VALUE) 
		{
			int iError = GetLastError();
			if (iError == ERROR_FILE_NOT_FOUND)
			{} //will happen
			else
				printf("ERROR FindFirstFile failed (%d) for type %s in %s for arg[%i].\r\n", iError, sType, s, i);
			continue;
		} 
		else 
		{
			//printf("The first file found is %s\r\n", ffd.cFileName);
			do
			{
				char *sPath = malloc(1 + sizeof(*sPath) * (lstrlen(s) + 1 + lstrlen(ffd.cFileName)));
				lstrcpy(sPath, s);
				enforcePathSeparator(sPath);
				lstrcat(sPath, ffd.cFileName);
				loadFont(i, sPath);
				free(sPath);
			} while (FindNextFile(hFind, &ffd));
			FindClose(hFind);
		}
	}
}

void loadSubdirectories(int i, char *s)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	const char *sWildcard = "*.*";
	char *sPattern = malloc(1 + sizeof(*sPattern) * (lstrlen(s) + 1 + lstrlen(sWildcard)));
	//printf("Recursion to path %s\r\n", s);
	if (sPattern == 0)
		printf("ERROR out-of-memory handling recursion in %s for arg[%i]\r\n", s, i);
	else
	{
		lstrcpy(sPattern, s);
		enforcePathSeparator(sPattern);
		lstrcat(sPattern, sWildcard);
		hFind = FindFirstFile(sPattern, &ffd);
		free(sPattern);					
		if (hFind == INVALID_HANDLE_VALUE) 
		{
			int iError = GetLastError();
			if (iError == ERROR_FILE_NOT_FOUND)
			{} //will happen
			else
				printf("ERROR FindFirstFile failed (%d) for recursion in %s for arg[%i].\r\n", iError, s, i);
		}
		else 
		{
			do
			{
				DWORD dwAttrib;
				char *sPath;
				if (lstrcmp(".", ffd.cFileName) == 0 || lstrcmp("..", ffd.cFileName) == 0)
					continue;
				sPath = malloc(1 + sizeof(*sPath) * (lstrlen(s) + 1 + lstrlen(ffd.cFileName)));
				lstrcpy(sPath, s);
				enforcePathSeparator(sPath);
				lstrcat(sPath, ffd.cFileName);
				dwAttrib = GetFileAttributes(sPath);
				if (dwAttrib == (DWORD)-1)
				{
					printf("ERROR can not get attributes (%d) for %s in %s for arg[%i].\r\n", GetLastError(), sPath, s, i);
					continue;
				}
				else if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
				{
					loadSubdirectories(i, sPath);
					loadDirectory(i, sPath);
				}
				free(sPath);
			} while (FindNextFile(hFind, &ffd));
			FindClose(hFind);
		}
	}
}

void loadPath(int i, char *s, int recur)
{
	DWORD dwAttrib = GetFileAttributes(s);
	if (dwAttrib == (DWORD)-1)
	{
		printf("ERROR can not get attributes (%d) of %s for arg[%i].\r\n", GetLastError(), s, i);
		return;
	}
	else if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (recur)
			loadSubdirectories(i, s);
		loadDirectory(i, s);
	}
	else if (recur)
		printf("ERROR can only recur over directory, not %s for arg[%]\r\n", s, i);
	else
		loadFont(i, s);
}

void readFontList(int i, char *s)
{
	char *pcFontList = NULL;
	HANDLE hFile = CreateFile(s, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD dwFileSize = GetFileSize(hFile, NULL);//not interested in files so large that we need filesize-high
		DWORD dwRead, dw;
		char *pcPath;
		if (dwFileSize == (DWORD)-1)
			printf("ERROR Cannot handle gigabyte font list file arg[%i] = %s\r\n", i, s);
		else
		{
			pcFontList = malloc(dwFileSize + 1);//1 extra for terminating with ascii-z
			if (!pcFontList)
			{
				printf("ERROR out-of-memory reading font list file arg[%i] = %s\r\n", i, s);
				CloseHandle(hFile);
				return;
			}
			if (!ReadFile(hFile, pcFontList, dwFileSize, &dwRead, NULL) || dwRead != dwFileSize)
			{
				printf("ERROR could not read complete font list file arg[%i] = %s\r\n", i, s);
				free(pcFontList);
				CloseHandle(hFile);
				return;
			}
			pcFontList[dwRead] = 0;//always terminate, in case last line is not terminated?
			pcPath = pcFontList;
			for (dw = 0; dw < dwRead; ++dw)
			{
				if (pcFontList[dw] == '\r' || pcFontList[dw] == '\n')
				{
					pcFontList[dw] = 0;
					loadPath(i, pcPath, 0);
					if (dw + 1 < dwRead)
					{
						if (pcFontList[dw + 1] == '\r' || pcFontList[dw + 1] == '\n')
							++dw;
					}
					pcPath = pcFontList + dw + 1;
				}
			}
			if (lstrlen(pcPath) > 0)
				loadPath(i, pcPath, 0);
			free(pcFontList);
		}
		CloseHandle(hFile);
	}
	else
		printf("ERROR (%d) opening font list file arg[%i] = %s\r\n", GetLastError(), i, s);
}

int main(int argc, char* argv[])
{
	int i;
	if (argc < 2)
	{//PRINT USAGE
		printf(
		"\r\n"
		"Usage : %s [ fontfile | [-R] fontdirectory | -L fonts.lst ]...\r\n\r\n"
		"  Font file can be a .ttf, .ttc, .fon, .fnt, .fot, .otf or .pfb type.\r\n"
		"    For a Type 1 .pfb (font bits), also the .pfm (font metrics) and eventual\r\n"
		"    .mmm (multiple master metrics) are loaded. Loading multiple master\r\n"
		"    instances however is not supported.\r\n"
		"  -R : load subdirectories of font directory recursively.\r\n"
		"  Font directory : load all font files in it with the above extensions.\r\n"
		"  Font list with fontdirectories (no recursion) and fontfiles, line by line.\r\n"
		, argv[0]);
		return;
	}
	for (i = 1; i < argc; ++i)
	{
		char *s = argv[i];
		//printf("%d \"%s\" %u bytes/char\r\n", i, s, sizeof(*s));
		if (lstrcmpi("-L", s) == 0)
		{
			if (++i >= argc)
				printf("ERROR \"-L\" with no file\r\n");
			else
				readFontList(i, argv[i]);	//printf("-L %s\r\n", argv[i]);
		}
		else if (lstrcmpi("-R", s) == 0)
		{
			if (++i >= argc)
				printf("ERROR \"-R\" with no directory\r\n");
			else
				loadPath(i, argv[i], 1);
		}
		else
			loadPath(i, s, 0);
	}
	/*Any application that adds or removes fonts from the system font table should notify other windows of the change by sending a WM_FONTCHANGE message to all top-level windows in the operating system. The application should send this message by calling the SendMessage function and setting the hwnd parameter to HWND_BROADCAST. */
	SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);	
	return 0;
}
