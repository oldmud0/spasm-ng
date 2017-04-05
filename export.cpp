/* wabbit.c -- Wabbitsign, the Free APP signer for TI-83 Plus
	by Spencer Putt and James Montelongo */

/* Modified for use in SPASM by Don Straney */

#include "stdafx.h"

#include "spasm.h"
#include "utils.h"
#include "errors.h"

#define name    (header8xk + 17)
#define hleng   sizeof(header8xk)
unsigned char header8xk[] = {
	'*','*','T','I','F','L','*','*',    /* required identifier */
	1, 1,                               /* version */
	1, 0x88,                            /* unsure, but always set like this */
	0x01, 0x01, 0x19, 0x97,             /* Always sets date to jan. 1, 1997 */
	8,                                  /* Length of name. */
	0,0,0,0,0,0,0,0};                   /* space for the name */

const unsigned char nbuf[]= {
	0xAD,0x24,0x31,0xDA,0x22,0x97,0xE4,0x17,
	0x5E,0xAC,0x61,0xA3,0x15,0x4F,0xA3,0xD8,
	0x47,0x11,0x57,0x94,0xDD,0x33,0x0A,0xB7,
	0xFF,0x36,0xBA,0x59,0xFE,0xDA,0x19,0x5F,
	0xEA,0x7C,0x16,0x74,0x3B,0xD7,0xBC,0xED,
	0x8A,0x0D,0xA8,0x85,0xE5,0xE5,0xC3,0x4D,
	0x5B,0xF2,0x0D,0x0A,0xB3,0xEF,0x91,0x81,
	0xED,0x39,0xBA,0x2C,0x4D,0x89,0x8E,0x87};
const unsigned char pbuf[]= {
	0x5B,0x2E,0x54,0xE9,0xB5,0xC1,0xFE,0x26,
	0xCE,0x93,0x26,0x14,0x78,0xD3,0x87,0x3F,
	0x3F,0xC4,0x1B,0xFF,0xF1,0xF5,0xF9,0x34,
	0xD7,0xA5,0x79,0x3A,0x43,0xC1,0xC2,0x1C};
const unsigned char qbuf[]= {
	0x97,0xF7,0x70,0x7B,0x94,0x07,0x9B,0x73,
	0x85,0x87,0x20,0xBF,0x6D,0x49,0x09,0xAB,
	0x3B,0xED,0xA1,0xBA,0x9B,0x93,0x11,0x2B,
	0x04,0x13,0x40,0xA1,0x6E,0xD5,0x97,0xB6,0x04};

// q ^ (p - 2))
const unsigned char qpowpbuf[] = {
	0xA3,0x82,0x96,0xAF,0x3D,0xDD,0x9B,0x94,
	0xAE,0xA0,0x2F,0x2C,0xE3,0x8B,0xCD,0xD9,
	0xC9,0x11,0x75,0x4F,0x00,0xE4,0xDF,0x47,
	0x38,0xCD,0x98,0x16,0x47,0xF5,0x2B,0x0F};
// (p + 1) / 4
const unsigned char p14buf[] = {
	0x97,0x0B,0x55,0x7A,0x6D,0xB0,0xBF,0x89,
	0xF3,0xA4,0x09,0x05,0xDE,0xF4,0xE1,0xCF,
	0x0F,0xF1,0xC6,0x7F,0x7C,0x7D,0x3E,0xCD,
	0x75,0x69,0x9E,0xCE,0x50,0xB0,0x30,0x07};
// (q + 1) / 4
const unsigned char q14buf[] = {
	0xE6,0x3D,0xDC,0x1E,0xE5,0xC1,0xE6,0x5C,
	0xE1,0x21,0xC8,0x6F,0x5B,0x52,0xC2,0xEA,
	0x4E,0x7B,0xA8,0xEE,0xE6,0x64,0xC4,0x0A,
	0xC1,0x04,0x50,0xA8,0x5B,0xF5,0xA5,0x2D,0x01};

const unsigned char fileheader[]= {
	'*','*','T','I','*',0x1A,0x0A,0x00};
const char comment[42]= "File generated by WabbitSign";

const char typearray[] = {
	'7','3','*',0x0B,
	'8','2','*',0x0B,
	'8','3','*',0x0B,
	'8','3','F',0x0D,
	'8','3','F',0x0D,
	'8','5','*',0x00,
	'8','6','*',0x0C,
	'8','5','*',0x00,
	'8','6','*',0x0C,
	};

const char extensions[][4] = {
	"73P","82P","83P","8XP","8XV","85P","86P","85S","86S","8XK","ROM","HEX","BIN"};

enum calc_type {
	TYPE_73P = 0,
	TYPE_82P,
	TYPE_83P,
	TYPE_8XP,
	TYPE_8XV,
	TYPE_85P,
	TYPE_86P,
	TYPE_85S,
	TYPE_86S,
	TYPE_8XK,
	TYPE_ROM,
	TYPE_HEX,
	TYPE_BIN
};

int findfield ( unsigned char byte, const unsigned char* buffer );
int siggen (const unsigned char* hashbuf, unsigned char* sigbuf, int* outf);
void intelhex (FILE * outfile , const unsigned char* buffer, int size, unsigned int address = 0x4000);
void alphanumeric (char* namestring, bool allow_lower);
void makerom (const unsigned char *output_contents, DWORD output_len, FILE *outfile);
void makehex (const unsigned char *output_contents, DWORD output_len, FILE *outfile);
void makeapp (const unsigned char *output_contents, DWORD output_len, FILE *outfile, const char *prgmname);
void makeprgm (const unsigned char *output_contents, int size, FILE *outfile, const char *prgmname, calc_type calc);

void write_file (const unsigned char *output_contents, int output_len, const char *output_filename) {
	FILE *outfile;
	int i;
	calc_type calc;

	free(curr_input_file);

	curr_input_file = strdup(_T("exporter"));
	line_num = -1;


	//get the type from the extension of the output filename
	for (i = strlen (output_filename); output_filename[i] != '.' && i; i--);
	if (i != 0) {
		const char *ext = output_filename + i + 1;

		int type;
		for (type = 0; type < ARRAYSIZE(extensions); type++) {
			if (!_stricmp (ext, extensions[type]))
				break;
		}

		if (type == ARRAYSIZE(extensions)) {
			SetLastSPASMWarning(SPASM_WARN_UNKNOWN_EXTENSION);
			type = ARRAYSIZE(extensions) - 1;
		}

		calc = (calc_type)type;

	} else {
		//show_warning ("No output extension given, assuming .bin");
		calc = TYPE_BIN;
	}

	//open the output file
	outfile = fopen (output_filename, "wb");
	if (!outfile) {
		int error = errno;
		if (errno == ENOENT)
			SetLastSPASMError(SPASM_ERR_FILE_NOT_FOUND, output_filename);
		else
			SetLastSPASMError(SPASM_ERR_NO_ACCESS, output_filename);
		free(curr_input_file);
		curr_input_file = NULL;
		return;
	}

	for (i = strlen (output_filename); output_filename[i] != '\\' && output_filename[i] != '/' && i; i--);
	if (i != 0)
		i++;
	
	char prgmname[MAX_PATH];
	strcpy(prgmname, output_filename);
	
	for (i = strlen (prgmname); prgmname[i] != '.' && i; i--);
	if (i != 0)
		prgmname[i] = '\0';

	//then decide how to write the contents
	switch (calc)
	{
	case TYPE_73P:
	case TYPE_82P:
	case TYPE_83P:
	case TYPE_8XP:
	case TYPE_8XV:
	case TYPE_85P:
	case TYPE_85S:
	case TYPE_86P:
	case TYPE_86S:
		makeprgm (output_contents, (DWORD) output_len, outfile, prgmname, calc);
		break;
	case TYPE_8XK:
		makeapp (output_contents, (DWORD) output_len, outfile, prgmname);
		break;
	case TYPE_ROM:
		makerom(output_contents, (DWORD) output_len, outfile);
		break;
	case TYPE_HEX:
		makehex(output_contents, (DWORD) output_len, outfile);
		break;
	//bin
	default:
		for (i = 0; i < output_len; i++)
			fputc(output_contents[i], outfile);
		break;
	}
		

	fclose (outfile);
	free(curr_input_file);
	curr_input_file = NULL;
}

void makerom (const unsigned char *output_contents, DWORD size, FILE *outfile) {
	unsigned int i;
	const int final_size = 512*1024;
	for(i = 0; i < size; i++)
		fputc(output_contents[i], outfile);
	for (; i < final_size; i++)
		fputc(0xFF, outfile);
}

void makehex (const unsigned char *output_contents, DWORD size, FILE *outfile) {
	intelhex(outfile, output_contents, size);
}

void makeapp (const unsigned char *output_contents, DWORD size, FILE *outfile, const char* prgmname) {
	unsigned char *buffer;
	int i,pnt,siglength,tempnum,f,pages;
	unsigned int total_size;

	/* Copy file to memory */
	buffer = (unsigned char *) calloc(1, size+256);
	memcpy (buffer, output_contents, sizeof (char) * size);

/* Check if size will fit in mem with signature */
	if ((tempnum = ((size+96)%16384))) {
		if (tempnum < 97 && size > 16384)
		{
			free(buffer);
			SetLastSPASMError(SPASM_ERR_SIGNER_ROOM_FOR_SIG);
			return;
		}
		if (tempnum<1024 && (size+96)>>14)
		{
			SetLastSPASMWarning(SPASM_WARN_SMALL_LAST_PAGE, tempnum);
		}
	}

/* Fix app header fields */
/* Length Field: set to size of app - 6 */
	if (!(buffer[0] == 0x80 && buffer[1] == 0x0F)) {
		free(buffer);
		SetLastSPASMError(SPASM_ERR_SIGNER_MISSING_LENGTH);
		return;
	}
	size -= 6;
	buffer[2] = size >> 24;         //Stored in Big Endian
	buffer[3] = (size>>16) & 0xFF;
	buffer[4] = (size>> 8) & 0xFF;
	buffer[5] = size & 0xFF;
	size += 6;
/* Program Type Field: Must be present and shareware (0104) */
	pnt = findfield(0x12, buffer);
	if (!pnt || ( buffer[pnt++]!=1) || (buffer[pnt]!=4) ) {
		free(buffer);
		SetLastSPASMError(SPASM_ERR_SIGNER_PRGM_TYPE);
		return;
	}
/* Pages Field: Corrects page num*/
	pnt = findfield(0x81, buffer);
	if (!pnt) {
		free(buffer);
		SetLastSPASMError(SPASM_ERR_SIGNER_MISSING_PAGES);
		return;
	}
	
	pages = size>>14; /* this is safe because we know there's enough room for the sig */
	if (size & 0x3FFF) pages++;
	buffer[pnt] = pages;
/* Name Field: MUST BE 8 CHARACTERS, no checking if valid */
	pnt = findfield(0x48, buffer);
	if (!pnt) {
		free(buffer);
		SetLastSPASMError(SPASM_ERR_SIGNER_MISSING_NAME);
		return;
	}
	for (i=0; i < 8 ;i++) name[i]=buffer[i+pnt];

#ifndef NO_APPSIGN
/* Calculate MD5 */
#ifdef WIN32
	unsigned char hashbuf[64];
	HCRYPTPROV hCryptProv; 
	HCRYPTHASH hCryptHash;
	DWORD sizebuf = ARRAYSIZE(hashbuf);
	CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET);
	CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hCryptHash);
	CryptHashData(hCryptHash, buffer, size, 0);
	CryptGetHashParam(hCryptHash, HP_HASHVAL, hashbuf, &sizebuf, 0);
#else
	unsigned char hashbuf[16];
	MD5 (buffer, size, hashbuf);  //This uses ssl but any good md5 should work fine.
#endif

/* Generate the signature to the buffer */
	siglength = siggen(hashbuf, buffer+size+3, &f );

/* append sig */
	buffer[size + 0] = 0x02;
	buffer[size + 1] = 0x2d;
	buffer[size + 2] = (unsigned char) siglength;
	total_size = size + siglength + 3;
	if (f) {
		buffer[total_size++] = 1;
		buffer[total_size++] = f;
	} else buffer[total_size++] = 0;
/* sig must be 96 bytes ( don't ask me why) */
	tempnum = 96 - (total_size - size);
	while (tempnum--) buffer[total_size++] = 0xFF;
#else /* NO_APPSIGN */
    show_warning("App signing is not available in this build of SPASM");
#endif /* NO_APPSIGN */


/* Do 8xk header */
	for (i = 0; i < hleng; i++) fputc(header8xk[i], outfile);
	for (i = 0; i < 23; i++)    fputc(0, outfile);
	fputc(0x73, outfile);
	fputc(0x24, outfile);
	for (i = 0; i < 24; i++)    fputc(0, outfile);
	tempnum =  77 * (total_size>>5) + pages * 17 + 11;
	size = total_size & 0x1F;
	if (size) tempnum += (size<<1) + 13;
	fputc( tempnum & 0xFF, outfile); //little endian
	fputc((tempnum >> 8) & 0xFF, outfile);
	fputc((tempnum >> 16)& 0xFF, outfile);
	fputc( tempnum >> 24, outfile);
	
/* Convert to 8xk */
	intelhex(outfile, buffer, total_size);

#ifdef WIN32
#ifndef NO_APPSIGN
	if (hCryptHash) {
		CryptDestroyHash(hCryptHash);
		hCryptHash = NULL;
	}
	if (hCryptProv) {
		CryptReleaseContext(hCryptProv,0);
		hCryptProv = NULL;
	}
#endif
#endif

	free(buffer);
//    if (pages==1) printf("%s (%d page",filename,pages);
//    else printf("%s (%d pages",filename,pages);
//	puts(") was successfully generated!");
}


/* Starting from 0006 searches for a field
 * in the in file buffer. */
int findfield( unsigned char byte, const unsigned char* buffer ) {
	int pnt=6;
	while (buffer[pnt++] == 0x80) {
		if (buffer[pnt] == byte) {
			pnt++;
			return pnt;
		} else
			pnt += (buffer[pnt]&0x0F);
		pnt++;
	}
	return 0;
}

#ifndef NO_APPSIGN
int siggen(const unsigned char* hashbuf, unsigned char* sigbuf, int* outf) {
	mpz_t mhash, p, q, r, s, temp, result;
	
	unsigned int lp,lq;
	int siglength;
	
/* Intiate vars */
	mpz_init(mhash);
	mpz_init(p);
	mpz_init(q);
	mpz_init(r);
	mpz_init(s);
	mpz_init(temp);
	mpz_init(result);
	
/* Import vars */
	mpz_import(mhash, 16, -1, 1, -1, 0, hashbuf);
	mpz_import(p, sizeof(pbuf), -1, 1, -1, 0, pbuf);
	mpz_import(q, sizeof(qbuf), -1, 1, -1, 0, qbuf);
/*---------Find F----------*/
/*      M' = m*256+1      */
	mpz_mul_ui(mhash, mhash, 256);
	mpz_add_ui(mhash, mhash, 1);
	
/* calc f {2, 3,  0, 1 }  */
	lp = mpz_legendre(mhash, p) == 1 ? 0 : 1;
	lq = mpz_legendre(mhash, q) == 1 ? 1 : 0;
	*outf = lp+lq+lq;

/*apply f */
	if (lp == lq)
		mpz_mul_ui(mhash, mhash, 2);
	if (lq == 0) {
		mpz_import(temp, sizeof(nbuf), -1, 1, -1, 0, nbuf);
		mpz_sub(mhash, temp, mhash);
	}

/* r = ( M' ^ ( ( p + 1) / 4 ) ) mod p */
	mpz_import(result, sizeof(p14buf), -1, 1, -1, 0, p14buf);
	mpz_powm(r, mhash, result, p);
	
/* s = ( M' ^ ( ( q + 1) / 4 ) ) mod q */
	mpz_import(result, sizeof(q14buf), -1, 1, -1, 0, q14buf);
	mpz_powm(s, mhash, result, q);
	
/* r-s */
	mpz_set_ui(temp, 0);
	mpz_sub(temp, r, s);
	
/* q ^ (p - 2)) */
	mpz_import(result, sizeof(qpowpbuf), -1, 1, -1, 0, qpowpbuf);
	
/* (r-s) * q^(p-2) mod p */
	mpz_mul(temp, temp, result);
	mpz_mod(temp, temp, p);
	
/* ((r-s) * q^(p-2) mod p) * q + s */
	mpz_mul(result, temp, q);
	mpz_add(result, result, s);
	
/* export sig */
	siglength = mpz_sizeinbase(result, 16);
	siglength = (siglength + 1) / 2;
	mpz_export(sigbuf, NULL, -1, 1, -1, 0, result);
	
/* Clean Up */
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(r);
	mpz_clear(s);
	mpz_clear(temp);
	mpz_clear(result);
	return siglength;
}
#endif /* NO_APPSIGN */

void makeprgm (const unsigned char *output_contents, int size, FILE *outfile, const char *prgmname, calc_type calc) {
	int i, temp, chksum;
	unsigned char* pnt;
	char *namestring;
	
	if (calc==TYPE_82P) {
		char name_buf[256];
		snprintf(name_buf, sizeof(name_buf), "\xdc%s", prgmname);
		namestring = strdup (name_buf);
	} else {
		// Find basename(prgmname)
		const char *baseName = prgmname + strlen(prgmname) - 1;
		bool found = false;
		while (baseName > prgmname) {
			if (*baseName == '/' || *baseName == '\\') {
				found = true;
				break;
			}
			baseName -= 1;
		}
		if (found) {
			baseName += 1;
		}

		// Use the base filename (no directory, if given) as the program name.
		namestring = strdup(baseName);
		/* The name must be capital letters and numbers */
		if (calc < TYPE_85P) {
			alphanumeric (namestring, calc == TYPE_8XV);
		}
	}
	/* get size */
	size += 2;
	/* 86ers don't need to put the asm token*/
	if (calc==TYPE_86P) {
		size += 2;
	}
	/* size must be smaller than z80 mem */
	if (size > 24000) {
		if (size > 65000) {
			SetLastSPASMWarning(SPASM_WARN_SIGNER_FILE_SIZE_64KB);
		} else {
			SetLastSPASMWarning(SPASM_WARN_SIGNER_FILE_SIZE_24KB);
		}
	}
	
	/* Lots of pointless header crap */
	for (i = 0; i < 4; i++)
		fputc (fileheader[i], outfile);
	pnt = ((unsigned char*)typearray+((int)calc<<2));
	fputc (pnt[0],outfile);
	fputc (pnt[1],outfile);
	fputc (pnt[2],outfile);
	fputc (fileheader[i++],outfile);
	fputc (fileheader[i++],outfile);
	if (calc == TYPE_85P) {
		fputc (0x0C,outfile);
	} else {
		fputc (fileheader[i++],outfile);
	}    
	fputc (fileheader[i++],outfile);
	
	// Copy in the comment
	for (i = 0; i < 42; i++)
		fputc(comment[i],outfile);

	if (calc == TYPE_82P) size+=3;
	/* For some reason TI thinks it's important to put the file size */
	/* dozens of times, I suppose duplicates add security...*/
	/* yeah right. */
	if (calc == TYPE_85P) {
		temp = size + 8 + strlen (namestring);
	} else {
		temp = size+15+((calc==TYPE_8XP||calc==TYPE_8XV)?2:0)+((calc==TYPE_86P)?1:0);
	}
	fputc(temp & 0xFF,outfile);
	fputc(temp >> 8,outfile);
	if (calc == TYPE_85P) {
		chksum = fputc((6+strlen(namestring)),outfile);
	} else {
		chksum = fputc(pnt[3],outfile);
	}
	fputc(0,outfile);
	/* OMG the Size again! */
	chksum += fputc(size & 0xFF,outfile);
	chksum += fputc(size>>8,outfile);
	if (calc >= TYPE_85P) {
		if (calc >= TYPE_85S) {
			chksum += fputc(0x0C,outfile);
		} else {
			chksum += fputc(0x12,outfile);
		}
		chksum += fputc(strlen(namestring),outfile);
	} else {
		if (calc != TYPE_8XV)
			chksum += fputc(6,outfile);
		else
			chksum += fputc(0x15,outfile);
	}
	
	/* The actual name is placed with padded with zeros */
	if (calc < TYPE_85P && calc != TYPE_82P) {
		if (!((temp=namestring[0])>='A' && temp<='Z')) show_warning ("First character in name must be a letter.");
	}
	for(i = 0; i < 8 && namestring[i]; i++) chksum += fputc(namestring[i], outfile);
	if (calc != TYPE_85P && calc != TYPE_85S) {
		for(;i < 8; i++) fputc(0,outfile);
	}
	/* 83+ requires 2 extra bytes */
	if (calc == TYPE_8XP || calc == TYPE_8XV) {
		fputc(0,outfile);
		fputc(0,outfile);
	}
	/*Yeah, lets put the size twice in a row  X( */
	chksum += fputc(size & 0xFF,outfile);
	chksum += fputc(size>>8,outfile);
	size-=2;
	chksum += fputc(size & 0xFF,outfile);
	chksum += fputc(size>>8,outfile);
	
	/* check for BB 6D on 83+ */
	if (calc == TYPE_8XP) {
		unsigned short header = ((unsigned char)output_contents[0])*256 + ((unsigned char)output_contents[1]);
		if (!(mode & MODE_EZ80) && header != 0xBB6D && header != 0xEF69) {
			show_warning("83+/84+ program does not begin with bytes BB 6D or EF 69.");
		}
		if ((mode & MODE_EZ80) && header != 0xEF7B) {
			show_warning("84+CE program does not begin with bytes EF 7B.");
		}
	}
	if (calc == TYPE_86P) {
	   chksum += fputc (0x8E, outfile);
	   chksum += fputc (0x28, outfile);
	   size -= 2;
	} else if (calc == TYPE_82P) {
		chksum += 
			fputc (0xD5, outfile);
		chksum += 
			fputc (0x00, outfile);
		chksum += 
			fputc (0x11, outfile);
	}
	
	if (calc == TYPE_82P) size -= 3;
	/* Actual program data! */
	for (i = 0; i < size; i++) {
		chksum += fputc (output_contents[i], outfile);
	}
	/* short little endian Checksum */
	fputc (chksum & 0xFF,outfile);
	fputc ((chksum >> 8) & 0xFF,outfile);
//    printf("%s (%d bytes) was successfully generated!\n",filestring,size);

	free (namestring);
}


void alphanumeric (char* namestring, bool allow_lower) {
	char temp;
	bool force_upper = true;

	while ((temp = *namestring)) {
		if (force_upper && temp>='a' && temp<='z') *namestring = temp =(temp-('a'-'A'));
		if (!(((temp >= 'A') && (temp <= 'Z')) || ((temp >= 'a') && (temp <= 'z')) || (temp >= '0' && temp <= '9') || (temp == 0))) {
			show_warning ("Invalid characters in name. Alphanumeric Only.");
		}
		namestring++;
		if (allow_lower)
			force_upper = false;
	}
}


/* Convert binary buffer to Intel hex in TI format
 * All pages addressed to $4000 and are only $4000
 * bytes long. */
void intelhex (FILE* outfile, const unsigned char* buffer, int size, unsigned int base_address) {
	const char hexstr[] = "0123456789ABCDEF";
	int page = 0;
	int bpnt = 0;
	unsigned int ci, temp, i, address;
	unsigned char chksum;
	unsigned char outbuf[128];
	
	//We are in binary mode, we must handle carriage return ourselves.
   
	while (bpnt < size){
		fprintf(outfile,":02000002%04X%02X\r\n",page,(unsigned char) ( (~(0x04 + page)) +1));
		page++;
		address = base_address;
		for (i = 0; bpnt < size && i < 512; i++) {
			 chksum = (address>>8) + (address & 0xFF);
			 for(ci = 0; ((ci < 64) && (bpnt < size)); ci++) {
				temp = buffer[bpnt++];
				outbuf[ci++] = hexstr[temp>>4];
				outbuf[ci] = hexstr[temp&0x0F];
				chksum += temp;
			}
			outbuf[ci] = 0;
			ci>>=1;
			fprintf(outfile,":%02X%04X00%s%02X\r\n",ci,address,outbuf,(unsigned char)( ~(chksum + ci)+1));
			address +=0x20;
		}         
	}
	fprintf(outfile,":00000001FF");
}
