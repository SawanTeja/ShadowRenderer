#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <gtk/gtk.h>
#include "Scene.h"

class MainWindow; // Forward declaration

class InputManager {
public:
    InputManager(Scene* scene, MainWindow* mainWindow);
    virtual ~InputManager();

    // Event Handlers
    gboolean on_key_press(GtkWidget* widget, GdkEventKey* event);
    gboolean on_key_release(GtkWidget* widget, GdkEventKey* event);
    gboolean on_motion_notify(GtkWidget* widget, GdkEventMotion* event);
    gboolean on_button_press(GtkWidget* widget, GdkEventButton* event);
    gboolean on_button_release(GtkWidget* widget, GdkEventButton* event);
    gboolean on_scroll(GtkWidget* widget, GdkEventScroll* event);

    // Callbacks to hook into GTK signals (static wrappers)
    static gboolean on_key_press_callback(GtkWidget* widget, GdkEventKey* event, gpointer data);
    static gboolean on_key_release_callback(GtkWidget* widget, GdkEventKey* event, gpointer data);
    static gboolean on_motion_notify_callback(GtkWidget* widget, GdkEventMotion* event, gpointer data);
    static gboolean on_button_press_callback(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_button_release_callback(GtkWidget* widget, GdkEventButton* event, gpointer data);
    static gboolean on_scroll_callback(GtkWidget* widget, GdkEventScroll* event, gpointer data);

private:
    Scene* scene;
    MainWindow* mainWindow; // To access UI state if needed (like check buttons, combos)

    // State
    int dragIndex;      // -1 = none
    float dragPlaneY;
    
    // View rotation state
    double prevMouseX;
    double prevMouseY;
};

#endif // INPUTMANAGER_H
