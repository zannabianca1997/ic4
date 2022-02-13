#ifdef __GNUC__
#define _GNU_SOURCE // fopencookie
#else
#error "GNU extension needed for custom streams"
#endif

#include <stddef.h> // size_t, ssize_t
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // strlen, strcmp, strcpy
#include <limits.h> // INT_MIN, INT_MAX
#include <ctype.h>  // isprint

#include "xml.h"

#define XML_TAG_STACK_INITIAL_SIZE 16

/**
 * @brief Contain all the info for a xml tag
 */
struct xml_tag_s
{
    /**
     * @brief The name of the tag
     */
    char *name;

    /**
     * @brief How many attributes are in the tag
     */
    size_t attributes_num;
    /**
     * @brief How many attrybutes space is allocated for.
     * Consent preallocation of space
     */
    size_t attributes_space;
    /**
     * @brief The attributes of the tag
     * Attributes are sorted by name.
     */
    struct
    {
        /**
         * @brief The name of the attribute
         */
        char *name;
        /**
         * @brief The value of the attribute
         * value is not XML escaped
         */
        char *value;
    } * attributes;
};

static bool check_name(char const *name);

xml_tag_t *xml_tag_new(char const *name)
{
    if (!check_name(name))
        return NULL;

    xml_tag_t *new_tag = malloc(sizeof(xml_tag_t));
    if (new_tag == NULL)
        return NULL;

    // setting NULL name
    new_tag->name = NULL;
    // setting 0 attributes
    new_tag->attributes_num = 0;
    new_tag->attributes_space = 0;
    new_tag->attributes = NULL;

    // copying name
    if (!xml_tag_name_set(new_tag, name))
    {
        xml_tag_free(new_tag);
        return NULL;
    }

    return new_tag;
}
xml_tag_t *xml_tag_copy(xml_tag_t const *tag)
{
    xml_tag_t *new_tag = xml_tag_new(tag->name);

    // copying attributes

    // preallocating space
    void *new_attributes = realloc(new_tag->attributes, sizeof(*(new_tag->attributes)) * tag->attributes_num);
    if (new_attributes == NULL)
    {
        xml_tag_free(new_tag);
        return NULL;
    }
    new_tag->attributes = new_attributes;

    new_tag->attributes_space = tag->attributes_num;
    new_tag->attributes_num = 0;
    for (size_t i = 0; i < tag->attributes_num; i++)
        if (!xml_tag_attribute_set(new_tag, tag->attributes[i].name, tag->attributes[i].value))
        {
            xml_tag_free(new_tag);
            return NULL;
        }

    return new_tag;
}
void xml_tag_free(xml_tag_t *tag)
{
    free(tag->name);
    for (size_t i = 0; i < tag->attributes_num; i++)
    {
        free(tag->attributes[i].name);
        free(tag->attributes[i].value);
    }
    free(tag->attributes);
    free(tag);
}

/**
 * @brief Check if name is a valid identifier
 * 
 * @param name the name to check
 * @return true the name is valid
 * @return false the name is invalid
 */
static bool check_name(char const *name)
{
    // first char must be alpha or _ or :
    if (!(isalpha(name[0]) || name[0] == '_' || name[0] == ':'))
        return false;
    // other chars can be also numeric or - or .
    for (size_t i = 1; i < strlen(name); i++)
        if (!(isalnum(name[i]) || name[i] == '_' || name[i] == ':' || name[i] == '-' || name[i] == '.'))
            return false;

    return true;
}

bool xml_tag_name_set(xml_tag_t *tag, const char *new_name)
{
    if (!check_name(new_name))
        return false;

    // reallocate name space
    char *new_tag_name = realloc(tag->name, strlen(new_name) + 1);
    if (new_tag_name == NULL)
        return false;
    tag->name = new_tag_name;
    // set name
    strcpy(tag->name, new_name);
    return true;
}
const char *xml_tag_name_get(xml_tag_t const *tag)
{
    return tag->name;
}

bool xml_tag_attribute_set(xml_tag_t *tag, const char *name, const char *value)
{
    if (!check_name(name))
        return false;

    // searching for attribute
    size_t idx;
    for (idx = 0; idx < tag->attributes_num && strcmp(tag->attributes[idx].name, name) < 0; idx++)
        ;
    if (idx < tag->attributes_num && strcmp(tag->attributes[idx].name, name) == 0)
    {
        // attribute is present at idx

        if (value != NULL)
        {
            // allocating space for new value
            char *new_attr_value = realloc(tag->attributes[idx].value, strlen(value) + 1);
            if (new_attr_value == NULL)
                return false;
            tag->attributes[idx].value = new_attr_value;

            strcpy(tag->attributes[idx].value, value);
        }
        else
        {
            // removing value
            free(tag->attributes[idx].value);
            free(tag->attributes[idx].name);

            for (idx++; idx < tag->attributes_num; idx++)
                tag->attributes[idx - 1] = tag->attributes[idx];
            tag->attributes_num--;
        }
    }
    else
    {
        // attribute need to be added at idx

        if (value == NULL)
            return true; // no problem

        // allocating space for new attribute

        // in memory
        char *name_mem = malloc(strlen(name) + 1);
        char *value_mem = malloc(strlen(value) + 1);
        if (name_mem == NULL || value_mem == NULL)
        {
            if (name_mem != NULL)
                free(name_mem);
            if (value_mem != NULL)
                free(value_mem);
            return false;
        }
        // in attribute list
        if (tag->attributes_space == tag->attributes_num)
        {
            void *new_attributes = realloc(tag->attributes, sizeof(*(tag->attributes)) * (tag->attributes_num + 1));
            if (new_attributes == NULL)
                return false;
            tag->attributes = new_attributes;
            tag->attributes_space = tag->attributes_num + 1;
        }

        // space cleared, we can proceed

        // moving all attributes over
        // reverse for to avoid corruption
        for (size_t i = tag->attributes_num; i > idx; i--)
            tag->attributes[i] = tag->attributes[i - 1];
        tag->attributes_num++;

        // now idx is free
        tag->attributes[idx].name = name_mem;
        tag->attributes[idx].value = value_mem;

        // coping strings
        strcpy(tag->attributes[idx].name, name);
        strcpy(tag->attributes[idx].value, value);
    }
    return true;
}
const char *xml_tag_attribute_get(xml_tag_t const *tag, const char *name)
{
    // searching for attribute
    size_t idx;
    for (idx = 0; strcmp(tag->attributes[idx].name, name) < 0 && idx < tag->attributes_num; idx++)
        ;
    if (strcmp(tag->attributes[idx].name, name) != 0)
        return NULL; // attribute is missing
    return tag->attributes[idx].value;
}

static int clamp_to_int(size_t x)
{
    if (x > INT_MAX)
        return INT_MAX;
    return (int)x;
}

int xml_tag_cmp(xml_tag_t const *a, xml_tag_t const *b)
{
    int cmp;

    // compare by name
    cmp = strcmp(a->name, b->name);
    if (cmp != 0)
        return cmp;

    // compare by attributes in order
    for (size_t idx = 0; idx < a->attributes_num && idx < b->attributes_num; idx++)
    {
        // compare by name
        cmp = strcmp(a->attributes[idx].name, b->attributes[idx].name);
        if (cmp != 0)
            return cmp;
        // compare by value
        cmp = strcmp(a->attributes[idx].value, b->attributes[idx].value);
        if (cmp != 0)
            return cmp;
    }

    // all common attributes are equals, compare by longest
    return clamp_to_int(a->attributes_num) - clamp_to_int(b->attributes_num);
}

// --- XML streams ---

/**
 * @brief Contain all data regarding an xml stream
 * 
 */
struct xml_stream_s
{
    /**
     * @brief The target stream
     * 
     */
    FILE *target;
    /**
     * @brief Escaped target stream
     * Thanks to fopencookie writing to this stream will send data directly to target,
     * escaping all sequences XML would deem invalid
     */
    FILE *escaped_target;
    /**
     * @brief The open tags stack
     * The first tag is the root, and cannot be closed
     */
    struct
    {
        /**
         * @brief Tag stack memory
         * stack[0] is the root, stack[size-1] the current tag
         */
        xml_tag_t **stack;
        /**
         * @brief the heigth of the stack
         */
        size_t size;
        /**
         * @brief How many element can the stack store before reallocating is needed
         */
        size_t max;
    } tag_stack;
};

static ssize_t escaped_write(void *cookie, const char *buf, size_t size);

xml_stream_t *xml_stream_open(FILE *target, xml_tag_t const *root_tag)
{
    // memory for the stream structure
    xml_stream_t *new_stream = malloc(sizeof(xml_stream_t));
    if (new_stream == NULL)
        return NULL;

    // writing targets
    new_stream->target = target;
    new_stream->escaped_target = fopencookie(target, "w", (cookie_io_functions_t){.read = NULL, .write = &escaped_write, .seek = NULL, .close = NULL});
    if (new_stream->escaped_target == NULL)
    {
        free(new_stream);
        return NULL;
    }

    // stack creation
    new_stream->tag_stack.stack = malloc(sizeof(xml_tag_t *) * XML_TAG_STACK_INITIAL_SIZE);
    if (new_stream->tag_stack.stack == NULL)
    {
        fclose(new_stream->escaped_target);
        free(new_stream);
        return NULL;
    }
    new_stream->tag_stack.max = XML_TAG_STACK_INITIAL_SIZE;
    new_stream->tag_stack.size = 0;

    // putting root
    if (!xml_stream_tag_open(new_stream, root_tag))
    {
        fclose(new_stream->escaped_target);
        free(new_stream);
        return NULL;
    }

    return new_stream;
}
static bool stream_tag_close_t_nopreserveroot(xml_stream_t *stream);
void xml_stream_close(xml_stream_t *stream, bool close_target)
{
    // closing all opened tags, even root
    while (stream->tag_stack.size)
        stream_tag_close_t_nopreserveroot(stream);

    // freeing resources
    fclose(stream->escaped_target);
    if (close_target)
        fclose(stream->target);
    free(stream->tag_stack.stack);
    free(stream);
}

// -- tag functions
enum tag_types_e
{
    TAG_OPEN,
    TAG_CLOSE,
    TAG_EMPTY
};
static bool write_tag(xml_stream_t *stream, xml_tag_t const *tag, enum tag_types_e type);

bool xml_stream_tag_open(xml_stream_t *stream, xml_tag_t const *tag)
{
    // check if we need more space on the tag stack
    if (stream->tag_stack.size == stream->tag_stack.max)
    {
        xml_tag_t **new_stack = realloc(stream->tag_stack.stack, sizeof(xml_tag_t *) * stream->tag_stack.max * 2);
        if (new_stack == NULL)
            return false;
        stream->tag_stack.stack = new_stack;
        stream->tag_stack.max *= 2;
    }

    // copy the tag on the stack
    stream->tag_stack.stack[stream->tag_stack.size] = xml_tag_copy(tag);
    if (stream->tag_stack.stack[stream->tag_stack.size] == NULL)
        return false;
    stream->tag_stack.size++;

    // write open tag to file
    if (!write_tag(stream, tag, TAG_OPEN))
    {
        xml_tag_free(stream->tag_stack.stack[--stream->tag_stack.size]);
        return false;
    }

    return true;
}

/**
 * @brief Close the top tag, even if it's root
 * 
 * @param stream the xml stream
 * @return true tag closed successfully
 * @return false an error happened
 */
static bool stream_tag_close_t_nopreserveroot(xml_stream_t *stream)
{
    xml_tag_t *popped_tag = stream->tag_stack.stack[--stream->tag_stack.size];
    // write close tag to file
    if (!write_tag(stream, popped_tag, TAG_CLOSE))
    {
        stream->tag_stack.stack[stream->tag_stack.size++] = popped_tag; // putting again the tag on top
        return false;
    }
    // freeing tag
    xml_tag_free(popped_tag);
    return true;
}
bool xml_stream_tag_close_t(xml_stream_t *stream)
{
    // root protection
    if (stream->tag_stack.size == 1)
        return false;

    return stream_tag_close_t_nopreserveroot(stream);
}
bool xml_stream_tag_close_n(xml_stream_t *stream, char const *tag_name)
{
    if (strcmp(tag_name, stream->tag_stack.stack[stream->tag_stack.size - 1]->name) != 0)
        return false; // tag did not match
    return xml_stream_tag_close_t(stream);
}
bool xml_stream_tag_close(xml_stream_t *stream, xml_tag_t const *tag)
{
    if (xml_tag_cmp(tag, stream->tag_stack.stack[stream->tag_stack.size - 1]) != 0)
        return false; // tag did not match
    return xml_stream_tag_close_t(stream);
}
bool xml_stream_tag_empty(xml_stream_t *stream, xml_tag_t const *tag)
{
    return write_tag(stream, tag, TAG_EMPTY);
}

// -- printing function

/**
 * @brief write a tag on the stream
 * 
 * @param stream the xml stream
 * @param tag the tag to write
 * @param type the type of tag
 * @return true writing successfull
 * @return false writing failed
 */
static bool write_tag(xml_stream_t *stream, xml_tag_t const *tag, enum tag_types_e type)
{
    //close tag only has name
    if (type == TAG_CLOSE)
        return fprintf(stream->target, "</%s>", tag->name) != EOF;

    // printing tag name
    if (fprintf(stream->target, "<%s", tag->name) == EOF)
        return false;
    // printing attributes
    for (size_t i = 0; i < tag->attributes_num; i++)
    {
        // printing attribute name
        if (fprintf(stream->target, " %s=\"", tag->attributes[i].name) == EOF)
            return false;
        // printing escaped attribute value
        if (fprintf(stream->escaped_target, "%s", tag->attributes[i].value) == EOF)
            return false;
        fflush(stream->escaped_target); // force to write on the stream
        // printing end of attribute value
        if (fprintf(stream->target, "\"") == EOF)
            return false;
    }

    // closing tag
    if (type == TAG_EMPTY)
    {
        if (fprintf(stream->target, " />") == EOF)
            return false;
    }
    else
    {
        if (fprintf(stream->target, ">") == EOF)
            return false;
    }

    return true;
}

/**
 * @brief write handler for xml escaping
 * 
 * @param cookie the target file handler
 * @param buf the data to write
 * @param size the size of the data
 * @return ssize_t the number of bytes copied from buf, or 0 on error.
 */
static ssize_t escaped_write(void *cookie, const char *buf, size_t size)
{
    FILE *target = cookie;
    for (size_t i = 0; i < size; i++)
    {
        int ch = (unsigned char)buf[i];

        switch (ch)
        {
            // default xml entity
        case '&':
            if (fprintf(target, "&apm;") == EOF)
                return i;
            break;
        case '<':
            if (fprintf(target, "&lt;") == EOF)
                return i;
            break;
        case '>':
            if (fprintf(target, "&gt;") == EOF)
                return i;
            break;
        case '"':
            if (fprintf(target, "&quot;") == EOF)
                return i;
            break;
        case '\'':
            if (fprintf(target, "&apos;") == EOF)
                return i;
            break;

        default:
            if (isprint(ch))
            {
                // printable chars go directly on stream
                if (fputc(ch, target) == EOF)
                    return i;
            }
            else
            {
                // use hex escape
                if (fprintf(target, "&#x%02x;", ch) == EOF)
                    return i;
            }
            break;
        }
    }

    return size;
}

int xml_stream_putc(int ch, xml_stream_t *stream)
{
    int res = fputc(ch, stream->escaped_target);
    fflush(stream->escaped_target);
    
    return res;
}
int xml_stream_puts(const char *s, xml_stream_t *stream)
{
    int res =  fputs(s, stream->escaped_target);
    fflush(stream->escaped_target);

    return res;
}
int xml_stream_printf(xml_stream_t *stream, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int res = vfprintf(stream->escaped_target, fmt, ap);
    fflush(stream->escaped_target);
    va_end(ap);

    return res;
}
int xml_stream_vprintf(xml_stream_t *stream, const char *fmt, va_list ap)
{
    int res =  vfprintf(stream->escaped_target, fmt, ap);
    fflush(stream->escaped_target);

    return res;
}