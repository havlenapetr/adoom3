/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Havlena Petr <havlenapetr@gmail.com>

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License which accompanied
the Doom 3 Source Code.  If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms,
you may contact in writing
id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

package com.idsoftware.doom3;

import java.io.File;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

/**
 * This class checks if we have doom's data for running engine
 * 
 * @author Petr Havlena <havlenapetr@gmail.com>
 *
 */
public class DataActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if(foundData()) {
            startActivity(new Intent("idsoftware.intent.action.MAIN"));
            finish();
        } else {
            setContentView(new NoDataView(this));
        }
    }

    private static boolean fileExists(String s) {
        File f = new File(s);
        return f.exists();
    }

    private static File getDataDir() {
        File sdcard = Environment.getExternalStorageDirectory();
        return new File(sdcard, String.format("Android/data/%s",
                DataActivity.class.getPackage().getName()));
    }

    private boolean foundData() {
        File dir = getDataDir();
        return fileExists(String.format("%s/base/pak000.pk4", dir.getAbsolutePath())) &&
            fileExists(String.format("%s/base/gl2progs", dir.getAbsolutePath()));
    }

    public class NoDataView extends View {

        public NoDataView(Context context) {
            super(context);
        }

        @Override
        protected void onDraw(Canvas canvas) {
            Paint paint = new Paint();
            paint.setColor(0xffffffff);
            paint.setStyle(Paint.Style.FILL);
            canvas.drawRect(0, 0, getWidth(), getHeight(), paint);
            paint.setColor(0xff000000);

            File dir = getDataDir();
            canvas.drawText("Missing data files. Looking for one of:",
                    10.0f, 20.0f, paint);
            canvas.drawText(String.format("%s/base/pak000.pk4", dir.getAbsolutePath()),
                    10.0f, 35.0f, paint);
            canvas.drawText(String.format("%s/base/gl2progs", dir.getAbsolutePath()),
                    10.0f, 50.0f, paint);
            canvas.drawText("Please copy a pak file to the device and restart app.",
                    10.0f, 65.0f, paint);
        }
    }
}
