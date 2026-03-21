/*
 * project_assert.h
 *
 *  Created on: Mar 19, 2026
 *      Author: arnol
 */

#ifndef TESTS_PROJECT_ASSERT_H_
#define TESTS_PROJECT_ASSERT_H_

#include "project.h"

#define ENABLE_PROJECT_ASSERT

void project_assert_failed(const char *file, uint32_t line);

#ifdef ENABLE_PROJECT_ASSERT
#define project_assert(expr) ((expr) ? (void)0U : project_assert_failed(__FILE__, __LINE__))
#else
#define project_assert(expr) ((void)0U)
#endif

#endif /* TESTS_PROJECT_ASSERT_H_ */
