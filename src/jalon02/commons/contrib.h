/*
 * contrib.h
 *
 *  Created on: Sep 26, 2014
 *      Author: nicolas
 */

#ifndef CONTRIB_H_
#define CONTRIB_H_

#include <unistd.h>

/**
 * function to read a line on a fd
 */
ssize_t readline(int fd, void *str, size_t maxlen);

#endif /* CONTRIB_H_ */
