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

gboolean InputManager::on_key_release_callback(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    return static_cast<InputManager*>(data)->on_key_release(widget, event);
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
    if (scene->getSelected() == -1) return FALSE; 

    PhysicsObject* physObj = scene->getPhysicsObject(scene->getSelected());
    if (!physObj) return FALSE;

    float acceleration = 10.0f; // Force/Acceleration magnitude
    float ax = 0.0f;
    float az = 0.0f;

    // Get Camera Yaw
    float yaw = scene->getCamera()->getYaw();
    
    // Calculate Forward and Right vectors on XZ plane
    float fwdX = -sin(yaw);
    float fwdZ = -cos(yaw);
    float rightX = -fwdZ;
    float rightZ = fwdX;

    switch (event->keyval) {
        case GDK_KEY_w:
        case GDK_KEY_W:
            ax += fwdX * acceleration;
            az += fwdZ * acceleration;
            break;
        case GDK_KEY_s:
        case GDK_KEY_S:
            ax -= fwdX * acceleration;
            az -= fwdZ * acceleration;
            break;
        case GDK_KEY_a:
        case GDK_KEY_A:
            ax -= rightX * acceleration;
            az -= rightZ * acceleration;
            break;
        case GDK_KEY_d:
        case GDK_KEY_D:
            ax += rightX * acceleration;
            az += rightZ * acceleration;
            break;
        default:
            return FALSE;
    }

    if (ax != 0.0f || az != 0.0f) {
        
        physObj->acceleration.x = ax;
        physObj->acceleration.z = az;
        
        return TRUE;
    }

    return FALSE;
}

gboolean InputManager::on_key_release(GtkWidget* widget, GdkEventKey* event) {
    if (scene->getSelected() == -1) return FALSE;
    PhysicsObject* physObj = scene->getPhysicsObject(scene->getSelected());
    if (!physObj) return FALSE;
    physObj->acceleration = Vector3(0,0,0);
    
    return TRUE;
}

gboolean InputManager::on_scroll(GtkWidget* widget, GdkEventScroll* event) {
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

    
    Vector3 target = scene->getCamera()->getTarget();

    if (mainWindow->isLightMode()) {
        float defaultLightY = 3.0f;
        float worldX, worldZ;
        if (unprojectScreenToFloor(screenX, screenY, defaultLightY, w, h, eyeX, eyeY, eyeZ, target.x, target.y, target.z, worldX, worldZ)) {
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
        if (unprojectScreenToFloor(screenX, screenY, 0.0f, w, h, eyeX, eyeY, eyeZ, target.x, target.y, target.z, worldX, worldZ)) {
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
    Vector3 target = scene->getCamera()->getTarget();
    if (unprojectScreenToFloor(screenX, screenY, dragPlaneY, w, h, eyeX, eyeY, eyeZ, target.x, target.y, target.z, worldX, worldZ)) {
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
