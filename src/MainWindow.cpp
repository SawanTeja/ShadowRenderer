#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(GtkApplication* app) : scene(new Scene()){
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Basic 3D");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Initial background color (white) - actually managing GL area now
    // GdkRGBA color;
    // gdk_rgba_parse(&color, "#ffffff");
    // gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &color);

    // Create a box to hold the button and GL area
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Control Box
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box), controlBox, FALSE, FALSE, 10);

    // Create Color Button
    color_button = gtk_color_button_new();
    GdkRGBA color;
    gdk_rgba_parse(&color, "blue"); // Default Blue to match logic
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_button), &color);
    gtk_box_pack_start(GTK_BOX(controlBox), color_button, FALSE, FALSE, 0);
    
    // Mode Combo
    mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Cube");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Light");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
    gtk_box_pack_start(GTK_BOX(controlBox), mode_combo, FALSE, FALSE, 0);

    // Create Add Light button
    add_light_button = gtk_button_new_with_label("Add Light");
    gtk_box_pack_start(GTK_BOX(controlBox), add_light_button, FALSE, FALSE, 0);
    g_signal_connect(add_light_button, "clicked", G_CALLBACK(on_add_light_clicked), this);

    // Create a button
    button = gtk_button_new_with_label("Add Cube");
    gtk_box_pack_start(GTK_BOX(controlBox), button, TRUE, TRUE, 0);

    // Connect button signal
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), this); // Passing 'this' now
    
    // GL Area
    gl_area = gtk_gl_area_new();
    gtk_widget_set_hexpand(gl_area, TRUE);
    gtk_widget_set_vexpand(gl_area, TRUE);
    
    // Request a depth buffer - CRITICAL for 3D
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), FALSE); // Not strictly needed for this shadow method but good practice to be explicit
    
    gtk_box_pack_start(GTK_BOX(box), gl_area, TRUE, TRUE, 0);
    
    // Connect GL signals
    g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), this);
    g_signal_connect(gl_area, "render", G_CALLBACK(on_render), this);
    g_signal_connect(gl_area, "resize", G_CALLBACK(on_resize), this);
    
    // Connect mouse signals to GL area
    gtk_widget_add_events(gl_area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
    g_signal_connect(gl_area, "motion-notify-event", G_CALLBACK(on_motion_notify), this);
    g_signal_connect(gl_area, "button-press-event", G_CALLBACK(on_button_press), this);
}

MainWindow::~MainWindow() {
    delete scene;
}

void MainWindow::show() {
    gtk_widget_show_all(window);
}

void MainWindow::on_button_clicked(GtkWidget* widget, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(mw->color_button), &color);
    
    mw->scene->addCube(color.red, color.green, color.blue);
    gtk_widget_queue_draw(mw->gl_area);
}

void MainWindow::on_add_light_clicked(GtkWidget* widget, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    mw->scene->addLight();
    gtk_widget_queue_draw(mw->gl_area);
}

void MainWindow::on_realize(GtkGLArea* area, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL) return;
    
    mw->scene->init();
}

gboolean MainWindow::on_render(GtkGLArea* area, GdkGLContext* context, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    mw->scene->render();
    return TRUE;
}

gboolean MainWindow::on_resize(GtkGLArea* area, gint width, gint height, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    mw->scene->resize(width, height);
    return TRUE;
}

gboolean MainWindow::on_motion_notify(GtkWidget* widget, GdkEventMotion* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    // Normalize coordinates to 0..1
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    
    float x = (float)event->x / w;
    float y = (float)event->y / h;
    
    // Update light position only if mouse button 1 is pressed (drag)
    // Or just update always? "place light source anywhere"
    // Let's do it on drag or click.
    if (event->state & GDK_BUTTON1_MASK) {
        mw->scene->setLightPosition(x, y);
        gtk_widget_queue_draw(mw->gl_area);
    }
    
    return TRUE;
}

gboolean MainWindow::on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    
    float screenX = (float)event->x / w;
    float screenY = (float)event->y / h;
    
    gchar* mode = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mw->mode_combo));
    bool isCube = (g_strcmp0(mode, "Cube") == 0);
    g_free(mode);
    
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(mw->color_button), &color);
    
    float worldX, worldZ;
    
    if (isCube) {
        // Cubes sit on the floor (Y=0), so ray-cast to the floor plane
        if (unprojectScreenToFloor(screenX, screenY, 0.0f, w, h, worldX, worldZ)) {
            mw->scene->addCubeAt(worldX, worldZ, color.red, color.green, color.blue);
            gtk_widget_queue_draw(mw->gl_area);
        }
    } else {
        // Lights: ray-cast to a plane at Y=3.0 so the light appears
        // visually at the exact screen position where the user clicked
        if (unprojectScreenToFloor(screenX, screenY, 3.0f, w, h, worldX, worldZ)) {
            mw->scene->addLightAt(worldX, worldZ, color.red, color.green, color.blue);
            gtk_widget_queue_draw(mw->gl_area);
        }
    }
    
    return TRUE;
}
