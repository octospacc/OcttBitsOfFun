#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <inttypes.h>
//#include "cjpeg.h"
//#include "cio.h"

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
	printf("The program will now exit.\n");
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
	//printf(". %s %s\n", path, indexStr);
	return atoi(indexStr);
}

bool IsXNinFolderFullFromIndex (int index)
{
	char path[PathStringSize], buffer[PathStringSize];
	char *curXNinFullPath = GetDcimNinFullPathFromIndex(index, path);
	char *lastHniXPath = GetLastDirectoryItem(curXNinFullPath, buffer);
	int lastHniXIndex = GetHniXIndexFromPath(lastHniXPath);
	//printf("%s %s %d", curXNinFullPath, lastHniXPath, lastHniXIndex);
	return ( lastHniXIndex >= 100 );
}
	
//bool ConvertImage (char *bmpPath, char *jpgPath) {
/*	FILE *bmpFile, *jpgFile;
	bmp_info bmpInfo;
	compress_io compressIo;

	if ( (bmpFile = fopen(bmpPath, "rb")) == NULL || !is_bmp(bmpFile) ) {
		return false;
	}

	if ( (jpgFile = fopen(jpgPath, "wb")) == NULL ) {
		return false;
	} 

	read_bmp(bmpFile, &bmpInfo);
	init_mem(&compressIo, bmpFile, ((bmpInfo.width * 3 + 3) / 4 * 4 * DCTSIZE), jpgFile, MEM_OUT_SIZE);
	jpeg_encode(&compressIo, &bmpInfo);

	if ( !(compressIo.out->flush_buffer)(&compressIo) ) {
		return false;
	}

	free_mem(&compressIo);
	fclose(bmpFile);
	fclose(jpgFile);
	return true;
*/
//}

char *ReadFilepathIntoBuffer (char filepath[], size_t *sizeOut) {
//size_t ReadFilepathIntoBuffer (char filepath[], char *buffer) {
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
	return buffer;//size;
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

	//printf("1\n");
	//sleep(1);

	ret = httpcBeginRequest(&context);
	if(ret!=0){
		httpcCloseContext(&context);
		return ret;
	}

	//printf("2\n");
	//sleep(1);

	ret = httpcGetResponseStatusCode(&context, &statuscode);
	//printf("r %d %d\n",ret,statuscode);
	//sleep(1);
	if(ret!=0){
		httpcCloseContext(&context);
		return ret;
	}

	//printf("3\n");
	//sleep(1);

	if(statuscode!=200){
		httpcCloseContext(&context);
		return -2;
	}

	//printf("4\n");
	//sleep(1);

	ret=httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if(ret!=0){
		httpcCloseContext(&context);
		return ret;
	}

	//printf("5\n");
	//sleep(1);

	buf = (u8*)malloc(0x1000);
	if(buf==NULL){
		httpcCloseContext(&context);
		return -1;
	}

	//printf("6\n");
	//sleep(1);

	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize; 
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = realloc(buf, size + 0x1000);
				if(buf==NULL){ 
					httpcCloseContext(&context);
					free(lastbuf);
					return -1;
				}
			}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);	

	//printf("7\n");
	//sleep(1);

	if(ret!=0){
		httpcCloseContext(&context);
		free(buf);
		return -1;
	}

	//printf("8\n");
	//sleep(1);

	// Resize the buffer back down to our actual final size
	lastbuf = buf;
	buf = realloc(buf, size);
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		return -1;
	}

	//printf("9\n");
	//sleep(1);

	httpcCloseContext(&context);
	*outSize = size;
	return buf;//size;
/*
	Result ret=0;
	httpcContext context;
	char *newurl=NULL;
	u32 statuscode=0;
	u32 contentsize=0, readsize=0, size=0;
	u8 *buf, *lastbuf;

	printf("POSTing %s\n", url);

	do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 0);
		printf("return from httpcOpenContext: %" PRIx32 "\n",ret);

		// This disables SSL cert verification, so https:// will be usable
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		printf("return from httpcSetSSLOpt: %" PRIx32 "\n",ret);

		// Enable Keep-Alive connections
		ret = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
		printf("return from httpcSetKeepAlive: %" PRIx32 "\n",ret);

		// Set a User-Agent header so websites can identify your application
		ret = httpcAddRequestHeaderField(&context, "User-Agent", "LumaBmp2NinJpg/1.0.0");
		printf("return from httpcAddRequestHeaderField: %" PRIx32 "\n",ret);

		// Set a Content-Type header so websites can identify the format of our raw body data.
		// If you want to send form data in your request, use:
		//ret = httpcAddRequestHeaderField(&context, "Content-Type", "multipart/form-data");
		// If you want to send raw JSON data in your request, use:
		ret = httpcAddRequestHeaderField(&context, "Content-Type", "image/bmp");//"application/json");
		printf("return from httpcAddRequestHeaderField: %" PRIx32 "\n",ret);

		// Post specified data.
		// If you want to add a form field to your request, use:
		//ret = httpcAddPostDataAscii(&context, "data", value);
		// If you want to add a form field containing binary data to your request, use:
		//ret = httpcAddPostDataBinary(&context, "field name", yourBinaryData, length);
		// If you want to add raw data to your request, use:
		ret = httpcAddPostDataRaw(&context, (u32*)data, len);
		printf("return from httpcAddPostDataRaw: %" PRIx32 "\n",ret);

		ret = httpcBeginRequest(&context);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if(newurl==NULL) newurl = malloc(0x1000); // One 4K page for new URL
			if (newurl==NULL){
				httpcCloseContext(&context);
				return -1;
			}
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			url = newurl; // Change pointer to the url that we just learned
			printf("redirecting to url: %s\n",url);
			httpcCloseContext(&context); // Close this context before we try the next
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

	if(statuscode!=200){
		printf("URL returned status: %" PRIx32 "\n", statuscode);
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -2;
	}

	// This relies on an optional Content-Length header and may be 0
	ret=httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return ret;
	}

	printf("reported size: %" PRIx32 "\n",contentsize);

	// Start with a single page buffer
	buf = (u8*)malloc(0x1000);
	if(buf==NULL){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		return -1;
	}

	do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize; 
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = realloc(buf, size + 0x1000);
				if(buf==NULL){ 
					httpcCloseContext(&context);
					free(lastbuf);
					if(newurl!=NULL) free(newurl);
					return -1;
				}
			}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);	

	if(ret!=0){
		httpcCloseContext(&context);
		if(newurl!=NULL) free(newurl);
		free(buf);
		return -1;
	}

	// Resize the buffer back down to our actual final size
	lastbuf = buf;
	buf = realloc(buf, size);
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
		if(newurl!=NULL) free(newurl);
		return -1;
	}

	printf("response size: %" PRIx32 "\n",size);

	printf("%s\n",buf);
	
	gfxFlushBuffers();
	gfxSwapBuffers();

	httpcCloseContext(&context);
	//free(buf);
	if (newurl!=NULL) free(newurl);

	*outSize = size;
	return buf;//size;
	*/
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

//size_t len1;
//char *buf1 = HttpPostBmpForJpg("https://httpbin.org/post", "{\"foo\": \"bar\"}", 19, &len1);
//printf("%d %s\n",len1,buf1);
//sleep(1);

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
			char *bmpData, *jpgData;
			size_t bmpSize, jpgSize;
			//size_t bmpSize = ReadFilepathIntoBuffer(srcPath, bmpData);
			bmpData = ReadFilepathIntoBuffer(srcPath, &bmpSize);
			//printf("%d %s\n", bmpSize, bmpData);
			//sleep(3);
			//size_t jpgSize = HttpPostBmpForJpg("https://hlb0.octt.eu.org/LumaBmp2NinJpgConvert.php", bmpData, bmpSize, jpgData);
			jpgData = HttpPostBmpForJpg("http://hlb0.octt.eu.org/LumaBmp2NinJpgConvert.php", bmpData, bmpSize, &jpgSize);
			//sleep(5);
			//printf("%d %s\n", jpgSize, jpgData);
			//sleep(3);
			free(bmpData);
			if (jpgSize > 0) {
				printf("Done!\n");
				WriteBufferIntoFilepath(jpgData, jpgSize, dstPath);
		#endif
				rename(srcPath, srcPathBak);
				//rename(srcPath, dstPath);
			#ifndef __3DS__
				printf("\n");
			#endif
		#ifdef __3DS__
			} else {
				printf("Fail!\n");
			}
			free(jpgData);
		#endif
	}
	closedir(directory);

	Exit();
}
