#include "MainWindow.h"
#include <iostream>
#include <cstring>

MainWindow::MainWindow(GtkApplication* app) : scene(new Scene()), dragIndex(-1), dragPlaneY(0.0f) {
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Basic 3D");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create a box to hold controls and GL area
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Control Box
    GtkWidget* controlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(box), controlBox, FALSE, FALSE, 10);

    // Create Color Button
    color_button = gtk_color_button_new();
    GdkRGBA color;
    gdk_rgba_parse(&color, "blue");
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_button), &color);
    gtk_box_pack_start(GTK_BOX(controlBox), color_button, FALSE, FALSE, 0);
    
    // View Mode Checkbox
    viewModeCheck = gtk_check_button_new_with_label("View Mode");
    gtk_box_pack_start(GTK_BOX(controlBox), viewModeCheck, FALSE, FALSE, 0);
    
    // Mode Combo: Shape Placement or Light
    mode_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Shape");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo), "Light");
    gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo), 0);
    gtk_box_pack_start(GTK_BOX(controlBox), mode_combo, FALSE, FALSE, 0);

    // Shape Type Combo
    shape_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(shape_combo), "Cube");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(shape_combo), "Sphere");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(shape_combo), "Cylinder");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(shape_combo), "Cone");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(shape_combo), "Tricone");
    gtk_combo_box_set_active(GTK_COMBO_BOX(shape_combo), 0);
    gtk_box_pack_start(GTK_BOX(controlBox), shape_combo, FALSE, FALSE, 0);

    // Add Shape button
    button = gtk_button_new_with_label("Add Shape");
    gtk_box_pack_start(GTK_BOX(controlBox), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), this);
    
    // GL Area
    gl_area = gtk_gl_area_new();
    gtk_widget_set_hexpand(gl_area, TRUE);
    gtk_widget_set_vexpand(gl_area, TRUE);
    
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), TRUE);
    
    gtk_box_pack_start(GTK_BOX(box), gl_area, TRUE, TRUE, 0);
    
    // Connect GL signals
    g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), this);
    g_signal_connect(gl_area, "render", G_CALLBACK(on_render), this);
    g_signal_connect(gl_area, "resize", G_CALLBACK(on_resize), this);
    
    // Connect mouse signals to GL area
    gtk_widget_add_events(gl_area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK);
    g_signal_connect(gl_area, "motion-notify-event", G_CALLBACK(on_motion_notify), this);
    g_signal_connect(gl_area, "button-press-event", G_CALLBACK(on_button_press), this);
    g_signal_connect(gl_area, "button-release-event", G_CALLBACK(on_button_release), this);
    g_signal_connect(gl_area, "scroll-event", G_CALLBACK(on_scroll), this);
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
    
    gchar* shapeText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mw->shape_combo));
    ShapeType type = SHAPE_CUBE;
    if (shapeText) {
        if (strcmp(shapeText, "Sphere") == 0) type = SHAPE_SPHERE;
        else if (strcmp(shapeText, "Cylinder") == 0) type = SHAPE_CYLINDER;
        else if (strcmp(shapeText, "Cone") == 0) type = SHAPE_CONE;
        else if (strcmp(shapeText, "Tricone") == 0) type = SHAPE_TRICONE;
        g_free(shapeText);
    }

    mw->scene->addShape(type, color.red, color.green, color.blue);
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

gboolean MainWindow::on_scroll(GtkWidget* widget, GdkEventScroll* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->viewModeCheck))) return TRUE;

    if (event->direction == GDK_SCROLL_UP) {
        mw->scene->zoomCamera(1.0f);
    } else if (event->direction == GDK_SCROLL_DOWN) {
        mw->scene->zoomCamera(-1.0f);
    }
    
    gtk_widget_queue_draw(mw->gl_area);
    return TRUE;
}

gboolean MainWindow::on_motion_notify(GtkWidget* widget, GdkEventMotion* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    if (mw->dragIndex < 0 && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->viewModeCheck))) return TRUE;
    
    if (!(event->state & GDK_BUTTON1_MASK)) {
        mw->dragIndex = -1;
        return TRUE;
    }
    
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    
    // Handle View Mode Rotation
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->viewModeCheck))) {
        static double prevX = 0;
        static double prevY = 0;
        
        double curX = event->x;
        double curY = event->y;
        
        double dx = curX - prevX;
        double dy = curY - prevY;
        
        prevX = curX;
        prevY = curY;
        
        if (abs(dx) > 100 || abs(dy) > 100) return TRUE; 
        
        mw->scene->rotateCamera(dx * 0.01f, dy * 0.01f);
        gtk_widget_queue_draw(mw->gl_area);
        return TRUE;
    }

    if (mw->dragIndex < 0) return TRUE;

    float screenX = (float)event->x / w;
    float screenY = (float)event->y / h;
    
    float eyeX, eyeY, eyeZ;
    mw->scene->getCameraPosition(eyeX, eyeY, eyeZ);
    
    float worldX, worldZ;
    if (unprojectScreenToFloor(screenX, screenY, mw->dragPlaneY, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
        int lightIdx = mw->scene->shapeCount();
        if (mw->dragIndex == lightIdx) {
            // Dragging the light
            mw->scene->setLightWorldPos(worldX, mw->dragPlaneY, worldZ);
        } else {
            // Dragging a shape
            mw->scene->moveShape(mw->dragIndex, worldX, worldZ);
        }
        gtk_widget_queue_draw(mw->gl_area);
    }
    
    return TRUE;
}

gboolean MainWindow::on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    if (event->button != 1) return TRUE; 
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->viewModeCheck))) {
        return TRUE; 
    }
    
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    
    float screenX = (float)event->x / w;
    float screenY = (float)event->y / h;
    
    float eyeX, eyeY, eyeZ;
    mw->scene->getCameraPosition(eyeX, eyeY, eyeZ);
    
    // --- Step 1: try to pick an existing object ---
    int hit = mw->scene->pickObject(screenX, screenY, w, h);
    
    if (hit >= 0) {
        mw->scene->setSelected(hit);
        mw->dragIndex = hit;

        int lightIdx = mw->scene->shapeCount();
        if (hit == lightIdx) {
            mw->dragPlaneY = mw->scene->getLightPosition().y;
        } else {
            mw->dragPlaneY = mw->scene->getShapePosition(hit).y;
        }

        gtk_widget_queue_draw(mw->gl_area);
        return TRUE;
    }
    
    // --- Step 2: nothing was hit — placement logic ---
    mw->scene->setSelected(-1);

    gchar* mode = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mw->mode_combo));
    bool isLightMode = (g_strcmp0(mode, "Light") == 0);
    g_free(mode);
    
    if (isLightMode) {
        float defaultLightY = 3.0f;
        float worldX, worldZ;
        if (unprojectScreenToFloor(screenX, screenY, defaultLightY, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
            mw->scene->setLightWorldPos(worldX, defaultLightY, worldZ);
            mw->scene->setSelected(mw->scene->shapeCount()); // light index
            mw->dragIndex = mw->scene->shapeCount();
            mw->dragPlaneY = defaultLightY;
            gtk_widget_queue_draw(mw->gl_area);
        }
    } else {
        // Shape placement mode
        GdkRGBA color;
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(mw->color_button), &color);
        
        gchar* shapeText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mw->shape_combo));
        ShapeType type = SHAPE_CUBE;
        if (shapeText) {
            if (strcmp(shapeText, "Sphere") == 0) type = SHAPE_SPHERE;
            else if (strcmp(shapeText, "Cylinder") == 0) type = SHAPE_CYLINDER;
            else if (strcmp(shapeText, "Cone") == 0) type = SHAPE_CONE;
            else if (strcmp(shapeText, "Tricone") == 0) type = SHAPE_TRICONE;
            g_free(shapeText);
        }
        
        float worldX, worldZ;
        if (unprojectScreenToFloor(screenX, screenY, 0.0f, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
            mw->scene->addShapeAt(type, worldX, worldZ, color.red, color.green, color.blue);
            gtk_widget_queue_draw(mw->gl_area);
        }
    }
    
    return TRUE;
}

gboolean MainWindow::on_button_release(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    if (event->button == 1) {
        mw->dragIndex = -1;
    }
    
    return TRUE;
}
