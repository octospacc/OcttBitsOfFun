#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <inttypes.h>

#ifdef __3DS__
	#include <3ds.h>
#endif

#define PathStringSize 128

#ifdef __3DS__
	#define ScreenshotsPath "sdmc:/luma/screenshots/"
	#define ScreenshotsBakPath "sdmc:/luma/screenshots-old/"
	#define DcimPath "sdmc:/DCIM/"
	#define mkdir(path)         { mkdir(path, 7777); }
	#define DirListPrepare(dir) {}
#else
	#define ScreenshotsPath "./luma/screenshots/"
	#define ScreenshotsBakPath "./luma/screenshots-old/"
	#define DcimPath "./DCIM/"
	#define DirListPrepare(dir) { readdir(dir); readdir(dir); } // exclude `.` and `..` from list
#endif

#define IsStringEqual(s1, s2)  ( strcmp(s1, s2) == 0 )
#define IsStringEmpty(str)     ( IsStringEqual(str, "") )
#define ExistDirectory(path)   ( opendir(path) != NULL )
#define MkDirIfNotExist(path)  { if (!ExistDirectory(path)) mkdir(path); }
#define Try(flag, then)        { if (!(flag)) { then; printf("A fatal error occured.\n"); Exit(); } }

void Exit (void)
{
	printf("\nThe program will now exit.\n");
	#ifdef __3DS__
		sleep(3);
		gfxExit();
	#endif
	exit(0);
}

char *GetLastDirectoryItem (const char path[], char *buffer)
{
	DIR *directory;
	struct dirent *dirEntry;
	memset(buffer, 0, PathStringSize);
	if ((directory = opendir(path)) != NULL) {
		DirListPrepare(directory);
		while ((dirEntry = readdir(directory)) != NULL) {
			strcpy(buffer, dirEntry->d_name);
		}
		closedir(directory);
	} else {
		printf("Error reading `%s` directory.\n", path);
		Exit();
	}
	return buffer;
}

bool IsDirectoryEmpty (char *path) {
	char buffer[PathStringSize];
	return IsStringEmpty(GetLastDirectoryItem(path, buffer));
}

char *GetDcimNinNameFromIndex (int index, char *buffer)
{
	char indexString[5];
	sprintf(buffer, "%dNIN03/", index);
	return buffer;
}

char *GetDcimNinFullPathFromIndex (int index, char *buffer)
{
	char name[PathStringSize];
	return strcat( strcpy(buffer, DcimPath), GetDcimNinNameFromIndex(index, name) );
}

void *MakeDcimNinFolderFromIndex (int index)
{
	char path[PathStringSize];
	mkdir(GetDcimNinFullPathFromIndex(index, path));
}

int GetDcimNinIndexFromPath (char *path)
{
	char indexStr[4];
	indexStr[0] = path[0];
	indexStr[1] = path[1];
	indexStr[2] = path[2];
	return atoi(indexStr);
}

int GetHniXIndexFromPath (char *path)
{
	char indexStr[5];
	int pathExtIndex = (strlen(path) - 1 - 4);
	indexStr[0] = path[pathExtIndex - 3];
	indexStr[1] = path[pathExtIndex - 2];
	indexStr[2] = path[pathExtIndex - 1];
	indexStr[3] = path[pathExtIndex - 0];
	return atoi(indexStr);
}

bool IsXNinFolderFullFromIndex (int index)
{
	char path[PathStringSize], buffer[PathStringSize];
	char *curXNinFullPath = GetDcimNinFullPathFromIndex(index, path);
	char *lastHniXPath = GetLastDirectoryItem(curXNinFullPath, buffer);
	int lastHniXIndex = GetHniXIndexFromPath(lastHniXPath);
	return ( lastHniXIndex >= 100 );
}

char *ReadFilepathIntoBuffer (char filepath[], size_t *sizeOut) {
	size_t size;
	char *buffer;
	FILE *file;
	Try((file = fopen(filepath, "rb")), {});
	fseek(file, 0L, SEEK_END);
	size = ftell(file);
	rewind(file);
	Try((buffer = calloc(1, size+1)), { fclose(file); })
	if ( fread(buffer, size, 1, file) != 1 ) {
		fclose(file);
		free(buffer);
		Exit();
	}
	fclose(file);
	*sizeOut = size;
	return buffer;
}

void WriteBufferIntoFilepath(char *buffer, size_t size, char filepath[]) {
	FILE *file;
	Try((file= fopen(filepath, "wb")), {});
	fwrite(buffer, size, 1, file);
	fclose(file);
}

#ifdef __3DS__
u8 *HttpPostBmpForJpg(const char* url, const char* data, size_t len, /*char *outBuffer*/size_t *outSize) {
	Result ret=0;
	httpcContext context;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *buf, *lastbuf;

	httpcOpenContext(&context, HTTPC_METHOD_POST, url, 0);
	httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
	httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
	httpcAddRequestHeaderField(&context, "User-Agent", "LumaBmp2NinJpg/1.0.0");
	httpcAddRequestHeaderField(&context, "Content-Type", "image/bmp");
	httpcAddPostDataRaw(&context, (u32*)data, len);

	if((ret = httpcBeginRequest(&context))!=0){
		httpcCloseContext(&context);
		return ret;
	}

	if((ret = httpcGetResponseStatusCode(&context, &statuscode))!=0){
		httpcCloseContext(&context);
		return ret;
	}

	if(statuscode!=200){
		httpcCloseContext(&context);
		return -2;
	}

	if((ret=httpcGetDownloadSizeState(&context, NULL, &contentsize))!=0){
		httpcCloseContext(&context);
		return ret;
	}

	if((buf = (u8*)malloc(0x1000))==NULL){
		httpcCloseContext(&context);
		return -1;
	}

	do {
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize; 
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
			lastbuf = buf;
			if((buf = realloc(buf, size + 0x1000))==NULL){ 
				httpcCloseContext(&context);
				free(lastbuf);
				return -1;
			}
		}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

	if(ret!=0){
		httpcCloseContext(&context);
		free(buf);
		return -1;
	}

	lastbuf = buf;
	if((buf = realloc(buf, size))==NULL){
		httpcCloseContext(&context);
		free(lastbuf);
		return -1;
	}

	httpcCloseContext(&context);
	*outSize = size;
	return buf;
}
#endif

int main (int argc, char** argv)
{
	int dcimFolderIndex = 100;
	char initDcimNinPath[PathStringSize];

	#ifdef __3DS__
		PrintConsole bottomScreen;
		gfxInitDefault();
		consoleInit(GFX_BOTTOM, &bottomScreen);
		consoleSelect(&bottomScreen);
		httpcInit(4 * 1024 * 1024);
	#endif

	if (!ExistDirectory(ScreenshotsPath)) {
		printf("The `%s` directory doesn't exist. Nothing to do.\n", ScreenshotsPath);
		Exit();
	}

	if (IsDirectoryEmpty(ScreenshotsPath)) {
		printf("No files present in `%s`. Nothing to do.\n", ScreenshotsPath);
		Exit();
	}

	MkDirIfNotExist(ScreenshotsBakPath);
	MkDirIfNotExist(DcimPath);

	if (IsDirectoryEmpty(DcimPath)) {
		MakeDcimNinFolderFromIndex(dcimFolderIndex);
	}

	GetLastDirectoryItem(DcimPath, initDcimNinPath);
	dcimFolderIndex = GetDcimNinIndexFromPath(initDcimNinPath);

	struct dirent *dirEntry;
	DIR *directory = opendir(ScreenshotsPath);
	DirListPrepare(directory);
	while ((dirEntry = readdir(directory)) != NULL) {
		char srcPath[PathStringSize], srcPathBak[PathStringSize], dstName[PathStringSize], dstPath[PathStringSize];
		char currDcimNinName[PathStringSize], currDcimNinPath[PathStringSize], lastJpgName[PathStringSize];
		
		if (IsXNinFolderFullFromIndex(dcimFolderIndex)) {
			MakeDcimNinFolderFromIndex(++dcimFolderIndex);
		}
		
		GetLastDirectoryItem(DcimPath, currDcimNinName);
		sprintf(currDcimNinPath, "%s%s/", DcimPath, currDcimNinName);

		GetLastDirectoryItem(currDcimNinPath, lastJpgName);
		int currJpgIndex = (GetHniXIndexFromPath(lastJpgName) + 1);
		
		sprintf(srcPath, "%s%s", ScreenshotsPath, dirEntry->d_name);
		sprintf(srcPathBak, "%s%s", ScreenshotsBakPath, dirEntry->d_name);
		sprintf(dstName, "%dNIN03/HNI_%04d.JPG", dcimFolderIndex, currJpgIndex);
		sprintf(dstPath, "%s%s", DcimPath, dstName);
		printf("* %s => %s ...", dirEntry->d_name, dstName);

		#ifdef __3DS__
			size_t bmpSize, jpgSize;
			char *bmpData = ReadFilepathIntoBuffer(srcPath, &bmpSize);
			char *jpgData = HttpPostBmpForJpg("http://hlb0.octt.eu.org/LumaBmp2NinJpgConvert.php", bmpData, bmpSize, &jpgSize);
			free(bmpData);
			if (jpgSize > 0) {
				printf("Done!\n\n");
				WriteBufferIntoFilepath(jpgData, jpgSize, dstPath);
		#endif
				rename(srcPath, srcPathBak);
			#ifndef __3DS__
				printf("\n");
			#endif
		#ifdef __3DS__
			} else {
				printf("Fail!\n\n");
			}
			free(jpgData);
		#endif
	}
	closedir(directory);

	Exit();
}
