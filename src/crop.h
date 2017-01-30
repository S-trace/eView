#define PIXEL_COLOR_MAX 5  /* Percentage of allowed colour pixels in border zone */
#define PIXEL_RANDOM_MAX 5 /* Max consecutive random pixels in border zone */
#define PIXEL_RESET_COUNT 100 /* Count of border pixels to reset random pixels counter */
#define WHITE (guchar)150 /* White is color in range WHITE...255 */
#define BLACK (guchar)40  /* Black is color in range 0...BLACK */
#define GREY 10   /* Max Percentage of non-black and non-white pixels in border zone for analysis */

/********************************************************************************
 * input: GdkPixbuf *pixbuf                                                     *
 * output: int coords[4] - cropped image coordinates in format x y width height *
 * return: TRUE if cropping is necessary, FALSE otherwise                       *
 ********************************************************************************/
int find_crop_image_coords(GdkPixbuf *pixbuf, int coords[4]);
