#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>
#include "Scene.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();

    void show();

private:
    GtkWidget* window;
    GtkWidget* button;
    GtkWidget* add_light_button;
    GtkWidget* mode_combo;
    GtkWidget* color_button;
    GtkWidget* box;
    GtkWidget* gl_area;

    Scene* scene;

    static void on_button_clicked(GtkWidget* widget, gpointer data);
    static void on_add_light_clicked(GtkWidget* widget, gpointer data);
    
    // GL Callbacks
    static void on_realize(GtkGLArea* area, gpointer data);
    static gboolean on_render(GtkGLArea* area, GdkGLContext* context, gpointer data);
    static gboolean on_resize(GtkGLArea* area, gint width, gint height, gpointer data);
    
    // Mouse interaction
    static gboolean on_motion_notify(GtkWidget* widget, GdkEventMotion* event, gpointer data);
    static gboolean on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data);
};

#endif // MAINWINDOW_H
