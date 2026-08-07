#include "system/analytics.h"

#ifdef __ANDROID__
#   include <SDL_system.h>
#   include <jni.h>
#endif // __ANDROID__

namespace resl {

#ifdef __ANDROID__

template <typename T>
class JNILocalRef
{
public:
    JNILocalRef(JNIEnv* env, T obj)
        : m_obj(obj)
        , m_env(env)
    {}

    ~JNILocalRef()
    {
        if (m_obj)
            m_env->DeleteLocalRef(m_obj);
    }

    JNILocalRef(const JNILocalRef&) = delete;
    JNILocalRef& operator=(const JNILocalRef&) = delete;

    T operator*() const { return m_obj; }
    explicit operator bool() const { return m_obj; }

private:
    T m_obj;
    JNIEnv* m_env;
};

void trackEvent(const char* name, const char* key, const char* value)
{
    if (!name)
        return;

    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    if (!env) [[unlikely]]
        return;

    const JNILocalRef<jclass> clazz(env, env->FindClass("com/konovalovaleks/resl/AnalyticsBridge"));
    if (!clazz) [[unlikely]]
        return;

    const jmethodID method = env->GetStaticMethodID(
        *clazz, "logEvent",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    if (!method) [[unlikely]]
        return;

    JNILocalRef<jobject> activity(env, static_cast<jobject>(SDL_AndroidGetActivity()));
    JNILocalRef<jstring> jName(env, env->NewStringUTF(name));
    JNILocalRef<jstring> jKey(env, key ? env->NewStringUTF(key) : nullptr);
    JNILocalRef<jstring> jValue(env, value ? env->NewStringUTF(value) : nullptr);

    env->CallStaticVoidMethod(*clazz, method, *activity, *jName, *jKey, *jValue);
}

#else

void trackEvent(const char* /* name */, const char* /* key */, const char* /* value */)
{
}

#endif

} // namespace resl
