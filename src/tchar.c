/**
 * @file tchar.c
 * @author Daniel Starke
 * @see tchar.h
 * @date 2017-05-24
 * @version 2017-05-24
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
#include "tchar.h"


/**
 * Returns the pointer to the last occurrence of a character from the given set.
 * NULL is returned if no set character was found.
 * 
 * @param[in] str1 - search within this string
 * @param[in] str1 - search for a character in this set
 * @return NULL if not found or the pointer to the last occurrence
 */
const TCHAR * _tcsrpbrk(const TCHAR * str1, const TCHAR * str2) {
	const TCHAR * last = NULL;
	const TCHAR * cur = str1;
	while ((cur = _tcspbrk(cur, str2)) != NULL) {
		last = cur;
		cur++;
	}
	return last;
}
