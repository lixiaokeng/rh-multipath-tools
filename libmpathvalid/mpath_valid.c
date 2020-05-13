#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <libdevmapper.h>
#include <libudev.h>
#include <errno.h>

#include "devmapper.h"
#include "structs.h"
#include "util.h"
#include "config.h"
#include "discovery.h"
#include "wwids.h"
#include "sysfs.h"
#include "mpath_cmd.h"
#include "valid.h"
#include "mpath_valid.h"

static struct config default_config = { .verbosity = -1 };
struct config *mpathvalid_conf = &default_config;

static unsigned int get_conf_mode(struct config *conf)
{
	if (conf->find_multipaths == FIND_MULTIPATHS_SMART)
		return MPATH_SMART;
	if (conf->find_multipaths == FIND_MULTIPATHS_GREEDY)
		return MPATH_GREEDY;
	return MPATH_STRICT;
}

static void set_conf_mode(struct config *conf, unsigned int mode)
{
	if (mode == MPATH_SMART)
		conf->find_multipaths = FIND_MULTIPATHS_SMART;
	else if (mode == MPATH_GREEDY)
		conf->find_multipaths = FIND_MULTIPATHS_GREEDY;
	else
		conf->find_multipaths = FIND_MULTIPATHS_STRICT;
}

unsigned int mpathvalid_get_mode(void)
{
	int mode;
	struct config *conf;

	conf = get_multipath_config();
	if (!conf)
		return -1;
	mode = get_conf_mode(conf);
	put_multipath_config(conf);
	return mode;
}

static int convert_result(int result) {
	switch (result) {
	case PATH_IS_ERROR:
		return MPATH_IS_ERROR;
	case PATH_IS_NOT_VALID:
		return MPATH_IS_NOT_VALID;
	case PATH_IS_VALID:
		return MPATH_IS_VALID;
	case PATH_IS_VALID_NO_CHECK:
		return MPATH_IS_VALID_NO_CHECK;
	case PATH_IS_MAYBE_VALID:
		return MPATH_IS_MAYBE_VALID;
	}
	return MPATH_IS_ERROR;
}

int
mpathvalid_init(int verbosity)
{
	unsigned int version[3];
	struct config *conf;

	default_config.verbosity = verbosity;
	skip_libmp_dm_init();
	conf = load_config(DEFAULT_CONFIGFILE);
	if (!conf)
		return -1;
	conf->verbosity = verbosity;
	if (dm_prereq(version))
		goto fail;
	memcpy(conf->version, version, sizeof(version));

	mpathvalid_conf = conf;
	return 0;
fail:
	free_config(conf);
	return -1;
}

int
mpathvalid_exit(void)
{
	struct config *conf = mpathvalid_conf;

	default_config.verbosity = -1;
	if (mpathvalid_conf == &default_config)
		return 0;
	mpathvalid_conf = &default_config;
	free_config(conf);
	return 0;
}

/*
 * name: name of path to check
 * mode: mode to use for determination. MPATH_DEFAULT uses configured mode
 * info: on success, contains the path wwid
 * paths: array of the returned mpath_info from other claimed paths
 * nr_paths: the size of the paths array
 */
int
mpathvalid_is_path(const char *name, unsigned int mode, char **wwid,
	           const char **path_wwids, unsigned int nr_paths)
{
	struct config *conf;
	int r = MPATH_IS_ERROR;
	unsigned int i;
	struct path *pp;

	if (!name || mode >= MPATH_MAX_MODE)
		return r;

	if (nr_paths > 0 && !path_wwids)
		return r;

	pp = alloc_path();
	if (!pp)
		return r;

	if (wwid) {
		*wwid = (char *)malloc(WWID_SIZE);
		if (!*wwid)
			goto out;
	}

	conf = get_multipath_config();
	if (!conf || conf == &default_config)
		goto out_wwid;
	if (mode != MPATH_DEFAULT)
		set_conf_mode(conf, mode);
	r = convert_result(is_path_valid(name, conf, pp, true));
	put_multipath_config(conf);

	if (r == MPATH_IS_MAYBE_VALID) {
		for (i = 0; i < nr_paths; i++) {
			if (strncmp(path_wwids[i], pp->wwid, WWID_SIZE) == 0) {
				r = MPATH_IS_VALID;
				break;
			}
		}
	}

out_wwid:
	if (wwid) {
		if (r == MPATH_IS_VALID || r == MPATH_IS_VALID_NO_CHECK ||
		    r == MPATH_IS_MAYBE_VALID)
			strlcpy(*wwid, pp->wwid, WWID_SIZE);
		else {
			free(*wwid);
			*wwid = NULL;
		}
	}
out:
	free_path(pp);
	return r;
}
