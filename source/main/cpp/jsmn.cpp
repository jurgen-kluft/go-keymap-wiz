#include "jsmn/jsmn.h"
#include <stddef.h>

//#define JSMN_STRICT
#define JSMN_API static

// Need to make sure it can read UTF-8 text

static inline int sequence_sizeof_utf8(unsigned int c)
{
    if (c < 0x80)
        return 1;
    else if ((c >> 5) == 0x6)
        return 2;
    else if ((c >> 4) == 0xe)
        return 3;
    else if ((c >> 3) == 0x1e)
        return 4;
    return 0;
}

// UTF-8; read a character
static unsigned int jsmn_read(const char *&str, const char *end)
{
    unsigned int c = *str;
    if (c != 0)
    {
        ++str;
        if (c < 0x80)
        {
        }
        else if ((c >> 5) == 0x6)
        {
            c = ((c << 6) & 0x7ff) + ((str[1]) & 0x3f);
            ++str;
        }
        else if ((c >> 4) == 0xe)
        {
            c = ((c << 12) & 0xffff) + (((str[1]) << 6) & 0xfff);
            c += (str[2]) & 0x3f;
            str += 2;
        }
        else if ((c >> 3) == 0x1e)
        {
            c = ((c << 18) & 0x1fffff) + (((str[1]) << 12) & 0x3ffff);
            c += ((str[2]) << 6) & 0xfff;
            c += (str[3]) & 0x3f;
            str += 3;
        }
        else
        {
            c = '?'; // Illegal character in utf8, replace with '?'
        }
    }
    return c;
}

/**
 * Constructs a new token and initializes it with type and boundaries.
 */
static inline jsmntok_t *jsmn_construct_token(jsmn_parser *parser, const jsmntype_t type, const int start, const int end, const int parent)
{
    jsmntok_t *token;
    token = &parser->tokens[parser->toknext++];
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
    token->parent = parent;
    return token;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js, const size_t len)
{
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        switch (js[parser->pos])
        {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        default:
            /* to quiet a warning from gcc*/
            break;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127)
        {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (parser->toknext >= parser->max_tokens)
    {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    token = jsmn_construct_token(parser, JSMN_PRIMITIVE, start, parser->pos, parser->toksuper);
    parser->pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js, const size_t len)
{
    jsmntok_t *token;

    int start = parser->pos;

    /* Skip starting quote */
    parser->pos++;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"')
        {
            if (parser->toknext >= parser->max_tokens)
            {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_construct_token(parser, JSMN_STRING, start + 1, parser->pos, parser->toksuper);
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->pos + 1 < len)
        {
            int i;
            parser->pos++;
            switch (js[parser->pos])
            {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                parser->pos++;
                for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++)
                {
                    /* If it isn't a hex character we have an error */
                    if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
                          (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
                          (js[parser->pos] >= 97 && js[parser->pos] <= 102)))
                    { /* a-f */
                        parser->pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    parser->pos++;
                }
                parser->pos--;
                break;
            /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
JSMN_API int jsmn_parse(jsmn_parser *parser, const char *js, const size_t len)
{
    int r;
    int i;
    jsmntok_t *token;
    int count = parser->toknext;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        char c;
        jsmntype_t type;

        c = js[parser->pos];
        switch (c)
        {
        case '{':
        case '[':
            count++;
            if (parser->toknext >= parser->max_tokens)
            {
                return JSMN_ERROR_NOMEM;
            }
            int parent = -1;
            if (parser->toksuper != -1)
            {
                jsmntok_t *t = &parser->tokens[parser->toksuper];
#ifdef JSMN_STRICT
                /* In strict mode an object or array can't become a key */
                if (t->type == JSMN_OBJECT)
                {
                    return JSMN_ERROR_INVAL;
                }
#endif
                t->size++;
                parent = parser->toksuper;
            }
            token = jsmn_construct_token(parser, (c == '{' ? JSMN_OBJECT : JSMN_ARRAY), parser->pos, -1, parent);
            parser->toksuper = parser->toknext - 1;
            break;
        case '}':
        case ']':
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
            if (parser->toknext < 1)
            {
                return JSMN_ERROR_INVAL;
            }
            token = &parser->tokens[parser->toknext - 1];
            for (;;)
            {
                if (token->start != -1 && token->end == -1)
                {
                    if (token->type != type)
                    {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = parser->pos + 1;
                    parser->toksuper = token->parent;
                    break;
                }
                if (token->parent == -1)
                {
                    if (token->type != type || parser->toksuper == -1)
                    {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &parser->tokens[token->parent];
            }
            break;
        case '\"':
            r = jsmn_parse_string(parser, js, len);
            if (r < 0)
            {
                return r;
            }
            count++;
            if (parser->toksuper != -1)
            {
                parser->tokens[parser->toksuper].size++;
            }
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            parser->toksuper = parser->toknext - 1;
            break;
        case ',':
            if (parser->toksuper != -1 && parser->tokens[parser->toksuper].type != JSMN_ARRAY && parser->tokens[parser->toksuper].type != JSMN_OBJECT)
            {
                parser->toksuper = parser->tokens[parser->toksuper].parent;
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (tokens != NULL && parser->toksuper != -1)
            {
                const jsmntok_t *t = &tokens[parser->toksuper];
                if (t->type == JSMN_OBJECT || (t->type == JSMN_STRING && t->size != 0))
                {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = jsmn_parse_primitive(parser, js, len);
            if (r < 0)
            {
                return r;
            }
            count++;
            if (parser->toksuper != -1)
            {
                parser->tokens[parser->toksuper].size++;
            }
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    for (i = parser->toknext - 1; i >= 0; i--)
    {
        /* Unmatched opened object or array */
        if (parser->tokens[i].start != -1 && parser->tokens[i].end == -1)
        {
            return JSMN_ERROR_PART;
        }
    }

    return count;
}

/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
JSMN_API void jsmn_init(jsmn_parser *parser)
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}
