#include "MainWindow.h"
#include "InputManager.h"
#include <iostream>
#include <cstring>

MainWindow::MainWindow(GtkApplication* app) : scene(new Scene()), inputManager(nullptr) {
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
    gtk_widget_set_can_focus(gl_area, TRUE); // Important for key events
    
    gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gl_area), TRUE);
    gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gl_area), TRUE);
    
    gtk_box_pack_start(GTK_BOX(box), gl_area, TRUE, TRUE, 0);
    
    // Connect GL signals
    g_signal_connect(gl_area, "realize", G_CALLBACK(on_realize), this);
    g_signal_connect(gl_area, "render", G_CALLBACK(on_render), this);
    g_signal_connect(gl_area, "resize", G_CALLBACK(on_resize), this);
    
    // Initialize InputManager
    inputManager = new InputManager(scene, this);

    // Connect input signals delegates to InputManager
    gtk_widget_add_events(gl_area, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK);
    
    g_signal_connect(gl_area, "motion-notify-event", G_CALLBACK(InputManager::on_motion_notify_callback), inputManager);
    g_signal_connect(gl_area, "button-press-event", G_CALLBACK(InputManager::on_button_press_callback), inputManager);
    g_signal_connect(gl_area, "button-release-event", G_CALLBACK(InputManager::on_button_release_callback), inputManager);
    g_signal_connect(gl_area, "scroll-event", G_CALLBACK(InputManager::on_scroll_callback), inputManager);
    g_signal_connect(window, "key-press-event", G_CALLBACK(InputManager::on_key_press_callback), inputManager); // Window handles keys
}

MainWindow::~MainWindow() {
    delete inputManager;
    delete scene;
}

void MainWindow::show() {
    gtk_widget_show_all(window);
}

// Getters for InputManager
bool MainWindow::isViewMode() const {
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(viewModeCheck));
}

bool MainWindow::isLightMode() const {
    gchar* mode = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(mode_combo));
    bool isLight = (g_strcmp0(mode, "Light") == 0);
    g_free(mode);
    return isLight;
}

ShapeType MainWindow::getSelectedShapeType() const {
    gchar* shapeText = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(shape_combo));
    ShapeType type = SHAPE_CUBE;
    if (shapeText) {
        if (strcmp(shapeText, "Sphere") == 0) type = SHAPE_SPHERE;
        else if (strcmp(shapeText, "Cylinder") == 0) type = SHAPE_CYLINDER;
        else if (strcmp(shapeText, "Cone") == 0) type = SHAPE_CONE;
        else if (strcmp(shapeText, "Tricone") == 0) type = SHAPE_TRICONE;
        g_free(shapeText);
    }
    return type;
}

Vector3 MainWindow::getSelectedColor() const {
    GdkRGBA color;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(color_button), &color);
    return Vector3(color.red, color.green, color.blue);
}

void MainWindow::on_button_clicked(GtkWidget* widget, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    
    Vector3 color = mw->getSelectedColor();
    ShapeType type = mw->getSelectedShapeType();

    mw->scene->addShape(type, color.x, color.y, color.z);
    gtk_widget_queue_draw(mw->gl_area);
}

void MainWindow::on_realize(GtkGLArea* area, gpointer data) {
    MainWindow* mw = static_cast<MainWindow*>(data);
    gtk_gl_area_make_current(area);
    if (gtk_gl_area_get_error(area) != NULL) return;
    
    mw->scene->init();
    
    // Debug: Print GPU info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* version = glGetString(GL_VERSION);
    
    if (renderer) std::cout << "Renderer: " << renderer << std::endl;
    if (vendor) std::cout << "Vendor: " << vendor << std::endl;
    if (version) std::cout << "OpenGL Version: " << version << std::endl;
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

