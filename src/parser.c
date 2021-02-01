/**
 * @file parser.c
 * @author Daniel Starke
 * @see parser.h
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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "target.h"


/**
 * Compares the given token with a passed string. Both are compared case sensitive. The token needs
 * to match the passed string exactly and completely to return 0.
 * 
 * @param[in] token - token to compare
 * @param[in] str - compare with this string
 * @return same as strcmp
 */
int p_cmpToken(const tPToken * token, const char * str) {
	if (token == NULL || token->start == NULL || str == NULL) {
		errno = EFAULT;
		return INT_MAX;
	}
	const unsigned char * left = (const unsigned char *)(token->start);
	const unsigned char * right = (const unsigned char *)str;
	size_t length = token->length;
	for (;length > 0 && *right != 0 && *left == *right; length--, left++, right++);
	if (length > 0 && *right == 0) {
		return *left;
	} else if (length == 0 && *right != 0) {
		return -(*right);
	} else if (length == 0 && *right == 0) {
		return 0;
	}
	return *left - *right;
}


/**
 * Compares the given token with a passed string. Both are compared case insensitive. The token
 * needs to match the passed string exactly and completely to return 0. toupper is being used to
 * normalize cases.
 * 
 * @param[in] token - token to compare
 * @param[in] str - compare with this string
 * @return same as stricmp
 * @warning Do not use this function if the character value range of str exceeds 7 bits (ANSI).
 */
int p_cmpTokenI(const tPToken * token, const char * str) {
	if (token == NULL || token->start == NULL || str == NULL) {
		errno = EFAULT;
		return INT_MAX;
	}
	const unsigned char * left = (const unsigned char *)(token->start);
	const unsigned char * right = (const unsigned char *)str;
	size_t length = token->length;
	for (;length > 0 && *right != 0 && toupper(*left) == toupper(*right); length--, left++, right++);
	if (length > 0 && *right == 0) {
		return toupper(*left);
	} else if (length == 0 && *right != 0) {
		return -toupper(*right);
	} else if (length == 0 && *right == 0) {
		return 0;
	}
	return toupper(*left) - toupper(*right);
}


/**
 * Compares the given tokens with each other. Both are compared case sensitive. The tokens needs
 * to match exactly and completely to return 0.
 * 
 * @param[in] lhs - left-hand statement of comparison
 * @param[in] rhs - right-hand statement of comparison
 * @return same as strcmp
 */
int p_cmpTokens(const tPToken * lhs, const tPToken * rhs) {
	if (lhs == NULL || lhs->start == NULL || rhs == NULL || rhs->start == NULL) {
		errno = EFAULT;
		return INT_MAX;
	}
	size_t l = lhs->length;
	size_t r = rhs->length;
	size_t m = PCF_MIN(l, r);
	const unsigned char * left  = (const unsigned char *)(lhs->start);
	const unsigned char * right = (const unsigned char *)(rhs->start);
	for (;m > 0 && *left == *right; m--, left++, right++);
	if (m == 0) {
		if (l > r) {
			return *((const unsigned char *)(lhs->start + r));
		} else if (l < r) {
			return -(*((const unsigned char *)(rhs->start + l)));
		} else {
			/* same length and same content */
			return 0;
		}
	}
	return *left - *right;
}


/**
 * Compares the given tokens with each other. Both are compared case insensitive. The tokens needs
 * to match exactly and completely to return 0. toupper is being used to normalize cases.
 * 
 * @param[in] lhs - left-hand statement of comparison
 * @param[in] rhs - right-hand statement of comparison
 * @return same as stricmp
 * @warning Do not use this function if the character value range of rhs exceeds 7 bits (ANSI).
 */
int p_cmpTokensI(const tPToken * lhs, const tPToken * rhs) {
	if (lhs == NULL || lhs->start == NULL || rhs == NULL || rhs->start == NULL) {
		errno = EFAULT;
		return INT_MAX;
	}
	size_t l = lhs->length;
	size_t r = rhs->length;
	size_t m = PCF_MIN(l, r);
	const unsigned char * left  = (const unsigned char *)(lhs->start);
	const unsigned char * right = (const unsigned char *)(rhs->start);
	for (;m > 0 && toupper(*left) == toupper(*right); m--, left++, right++);
	if (m == 0) {
		if (l > r) {
			return toupper(*((const unsigned char *)(lhs->start + r)));
		} else if (l < r) {
			return -toupper(*((const unsigned char *)(rhs->start + l)));
		} else {
			/* same length and same content */
			return 0;
		}
	}
	return toupper(*left) - toupper(*right);
}


/**
 * Creates a null-terminated string copy of the given parser token.
 * 
 * @param[in] token - token to copy
 * @return Copy on success, NULL on error
 */
char * p_copyToken(const tPToken * token) {
	if (token->start == NULL) return NULL;
	char * result = (char *)malloc(token->length + 1);
	if (result == NULL) return result;
	if (token->length > 0) memcpy(result, token->start, token->length);
	result[token->length] = 0;
	return result;
}
