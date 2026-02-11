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

    // Create a button
    button = gtk_button_new_with_label("Add Cube");
    gtk_box_pack_start(GTK_BOX(controlBox), button, TRUE, TRUE, 0);

    // Connect button signal
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), this); // Passing 'this' now
    
    // GL Area
    gl_area = gtk_gl_area_new();
    gtk_widget_set_hexpand(gl_area, TRUE);
    gtk_widget_set_vexpand(gl_area, TRUE);
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
    mw->scene->addCube();
    gtk_widget_queue_draw(mw->gl_area);
}

void MainWindow::on_realize(GtkGLArea* area, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL) return;
    
    mw->scene->init();
}

void MainWindow::on_render(GtkGLArea* area, GdkGLContext* context, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    mw->scene->render();
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
    
    float x = (float)event->x / w;
    float y = (float)event->y / h;
    
    mw->scene->setLightPosition(x, y);
    gtk_widget_queue_draw(mw->gl_area);
    
    return TRUE;
}
