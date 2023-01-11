#ifndef LUCK_MTP_UTILS
#define LUCK_MTP_UTILS

#include <string.h>
#include <regex>
#include <vector>
#include "libmtp.h"

using namespace std;

/**
 * string split helper
 */
vector<string> split(const string &input, const string &regex);

/**
 * format target path
*/
string formatMtpPath(const string &path);

/**
 * find file type
*/
LIBMTP_filetype_t
find_filetype(const char *filename);

#endif