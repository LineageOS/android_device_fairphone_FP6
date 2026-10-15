/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "FairphoneSensorPropsShim"

#include <aidl/android/hardware/biometrics/fingerprint/SensorProps.h>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_status.h>

#include <dlfcn.h>

using android::base::GetProperty;

using aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType;
using aidl::android::hardware::biometrics::fingerprint::SensorProps;

namespace {

SensorProps SensorPropsInit(SensorProps props) {
    auto type = GetProperty("persist.vendor.fingerprint.sensor_type", "side");

    if (type == "back")
        props.sensorType = FingerprintSensorType::REAR;
    else if (type == "front")
        props.sensorType = FingerprintSensorType::HOME_BUTTON;
    else if (type == "optical")
        props.sensorType = FingerprintSensorType::UNDER_DISPLAY_OPTICAL;
    else if (type == "ultrasonic")
        props.sensorType = FingerprintSensorType::UNDER_DISPLAY_ULTRASONIC;
    else
        props.sensorType = FingerprintSensorType::POWER_BUTTON;

    // Side/rear/home-button sensors never need the framework to draw an
    // under-display UI.
    if (type != "optical" && type != "ultrasonic") {
        props.halHandlesDisplayTouches = false;
        props.halControlsIllumination = false;
    }

    return props;
}

}  // anonymous namespace

extern "C" binder_status_t
_ZNK4aidl7android8hardware10biometrics11fingerprint11SensorProps13writeToParcelEP7AParcel(
        const SensorProps* thisptr, AParcel* parcel) {
    using WriteFn = binder_status_t (*)(const SensorProps*, AParcel*);
    static WriteFn real = reinterpret_cast<WriteFn>(dlsym(RTLD_NEXT, __func__));

    if (real == nullptr) {
        LOG(ERROR) << "Could not resolve original writeToParcel";
        return STATUS_UNKNOWN_ERROR;
    }

    SensorProps props = SensorPropsInit(*thisptr);
    LOG(DEBUG) << "Original props=" << thisptr->toString() << ", new props=" << props.toString();
    return real(&props, parcel);
}
