/**
 * @file sm2pspp.c
 * @author Daniel Starke
 * @date 2021-01-30
 * @version 2021-02-12
 * 
 * DISCLAIMER
 * This file has no copyright assigned and is placed in the Public Domain.
 * All contributions are also assumed to be in the Public Domain.
 * Other contributions are not permitted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "sm2pspp.h"
#include "mingw-unicode.h"


FILE * fin = NULL;
FILE * fout = NULL;
FILE * ferr = NULL;


const TCHAR * fmsg[MSG_COUNT] = {
	/* MSGT_SUCCESS                    */ _T(""), /* never used for output */
	/* MSGT_ERR_NO_MEM                 */ _T("Error: Failed to allocate memory.\n"),
	/* MSGT_ERR_FILE_NOT_FOUND         */ _T("Error: Input file not found.\n"),
	/* MSGT_ERR_FILE_OPEN              */ _T("Error: Failed to open file for reading.\n"),
	/* MSGT_ERR_FILE_READ              */ _T("Error: Failed to read data from file.\n"),
	/* MSGT_ERR_FILE_CREATE            */ _T("Error: Failed to create file for writing.\n"),
	/* MSGT_ERR_FILE_WRITE             */ _T("Error: Failed to write data to file.\n"),
	/* MSGT_WARN_NO_FILAMENT_USED      */ _T("Warning: Filament used value not found.\n"),
	/* MSGT_WARN_NO_LAYER_HEIGHT       */ _T("Warning: Layer height value not found.\n"),
	/* MSGT_WARN_NO_EST_TIME           */ _T("Warning: Estimated time value not found.\n"),
	/* MSGT_WARN_NO_NOZZLE_TEMP        */ _T("Warning: Nozzle temperature value not found.\n"),
	/* MSGT_WARN_NO_PLATE_TEMP         */ _T("Warning: Building plate temperature value not found.\n"),
	/* MSGT_WARN_NO_PRINT_SPEED        */ _T("Warning: Print speed value not found.\n"),
	/* MSGT_WARN_NO_THUMBNAIL          */ _T("Warning: Thumbnail data not found.\n")
};


/**
 * Main entry point.
 */
int _tmain(int argc, TCHAR ** argv) {
	/* set the output file descriptors */
	fin  = stdin;
	fout = stdout;
	ferr = stderr;
	
#ifdef UNICODE
	/* http://msdn.microsoft.com/en-us/library/z0kc8e3z(v=vs.80).aspx */
	if (_isatty(_fileno(fout))) {
		_setmode(_fileno(fout), _O_U16TEXT);
	} else {
		_setmode(_fileno(fout), _O_U8TEXT);
	}
	if (_isatty(_fileno(ferr))) {
		_setmode(_fileno(ferr), _O_U16TEXT);
	} else {
		_setmode(_fileno(ferr), _O_U8TEXT);
	}
#endif /* UNICODE */

	if (argc < 2) {
		printHelp();
		return EXIT_FAILURE;
	}
	
	return (processFile(argv[1], &errorCallback) == 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}


/**
 * Write the help for this application to standard out.
 */
void printHelp(void) {
	_ftprintf(ferr,
	_T("sm2pspp <g-code file>\n")
	_T("\n")
	_T("sm2pspp ") _T2(PROGRAM_VERSION_STR) _T("\n")
	_T("https://github.com/daniel-starke/sm2pspp\n")
	);
}


/**
 * Helper function to compare the start of a token against a given string.
 * 
 * @param[in] lhs - token to compare with
 * @param[in] rhs - string to compare with
 * @return same as strcmp
 */
static int p_cmpTokenStart(const tPToken * lhs, const char * rhs) {
	const size_t rhsLen = strlen(rhs);
	tPToken aToken;
	aToken.start = lhs->start;
	aToken.length = (lhs->length > rhsLen) ? rhsLen : lhs->length;
	return p_cmpToken(&aToken, rhs);
}


/**
 * Parses the given dhms time token and returns the value in seconds.
 * 
 * @param[in] aToken - input token
 * @return time in seconds
 */
static size_t p_dtms(const tPToken * aToken) {
	if (aToken->start == NULL || aToken->length <= 0) return 0;
	size_t res = 0;
	size_t val = 0;
	for (size_t i = 0; i < aToken->length; i++) {
		const char ch = aToken->start[i];
		switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			val = (val * 10) + ((size_t)(ch - '0'));
			break;
		case 'd':
			res = res + (val * 86400);
			val = 0;
			break;
		case 'h':
			res = res + (val * 3600);
			val = 0;
			break;
		case 'm':
			res = res + (val * 60);
			val = 0;
			break;
		case 's':
			res = res + val;
			val = 0;
			break;
		default:
			break;
		}
	}
	return res;
}


/**
 * Converts the given token into a float value. Simple float values are assumed.
 * 
 * @param[in] aToken - token to convert
 * @return float value from the token
 */
static float p_float(const tPToken * aToken) {
	if (aToken->start == NULL || aToken->length <= 0) return 0.0f;
	size_t val = 0;
	size_t frac = 0;
	float fracDiv = 1.0f;
	int isFrac = 0;
	for (size_t i = 0; i < aToken->length; i++) {
		const char ch = aToken->start[i];
		switch (ch) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			if (isFrac == 0) {
				val = (val * 10) + ((size_t)(ch - '0'));
			} else {
				frac = (frac * 10) + ((size_t)(ch - '0'));
				fracDiv *= 10.0f;
			}
			break;
		case '.':
			isFrac = 1;
			break;
		default:
			break;
		}
	}
	return (float)val + ((float)frac / fracDiv);
}


/**
 * Processes the given PrusaSlicer generated G-Code file and converts
 * it into a Snapmaker 2.0 terminal compatible G-Code file.
 * 
 * @param[in] file - PrusaSlicer generated G-Code file
 * @param[in] cb - error output callback function
 * @return 1 on success, 0 on failure, -1 if aborted by callback function
 */
int processFile(const TCHAR * file, const tCallback cb) {
#define ON_WARN(msg) do { \
	if (cb(msg, file, lineNr) != 1) goto onError; \
} while (0) \

#define ON_ERROR(msg) do { \
	cb(msg, file, lineNr); \
	goto onError; \
} while (0)

	if (file == NULL || cb == NULL) return 0;
	int res = 0;
	size_t lineNr = 1;
	char * inputBuf = NULL;
	size_t inputLen = 0;
	FILE * fp = NULL;
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
	size_t origThumbnailLines = 0;
	tPToken origThumbnail = {0};
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
	tPToken filamentUsed = {0};
	tPToken layerHeight = {0};
	tPToken estTime = {0};
	tPToken nozzleTemp = {0};
	tPToken plateTemp = {0};
	tPToken printSpeed = {0};
	tPToken thumbnail = {0};
	tPToken aToken = {0};
	tPToken * valueToken = NULL;
	const char * lineStart = NULL;
	enum tState {
		ST_LINE_START,
		ST_FIND_LINE_START,
		ST_COMMENT,
		ST_PARAMETER_VALUE,
		ST_THUMBNAIL
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
		, ST_THUMBNAIL_TAIL
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
	} state = ST_LINE_START;
#ifdef DEBUG
	static const TCHAR * stateStr[] = {
		_T("ST_LINE_START"),
		_T("ST_FIND_LINE_START"),
		_T("ST_COMMENT"),
		_T("ST_PARAMETER_VALUE"),
		_T("ST_THUMBNAIL")
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
		, _T("ST_THUMBNAIL_TAIL")
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
	};
#endif /* DEBUG */
	
	/* open input file for reading */
	fp = _tfopen(file, _T("rb"));
	if (fp == NULL) ON_ERROR(MSGT_ERR_FILE_OPEN);
	
	/* get file size */
	fseeko64(fp, 0, SEEK_END);
	inputLen = (size_t)ftello64(fp);
	if (inputLen < 1) goto onSuccess;
	fseek(fp, 0, SEEK_SET);
	
	/* allocate input buffer from file data */
	inputBuf = (char *)malloc(inputLen);
	if (inputBuf == NULL) ON_ERROR(MSGT_ERR_NO_MEM);
	if (fread(inputBuf, inputLen, 1, fp) < 1) ON_ERROR(MSGT_ERR_FILE_READ);
	
	/* close input file */
	fclose(fp);
	fp = NULL;
	
	/* parse tokens */
	lineStart = inputBuf;
	for (const char * it = inputBuf, * endIt = inputBuf + inputLen; it < endIt; it++) {
		const char ch = *it;
#ifdef DEBUG
		_ftprintf(ferr, _T("%u:%s: '%c'"), (unsigned)lineNr, stateStr[(int)state], ch);
		if (aToken.start != NULL) {
#ifdef UNICODE
			_ftprintf(ferr, _T(", token: \"%.*S\""), (unsigned)aToken.length, aToken.start);
#else /* not UNICODE */
			_ftprintf(ferr, _T(", token: \"%.*s\""), (unsigned)aToken.length, aToken.start);
#endif /* not UNICODE */
		}
		if (valueToken != NULL && valueToken->start != NULL) {
#ifdef UNICODE
			_ftprintf(ferr, _T(", value: \"%.*S\""), (unsigned)valueToken->length, valueToken->start);
#else /* not UNICODE */
			_ftprintf(ferr, _T(", value: \"%.*s\""), (unsigned)valueToken->length, valueToken->start);
#endif /* not UNICODE */
		}
		_ftprintf(ferr, _T("\n"));
#endif /* DEBUG */
		switch (state) {
		case ST_LINE_START:
			 if (ch == ';') {
				/* comment */
				memset(&aToken, 0, sizeof(aToken));
				state = ST_COMMENT;
			} else if (isspace(ch) == 0) {
				/* code */
				state = ST_FIND_LINE_START;
			}
			/* spaces */
			break;
		case ST_FIND_LINE_START:
			if (ch == '\n') {
				/* new line */
				state = ST_LINE_START;
			}
			break;
		case ST_COMMENT:
			if (ch == '\n') {
				/* end of comment line */
				state = ST_LINE_START;
			} else if (aToken.start == NULL) {
				if (isspace(ch) == 0) {
					/* start of first word in comment */
					aToken.start = it;
					aToken.length = 1;
				}
			} else if (ch == ' ' && aToken.length > 0) {
				if (p_cmpToken(&aToken, "post-processed by sm2pspp") == 0) {
					/* already post-processed file */
					goto onSuccess;
				} else if (p_cmpToken(&aToken, "thumbnail begin") == 0) {
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
					if (origThumbnail.start == NULL) {
						origThumbnail.start = lineStart;
						origThumbnailLines = 1;
					}
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
					memset(&aToken, 0, sizeof(aToken));
					state = (thumbnail.start == NULL) ? ST_THUMBNAIL : ST_FIND_LINE_START;
				}
			} else if (ch == '=') {
				/* end of commented parameter key */
				if (aToken.length == 0) {
					aToken.length = (size_t)(it - aToken.start);
				}
				if (p_cmpToken(&aToken, "filament used [mm]") == 0) {
					valueToken = &filamentUsed;
				} else if (p_cmpToken(&aToken, "layer_height") == 0) {
					valueToken = &layerHeight;
				} else if (p_cmpTokenStart(&aToken, "estimated printing time") == 0) {
					valueToken = &estTime;
				} else if (p_cmpToken(&aToken, "first_layer_temperature") == 0) {
					valueToken = &nozzleTemp;
				} else if (p_cmpToken(&aToken, "first_layer_bed_temperature") == 0) {
					valueToken = &plateTemp;
				} else if (p_cmpToken(&aToken, "max_print_speed") == 0) {
					valueToken = &printSpeed;
				} else {
					state = ST_FIND_LINE_START;
				}
				if (valueToken != NULL) {
					memset(&aToken, 0, sizeof(aToken));
					if (valueToken->start == NULL) {
						state = ST_PARAMETER_VALUE;
					} else {
						/* ignore duplicate keys */
						valueToken = NULL;
						state = ST_FIND_LINE_START;
					}
				}
			} else if (isspace(ch) == 0) {
				/* ignore trailing spaces */
				aToken.length = (size_t)(it - aToken.start + 1);
			}
			break;
		case ST_PARAMETER_VALUE:
			if (ch == '\n') {
				/* end of comment line */
				valueToken = NULL;
				state = ST_LINE_START;
			} else if (valueToken->start == NULL) {
				if (isspace(ch) == 0) {
					/* start of comment parameter value */
					valueToken->start = it;
					valueToken->length = 1;
				}
			} else if (isspace(ch) == 0) {
				/* ignore trailing spaces */
				valueToken->length = (size_t)(it - valueToken->start + 1);
			}
			break;
		case ST_THUMBNAIL:
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
			if (ch == '\n') {
				/* count thumbnail lines to compensate cut */
				origThumbnailLines++;
			}
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
			if (thumbnail.start == NULL) {
				if (ch == '\n') {
					/* start of thumbnail data */
					thumbnail.start = it + 1;
				}
			} else if (ch == ';') {
				/* start of comment */
				aToken.start = it + 1;
				aToken.length = 0;
			} else if (aToken.start != NULL) {
				if (isspace(aToken.start[0]) != 0) {
					/* ignore leading spaces */
					aToken.start = it;
					aToken.length = 1;
				} else {
					aToken.length++;
					if (p_cmpToken(&aToken, "thumbnail end") == 0) {
						/* got complete Base64 encoded thumbnail image data (PNG) */
						thumbnail.length = (size_t)(lineStart - thumbnail.start);
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
						origThumbnail.length = (size_t)(it - origThumbnail.start);
						state = ST_THUMBNAIL_TAIL;
#else /* !FEATURE_REMOVE_ORIG_THUMBNAIL */
						state = ST_FIND_LINE_START;
#endif /* !FEATURE_REMOVE_ORIG_THUMBNAIL */
					}
				}
			}
			break;
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
		case ST_THUMBNAIL_TAIL:
			if (ch == '\n') {
				/* new line */
				origThumbnail.length = (size_t)(it + 1 - origThumbnail.start);
				state = ST_LINE_START;
			}
			break;
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
		}
		if (ch == '\n') {
			lineNr++;
			lineStart = it + 1;
		} else if (ch == '\r') {
			lineStart = it + 1;
		}
	}
	
	/* check missing tokens */
	if (filamentUsed.start == NULL || filamentUsed.length == 0) ON_WARN(MSGT_WARN_NO_FILAMENT_USED);
	if (layerHeight.start == NULL || layerHeight.length == 0) ON_WARN(MSGT_WARN_NO_LAYER_HEIGHT);
	if (estTime.start == NULL || estTime.length == 0) ON_WARN(MSGT_WARN_NO_EST_TIME);
	if (nozzleTemp.start == NULL || nozzleTemp.length == 0) ON_WARN(MSGT_WARN_NO_NOZZLE_TEMP);
	if (plateTemp.start == NULL || plateTemp.length == 0) ON_WARN(MSGT_WARN_NO_PLATE_TEMP);
	if (printSpeed.start == NULL || printSpeed.length == 0) ON_WARN(MSGT_WARN_NO_PRINT_SPEED);
	if (thumbnail.start == NULL || thumbnail.length == 0) ON_WARN(MSGT_WARN_NO_THUMBNAIL);
	
	/* re-create file */
	fp = _tfopen(file, _T("wb"));
	if (fp == NULL) ON_ERROR(MSGT_ERR_FILE_CREATE);
	
	/* output Snapmaker 2.0 specific start header */
	clearerr(fp);
	fprintf(fp, ";post-processed by sm2pspp (https://github.com/daniel-starke/sm2pspp)\n");
	fprintf(fp, ";Header Start\n\n");
	fprintf(fp, ";FLAVOR:Marlin\n");
	fprintf(fp, ";TIME:6666\n\n\n");
	fprintf(fp, ";Filament used: %.0fm\n", p_float(&filamentUsed) / 1000.0f);
	fprintf(fp, ";Layer height: %.2f\n", p_float(&layerHeight));
	fprintf(fp, ";header_type: 3dp\n");
	if (thumbnail.start != NULL || thumbnail.length > 0) {
		/* output thumbnail */
		fprintf(fp, ";thumbnail: data:image/png;base64,");
		memset(&aToken, 0, sizeof(aToken));
		for (size_t i = 0; i < thumbnail.length; i++) {
			const char ch = thumbnail.start[i];
			if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '+' || ch == '/' || ch == '=') {
				if (aToken.start == NULL) aToken.start = thumbnail.start + i;
				aToken.length++;
			} else {
				if (aToken.start != NULL && aToken.length > 0) {
					fwrite(aToken.start, aToken.length, 1, fp);
					memset(&aToken, 0, sizeof(aToken));
				}
			}
		}
		fprintf(fp, "\n");
	}
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
	fprintf(fp, ";file_total_lines: %lu\n", (unsigned long)(lineNr + 25 - origThumbnailLines));
#else /* !FEATURE_REMOVE_ORIG_THUMBNAIL */
	fprintf(fp, ";file_total_lines: %lu\n", (unsigned long)(lineNr + 25));
#endif /* !FEATURE_REMOVE_ORIG_THUMBNAIL */
	fprintf(fp, ";estimated_time(s): %.0f\n", (float)p_dtms(&estTime));
	fprintf(fp, ";nozzle_temperature(°C): %.0f\n", p_float(&nozzleTemp));
	fprintf(fp, ";build_plate_temperature(°C): %.0f\n", p_float(&plateTemp));
	fprintf(fp, ";work_speed(mm/minute): %.0f\n", p_float(&printSpeed) * 60.0f);
	fprintf(fp, ";max_x(mm): 0\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";max_y(mm): 0\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";max_z(mm): 0\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";min_x(mm): 0\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";min_y(mm): 0\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";min_z(mm): 0\n\n"); /* not set by Snapmaker Luban */
	fprintf(fp, ";Header End\n\n", p_float(&printSpeed) * 60.0f);
	if (ferror(fp) != 0) ON_ERROR(MSGT_ERR_FILE_WRITE);
	
	/* output remaining file */
#ifdef FEATURE_REMOVE_ORIG_THUMBNAIL
	if (origThumbnail.start != NULL && origThumbnail.length != 0) {
		/* cut out original thumbnail */
		const char * thumbnailEndIt = origThumbnail.start + origThumbnail.length;
		if (fwrite(inputBuf, (size_t)(origThumbnail.start - inputBuf), 1, fp) < 1) ON_ERROR(MSGT_ERR_FILE_WRITE);
		if (fwrite(thumbnailEndIt, (size_t)(inputLen - (thumbnailEndIt - inputBuf)), 1, fp) < 1) ON_ERROR(MSGT_ERR_FILE_WRITE);
	} else
#endif /* FEATURE_REMOVE_ORIG_THUMBNAIL */
	{
		if (fwrite(inputBuf, inputLen, 1, fp) < 1) ON_ERROR(MSGT_ERR_FILE_WRITE);
	}
onSuccess:
	res = 1;
onError:
	if (fp != NULL) fclose(fp);
	if (inputBuf != NULL) free(inputBuf);
	return res;
	
#undef ON_WARN
#undef ON_ERROR
}


/**
 * Error output callback for processFile().
 * 
 * @param[in] msg - error message ID
 * @param[in] file - input file path
 * @param[in] line - input file path line number (0 if not applicable)
 * @return 1 to continue, 0 to abort file processing
 * @remarks File processing is always aborted on error.
 */
int errorCallback(const tMessage msg, const TCHAR * file, const size_t line) {
	if (line > 0) {
		_ftprintf(ferr, _T("%s:%u: %s"), file, (unsigned)line, fmsg[msg]);
	} else {
		_ftprintf(ferr, _T("%s: %s"), file, fmsg[msg]);
	}
	return 1;
}
