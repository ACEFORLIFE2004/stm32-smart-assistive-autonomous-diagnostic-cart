/*
 * project_assert.c
 *
 *  Created on: Mar 19, 2026
 *      Author: arnol
 */

#include "project_assert.h"

void project_assert_failed(const char *file, uint32_t line)
{
    print_msg("  <!!> Project assert failed:\r\n       file '%s' line %lu\r\n", file, line);

//    while (1) {
//    }
}
