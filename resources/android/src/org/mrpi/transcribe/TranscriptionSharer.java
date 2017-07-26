package org.mrpi.Transcribe;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

/** Send the text of a transcription to an external app with an intent. */
public class TranscriptionSharer {
  public static void shareTranscription(Activity context, String text) {
    Intent intent = new Intent();
    intent.setAction(Intent.ACTION_SEND);
    intent.putExtra(Intent.EXTRA_TEXT, text);
    intent.setType("text/plain");
    context.startActivity(Intent.createChooser(intent, "Send transcription to"));
  }
}
