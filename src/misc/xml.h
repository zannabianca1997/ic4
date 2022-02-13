/**
 * @file xml.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Methods to produce a well formed XML output
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _XML_H
#define _XML_H

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief Store all data regarding an XML tag.
 * Use with a xml_stream to open and close the tag
 */
typedef struct xml_tag_s xml_tag_t;

/**
 * @brief Create a new xml tag.
 * Tag is without attributes.
 * @param name the name of the tag
 * @return xml_tag_t* the generated tag
 */
xml_tag_t *xml_tag_new(char const *name);
/**
 * @brief Copy an existing xml tag
 * 
 * @param tag the tag to copy
 * @return xml_tag_t* the copied tag
 */
xml_tag_t *xml_tag_copy(xml_tag_t const *tag);
/**
 * @brief Destroy an xml tag
 * 
 * @param tag the tag to destroy
 */
void xml_tag_free(xml_tag_t *tag);

/**
 * @brief Set the name of a xml tag.
 * 
 * @param tag the tag to change
 * @param new_name the new name
 * @return true Set was successfull
 * @return false Set failed
 */
bool xml_tag_name_set(xml_tag_t *tag, const char *new_name);
/**
 * @brief Get the name of an xml tag
 * 
 * @param tag the xml tag
 * @return const char* the recovered name
 */
const char *xml_tag_name_get(xml_tag_t const *tag);

/**
 * @brief Set an attribute of an xml tag.
 * If attribute wasn't present it's created.
 * If value is NULL the tag is destroyed, if present.
 * 
 * @param tag the tag to modify
 * @param name the name of the attribute
 * @param value the content of the attribute, or NULL to destroy it
 * @return true Set was successfull
 * @return false Set failed
 */
bool xml_tag_attribute_set(xml_tag_t *tag, const char *name, const char *value);
/**
 * @brief Get the value of an attribute of an xml tag.
 * Return NULL if the attribute is missing
 * 
 * @param tag the tag to read
 * @param name the name of the attribute
 * @return const char* the value retrieved, or NULL if missing
 */
const char *xml_tag_attribute_get(xml_tag_t const *tag, const char *name);

/**
 * @brief Compare two tags.
 * Tags are sorted alphabetically by name, then by first attribute by name, then by first attribute value, etc..
 * 
 * @param a the first tag to compare
 * @param b the second tag to compare
 * @return int ==0 if a==b, <0 if a<b, >0 if a>b
 */
int xml_tag_cmp(xml_tag_t const *a, xml_tag_t const *b);

// --- XML streams ---

/**
 * @brief Write an xml stream;
 * Wrap a FILE * stream
 */
typedef struct xml_stream_s xml_stream_t;

/**
 * @brief Open a xml stream to "target".
 * 
 * @param target the file the xml will be written to
 * @param root_tag the root tag of the stream
 * @param encoding the encoding of the stream
 * @return xml_stream_t* the resulting xml stream, or NULL if an error occurred
 */
xml_stream_t *xml_stream_open(FILE *target, xml_tag_t const *root_tag);
/**
 * @brief Close a xml stream.
 * Close all tags still open and then free the resources
 * 
 * @param stream the stream to close
 * @param close_target if true the underliyng FILE is closed too 
 */
void xml_stream_close(xml_stream_t *stream, bool close_target);

// -- tag functions

/**
 * @brief Open a tag
 * 
 * @param stream the xml stream
 * @param tag the tag to open
 * @return true tag open successfully
 * @return false tag failed to open
 */
bool xml_stream_tag_open(xml_stream_t *stream, xml_tag_t const *tag);
/**
 * @brief Close a tag.
 * Last top open tag must compare equal to "tag".
 * 
 * @param stream the xml stream
 * @param tag the tag to close
 * @return true tag closed successfully
 * @return false a problem arised (tag did not compare equal, tag was root, or general stream problem)
 */
bool xml_stream_tag_close(xml_stream_t *stream, xml_tag_t const *tag);
/**
 * @brief Close a tag by name.
 * Last top open tag name must compare equal to "tag_name".
 * 
 * @param stream the xml stream
 * @param tag_name the name of the tag to close
 * @return true tag closed successfully
 * @return false a problem arised (tag name did not compare equal, tag was root, or general stream problem)
 */
bool xml_stream_tag_close_n(xml_stream_t *stream, char const *tag_name);
/**
 * @brief Close top tag.
 * 
 * @param stream the xml stream
 * @return true tag closed successfully
 * @return false a problem arised (tag was root, or general stream problem)
 */
bool xml_stream_tag_close_t(xml_stream_t *stream);
/**
 * @brief Put an empty tag on the stream.
 * Notice this is different than 
 *     xml_stream_tag_open(stream, tag);
 *     xml_stream_tag_close(stream, tag);
 * cause that produce <name></name> while 
 *     xml_stream_tag_empty(stream, tag);
 * produce <name />
 * 
 * @param stream the xml stream
 * @param tag the tag to write
 * @return true tag writed successfully
 * @return false a problem arised (general stream problem)
 */
bool xml_stream_tag_empty(xml_stream_t *stream, xml_tag_t const *tag);

// -- printing function

/**
 * @brief Put a char as tag content
 * 
 * @param ch the char to put
 * @param stream the xml stream
 * @return int ch or EOF in case of error
 */
int xml_stream_putc(int ch, xml_stream_t *stream);
/**
 * @brief Put a string as tag content
 * 
 * @param s the string to put
 * @param stream the xml stream
 * @return int non-negative number for success, EOF for error (general stream problem)
 */
int xml_stream_puts(const char *s, xml_stream_t *stream);
/**
 * @brief Print formatted as tag content
 * 
 * @param stream the xml stream
 * @param fmt the format string (see printf)
 * @param ... formatting args
 * @return int number of written chars, or negative int in case of error
 */
int xml_stream_printf(xml_stream_t *stream, const char *fmt, ...);
/**
 * @brief Print variadic formatted as tag content
 * 
 * 
 * @param stream the xml stream
 * @param fmt the format string (see printf)
 * @param ap formatting args
 * @return int number of written chars, or negative int in case of error
 */
int xml_stream_vprintf(xml_stream_t *stream, const char *fmt, va_list ap);


#endif