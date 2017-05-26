#ifndef _FB_H_
#define _FB_H_

int fb_init(void);
void draw_back(unsigned int width, unsigned int height, unsigned int color);
void draw_pic(unsigned int x, unsigned int y, unsigned int color);
int fb_close(void);

#endif /* _FB_H_ */