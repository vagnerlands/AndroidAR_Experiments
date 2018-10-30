//================================================================================================================================
//
//  Copyright (c) 2015-2018 VisionStar Information Technology (Shanghai) Co., Ltd. All Rights Reserved.
//  EasyAR is the registered trademark or trademark of VisionStar Information Technology (Shanghai) Co., Ltd in China
//  and other countries for the augmented reality technology developed by VisionStar Information Technology (Shanghai) Co., Ltd.
//
//================================================================================================================================

#include "helloar.hpp"

#include "boxrenderer.hpp"

#include "easyar/vector.hxx"
#include "easyar/camera.hxx"
#include "easyar/framestreamer.hxx"
#include "easyar/imagetracker.hxx"
#include "easyar/imagetarget.hxx"
#include "easyar/renderer.hxx"

#include <math.h>
#include <string.h>
#include <GLES2/gl2.h>
#include <android/log.h>

easyar::CameraDevice * camera;
easyar::CameraFrameStreamer * streamer;
easyar::ImageTracker * trackers[1] = {};
easyar::Renderer * videobg_renderer;
easyar::samples::BoxRenderer * box_renderer;
bool viewport_changed = false;
easyar::Vec2I view_size = easyar::Vec2I(0, 0);
int rotation = 0;
easyar::Vec4I viewport = easyar::Vec4I(0, 0, 1280, 720);

void loadTargetCallback(void * _state, easyar::Target * target, bool status)
{
    if (target == NULL) { return; }

    easyar::String * name = NULL;
    target->name(&name);
    __android_log_print(ANDROID_LOG_INFO, "HelloAR", "load target (%d): %s (%d)", status, name->begin(), target->runtimeID());
    delete name;
}

void loadFromImage(easyar::ImageTracker * tracker, const char * path)
{
    easyar::ImageTarget * target = new easyar::ImageTarget();

    char name[256] = {};
    const char * pdot = strstr(path, ".");
    if (pdot == NULL) {
        strcpy(name, path);
    } else {
        strncpy(name, path, pdot - path);
    }

    char jstr[256] = {};
    strcat(jstr, "{\n"
        "  \"images\" :\n"
        "  [\n"
        "    {\n"
        "      \"image\" : \"");
    strcat(jstr, path);
    strcat(jstr, "\",\n"
        "      \"name\" : \"");
    strcat(jstr, name);
    strcat(jstr, "\"\n"
        "    }\n"
        "  ]\n"
        "}");

    easyar::String * e_jstr;
    easyar::String::from_utf8_begin(jstr, &e_jstr);

    easyar::String * e_name;
    easyar::String::from_utf8_begin("", &e_name);

    target->setup(e_jstr, static_cast<int>(easyar::StorageType_Assets) | static_cast<int>(easyar::StorageType_Json), e_name);
    delete e_jstr;
    delete e_name;

    tracker->loadTarget(target, easyar::FunctorOfVoidFromPointerOfTargetAndBool(NULL, loadTargetCallback, NULL));
    delete target;
}

void loadFromJsonFile(easyar::ImageTracker * tracker, const char * path, const char * targetname)
{
    easyar::ImageTarget * target = new easyar::ImageTarget();

    easyar::String * e_path = NULL;
    easyar::String::from_utf8_begin(path, &e_path);

    easyar::String * e_targetname = NULL;
    easyar::String::from_utf8_begin(targetname, &e_targetname);

    target->setup(e_path, static_cast<int>(easyar::StorageType_Assets), e_targetname);
    delete e_path;
    delete e_targetname;

    tracker->loadTarget(target, easyar::FunctorOfVoidFromPointerOfTargetAndBool(NULL, loadTargetCallback, NULL));
    delete target;
}

void loadAllFromJsonFile(easyar::ImageTracker * tracker, const char * path)
{
    easyar::String * e_path = NULL;
    easyar::String::from_utf8_begin(path, &e_path);

    easyar::ListOfPointerOfImageTarget * targets = NULL;
    easyar::ImageTarget::setupAll(e_path, static_cast<int>(easyar::StorageType_Assets), &targets);

    int targetSize = targets->size();
    for (int k = 0; k < targetSize; k += 1) {
        easyar::ImageTarget * target = targets->at(k);
        tracker->loadTarget(target, easyar::FunctorOfVoidFromPointerOfTargetAndBool(NULL, loadTargetCallback, NULL));
    }
    delete e_path;
    delete targets;
}

bool initialize()
{
    do {
        camera = new easyar::CameraDevice();
        streamer = new easyar::CameraFrameStreamer();
        streamer->attachCamera(camera);

        bool status = true;
        status &= camera->open(static_cast<int>(easyar::CameraDeviceType_Default));
        camera->setSize(easyar::Vec2I(1280, 720));

        if (!status) { break; }

        easyar::ImageTracker * tracker = new easyar::ImageTracker();
        tracker->attachStreamer(streamer);
        loadFromJsonFile(tracker, "targets.json", "argame");
        loadFromJsonFile(tracker, "targets.json", "idback");
        loadAllFromJsonFile(tracker, "targets2.json");
        loadFromImage(tracker, "paulaner.png");
        trackers[0] = tracker;

        return true;
    } while (0);

    delete streamer;
    streamer = NULL;
    delete camera;
    camera = NULL;
    return false;
}

void finalize()
{
    int size = static_cast<int>(sizeof(trackers) / sizeof(easyar::ImageTracker *));
    for (int k = 0; k < size; k += 1) {
        delete trackers[k];
        trackers[k] = NULL;
    }
    delete box_renderer;
    box_renderer = NULL;
    delete videobg_renderer;
    videobg_renderer = NULL;
    delete streamer;
    streamer = NULL;
    delete camera;
    camera = NULL;
}

bool start()
{
    do {
        bool status = true;
        status &= (camera != NULL) && camera->start();
        status &= (streamer != NULL) && streamer->start();
        camera->setFocusMode(easyar::CameraDeviceFocusMode_Continousauto);

        if (!status) { break; }

        int size = static_cast<int>(sizeof(trackers) / sizeof(easyar::ImageTracker *));
        for (int k = 0; k < size; k += 1) {
            if (trackers[k] != NULL) {
                status &= trackers[k]->start();
            }
        }

        if (!status) { break; }

        return true;
    } while (0);

    return false;
}

bool stop()
{
    do {
        bool status = true;
        int size = static_cast<int>(sizeof(trackers) / sizeof(easyar::ImageTracker *));
        for (int k = 0; k < size; k += 1) {
            if (trackers[k] != NULL) {
                status &= trackers[k]->stop();
            }
        }
        status &= (streamer != NULL) && streamer->stop();
        status &= (camera != NULL) && camera->stop();

        if (!status) { break; }

        return true;
    } while (0);

    return false;
}

void initGL()
{
    if (videobg_renderer != NULL) {
        delete videobg_renderer;
        videobg_renderer = NULL;
    }
    if (box_renderer != NULL) {
        delete box_renderer;
        box_renderer = NULL;
    }

    do {
        videobg_renderer = new easyar::Renderer();
        box_renderer = new easyar::samples::BoxRenderer();

        box_renderer->init();
        return;
    } while (0);

    if (videobg_renderer != NULL) {
        delete videobg_renderer;
        videobg_renderer = NULL;
    }
    if (box_renderer != NULL) {
        delete box_renderer;
        box_renderer = NULL;
    }
}

void resizeGL(int width, int height)
{
    view_size = easyar::Vec2I(width, height);
    viewport_changed = true;
}

void updateViewport()
{
    easyar::CameraCalibration * calib = NULL;
    if (camera != NULL) {
        camera->cameraCalibration(&calib);
    }
    int rotation = calib != NULL ? calib->rotation() : 0;
    delete calib;
    if (rotation != ::rotation) {
        ::rotation = rotation;
        viewport_changed = true;
    }
    if (viewport_changed) {
        easyar::Vec2I size = easyar::Vec2I(1, 1);
        if (camera && camera->isOpened())
            size = camera->size();
        if (rotation == 90 || rotation == 270) {
            int temp = size.data[0];
            size.data[0] = size.data[1];
            size.data[1] = temp;
        }

        float scaleRatio0 = (float)view_size.data[0] / (float)size.data[0];
        float scaleRatio1 = (float)view_size.data[1] / (float)size.data[1];
        float scaleRatio = scaleRatio0 >= scaleRatio1 ? scaleRatio0 : scaleRatio1;
        easyar::Vec2I viewport_size = easyar::Vec2I((int)roundf(size.data[0] * scaleRatio), (int)roundf(size.data[1] * scaleRatio));
        viewport = easyar::Vec4I((view_size.data[0] - viewport_size.data[0]) / 2, (view_size.data[1] - viewport_size.data[1]) / 2, viewport_size.data[0], viewport_size.data[1]);

        if (camera && camera->isOpened())
            viewport_changed = false;
    }
}

void render()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (videobg_renderer != NULL) {
        easyar::Vec4I default_viewport = easyar::Vec4I(0, 0, view_size.data[0], view_size.data[1]);
        glViewport(default_viewport.data[0], default_viewport.data[1], default_viewport.data[2], default_viewport.data[3]);
        if (videobg_renderer->renderErrorMessage(default_viewport)) {
            return;
        }
    }

    if (streamer == NULL) { return; }
    easyar::Frame * frame = NULL;

    do {
        streamer->peek(&frame);

        updateViewport();
        glViewport(viewport.data[0], viewport.data[1], viewport.data[2], viewport.data[3]);

        if (videobg_renderer != NULL) {
            videobg_renderer->render(frame, viewport);
        }

        easyar::ListOfPointerOfTargetInstance * targetInstances;
        frame->targetInstances(&targetInstances);

        int targetInstanceSize = targetInstances->size();
        static bool alreadyDetected = false;
        static easyar::TargetInstance * permanentTargetInstance = NULL;
        if (!alreadyDetected) {
            for (int k = 0; k < targetInstanceSize; k += 1) {
                easyar::TargetInstance *targetInstance = targetInstances->at(k);
                easyar::TargetStatus status = targetInstance->status();

                if (status == easyar::TargetStatus_Tracked) {
                    easyar::Target *target;
                    targetInstance->target(&target);
                    easyar::ImageTarget *imagetarget = NULL;
                    easyar::ImageTarget::tryCastFromTarget(target, &imagetarget);
                    if (!imagetarget)
                        continue;
                    permanentTargetInstance = targetInstance;
                    //alreadyDetected = true;
                    box_renderer->render(camera->projectionGL(0.2f, 500.f),
                                         targetInstance->poseGL(), imagetarget->size());
                }
            }
        } else
        {
            box_renderer->render(camera->projectionGL(0.2f, 500.f),
                                 permanentTargetInstance->poseGL(), easyar::Vec2F(1.f, 1.f));
        }

        break;
    } while (0);

    if (frame != NULL) {
        delete frame;
    }
}
