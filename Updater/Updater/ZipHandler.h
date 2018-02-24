//  Copyright (c) 2018, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

//#include <windows.h>
#include <string>
#include "contrib/minizip/zip.h"
const int CHUNK = 16384;

class ZipHandler
{
public:
	ZipHandler();
	~ZipHandler(){};
	bool UnZipFile(const wchar_t *pathOfZip, const wchar_t *destinationDir);
	bool ZipFolder(const wchar_t *destinationDir, const wchar_t *pathOfZip, const wchar_t **excludes, int numExcludes);
	const std::string & GetError(){ return log; };
private:
	bool ConvertToWchar(char *source, wchar_t *dest);
	bool ConvertToChar(const wchar_t *source, char **dest);
	bool CheckExcludes(wchar_t *path, const wchar_t **excludes, int numExcludes);
	wchar_t * GetSubstring(wchar_t *string, int numChar);
	bool ZipFolderFiles(zipFile zf, const wchar_t *destinationDir, const wchar_t *pathOfZip, const wchar_t **excludes, int numExcludes);
	bool ZipFile(zipFile zf, const wchar_t *name, const wchar_t *filepath);
	std::string log;
	size_t firstFolderStart = 0;
	char buffer[CHUNK];
};