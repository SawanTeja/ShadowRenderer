#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>
#include "Scene.h"

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();

    void show();

    // Accessors for InputManager
    bool isViewMode() const;
    bool isLightMode() const;
    ShapeType getSelectedShapeType() const;
    Vector3 getSelectedColor() const;

private:
    GtkWidget* window;
    GtkWidget* button;
    GtkWidget* mode_combo;
    GtkWidget* shape_combo;

    GtkWidget* viewModeCheck;
    GtkWidget* color_button;
    GtkWidget* box;
    GtkWidget* gl_area;

    Scene* scene;
    class InputManager* inputManager; // Managed input

    static void on_button_clicked(GtkWidget* widget, gpointer data);
    
    // GL Callbacks
    static void on_realize(GtkGLArea* area, gpointer data);
    static gboolean on_render(GtkGLArea* area, GdkGLContext* context, gpointer data);
    static gboolean on_resize(GtkGLArea* area, gint width, gint height, gpointer data);

};

#endif // MAINWINDOW_H
