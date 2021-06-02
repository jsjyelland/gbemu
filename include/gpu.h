#ifndef GPU_H
#define GPU_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <string.h>
#include <stdio.h>
#include <time.h>

#include <gb.h>
#include <cpu.h>

void gpu_update();

// Function that does the drawing
// glut calls this function whenever it needs to redraw
void display();

#endif