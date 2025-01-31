#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>    /* HUGE_VAL DEFINATION*/
#include <errno.h>   /* NUMBER TOO BIG */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#define IS_DIGIT1TO9(ch) (ch > '0' && ch <= '9' )
#define IS_DIGIT(ch) (ch >= '0' && ch <='9')

/*
* 按照number的语法类型解析c风格字符串c->json是否符合json里number的语法
* 不同的编译器对double类型的相等判定方式不同
* 最好按照两数差小于等于某个值来判定double是否相同
*/

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    char* begin = c->json;
    /* \TODO validate number */
    if (*begin == '-')++begin;
    if (*begin == '0')++begin;
    else
    {
        if (!IS_DIGIT1TO9(*begin))return LEPT_PARSE_INVALID_VALUE;
        else{
            ++begin;
            while (IS_DIGIT(*begin))++begin;
        }
    }
    if (*begin == '.') {
        ++begin;
        if (!IS_DIGIT(*begin))return LEPT_PARSE_INVALID_VALUE;
        while (IS_DIGIT(*begin))++begin;
    }
    if (*begin == 'e' || *begin == 'E') {
        ++begin;
        if (*begin == '-' || *begin == '+')++begin;
        if (!IS_DIGIT(*begin))return LEPT_PARSE_INVALID_VALUE;
        while (IS_DIGIT(*begin))++begin;
    }
    v->n = strtod(c->json, &end);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        errno = 0;
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    /* 0123 bug here */
    /* c->json = end; */
    c->json = begin;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
