/**
 * @file target.h
 * @author Daniel Starke
 * @date 2012-12-08
 * @version 2018-08-13
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
#ifndef __LIBPCF_TARGET_H__
#define __LIBPCF_TARGET_H__

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifndef TO_STR
/** Converts the passed argument to a string literal. */
# define TO_STR(x) #x
#endif /* to string */

#ifndef TO_STR2
/** Converts the passed argument to a string literal. */
# define TO_STR2(x) TO_STR(x)
#endif /* to string */


#if defined(__WIN32__) || defined(__WIN64__) || defined(WIN32) \
 || defined(WINNT) || defined(_WIN32) || defined(__WIN32) || defined(__WINNT) \
 || defined(__MINGW32__) || defined(__MINGW64__)
# ifndef PCF_IS_WIN
/** Defined if compiler target is windows. */
#  define PCF_IS_WIN 1
# endif
# undef PCF_IS_NO_WIN
#else /* no windows */
# ifndef PCF_IS_NO_WIN
/** Defined if compiler target is _not_ windows. */
#  define PCF_IS_NO_WIN 1
# endif
# undef PCF_IS_WIN
#endif /* windows */


#if !defined(PCF_IS_WIN) && (defined(unix) || defined(__unix) || defined(__unix__) \
 || defined(__gnu_linux__) || defined(linux) || defined(__linux) \
 || defined(__linux__))
# ifndef PCF_IS_LINUX
/** Defined if compiler target is linux/unix. */
# define PCF_IS_LINUX 1
# endif
# undef PCF_IS_NO_LINUX
#else /* no linux */
# ifndef PCF_IS_NO_LINUX
/** Defined if compiler target is _not_ linux/unix. */
#  define PCF_IS_NO_LINUX 1
# endif
# undef PCF_IS_LINUX
#endif /* linux */


#ifdef PCF_PATH_SEP
# undef PCF_PATH_SEP
#endif
#ifdef PCF_IS_WIN
/** Defines the Windows path separator. */
# define PCF_PATH_SEP "\\"
# define PCF_PATH_SEPU L"\\"
#else /* PCF_IS_NO_WIN */
/** Defines the non-Windows path separator. */
# define PCF_PATH_SEP "/"
# define PCF_PATH_SEPU L"/"
#endif /* PCF_IS_WIN */


/* check whether type is 32-bit aligned */
#ifndef PCF_UNALIGNED_P32
# if __GNUC__ >= 2
#  define PCF_UNALIGNED_P32(p) (((uintptr_t)p) % __alignof__(uint32_t) != 0)
# elif _MSC_VER
#  define PCF_UNALIGNED_P32(p) (((uintptr_t)p) % __alignof(uint32_t) != 0)
# else
#  define alignof(type) offsetof(struct {char c; type x;}, x)
#  define PCF_UNALIGNED_P32(p) (((int)p) % alignof(uint32_t) != 0)
# endif
#endif /* PCF_UNALIGNED_P32 */


#define PCF_MIN(x, y) ((x) > (y) ? (y) : (x))
#define PCF_MAX(x, y) ((x) >= (y) ? (x) : (y))


/* suppress unused parameter warning */
#ifdef _MSC_VER
# define PCF_UNUSED(x)
# define PCF_TYPEOF(x) decltype(x)
#else /* not _MSC_VER */
# define PCF_UNUSED(x) (void)x;
# define PCF_TYPEOF(x) __typeof__(x)
#endif /* not _MSC_VER */


/* Windows workarounds */
#ifdef PCF_IS_WIN
# define fileno _fileno
# define fdopen _fdopen
# define setmode _setmode
# define get_osfhandle _get_osfhandle
# define open_osfhandle _open_osfhandle
# define O_TEXT _O_TEXT
# define O_WTEXT _O_WTEXT
# define O_U8TEXT _O_U8TEXT
# define O_U16TEXT _O_U16TEXT
# define O_BINARY _O_BINARY
# define O_RDONLY _O_RDONLY
#endif


/* MSVS workarounds */
#ifdef _MSC_VER
# define strdup _strdup
# define putenv _putenv
# define snprintf _snprintf
# define snwprintf _snwprintf
# define vsnprintf _vsnprintf
# define vsnwprintf _vsnwprintf
# define va_copy(dest, src) (dest = src)
#endif /* _MSC_VER */


/* Cygwin workaround */
#ifdef __CYGWIN__
# define O_U8TEXT 0x00040000
# define O_U16TEXT 0x00020000
#endif /* __CYGWIN__ */


/* Define vsnwprintf from vswprintf on Linux systems. */
#ifdef PCF_IS_NO_WIN
# define vsnwprintf vswprintf
# define stricmp strcasecmp
#endif


#ifdef __cplusplus
}
#endif


#endif /* __LIBPCF_TARGET_H__ */
