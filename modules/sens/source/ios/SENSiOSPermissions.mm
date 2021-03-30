#include "SENSiOSPermissions.h"

SENSiOSPermissions::SENSiOSPermissions() {
}

void SENSiOSPermissions::askPermissions() {
    // cannot ask for permissions a second time,
    // so only here to complete the interface
}

bool SENSiOSPermissions::hasCameraPermission() {
    bool result = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo] ==
        AVAuthorizationStatusAuthorized;
    
    return result;
}

bool SENSiOSPermissions::hasGPSPermission() {
    bool result =
        [CLLocationManager authorizationStatus] == kCLAuthorizationStatusAuthorizedAlways ||
        [CLLocationManager authorizationStatus] == kCLAuthorizationStatusAuthorizedWhenInUse;; //[CLLocationManager locationServicesEnabled];
    
    return result;
}

bool SENSiOSPermissions::hasInternetPermission() {
    // no special permissions required (as far as I could tell...)
    
    return true;
}

bool SENSiOSPermissions::hasStoragePermission() {
    // no special permissions required
    
    return true;
}

bool SENSiOSPermissions::canShowCameraPermissionDialog() {
    // iOS will only prompt users for permissions once. After they have to manually
    // change permissions in the settings.
    bool result = false;
    
    return result;
}

bool SENSiOSPermissions::canShowGPSPermissionDialog() {
    // iOS will only prompt users for permissions once. After they have to manually
    // change permissions in the settings.
    bool result = false;
    
    return result;
}

bool SENSiOSPermissions::canShowInternetPermissionDialog() {
    // iOS will only prompt users for permissions once. After they have to manually
    // change permissions in the settings.
    bool result = false;
    
    return result;
}

bool SENSiOSPermissions::canShowStoragePermissionDialog() {
    // iOS will only prompt users for permissions once. After they have to manually
    // change permissions in the settings.
    bool result = false;
    
    return result;
}