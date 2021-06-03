#include <stdio.h>
#include <cpu.h>
#include <gpu.h>
#include <gb_memory.h>

int main(int argc, char *argv[]) {
    gb_t *gb = get_gb_instance();

    cpu_init(gb);
    mem_init(gb);

    printf("GBEMU begin\r\n");

    mem_load_rom(gb, "roms/pokemon blue.gb");

    // Main tick loop
    while (cpu_tick(gb) != CPU_TICK_ERROR) {
        gpu_update();
    }

    // glutInit(&argc, argv);
    // glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // glutInitWindowPosition(80, 80);
    // glutInitWindowSize(500,500);

    // glutCreateWindow("A Simple OpenGL Program");

    // glClear(GL_COLOR_BUFFER_BIT);
    // glMatrixMode( GL_PROJECTION );
    // glLoadIdentity();
    // gluOrtho2D( 0.0, 500.0, 500.0,0.0 );
    // glutDisplayFunc(display);
    // glutMainLoop();
    
    return 0;
}