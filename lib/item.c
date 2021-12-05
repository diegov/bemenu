#include "internal.h"
#include "text_display.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct bm_item*
bm_item_new(const char *text)
{
    struct bm_item *item;
    if (!(item = calloc(1, sizeof(struct bm_item))))
        return NULL;

    bm_item_set_text(item, text);
    return item;
}

void
bm_item_free(struct bm_item *item)
{
    assert(item);
    free(item->text);
    free(item->source_text);
    free(item);
}

void
bm_item_set_userdata(struct bm_item *item, void *userdata)
{
    assert(item);
    item->userdata = userdata;
}

void*
bm_item_get_userdata(struct bm_item *item)
{
    assert(item);
    return item->userdata;
}

bool
bm_item_set_text(struct bm_item *item, const char *text)
{
    assert(item);

    // TODO don't nest setters, find a better way
    if (!bm_item_set_source_text(item, text)) {
        return false;
    }

    char *result = NULL;
    if (!format_string(text, &result)) {
        return false;
    }

    /* char *copy = NULL; */
    /* if (text && !(copy = bm_strdup(text))) */
    /*     return false; */

    free(item->text);
    item->text = result;

    return true;
}

const char*
bm_item_get_text(const struct bm_item *item)
{
    assert(item);
    return item->text;
}

bool
bm_item_set_source_text(struct bm_item *item, const char *source_text)
{
    assert(item);

    char *copy = NULL;
    if (source_text && !(copy = bm_strdup(source_text)))
        return false;

    free(item->source_text);
    item->source_text = copy;
    return true;
}

const char*
bm_item_get_source_text(const struct bm_item *item)
{
    assert(item);
    return item->source_text;
}

/* vim: set ts=8 sw=4 tw=0 :*/
