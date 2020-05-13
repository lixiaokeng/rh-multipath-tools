/*
 * Copyright (C) 2015 Red Hat, Inc.
 *
 * This file is part of the device-mapper multipath userspace tools.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIB_MPATH_VALID_H
#define LIB_MPATH_VALID_H

#ifdef __cpluscplus
extern "C" {
#endif

enum mpath_valid_mode {
	MPATH_DEFAULT,
	MPATH_STRICT,
	MPATH_SMART,
	MPATH_GREEDY,
	MPATH_MAX_MODE,  /* used only for bounds checking */
};

/* MPATH_IS_VALID_NO_CHECK is used to skip checks to see if the device
 * has already been unclaimed by multipath in the past */
enum mpath_valid_result {
	MPATH_IS_ERROR = -1,
	MPATH_IS_NOT_VALID,
	MPATH_IS_VALID,
	MPATH_IS_VALID_NO_CHECK,
	MPATH_IS_MAYBE_VALID,
};

struct config;
extern struct config *mpathvalid_conf;
int mpathvalid_init(int verbosity);
int mpathvalid_exit(void);
unsigned int mpathvalid_get_mode(void);
int mpathvalid_is_path(const char *name, unsigned int mode, char **wwid,
		       const char **path_wwids, unsigned int nr_paths);


#ifdef __cplusplus
}
#endif
#endif /* LIB_PATH_VALID_H */
