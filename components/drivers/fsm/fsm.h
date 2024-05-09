/* Mealy FSM public interface

   Copyright (C) 2017 by Jose M. Moya

   This file is part of GreenLSI Unlessons.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef FSM_H
#define FSM_H

typedef struct fsm_t fsm_t;

typedef int (*fsm_input_func_t)(fsm_t *);
typedef void (*fsm_output_func_t)(fsm_t *);

/**
 * @brief Transition structure for the FSM
 *
 */
typedef struct fsm_trans_t
{
  int orig_state;        /**< Original state of the transition */
  fsm_input_func_t in;   /**< Input function, act as a checker */
  int dest_state;        /**< Destination state of the transition */
  fsm_output_func_t out; /**< Output function, act as a handler */
} fsm_trans_t;

/**
 * @brief Finite state machine structure
 *
 */
struct fsm_t
{
  int current_state; /**< Current state of the FSM */
  fsm_trans_t *tt;   /**< Transition table */
};

fsm_t *fsm_new(fsm_trans_t *tt);
void fsm_init(fsm_t *this, fsm_trans_t *tt);
void fsm_fire(fsm_t *this);

#endif
