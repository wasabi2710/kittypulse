#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Create a circular region
XserverRegion create_circle_region(Display *dpy, int x, int y, int radius) {
    XserverRegion region = XFixesCreateRegion(dpy, NULL, 0);
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx * dx + dy * dy <= radius * radius) {
                XRectangle rect = { x + dx, y + dy, 1, 1 };
                XFixesUnionRegion(dpy, region, region, XFixesCreateRegion(dpy, &rect, 1));
            }
        }
    }
    return region;
}

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open X display\n");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    // Create a Bigger Window (800x600)
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = 0; // Transparent background

    int win_width = 800, win_height = 600;
    Window win = XCreateWindow(dpy, root, 100, 100, win_width, win_height, 0,
        CopyFromParent, InputOutput, CopyFromParent,
        CWOverrideRedirect | CWBackPixel, &attrs);

    // Make Entire Window Click-Through
    XserverRegion region = XFixesCreateRegion(dpy, NULL, 0);
    XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, region);
    XFixesDestroyRegion(dpy, region);

    // Make a Clickable Circle in the Center
    int circle_radius = 100;
    XserverRegion clickable = create_circle_region(dpy, win_width / 2, win_height / 2, circle_radius);
    XFixesSetWindowShapeRegion(dpy, win, ShapeInput, 0, 0, clickable);
    XFixesDestroyRegion(dpy, clickable);

    // Draw the Circle (Red)
    GC gc = XCreateGC(dpy, win, 0, NULL);
    XSetForeground(dpy, gc, 0xff0000); // Red color
    XFillArc(dpy, win, gc, (win_width / 2) - circle_radius, (win_height / 2) - circle_radius, 
             circle_radius * 2, circle_radius * 2, 0, 360 * 64);

    XMapWindow(dpy, win);
    //baljh

    // Keep Running
    XEvent ev;
    while (1) {
        XNextEvent(dpy, &ev);
    }

    XCloseDisplay(dpy);
    return 0;
}
