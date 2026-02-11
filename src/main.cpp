#include <gtk/gtk.h>
#include <cstdlib>
#include "MainWindow.h"

static void activate(GtkApplication* app, gpointer user_data) {
    auto* window = new MainWindow(app);
    window->show();
}

int main(int argc, char **argv) {
    // Force GDK to use a legacy (compatibility) OpenGL context.
    // GTK3's GtkGLArea defaults to a core profile which does NOT support
    // the fixed-function pipeline (glBegin/glEnd, GL_LIGHTING, etc.).
    // Without this, all legacy GL calls silently fail -> blank white screen.
    setenv("GDK_GL", "legacy", 1);

    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.basic", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
