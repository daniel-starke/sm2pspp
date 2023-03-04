/**
 * @file sm2pspp.h
 * @author Daniel Starke
 * @date 2021-01-30
 * @version 2023-03-04
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
#ifndef __SM2PSPP_H__
#define __SM2PSPP_H__

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "target.h"
#include "tchar.h"
#include "version.h"


/** Input line buffer size. */
#define LINE_BUFFER_SIZE 0x80000


/** The original thumbnail is removed if this macro is defined. */
#define FEATURE_REMOVE_ORIG_THUMBNAIL 1


/** Enumeration of possible error values. */
typedef enum {
	MSGT_SUCCESS = 0,
	MSGT_ERR_NO_MEM,
	MSGT_ERR_FILE_NOT_FOUND,
	MSGT_ERR_FILE_OPEN,
	MSGT_ERR_FILE_READ,
	MSGT_ERR_FILE_CREATE,
	MSGT_ERR_FILE_WRITE,
	MSGT_WARN_NO_FILAMENT_USED,
	MSGT_WARN_NO_LAYER_HEIGHT,
	MSGT_WARN_NO_EST_TIME,
	MSGT_WARN_NO_NOZZLE_TEMP,
	MSGT_WARN_NO_PLATE_TEMP,
	MSGT_WARN_NO_PRINT_SPEED,
	MSGT_WARN_NO_THUMBNAIL,
	MSG_COUNT
} tMessage;


/** Error callback type. */
typedef int (* tCallback)(const tMessage msg, const TCHAR * file, const size_t line);


extern FILE * fin;
extern FILE * fout;
extern FILE * ferr;
extern const TCHAR * fmsg[MSG_COUNT];


/* helper functions */
void printHelp(void);
int processFile(const TCHAR * file, const tCallback cb);
int errorCallback(const tMessage msg, const TCHAR * file, const size_t line);


#endif /* __SM2PSPP_H__ */
