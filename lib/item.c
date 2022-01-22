#include "internal.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct bm_item*
bm_item_new(const struct bm_menu *menu, const char *text)
{
    struct bm_item *item;
    if (!(item = calloc(1, sizeof(struct bm_item))))
        return NULL;

    bm_item_set_text(item, text, menu->display_format);
    return item;
}

void
bm_item_free(struct bm_item *item)
{
    assert(item);

    if (item->text != item->source_text) {
        free(item->source_text);
    }

    free(item->text);
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
bm_item_set_text(struct bm_item *item, const char *text, const struct bm_display_format *format)
{
    assert(item);

    // TODO don't nest setters, find a better way
    if (!bm_item_set_source_text(item, text)) {
        return false;
    }

    char *result = NULL;
    if (format != NULL) {
        if (!bm_format_item_text(format, text, &result)) {
            return false;
        }
    }

    /* char *copy = NULL; */
    /* if (text && !(copy = bm_strdup(text))) */
    /*     return false; */

    free(item->text);
    if (result) {
        item->text = result;
    } else if (text) {
        // TODO: Review original logic, this used to work without null checks
        // Also bm_item_free doesn't null check, so it seems an item should
        // never have its text set to null.
        item->text = bm_strdup(text);
    }
    
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
    // TODO: Avoid copy when text and source_text are the same
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
