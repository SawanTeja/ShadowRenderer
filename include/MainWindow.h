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
    
    // Stack for switching views
    GtkWidget* stack;
    
    // Menu VBox and elements
    GtkWidget* menu_vbox;
    GtkWidget* start_button;
    GtkWidget* settings_button;
    
    // Simulation Box
    GtkWidget* sim_vbox;
    GtkWidget* button;
    GtkWidget* mode_combo;
    GtkWidget* shape_combo;

    GtkWidget* viewModeCheck;
    GtkWidget* color_button;
    GtkWidget* box;
    GtkWidget* gl_area;

    Scene* scene;
    class InputManager* inputManager; // Managed input

    static void on_start_clicked(GtkWidget* widget, gpointer data);
    static void on_settings_clicked(GtkWidget* widget, gpointer data);
    static void on_button_clicked(GtkWidget* widget, gpointer data);
    
    // GL Callbacks
    static void on_realize(GtkGLArea* area, gpointer data);
    static gboolean on_render(GtkGLArea* area, GdkGLContext* context, gpointer data);
    static gboolean on_resize(GtkGLArea* area, gint width, gint height, gpointer data);
    static gboolean on_tick(gpointer data);

    bool is_simulation_running = false;
};

#endif // MAINWINDOW_H
