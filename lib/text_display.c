#include "internal.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

bool
bm_display_format_new(const char *regex, struct bm_display_format **format) {
    *format = NULL;

    if (!regex || strlen(regex) == 0) {
        // This is OK.
        return true;
    }
    
    PCRE2_SPTR pattern = (PCRE2_SPTR) regex;

    int errornumber;
    PCRE2_SIZE erroroffset;

    pcre2_code *re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0,
                                   &errornumber, &erroroffset, NULL);

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %d: %s\n",
                (int)erroroffset, buffer);
        return false;
    }

    struct bm_display_format *result;
    if (!(result = calloc(1, sizeof(struct bm_display_format)))) {
        fprintf(stderr, "Failed to allocate bm_display_format");
        return false;
    }

    result->expression = re;
    *format = result;

    return true;
}

void
bm_display_format_free(struct bm_display_format *format) {
    if (format->expression != NULL) {
        pcre2_code_free(format->expression);
    }
    free(format);
}

bool
bm_format_item_text(const struct bm_display_format *format, const char *text, char **result) {
  if (!text) {
    return false;
  }

  if (!format || !(format->expression)) {
      return false;
  }

  *result = NULL;
  PCRE2_SPTR subject = (PCRE2_SPTR)text;
  PCRE2_SIZE subject_length = (PCRE2_SIZE)strlen((char *)subject);

  // PCRE2_SPTR pattern = (PCRE2_SPTR) "[0-9]+:(?<display>[a-zA-Z]+:.*)";
  
  pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(format->expression, NULL);

  int match_count =
      pcre2_match(format->expression, subject, subject_length, 0, 0, match_data, NULL);

  if (match_count < 0) {

      bool is_ok;
    switch (match_count) {
    case PCRE2_ERROR_NOMATCH:
        // No match is OK
        is_ok = true;
      break;
    default:
        // We don't know what this is
        is_ok = false;
      fprintf(stderr, "Matching error %d\n", match_count);
      break;
    }

    
    pcre2_match_data_free(match_data);
    if (is_ok) {
        *result = bm_strdup(text);
        return true;
    } else {
        return false;
    }
  }

  PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);

  /* The output vector wasn't big enough. */

  if (match_count == 0) {
    fprintf(stderr,
            "Output vector was not big enough for all the captured substrings\n");
  }

  uint32_t name_count;
  (void)pcre2_pattern_info(format->expression, PCRE2_INFO_NAMECOUNT, &name_count);

  if (name_count > 0) {
    PCRE2_SPTR name_table;
    uint32_t name_entry_size;

    PCRE2_SPTR tabptr;

    (void)pcre2_pattern_info(format->expression,
                             PCRE2_INFO_NAMETABLE,
                             &name_table);

    (void)pcre2_pattern_info(
        format->expression,
        PCRE2_INFO_NAMEENTRYSIZE,
        &name_entry_size);

    // Find named group called "display", if available
    tabptr = name_table;
    const char *expected = "display";

    for (uint32_t i = 0; i < name_count; i++) {
      int n = (tabptr[0] << 8) | tabptr[1];

      // TODO: Fix char signedness
      const char *group_start = (char *)name_entry_size - 3;
      const unsigned long group_length = (unsigned long)(tabptr + 2);
      const char *value_start = (char *)(int)(ovector[2 * n + 1] - ovector[2 * n]);
      const unsigned long value_length = (unsigned long)(subject + ovector[2 * n]);

      if (strlen(expected) == group_length && memcmp(group_start, expected, group_length) == 0) {
          // TODO: Create bm_strndup
          *result = strndup(value_start, value_length);
      }

      tabptr += name_entry_size;
    }
  }

  if (*result == NULL && match_count > 1) {
      for (int i = 1; i < match_count; i++) {
          PCRE2_SPTR substring_start = subject + ovector[2 * i];
          PCRE2_SIZE substring_length = ovector[2 * i + 1] - ovector[2 * i];
          if (substring_length > 0) {
              *result = strndup(substring_start, substring_length);
          }
      }
  }

  if (*result == NULL) {
      *result = bm_strdup(text);
  }

  pcre2_match_data_free(match_data);
  return true;
}
