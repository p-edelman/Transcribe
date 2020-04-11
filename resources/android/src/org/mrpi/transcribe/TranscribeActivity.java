package org.mrpi.transcribe;

import android.content.pm.PackageManager;
import android.Manifest;

public class TranscribeActivity extends org.qtproject.qt5.android.bindings.QtActivity {
  // The permissions we need
  private static final String[] PERMS = {
    Manifest.permission.READ_EXTERNAL_STORAGE
  };

  // A unique code for the request for the permissions we need
  private static int PERMS_REQUEST_CODE = 31415;

  /** Present the user with a popup to request storage permissions. */
  public void requestStoragePerm() {
    requestPermissions(PERMS, PERMS_REQUEST_CODE);
  }

  /** Handle the answer to the request for file storage permissions. */
  @Override
  public void onRequestPermissionsResult(int request_code,
                                         String[] permissions,
                                         int[] results) {
    if (request_code == PERMS_REQUEST_CODE) {
      handleStorageRequestResponse(results[0] == PackageManager.PERMISSION_GRANTED);
    }
  }

  /** Method defined in C to signal that the user picked her storage permissions
    * preference. */
  private native void handleStorageRequestResponse(boolean has_permission);
}
