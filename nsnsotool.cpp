// Copyright (c) 2018, CBH <maodatou88@163.com>
// Licensed under the terms of the BSD 3-Clause License
// https://github.com/0CBH0/nsnsotool/blob/master/LICENSE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lz4.h>
#include "nsnsotool.h"

#pragma warning(disable : 4996)

using namespace std;

int decompress(FILE *in, FILE *out)
{
	NSOHeader nsoHeader;
	fread(&nsoHeader, sizeof(NSOHeader), 1, in);
	nsoHeader.flags = 0;
	fwrite(&nsoHeader, sizeof(NSOHeader), 1, out);
	tempa_u32 = 0;
	for (u32 i = 0; i < 0x18; i++) fwrite(&tempa_u32, 4, 1, out);
	while ((u32)ftell(out) < nsoHeader.fileOffset_text) putc(0, out);
	char *textData = new char[nsoHeader.cmpSize_text];
	char *dcmpTextData = new char[nsoHeader.dcmpSize_text];
	fseek(in, nsoHeader.fileOffset_text, 0);
	fread(textData, 1, nsoHeader.cmpSize_text, in);
	LZ4_decompress_safe(textData, dcmpTextData, nsoHeader.cmpSize_text, nsoHeader.dcmpSize_text);
	fwrite(dcmpTextData, 1, nsoHeader.dcmpSize_text, out);
	nsoHeader.cmpSize_text = nsoHeader.dcmpSize_text;
	char *rodataData = new char[nsoHeader.cmpSize_rodata];
	char *dcmpRodataData = new char[nsoHeader.dcmpSize_rodata];
	fseek(in, nsoHeader.fileOffset_rodata, 0);
	nsoHeader.fileOffset_rodata = ftell(out);
	fread(rodataData, 1, nsoHeader.cmpSize_rodata, in);
	LZ4_decompress_safe(rodataData, dcmpRodataData, nsoHeader.cmpSize_rodata, nsoHeader.dcmpSize_rodata);
	fwrite(dcmpRodataData, 1, nsoHeader.dcmpSize_rodata, out);
	nsoHeader.cmpSize_rodata = nsoHeader.dcmpSize_rodata;
	char *dataData = new char[nsoHeader.cmpSize_data];
	char *dcmpDataData = new char[nsoHeader.dcmpSize_data];
	fseek(in, nsoHeader.fileOffset_data, 0);
	nsoHeader.fileOffset_data = ftell(out);
	fread(dataData, 1, nsoHeader.cmpSize_data, in);
	LZ4_decompress_safe(dataData, dcmpDataData, nsoHeader.cmpSize_data, nsoHeader.dcmpSize_data);
	fwrite(dcmpDataData, 1, nsoHeader.dcmpSize_data, out);
	nsoHeader.cmpSize_data = nsoHeader.dcmpSize_data;
	rewind(out);
	fwrite(&nsoHeader, sizeof(NSOHeader), 1, out);
	delete[]textData;
	delete[]dcmpTextData;
	delete[]rodataData;
	delete[]dcmpRodataData;
	delete[]dataData;
	delete[]dcmpDataData;
	return 0;
}

int compress(FILE *in, FILE *out)
{
	u32 sha256_text[8];
	u32 sha256_rodata[8];
	u32 sha256_data[8];
	NSOHeader nsoHeader;
	fread(&nsoHeader, sizeof(NSOHeader), 1, in);
	nsoHeader.flags = 0x3F;
	fwrite(&nsoHeader, sizeof(NSOHeader), 1, out);
	tempa_u32 = 0;
	for (u32 i = 0; i < 0x18; i++) fwrite(&tempa_u32, 4, 1, out);
	while ((u32)ftell(out) < nsoHeader.fileOffset_text) putc(0, out);
	char *dcmpTextData = new char[nsoHeader.dcmpSize_text];
	fseek(in, nsoHeader.fileOffset_text, 0);
	fread(dcmpTextData, 1, nsoHeader.dcmpSize_text, in);
	sha256_nso(dcmpTextData, nsoHeader.dcmpSize_text, sha256_text);
	char *textData = new char[LZ4_compressBound(nsoHeader.dcmpSize_text)];
	nsoHeader.cmpSize_text = LZ4_compress_default(dcmpTextData, textData, nsoHeader.dcmpSize_text, LZ4_compressBound(nsoHeader.dcmpSize_text));
	fwrite(textData, 1, nsoHeader.cmpSize_text, out);
	char *dcmpRodataData = new char[nsoHeader.dcmpSize_rodata];
	fseek(in, nsoHeader.fileOffset_rodata, 0);
	nsoHeader.fileOffset_rodata = ftell(out);
	fread(dcmpRodataData, 1, nsoHeader.dcmpSize_rodata, in);
	sha256_nso(dcmpRodataData, nsoHeader.dcmpSize_rodata, sha256_rodata);
	char *rodataData = new char[LZ4_compressBound(nsoHeader.dcmpSize_rodata)];
	nsoHeader.cmpSize_rodata = LZ4_compress_default(dcmpRodataData, rodataData, nsoHeader.dcmpSize_rodata, LZ4_compressBound(nsoHeader.dcmpSize_rodata));
	fwrite(rodataData, 1, nsoHeader.cmpSize_rodata, out);
	char *dcmpDataData = new char[nsoHeader.dcmpSize_data];
	fseek(in, nsoHeader.fileOffset_data, 0);
	nsoHeader.fileOffset_data = ftell(out);
	fread(dcmpDataData, 1, nsoHeader.dcmpSize_data, in);
	sha256_nso(dcmpDataData, nsoHeader.dcmpSize_data, sha256_data);
	char *dataData = new char[LZ4_compressBound(nsoHeader.dcmpSize_data)];
	nsoHeader.cmpSize_data = LZ4_compress_default(dcmpDataData, dataData, nsoHeader.dcmpSize_data, LZ4_compressBound(nsoHeader.dcmpSize_data));
	fwrite(dataData, 1, nsoHeader.cmpSize_data, out);
	rewind(out);
	fwrite(&nsoHeader, sizeof(NSOHeader), 1, out);
	fwrite(&sha256_text, 1, 0x20, out);
	fwrite(&sha256_rodata, 1, 0x20, out);
	fwrite(&sha256_data, 1, 0x20, out);
	delete[]textData;
	delete[]dcmpTextData;
	delete[]rodataData;
	delete[]dcmpRodataData;
	delete[]dataData;
	delete[]dcmpDataData;
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("nsnsotool\n");
		printf("Compress or decompress NSO/NRO files for Nintendo Switch\n\n");
		printf("Usage:\n");
		printf("nsnsotool <file_name> [out_name]\n");
		return 0;
	}
	FILE *in = fopen(argv[1], "rb");
	if(in == NULL) return -1;
	fseek(in, 0xC, 0);
	u32 flags = 0;
	fread(&flags, 4, 1, in);
	rewind(in);
	FILE *out;
	if(argc == 2)
		out = fopen("temp.bin", "wb");
	else
	{
		out = fopen(argv[2], "wb");
		if (out == NULL) return -1;
	}
	s32 result = -1;
	switch (flags)
	{
	case 0:
		printf("compressing...\n");
		result = compress(in, out);
		break;
	case 0x3F:
		printf("decompressing...\n");
		result = decompress(in, out);
		break;
	default:
		printf("unsupported flags 0x%08X!\n", flags);
	}
	fclose(in);
	fclose(out);
	if(argc == 2 && result == 0)
		fcopy((char *)"temp.bin", argv[1]);
	remove("temp.bin");
	return 0;
}

void sha256_nso(char *data, long length, u32 *sha256)
{
	char *pp, *ppend;
	long l, i, W[64], T1, T2, A, B, C, D, E, F, G, H;
	sha256[0] = 0x6a09e667, sha256[1] = 0xbb67ae85, sha256[2] = 0x3c6ef372, sha256[3] = 0xa54ff53a;
	sha256[4] = 0x510e527f, sha256[5] = 0x9b05688c, sha256[6] = 0x1f83d9ab, sha256[7] = 0x5be0cd19;
	u32 K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
	};
	l = length + ((length % 64 > 56) ? (128 - length % 64) : (64 - length % 64));
	pp = (char*)malloc((unsigned long)l);
	for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = data[i], i++);
	for (pp[i + 3 - 2 * (i % 4)] = (char)128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);
	*((long*)(pp + l - 4)) = length << 3;
	*((long*)(pp + l - 8)) = length >> 29;
	for (ppend = pp + l; pp < ppend; pp += 64) {
		for (i = 0; i < 16; W[i] = ((long*)pp)[i], i++);
		for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);
		A = sha256[0], B = sha256[1], C = sha256[2], D = sha256[3], E = sha256[4], F = sha256[5], G = sha256[6], H = sha256[7];
		for (i = 0; i < 64; i++) {
			T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];
			T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);
			H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;
		}
		sha256[0] += A, sha256[1] += B, sha256[2] += C, sha256[3] += D, sha256[4] += E, sha256[5] += F, sha256[6] += G, sha256[7] += H;
	}
	free(pp - l);
	for (i = 0; i < 8; i++) sha256[i] = (sha256[i] & 0xFF000000) >> 24 | (sha256[i] & 0xFF0000) >> 8 | (sha256[i] & 0xFF00) << 8 | (sha256[i] & 0xFF) << 24;
}

int fcopy(char *src_name, char *dest_name)
{
	FILE *src=fopen(src_name, "rb");
	if(src == NULL) return -1;
	FILE *dest=fopen(dest_name, "wb");
	fseek(src, 0, 2);
	unsigned int data_size=ftell(src);
	rewind(src);
	unsigned int block_size = 512;
	while(data_size>0)
	{
		char data[512];
		block_size = 512;
		if(data_size<block_size)
			block_size = data_size;
		data_size -= block_size;
		fread(data, 1, block_size, src);
		fwrite(data, 1, block_size, dest);
	}
	fclose(src);
	fclose(dest);
	return 0;
}
