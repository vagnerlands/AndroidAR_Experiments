#include "helloar.hpp"

#include <jni.h>
#include <pthread.h>

#define JNIFUNCTION_NATIVE(sig) Java_cn_easyar_samples_helloar_GLView_##sig

pthread_mutex_t mtx;

class LockGuard
{
public:
    LockGuard()
    {
        pthread_mutex_lock(&mtx);
    }
    ~LockGuard()
    {
        pthread_mutex_unlock(&mtx);
    }
};

extern "C" {

JNIEXPORT jboolean JNICALL JNIFUNCTION_NATIVE(nativeInit(JNIEnv*, jobject))
{
    LockGuard lock;
    if (initialize()) {
        return start() ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeDestory(JNIEnv*, jobject))
{
    LockGuard lock;
    stop();
    finalize();
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeInitGL(JNIEnv*, jobject))
{
    LockGuard lock;
    initGL();
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeResizeGL(JNIEnv*, jobject, jint w, jint h))
{
    LockGuard lock;
    resizeGL(w, h);
}

JNIEXPORT void JNICALL JNIFUNCTION_NATIVE(nativeRender(JNIEnv*, jobject))
{
    LockGuard lock;
    render();
}

}
