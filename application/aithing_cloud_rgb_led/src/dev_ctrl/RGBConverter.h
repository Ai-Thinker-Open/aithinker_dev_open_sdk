/*
  * AI-THINKER MIT License
  *
  * Copyright (c) 2017 <AI-THINKER SYSTEMS (SHENZHEN) PTE LTD>
  *
  * Permission is hereby granted for use on AI-THINKER SYSTEMS products only, in which case,
  * it is free of charge, to any person obtaining a copy of this software and associated
  * documentation files (the "Software"), to deal in the Software without restriction, including
  * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
  * to do so, subject to the following conditions:
  *
  *
  * The above copyright notice and this permission notice shall be included in all copies or
  * substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  *
  */
#ifndef _RGB_H_
#define _RGB_H_

#ifdef __cplusplus
extern "C" {
#endif
//transplant freertos --> rgb2hsv.h
/**
  * Converts an RGB color value to HSL. Conversion formula
  * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
  * Assumes r, g, and b are contained in the set [0, 255] and
  * returns h, s, and l in the set [0, 1].
  *
  * @param   byte   r       The red color value
  * @param   byte   g       The green color value
  * @param   byte   b       The blue color value
  * @param   double hsl[]   The HSL representation
  */
void rgbToHsl(char r, char g, char b, double hsl[]);

/**
  * Converts an HSL color value to RGB. Conversion formula
  * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
  * Assumes h, s, and l are contained in the set [0, 1] and
  * returns r, g, and b in the set [0, 255].
  *
  * @param   double h       The hue
  * @param   double s       The saturation
  * @param   double l       The lightness
  * @return byte   rgb[]   The RGB representation
  */
void hslToRgb(double h, double s, double l, char rgb[]);
/**
  * Converts an RGB color value to HSV. Conversion formula
  * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
  * Assumes r, g, and b are contained in the set [0, 255] and
  * returns h, s, and v in the set [0, 1].
  *
  * @param   byte r       The red color value
  * @param   byte g       The green color value
  * @param   byte b       The blue color value
  * @return double hsv[] The HSV representation
  */
void rgbToHsv(char r, char g, char b, double hsv[]);

/**
  * Converts an HSV color value to RGB. Conversion formula
  * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
  * Assumes h, s, and v are contained in the set [0, 1] and
  * returns r, g, and b in the set [0, 255].
  *
  * @param   double h       The hue
  * @param   double s       The saturation
  * @param   double v       The value
  * @return byte   rgb[]   The RGB representation
  */
void hsvToRgb(double h, double s, double v, char rgb[]);


double threeway_max(double a, double b, double c);
double threeway_min(double a, double b, double c);
double hue2rgb(double p, double q, double t);
void HSV2RGB(unsigned char *r, unsigned char *g, unsigned char *b, int h, int s, int v);

#ifdef __cplusplus
}
#endif

#endif  //_RGB_H_
