// flipperNFCFormatToBin.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char dataBegin[] = "Data Content:";
char dataEnd[] = "\n# Subtype";

#define DATA_BLOCK_SIZE 3
#define RET_OK 0
#define RET_MEM_ERROR -1
#define RET_NOT_FOUND -2
#define RET_BAD_FORMAT -3
#define RET_FILE_ERROR -4

int findDataBegin(FILE* fp, const char* dataInitiator);
uint8_t translateByte(uint8_t value);
int decodeFile(FILE* fp, FILE* output_fp, const char* dataEnd);

int main(int argc, char* argv[])
{
    FILE* input, * output;
	int returnCode = 0;

    if (argc != 3) {
        printf("\nUsage: flipperNFCToBin <flipperDump> <outputfile>");
        return RET_OK;
    }

    if (fopen_s(&input, argv[1], "rb")) {
        printf("\nError opening %s", argv[1]);
        return RET_FILE_ERROR;
    }

    if (fopen_s(&output, argv[2], "wb")) {
        printf("\nError creating %s", argv[2]);
        return RET_FILE_ERROR;
    }

    returnCode = findDataBegin(input, dataBegin);
    
    if (returnCode < RET_OK) {
        fclose(input);
        fclose(output);
    }

	switch (returnCode) {
	case RET_MEM_ERROR:
		printf("\nError: Memory error");
		return RET_MEM_ERROR;
	case RET_NOT_FOUND:
		printf("\nError: Data Content not found in file %s. Is this a flipperZero NFC dump file?", argv[1]);
		return RET_NOT_FOUND;
	}
    
	printf("\nData block begin found at: %d", returnCode);

	returnCode = decodeFile(input, output, dataEnd);
	fclose(input);
	fclose(output);
    
	switch (returnCode) {
	case RET_MEM_ERROR:
		printf("\nError: Memory error");
		return RET_MEM_ERROR;
        
    case 0:
	case RET_NOT_FOUND:
		printf("\nError: Data Content not found in file %s. Is this a flipperZero NFC dump file?", argv[1]);
		return RET_NOT_FOUND;
	case RET_BAD_FORMAT:
		printf("\nError: Bad format in file %s. Is this a flipperZero NFC dump file?", argv[1]);
		return RET_BAD_FORMAT;
	}

	printf("\nSuccesfully %d bytes written to %s\n\n", returnCode, argv[2]);

    system("pause");
    return RET_OK;
}

int decodeFile(FILE* input_fp, FILE * output_fp, const char* dataEnd)
{
    char* buffer = NULL;
    uint8_t outputByte = 0;
    int counter = 0;

    if ((buffer = (char*)calloc(1, strlen(dataEnd) + DATA_BLOCK_SIZE)) == NULL) {
        return RET_MEM_ERROR;
    }
    #if _MSC_VER
        fread_s(buffer, strlen(dataEnd) + DATA_BLOCK_SIZE, strlen(dataEnd) + DATA_BLOCK_SIZE, 1, input_fp);
    #else
	    fread(buffer, strlen(dataEnd) + DATA_BLOCK_SIZE, 1, input_fp);
    #endif

    while (!feof(input_fp)) {
        if (memcmp(buffer, dataEnd, strlen(dataEnd)) == 0) {
            return counter;
        }

        if (buffer[0] != ' ') {
            return RET_BAD_FORMAT;
        }
        outputByte = (translateByte(buffer[1]) * 16) + translateByte(buffer[2]);
        fwrite(&outputByte, 1, 1, output_fp);
        counter++;

        for (int i = 0; i < strlen(dataEnd); i++) {
            buffer[i] = buffer[i + 3];
        }
        #if _MSC_VER
            fread_s(buffer + strlen(dataEnd), strlen(dataEnd) + DATA_BLOCK_SIZE, DATA_BLOCK_SIZE, 1, input_fp);
        #else
            fread(buffer + strlen(dataEnd), 1, DATA_BLOCK_SIZE, fp);
        #endif

    }
    return RET_NOT_FOUND;
}

uint8_t translateByte(uint8_t value) 
{
    if ((value >= '0') &&
        (value <= '9')) {
        return value - '0';
    }
    if ((value >= 'A') &&
        (value <= 'F')) {
        return value - 'A' + 10;
    }
}

int findDataBegin(FILE* fp, const char* dataInitiator)
{
    char * buffer = NULL;
    int counter = 0;

    if ((buffer = (char*)calloc(1, strlen(dataBegin))) == NULL) {
        return RET_MEM_ERROR;
    }
    rewind(fp);

    //Find begin string dataBegin in input file
    #if _MSC_VER
        fread_s(buffer, strlen(dataBegin), strlen(dataBegin), 1, fp);
    #else
        fread(buffer, strlen(dataBegin), 1, fp);
    #endif
    
    while (!feof(fp)) {
        for (int i = 0; i < strlen(dataBegin) - 1; i++) {
            buffer[i] = buffer[i + 1];
        }
        #if _MSC_VER
		    fread_s(buffer + strlen(dataBegin) - 1, strlen(dataBegin), 1, 1, fp);
        #else
            fread(buffer + strlen(dataBegin) - 1, 1, 1, fp);
        #endif
        if (memcmp(buffer, dataBegin, strlen(dataBegin)) == 0) {
            return counter;
        }
        counter++;
    }
    return RET_NOT_FOUND;
}

