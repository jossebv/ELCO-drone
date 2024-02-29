/**
 * @file comb_filter.h
 * @author Jose Manuel Bravo
 * @brief Applies the combinatory filter to the input data.
 * @version 0.1
 * @date 2024-02-28
 *
 * @copyright Copyright (c) 2024
 *
 */

/* TYPEDEFS */

/**
 * @brief Type for storing the angles of the drone
 *
 */
typedef struct drone_angles_t
{
  double pitch;
  double roll;
} drone_angles_t;

void comb_filter_init();
drone_angles_t comb_filter_get_angles(drone_angles_t gyros_delta_angle, drone_angles_t acc_angle);
