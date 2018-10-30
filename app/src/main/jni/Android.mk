LOCAL_PATH_TOP := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_PATH_TOP)/../jniLibs/armeabi-v7a
LOCAL_MODULE := EasyAR
LOCAL_SRC_FILES := libEasyAR.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_PATH_TOP)
LOCAL_LDLIBS += -llog -lGLESv2
LOCAL_SHARED_LIBRARIES += EasyAR
LOCAL_CPP_EXTENSION := .cc .cpp
LOCAL_MODULE := HelloAR
LOCAL_SRC_FILES := jni.cc helloar.cc boxrenderer.cc objParser/list.cpp objParser/obj_parser.cpp objParser/objLoader.cpp objParser/string_extra.cpp
include $(BUILD_SHARED_LIBRARY)
