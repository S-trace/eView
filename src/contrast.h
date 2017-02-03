#ifndef HAVE_CONTRAST_H
#define HAVE_CONTRAST_H
/**
 * @brief Adjust image contrast to desired value
 *
 * @param target Pixbuf to process
 * @param contrast Desired contrast value (256 - normal)
 * @return void
 *
 * http://habrahabr.ru/post/139428/
 * http://habrahabr.ru/post/304210/
 */
void pixbuf_adjust_contrast(GdkPixbuf * target, int contrast);
#endif //HAVE_CONTRAST_H
