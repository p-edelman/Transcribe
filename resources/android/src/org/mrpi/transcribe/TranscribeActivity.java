package org.mrpi.transcribe;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.pm.PackageManager;
import android.content.DialogInterface;
import android.os.Bundle;
import android.Manifest;

public class TranscribeActivity extends org.qtproject.qt5.android.bindings.QtActivity {
  // The permissions we need
  private static final String[] PERMS = {
    Manifest.permission.READ_EXTERNAL_STORAGE,
    Manifest.permission.WRITE_EXTERNAL_STORAGE
  };

  // A unique code for the request for the permissions we need
  private static int PERMS_REQUEST_CODE = 31415;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Check if we have permissions to read and write file storage, and if not
    // ask for them.
    if (!hasStoragePermissions()) {
      requestStoragePermissions();
    }
  }

  /** Figure out if we have permissions to read and write the device storage. */
  public boolean hasStoragePermissions() {
    boolean has_permissions = true;
    for (String permission: PERMS) {
      if (checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED) {
        has_permissions = false;
      }
    }

    return has_permissions;
  }

  /** If the storage permissions are not given, and the user hasn't indicated
      that she won't give them, then open a pop-up to ask for the permissions.
    */
  private void requestStoragePermissions() {
    if (!hasStoragePermissions()) {
      if (shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE) ||
          shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
        requestPermissions(PERMS, PERMS_REQUEST_CODE);
      }
    }
  }

  /** Handle the answer to the request for file storage permissions. */
  @Override
  public void onRequestPermissionsResult(int request_code,
                                         String[] permissions,
                                         int[] results) {
    if (request_code == PERMS_REQUEST_CODE) {
      if (!hasStoragePermissions()) {
        if (shouldShowRequestPermissionRationale(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
          AlertDialog.Builder builder = new AlertDialog.Builder(this);
          builder.setTitle("Storage permission")
                 .setMessage("This app needs access to your device memory to read audio files and write text files. It can't function properly if you don't grant it permissions to do so.");
          builder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              requestStoragePermissions();
          }});
          AlertDialog dialog = builder.create();
          dialog.show();
        }
      }
    }
  }
}
