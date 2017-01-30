// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* Norin Maxim, 2011, Distributed under GPLv2 Terms */
/* Rewritten by S-trace <S-trace@list.ru>, 2017, Distributed under GPLv2 Terms */

#include <gtk/gtk.h>
#include "crop.h"
#include "gtk_file_manager.h"	// typedef panel
#include "mylib.h"		// TRACE

static int find_top_coord(GdkPixbuf * pixbuf)
{
	guchar *pixels, *current_pixel, border_color;
	int x, y, width, height, rowstride, n_channels;
	int black = 0;		// black pixels count
	int white = 0;		// white pixels count
	int pcm = 0;		// count random color pixel
	int prm = 0;		// count any random pixel
	int x_tmp = 0;		// count for PIXEL_RESET_COUNT

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	y = 0;			// We are scanning 0th row
	for (x = 0; x < width - 1; ++x) {	// Determine border color
		guchar red, green, blue;
		current_pixel = pixels + y * rowstride + x * n_channels;
		red = current_pixel[0];
		green = current_pixel[1];
		blue = current_pixel[2];

		/* Are there many colored pixels on the border? */
		if (red != green || red != blue || green != blue)
			if (++pcm > width * PIXEL_COLOR_MAX / 100)
				return 0;

		if (red < BLACK && green < BLACK && blue < BLACK)
			++black;
		if (red > WHITE && green > WHITE && blue > WHITE)
			++white;
	}

	if (height - white < height * GREY / 100
	    && height - black < height * GREY / 100)
		return 0;

	if (black > white)
		border_color = BLACK;
	else
		border_color = WHITE;

	/* Determine border size */
	if (border_color == WHITE) {
		for (y = 1; y < height; ++y) {	// 0th row already was scanned
			prm = 0;	// Reset counter every row
			for (x = 0; x < width; ++x) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] < WHITE) {
					if (++prm > PIXEL_RANDOM_MAX)
						return y;
					if (x - x_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					x_tmp = x;
				}
			}
		}
		return y;
	} else {
		for (y = 1; y < height; ++y) {	// 0th row already was scanned
			prm = 0;	// Reset counter every row
			for (x = 0; x < width; ++x) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] > BLACK) {
					if (++prm > PIXEL_RANDOM_MAX)
						return y;
					if (x - x_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					x_tmp = x;
				}
			}
		}
		return y;
	}
}

static int find_bottom_coord(GdkPixbuf * pixbuf)
{
	guchar *pixels, *current_pixel, border_color;
	int x, y, width, height, rowstride, n_channels;
	int black = 0;		// black pixels count
	int white = 0;		// white pixels count
	int pcm = 0;		// count random color pixel
	int prm = 0;		// count any random pixel
	int x_tmp = 0;		// count for PIXEL_RESET_COUNT

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	y = height - 1;		// We are scanning last row
	for (x = 0; x < width - 1; ++x) {	// Determine border color
		guchar red, green, blue;
		current_pixel = pixels + y * rowstride + x * n_channels;
		red = current_pixel[0];
		green = current_pixel[1];
		blue = current_pixel[2];

		/* Are there many colored pixels on the border? */
		if (red != green || red != blue || green != blue)
			if (++pcm > width * PIXEL_COLOR_MAX / 100)
				return height;

		if (red < BLACK && green < BLACK && blue < BLACK)
			++black;
		if (red > WHITE && green > WHITE && blue > WHITE)
			++white;
	}

	if (height - white < height * GREY / 100
	    && height - black < height * GREY / 100)
		return height;

	if (black > white)
		border_color = BLACK;
	else
		border_color = WHITE;

	/* Determine border size */
	if (border_color == WHITE) {
		for (y = height - 2; y > 0; --y) {	// Last row already was scanned
			prm = 0;	// Reset counter every row
			for (x = 0; x < width; ++x) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] < WHITE) {
					if (++prm > PIXEL_RANDOM_MAX)
						return y;
					if (x - x_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					x_tmp = x;
				}
			}
		}
		return y;
	} else {
		for (y = height - 2; y > 0; --y) {	// Last row already was scanned
			prm = 0;	// Reset counter every row
			for (x = 0; x < width; ++x) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] > BLACK) {
					if (++prm > PIXEL_RANDOM_MAX)
						return y;
					if (x - x_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					x_tmp = x;
				}
			}
		}
		return y;
	}
}

static int find_left_coord(GdkPixbuf * pixbuf, int top_coord, int bottom_coord)
{
	guchar *pixels, *current_pixel, border_color;
	int x, y, width, height, rowstride, n_channels;
	int black = 0;		// black pixels count
	int white = 0;		// white pixels count
	int pcm = 0;		// count random color pixel
	int prm = 0;		// count any random pixel
	int y_tmp = top_coord;	// count for PIXEL_RESET_COUNT

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	x = 0;			// We are scanning 0th column
	for (y = top_coord; y < bottom_coord; ++y) {	// Determine border color
		guchar red, green, blue;
		current_pixel = pixels + y * rowstride + x * n_channels;
		red = current_pixel[0];
		green = current_pixel[1];
		blue = current_pixel[2];

		/* Are there many colored pixels on the border? */
		if (red != green || red != blue || green != blue)
			if (++pcm > height * PIXEL_COLOR_MAX / 100)
				return 0;

		if (red < BLACK && green < BLACK && blue < BLACK)
			++black;
		if (red > WHITE && green > WHITE && blue > WHITE)
			++white;
	}

	if (height - white < height * GREY / 100
	    && height - black < height * GREY / 100)
		return 0;

	if (black > white)
		border_color = BLACK;
	else
		border_color = WHITE;

	/* Determine border size */
	if (border_color == WHITE) {
		for (x = 1; x < width; ++x) {	// 0th column already was scanned
			prm = 0;	// Reset counter every column
			for (y = top_coord; y < bottom_coord; ++y) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] < WHITE) {
					if (++prm > PIXEL_RANDOM_MAX)
						return x;
					if (y - y_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					y_tmp = y;
				}
			}
		}
		return x;
	} else {
		for (x = 1; x < width; ++x) {	// 0th column already was scanned
			prm = 0;	// Reset counter every column
			for (y = top_coord; y < bottom_coord; ++y) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] > BLACK) {
					if (++prm > PIXEL_RANDOM_MAX)
						return x;
					if (y - y_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					y_tmp = y;
				}
			}
		}
		return x;
	}
}

static int find_right_coord(GdkPixbuf * pixbuf, int top_coord, int bottom_coord)
{
	guchar *pixels, *current_pixel, border_color;
	int x, y, width, height, rowstride, n_channels;
	int black = 0;		// black pixels count
	int white = 0;		// white pixels count
	int pcm = 0;		// count random color pixel
	int prm = 0;		// count any random pixel
	int y_tmp = top_coord;	// count for PIXEL_RESET_COUNT

	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	x = width - 1;		// We are scanning last column
	for (y = top_coord; y < bottom_coord; ++y) {	// Determine border color
		guchar red, green, blue;
		current_pixel = pixels + y * rowstride + x * n_channels;
		red = current_pixel[0];
		green = current_pixel[1];
		blue = current_pixel[2];

		/* Are there many colored pixels on the border? */
		if (red != green || red != blue || green != blue)
			if (++pcm > height * PIXEL_COLOR_MAX / 100)
				return width;

		if (red < BLACK && green < BLACK && blue < BLACK)
			++black;
		if (red > WHITE && green > WHITE && blue > WHITE)
			++white;
	}

	if (height - white < height * GREY / 100
	    && height - black < height * GREY / 100)
		return width;

	if (black > white)
		border_color = BLACK;
	else
		border_color = WHITE;

	/* Determine border size */
	if (border_color == WHITE) {
		for (x = width - 2; x > 0; --x) {	// Last column already was scanned
			prm = 0;	// Reset counter every column
			for (y = top_coord; y < bottom_coord; ++y) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] < WHITE) {
					if (++prm > PIXEL_RANDOM_MAX)
						return x;
					if (y - y_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					y_tmp = y;
				}
			}
		}
		return x;
	} else {
		for (x = width - 2; x > 0; --x) {	// Last column already was scanned
			prm = 0;	// Reset counter every column
			for (y = top_coord; y < bottom_coord; ++y) {
				current_pixel =
				    pixels + y * rowstride + x * n_channels;
				if (current_pixel[0] > BLACK) {
					if (++prm > PIXEL_RANDOM_MAX)
						return x;
					if (y - y_tmp > PIXEL_RESET_COUNT)
						prm = 0;
					y_tmp = y;
				}
			}
		}
		return x;
	}
}

/********************************************************************************
 * input: GdkPixbuf *pixbuf                                                     *
 * output: int coords[4] - cropped image coordinates in format x y width height *
 * return: TRUE if cropping is necessary, FALSE otherwise                       *
 ********************************************************************************/
int find_crop_image_coords(GdkPixbuf * pixbuf, int coords[4])
{
	int left, top, right, bottom, width, height;
	height = gdk_pixbuf_get_height(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);

	coords[0] = 0;
	coords[1] = 0;
	coords[2] = width;
	coords[3] = height;

	top = find_top_coord(pixbuf);
	if (top == height)
		return FALSE;

	bottom = find_bottom_coord(pixbuf);
	if (bottom == 0)
		return FALSE;

	left = find_left_coord(pixbuf, top, bottom);
	if (left == width)
		return FALSE;

	right = find_right_coord(pixbuf, top, bottom);
	if (right == 0)
		return FALSE;

	TRACE("left=%d, top=%d, right=%d, bottom=%d\n", left, top, right,
	      bottom);

	coords[0] = left;
	coords[1] = top;
	coords[2] = right - left;
	coords[3] = bottom - top;
	TRACE("x=%d, y=%d, w=%d, h=%d\n", coords[0], coords[1], coords[2],
	      coords[3]);
	return TRUE;
}
