/**
 * @file tchar.h
 * @author Daniel Starke
 * @date 2014-05-04
 * @version 2021-02-01
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

#ifndef __TCHAR_H__
#define __TCHAR_H__

#if defined(__MINGW32__) || defined(_MSC_VER)
# if defined(_UNICODE) || defined(UNICODE)
#  undef _UNICODE
#  define _UNICODE 1
#  undef UNICODE
#  define UNICODE 1
#  include <wchar.h>
#  include <io.h>
# else
#  include <string.h>
# endif /* defined(_UNICODE) || defined(UNICODE) */
#else /* not __MINGW32__ */
# include <string.h>
#endif


#ifdef UNICODE
typedef wchar_t TCHAR;
#define _T2(x) _T(x)
#define _T(x) L##x
/* std C */
#define _totlower towlower
#define _totupper towupper
#define _istprint iswprint
#define _istdigit iswdigit
#define _tcslen wcslen
#define _tcscmp wcscmp
#define _tcsncmp wcsncmp
#define _tcsrchr wcsrchr
#define _tcspbrk wcspbrk
#define _tcschr wcschr
#define _tcstol wcstol
#define _ttoi _wtoi
#define _fgetts fgetws
#define _fputts fputws
#define _putts _putws
#define _tprintf wprintf
#define _ftprintf fwprintf
#define _sntprintf snwprintf
#define _tfopen _wfopen
#define _tstat wstat
#define _trename _wrename
#define _tcserror _wcserror

#else /* not UNICODE */
typedef char TCHAR;
#define _T2(x) _T(x)
#define _T(x) x
/* std C */
#define _totlower tolower
#define _totupper toupper
#define _istprint isprint
#define _istdigit isdigit
#define _tcslen strlen
#define _tcscmp strcmp
#define _tcsncmp strncmp
#define _tcsrchr strrchr
#define _tcspbrk strpbrk
#define _tcschr strchr
#define _tcstol strtol
#define _ttoi atoi
#define _fgetts fgets
#define _fputts fputs
#define _putts puts
#define _tprintf printf
#define _ftprintf fprintf
#define _sntprintf snprintf
#define _tfopen fopen
#define _tstat stat
#define _trename rename
#define _tcserror strerror

#endif /* not UNICODE */


#ifdef _MSC_VER
# define wstat _wstat
# define stat _stat
# define fseeko64 _fseeki64
# define ftello64(x) _telli64(_fileno(x))
# define S_ISREG(x) ((x) & _S_IFREG) == _S_IFREG
# define S_ISDIR(x) ((x) & _S_IFDIR) == _S_IFDIR
# define UINT64_FMT _T("%I64u")
# if _MSC_VER <= 1600
#  define isblank(x) ((x) == '\t' || (x) == ' ')
# endif
#else /* not _MSC_VER */
#if ! defined(__MINGW64__) && defined(__MINGW32__)
extern __attribute__((__dllimport__)) FILE * _fdopen(int, const char *);
extern __attribute__((__dllimport__)) FILE * _wfdopen(int, const wchar_t *);
extern __attribute__((__dllimport__)) int _wtoi(const wchar_t *);
extern __attribute__((__dllimport__)) FILE * _fopen(const char *, const char *);
extern __attribute__((__dllimport__)) FILE * _wfopen(const wchar_t *, const wchar_t *);
#endif
# ifdef UNICODE
#  if !defined(_O_U16TEXT) && ! defined(__MINGW64__)
#   define _O_U16TEXT 0x20000
#   ifndef _O_U8TEXT
#    define _O_U8TEXT 0x40000
#   endif /* _O_U8TEXT */
#   define wstat _wstat
#  endif /* !defined(_O_U16TEXT) && !defined(__MINGW64__) */
# endif /* UNICODE */
# define UINT64_FMT _T("%" PRIu64)
#endif /* not _MSC_VER */


#endif /* __TCHAR_H__ */
