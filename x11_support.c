#include "x11_support.h"
#include "protocol.h"

#include <X11/Xlib.h>
#include <X11/X.h>

#include <stdio.h>
#include <stdlib.h>

struct Image get_root_window_data() {
   Display *display = XOpenDisplay(NULL);
   Window root = DefaultRootWindow(display);

   XWindowAttributes gwa;

   XGetWindowAttributes(display, root, &gwa);
   int width = gwa.width;
   int height = gwa.height;

   XImage *image = XGetImage(display,root, 0,0 , width,height,AllPlanes, ZPixmap);

   int array_size = width * height * 3;
   char *array =  malloc(array_size);

   long red_mask = image->red_mask;
   long green_mask = image->green_mask;
   long blue_mask = image->blue_mask;

   for (int x = 0; x < width; x++)
      for (int y = 0; y < height ; y++)
      {
         long pixel = XGetPixel(image,x,y);

         char blue = pixel & blue_mask;
         char green = (pixel & green_mask) >> 8;
         char red = (pixel & red_mask) >> 16;

         array[(x + width * y) * 3] = red;
         array[(x + width * y) * 3+1] = green;
         array[(x + width * y) * 3+2] = blue;
      }

   struct Image im;
   im.data = array;
   im.size = array_size;
   im.width = width;
   im.height = height;
   return im;
}
