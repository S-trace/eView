#ifndef __KOBO_HW_H
#define __KOBO_HW_H
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include <sys/ioctl.h>
// #include <stdio.h>
// #include <errno.h>
// #include <string.h>

#define __u32 int

struct mxcfb_rect {
	__u32 top;
	__u32 left;
	__u32 width;
	__u32 height;
};


struct mxcfb_alt_buffer_data {
	void *virt_addr;
	__u32 phys_addr;
	__u32 width;	/* width of entire buffer */
	__u32 height;	/* height of entire buffer */
	struct mxcfb_rect alt_update_region;	/* region within buffer to update */
};


//sizeof() == 68
struct mxcfb_update_data {
	struct mxcfb_rect update_region;
	__u32 waveform_mode;
	__u32 update_mode;
	__u32 update_marker;
	int temp;
	uint flags;
	struct mxcfb_alt_buffer_data alt_buffer_data;
};


struct mxcfb_alt_buffer_data_org {
	__u32 phys_addr;
	__u32 width;	/* width of entire buffer */
	__u32 height;	/* height of entire buffer */
	struct mxcfb_rect alt_update_region;	/* region within buffer to update */
};

//sizeof() == 64
struct mxcfb_update_data_org {
	struct mxcfb_rect update_region;
	__u32 waveform_mode;
	__u32 update_mode;
	__u32 update_marker;
	int temp;
	uint flags;
	struct mxcfb_alt_buffer_data_org alt_buffer_data;
};

#define MXCFB_SEND_UPDATE		_IOW('F', 0x2E, struct mxcfb_update_data)
#define MXCFB_SEND_UPDATE_ORG		_IOW('F', 0x2E, struct mxcfb_update_data_org)
#define MXCFB_WAIT_FOR_UPDATE_COMPLETE	_IOW('F', 0x2F, __u32)

#define UPDATE_MODE_PARTIAL		0x0
#define UPDATE_MODE_FULL		0x1

#define WAVEFORM_MODE_AUTO		257
#define TEMP_USE_AMBIENT                0x1000

#endif // __KOBO_HW_H
