#include <gpu.h>

static uint32_t gpu_counter = 0;

static uint32_t last_cpu_ticks = 0;

void gpu_update() {

    gpu_counter += (get_ticks() - last_cpu_ticks);
    last_cpu_ticks = get_ticks();

    if (gpu_counter > 456) {
        gpu_counter = 0;
        get_gb_instance()->io_registers[0x44] = get_gb_instance()->io_registers[0x44] + 1;

        if (get_gb_instance()->io_registers[0x44] > 153) {
            get_gb_instance()->io_registers[0x44] = 0;
        }
    }

    if (gpu_counter >= 70224) {
        gpu_counter = 0;
    }
}

// Function that does the drawing
// glut calls this function whenever it needs to redraw
extern void display() {
    
    // clear the color buffer before each drawing
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, 500.0, 500.0,0.0 );

    glBegin(GL_POINTS);
    glColor3f(1,1,1);
    glVertex2i(100,100);
    glEnd();

    // swap the buffers and hence show the buffers
    // content to the screen
    glutSwapBuffers();
}