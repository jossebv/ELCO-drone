/**
 * @file system.h
 * @author Jose Manuel Bravo
 * @brief system.c header file. Contains all types and methods for system initialization.
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "fsm.h"

void system_task();
fsm_t *system_fsm_create();

#endif