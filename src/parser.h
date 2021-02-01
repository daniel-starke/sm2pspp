/**
 * @file parser.h
 * @author Daniel Starke
 * @see parser.c
 * @see sax.c
 * @date 2018-06-23
 * @version 2021-01-31
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
#ifndef __LIBPCF_PARSER_H__
#define __LIBPCF_PARSER_H__

#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * A single text parser token.
 */
typedef struct {
	const char * start;        /**< pointer to the start of the token */
	size_t length;             /**< number of bytes length of the token */
} tPToken;


int p_cmpToken(const tPToken * token, const char * str);
int p_cmpTokenI(const tPToken * token, const char * str);
int p_cmpTokens(const tPToken * lhs, const tPToken * rhs);
int p_cmpTokensI(const tPToken * lhs, const tPToken * rhs);
char * p_copyToken(const tPToken * token);


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_PARSER_H__ */
