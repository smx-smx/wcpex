/*
 * Exctraction tool for files handled by WCP - Windows Componentization Platform
 * Copyright 2017 Smx <smxdev4@gmail.com>
 * 
 * Program to unpack and (TODO) repack Manifest files, like the ones found in
 * %windir%\WinSxS\Manifests
 */

/*
 * The manifest files are MSDelta compressed, with a header appended to indicate their type
 */
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

struct __attribute__((packed)) BlobData {
	size_t length;
	size_t fill;
	void *pData;
};

static HMODULE wcp;
static long (__fastcall *InitializeDeltaCompressor)(uintptr_t arg);
static unsigned long (__fastcall *GetCompressedFileType)(struct BlobData *arg);
/*
static long (__fastcall *DeltaCompressFile)(
	unsigned long DeltaFlagType,
	void *pDictionary,
	unsigned long headerSize,
	struct BlobData *inData,
	struct BlobData *outData
);
*/
static long (__fastcall *DeltaDecompressBuffer)(
	unsigned long DeltaFlagType,
	void *pDictionary,
	unsigned long headerSize,
	struct BlobData *inData,
	struct BlobData *outData
);
static long (__fastcall *LoadFirstResourceLanguageAgnostic)(
	uintptr_t,
	HMODULE hModule,
	LPCWSTR lpType,
	LPCWSTR lpName,
	void *pOutDict
);

void hexdump(void *pAddressIn, long lSize) {
	char szBuf[100];
	long lIndent = 1;
	long lOutLen, lIndex, lIndex2, lOutLen2;
	long lRelPos;
	struct {
		char *pData;
		unsigned long lSize;
	} buf;
	unsigned char *pTmp, ucTmp;
	unsigned char *pAddress = (unsigned char *)pAddressIn;

	buf.pData = (char *)pAddress;
	buf.lSize = lSize;

	while (buf.lSize > 0) {
		pTmp = (unsigned char *)buf.pData;
		lOutLen = (int)buf.lSize;
		if (lOutLen > 16)
			lOutLen = 16;

		// create a 64-character formatted output line:
		sprintf(szBuf, " >                                                      %08zX", pTmp - pAddress);
		lOutLen2 = lOutLen;

		for (lIndex = 1 + lIndent, lIndex2 = 53 - 15 + lIndent, lRelPos = 0; lOutLen2; lOutLen2--, lIndex += 2, lIndex2++) {
			ucTmp = *pTmp++;
			sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
			if (!isprint(ucTmp))
				ucTmp = '.';	// nonprintable char
			szBuf[lIndex2] = ucTmp;

			if (!(++lRelPos & 3)) {	// extra blank after 4 bytes
				lIndex++;
				szBuf[lIndex + 2] = ' ';
			}
		}
		if (!(lRelPos & 3))
			lIndex--;
		szBuf[lIndex] = '<';
		szBuf[lIndex + 1] = ' ';
		printf("%s\n", szBuf);
		buf.pData += lOutLen;
		buf.lSize -= lOutLen;
	}
}

int init(){
	wcp = LoadLibraryA("wcp.dll");
	if(!wcp){
		fprintf(stderr, "wcp load fail\n");
		return -1;
	}
	GetCompressedFileType = (void *)GetProcAddress(wcp, "?GetCompressedFileType@Rtl@WCP@Windows@@YAKPEBU_LBLOB@@@Z");
	InitializeDeltaCompressor = (void *)GetProcAddress(wcp, "?InitializeDeltaCompressor@Rtl@Windows@@YAJPEAX@Z");
	//DeltaCompressFile = (void *)GetProcAddress(wcp, "?DeltaCompressFile@Rtl@Windows@@YAJKPEAUIRtlFile@12@PEBU_LBLOB@@00@Z");
	DeltaDecompressBuffer = (void *)GetProcAddress(wcp, "?DeltaDecompressBuffer@Rtl@Windows@@YAJKPEAU_LBLOB@@_K0PEAVAutoDeltaBlob@12@@Z");
	LoadFirstResourceLanguageAgnostic = (void *)GetProcAddress(wcp, "?LoadFirstResourceLanguageAgnostic@Rtl@Windows@@YAJKPEAUHINSTANCE__@@PEBG1PEAU_LBLOB@@@Z");
	if(
		GetCompressedFileType == NULL ||
		InitializeDeltaCompressor == NULL ||
		//DeltaCompressFile == NULL ||
		DeltaDecompressBuffer == NULL ||
		LoadFirstResourceLanguageAgnostic == NULL
	){
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]){
	int eCode = 1;	//exit code
	size_t dataSize; //size of input data
	uint8_t *manifestData = NULL; //copy of input data

	long result = -1; //result of calls
	if(argc < 2){
		fprintf(stderr, "Usage: %s [infile][[outfile]]\n", argv[0]);
		goto end;
	}
	
	if(init() != 0){
		fprintf(stderr, "Cannot load wcp.dll\n");
		goto end;
	}
	
	FILE *f = fopen(argv[1], "rb");
	if(!f){
		fprintf(stderr, "Open Fail\n");
		return 1;
	}
	fseek(f, 0, SEEK_END);
	dataSize = ftell(f);
	rewind(f);
	manifestData = calloc(1, dataSize);
	fread(manifestData, dataSize, 1, f);
	fclose(f);

	printf("Size: %u\n", dataSize);

	struct BlobData inData = {
		.length = dataSize,
		.fill = dataSize,
		.pData = (void *)manifestData
	};
	unsigned long type = GetCompressedFileType(&inData);
	printf("Type is %d\n", type);

	if(type != 4){
		printf("Unsupported compression type '%d'\n", type);
		goto end;
	}

	result = InitializeDeltaCompressor((uintptr_t)NULL);
	printf("InitializeDeltaCompressor: 0x%08X\n", result);
	if(result >= 0){
		result = 0;
	} else {
		printf("InitializeDeltaCompressor failed\n");
		goto end;
	}

	uint64_t DictData[3];
	result = LoadFirstResourceLanguageAgnostic(
		0, //unneded
		wcp, //HMODULE

		// These seem to have a special meaning
		(LPCWSTR)0x266, //lpType
		(LPCWSTR)1, //lpName

		&DictData
	);

	printf("LoadFirstResourceLanguageAgnostic: 0x%08X\n", result);
	if(result >= 0){
		result = 0;
	} else {
		printf("LoadFirstResourceLanguageAgnostic failed\n");
		goto end;
	}

	printf("==> Dictionary\n");
	hexdump(&DictData, sizeof(DictData));


	struct BlobData outData;
	result = DeltaDecompressBuffer(
		2, //type?
		&DictData,
		4, //headerSize
		&inData,
		&outData
	);
	printf("DeltaDecompressBuffer: 0x%08X\n", result);
	if(result >= 0){
		result = 0;
	} else {
		printf("DeltaDecompressBuffer failed\n");
		goto end;
	}

	printf("==> Out Blob\n");
	//hexdump(&outData, sizeof(outData));
	FILE *outFile;
	if(argc < 3)
		outFile = stdout;
	else
		outFile = fopen(argv[2], "w");

	if(!outFile){
		fprintf(stderr, "Cannot open output file '%s' for writing\n", argv[2]);
		goto end;
	}
	fwrite(outData.pData, outData.length, 1, outFile);
	//fputs(outData.pData, outFile);
	if(argc > 2)
		fclose(outFile);

	eCode = 0;

	end:
	if(manifestData != NULL)
		free(manifestData);
	return eCode;
}
