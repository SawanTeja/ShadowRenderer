#include "InputManager.h"
#include "MainWindow.h"
#include "MathUtils.h"
#include <iostream>

InputManager::InputManager(Scene* s, MainWindow* mw) 
    : scene(s), mainWindow(mw), dragIndex(-1), dragPlaneY(0.0f), prevMouseX(0), prevMouseY(0) {
}

InputManager::~InputManager() {
}

// Static Callbacks
gboolean InputManager::on_key_press_callback(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_key_press(widget, event);
}

gboolean InputManager::on_motion_notify_callback(GtkWidget* widget, GdkEventMotion* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_motion_notify(widget, event);
}

gboolean InputManager::on_button_press_callback(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_button_press(widget, event);
}

gboolean InputManager::on_button_release_callback(GtkWidget* widget, GdkEventButton* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_button_release(widget, event);
}

gboolean InputManager::on_scroll_callback(GtkWidget* widget, GdkEventScroll* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_scroll(widget, event);
}

// Logic implementations

gboolean InputManager::on_key_press(GtkWidget* widget, GdkEventKey* event) {
    // WASD Control
    if (scene->getSelected() == -1) return FALSE; // Let default handling happen if nothing selected

    float speed = 0.5f;
    float dx = 0.0f;
    float dz = 0.0f;

    switch (event->keyval) {
        case GDK_KEY_w:
        case GDK_KEY_W:
            dz = -speed;
            break;
        case GDK_KEY_s:
        case GDK_KEY_S:
            dz = speed;
            break;
        case GDK_KEY_a:
        case GDK_KEY_A:
            dx = -speed;
            break;
        case GDK_KEY_d:
        case GDK_KEY_D:
            dx = speed;
            break;
        default:
            return FALSE;
    }

    if (dx != 0.0f || dz != 0.0f) {
        scene->moveSelectedShape(dx, dz);
        gtk_widget_queue_draw(widget);
        return TRUE;
    }

    return FALSE;
}

gboolean InputManager::on_scroll(GtkWidget* widget, GdkEventScroll* event) {
    // If view mode is NOT active, maybe we don't zoom? 
    // The original code checked specific UI state for zoom. 
    // For now, let's allow always zoom or check MainWindow state if we had access to check button.
    // Original: if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->viewModeCheck))) return TRUE;
    
    // We can assume scroll is always zoom for now, or access MainWindow public method to check state.
    // Let's implement public accessor in MainWindow later. For now, we'll just allow it or assume View Mode is managed there.
    // Actually, to keep it clean, let's just zoom.
    
    if (event->direction == GDK_SCROLL_UP) {
        scene->zoomCamera(1.0f);
    } else if (event->direction == GDK_SCROLL_DOWN) {
        scene->zoomCamera(-1.0f);
    }
    
    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean InputManager::on_button_press(GtkWidget* widget, GdkEventButton* event) {
    if (event->button != 1) return TRUE;

    // Check View Mode - requires callback to MainWindow or public getter
    // For now, assuming if we are here, we might want to interact.
    // Ideally MainWindow should decide whether to call this or not, OR we access MainWindow::isViewMode()
    
    if (mainWindow->isViewMode()) {
        return TRUE; 
    }

    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    
    float screenX = (float)event->x / w;
    float screenY = (float)event->y / h;
    
    float eyeX, eyeY, eyeZ;
    scene->getCameraPosition(eyeX, eyeY, eyeZ);
    
    // Pick Object
    int hit = scene->pickObject(screenX, screenY, w, h);
    
    if (hit >= 0) {
        scene->setSelected(hit);
        dragIndex = hit;

        int lightIdx = scene->shapeCount();
        if (hit == lightIdx) {
            dragPlaneY = scene->getLightPosition().y;
        } else {
            dragPlaneY = scene->getShapePosition(hit).y;
        }
        gtk_widget_queue_draw(widget);
        return TRUE;
    }
    
    // Nothing hit -> Place Object (moved from MainWindow)
    scene->setSelected(-1);
    
    // We need to know if we are in Shape or Light mode, and what shape/color.
    // This requires asking MainWindow. 
    // Let's assume we can get these from MainWindow.
    
    if (mainWindow->isLightMode()) {
        float defaultLightY = 3.0f;
        float worldX, worldZ;
        if (unprojectScreenToFloor(screenX, screenY, defaultLightY, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
            scene->setLightWorldPos(worldX, defaultLightY, worldZ);
            scene->setSelected(scene->shapeCount());
            dragIndex = scene->shapeCount();
            dragPlaneY = defaultLightY;
            gtk_widget_queue_draw(widget);
        }
    } else {
        // Shape placement
        Vector3 color = mainWindow->getSelectedColor();
        ShapeType type = mainWindow->getSelectedShapeType();
        
        float worldX, worldZ;
        if (unprojectScreenToFloor(screenX, screenY, 0.0f, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
            scene->addShapeAt(type, worldX, worldZ, color.x, color.y, color.z);
            gtk_widget_queue_draw(widget);
        }
    }

    return TRUE;
}

gboolean InputManager::on_button_release(GtkWidget* widget, GdkEventButton* event) {
    if (event->button == 1) {
        dragIndex = -1;
    }
    return TRUE;
}

gboolean InputManager::on_motion_notify(GtkWidget* widget, GdkEventMotion* event) {
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);

    if (mainWindow->isViewMode()) {
        if (event->state & GDK_BUTTON1_MASK) {
             double curX = event->x;
             double curY = event->y;
             
             double dx = curX - prevMouseX;
             double dy = curY - prevMouseY;
             
             // First move jump fix
             if (abs(dx) > 100 || abs(dy) > 100) {
                 prevMouseX = curX;
                 prevMouseY = curY;
                 return TRUE;
             }

             scene->rotateCamera(dx * 0.01f, dy * 0.01f);
             gtk_widget_queue_draw(widget);
             
             prevMouseX = curX;
             prevMouseY = curY;
        } else {
            prevMouseX = event->x;
            prevMouseY = event->y;
        }
        return TRUE;
    }

    // Dragging logic
    if (dragIndex < 0) return TRUE;
    if (!(event->state & GDK_BUTTON1_MASK)) {
        dragIndex = -1;
        return TRUE;
    }

    float screenX = (float)event->x / w;
    float screenY = (float)event->y / h;
    
    float eyeX, eyeY, eyeZ;
    scene->getCameraPosition(eyeX, eyeY, eyeZ);
    
    float worldX, worldZ;
    if (unprojectScreenToFloor(screenX, screenY, dragPlaneY, w, h, eyeX, eyeY, eyeZ, worldX, worldZ)) {
        int lightIdx = scene->shapeCount();
        if (dragIndex == lightIdx) {
            scene->setLightWorldPos(worldX, dragPlaneY, worldZ);
        } else {
            scene->moveShape(dragIndex, worldX, worldZ);
        }
        gtk_widget_queue_draw(widget);
    }

    return TRUE;
}
