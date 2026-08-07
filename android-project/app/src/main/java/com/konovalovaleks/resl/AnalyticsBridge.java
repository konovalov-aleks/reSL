package com.konovalovaleks.resl;

import android.content.Context;
import android.os.Bundle;

import com.google.firebase.analytics.FirebaseAnalytics;

// JNI bridge between the native (C++) game code and Google Analytics 4.
public final class AnalyticsBridge {

    private AnalyticsBridge() {
    }

    /**
     * Logs a GA4 event, optionally with a single key/value parameter.
     *
     * @param context application or activity context
     * @param name    event name, e.g. "run_game"
     * @param key     optional parameter key (may be null)
     * @param value   optional parameter value (may be null)
     */
    public static void logEvent(Context context, String name, String key, String value) {
        Bundle params = new Bundle();
        if (key != null && value != null)
            params.putString(key, value);
        FirebaseAnalytics.getInstance(context).logEvent(name, params);
    }
}
