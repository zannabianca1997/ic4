/**
 * @file charescape.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Tables to escape chars
 * @version 0.1
 * @date 2022-02-14
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _CHARESCAPE_H
#define _CHARESCAPE_H

/**
 * @brief Maximal lenght of a escape code
 *
 */
#define CHARESCAPE_MAX_LEN 4

/**
 * @brief Get the escape string for a given char
 *
 */
#define CHARESCAPE(ch) (_CHARESCAPE[(unsigned char)(ch)])
/**
 * @brief Get the len of the escape string for a given char
 *
 */
#define CHARESCAPE_LEN(ch) (_CHARESCAPE_LEN[(unsigned char)(ch)])

/*
# The next arrays is given by this python3 code
escaped = {0x07: "a", 0x08: "b", 0x0c: "f", 0x0a: "n", 0x0d: "r",
        0x09: "t", 0x0b: "v", 0x27: "\\'", 0x22: "\\\"", 0x5c: "\\\\"}
escape_seqs = "    "
for i in range(256):
    if i < 128:
        if i in escaped:
            escape_seqs += f"\"\\\\{escaped[i]}\", "
        elif chr(i).isprintable():
            escape_seqs += f"\"{chr(i)}\", "
        else:
            escape_seqs += f"\"\\\\{i:03o}\", "
    else:
        escape_seqs += f"\"\\\\{i:03o}\", "
    if i % 8 == 7:
        escape_seqs += "\n    "
escape_lens = "    "
for i, l in enumerate(len(s) for s in eval(f"[{escape_seqs}]")):
    escape_lens += f"{l}, "
    if i % 16 == 15:
        escape_lens += "\n    "

print(f"""
/**
 * @brief Escape codes for all bytes
 *""" + """/
static const char _CHARESCAPE[][CHARESCAPE_MAX_LEN + 1] = {{
    // @formatter:off
{escape_seqs}// @formatter:on
}};
/**
 * @brief Lenght of the escape codes for all bytes
 *""" + """/
static const unsigned char _CHARESCAPE_LEN[] = {{
    // @formatter:off
{escape_lens}// @formatter:on
}};""")
*/

/**
 * @brief Escape codes for all bytes
 */
static const char _CHARESCAPE[][CHARESCAPE_MAX_LEN + 1] = {
    // @formatter:off
    "\\000", "\\001", "\\002", "\\003", "\\004", "\\005", "\\006", "\\a",
    "\\b", "\\t", "\\n", "\\v", "\\f", "\\r", "\\016", "\\017",
    "\\020", "\\021", "\\022", "\\023", "\\024", "\\025", "\\026", "\\027",
    "\\030", "\\031", "\\032", "\\033", "\\034", "\\035", "\\036", "\\037",
    " ", "!", "\\\"", "#", "$", "%", "&", "\\\'",
    "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7",
    "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G",
    "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "[", "\\\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g",
    "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w",
    "x", "y", "z", "{", "|", "}", "~", "\\177",
    "\\200", "\\201", "\\202", "\\203", "\\204", "\\205", "\\206", "\\207",
    "\\210", "\\211", "\\212", "\\213", "\\214", "\\215", "\\216", "\\217",
    "\\220", "\\221", "\\222", "\\223", "\\224", "\\225", "\\226", "\\227",
    "\\230", "\\231", "\\232", "\\233", "\\234", "\\235", "\\236", "\\237",
    "\\240", "\\241", "\\242", "\\243", "\\244", "\\245", "\\246", "\\247",
    "\\250", "\\251", "\\252", "\\253", "\\254", "\\255", "\\256", "\\257",
    "\\260", "\\261", "\\262", "\\263", "\\264", "\\265", "\\266", "\\267",
    "\\270", "\\271", "\\272", "\\273", "\\274", "\\275", "\\276", "\\277",
    "\\300", "\\301", "\\302", "\\303", "\\304", "\\305", "\\306", "\\307",
    "\\310", "\\311", "\\312", "\\313", "\\314", "\\315", "\\316", "\\317",
    "\\320", "\\321", "\\322", "\\323", "\\324", "\\325", "\\326", "\\327",
    "\\330", "\\331", "\\332", "\\333", "\\334", "\\335", "\\336", "\\337",
    "\\340", "\\341", "\\342", "\\343", "\\344", "\\345", "\\346", "\\347",
    "\\350", "\\351", "\\352", "\\353", "\\354", "\\355", "\\356", "\\357",
    "\\360", "\\361", "\\362", "\\363", "\\364", "\\365", "\\366", "\\367",
    "\\370", "\\371", "\\372", "\\373", "\\374", "\\375", "\\376", "\\377",
    // @formatter:on
};
/**
 * @brief Lenght of the escape codes for all bytes
 */
static const unsigned char _CHARESCAPE_LEN[] = {
    // @formatter:off
    4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    // @formatter:on
};

/**
 * @brief Calculate the escaped lenght of a string
 * 
 * @param string the string to escape
 * @param len the len of the string (to permit NUL in strings)
 * @return size_t the length of the escaped string
 */
static inline size_t escaped_len(char const *string, size_t len)
{
    size_t len = 0;
    for (size_t i = 0; i < len; i++)
        len += CHARESCAPE_LEN(string[i]);
    return len;
}
/**
 * @brief Escape a string
 * 
 * @param dest the destination string
 * @param string the string to escape
 * @param len the len of the string (to permit NUL in strings)
 * @return char* the end of the escaped string
 */
static inline char *escaped_string(char *dest, char const *string, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        for (size_t j = 0; j < CHARESCAPE_LEN(string[i]); j++)
            dest[j] = CHARESCAPE(string[i])[j];
        dest += CHARESCAPE_LEN(string[i]);
    }
    return dest;
}

#endif // _CHARESCAPE_H