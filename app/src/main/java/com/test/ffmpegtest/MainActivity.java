package com.test.ffmpegtest;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Toast;

import com.yanzhenjie.permission.Action;
import com.yanzhenjie.permission.AndPermission;
import com.yanzhenjie.permission.Permission;

import java.io.File;
import java.util.List;


public class MainActivity extends AppCompatActivity {

    private SurfaceView mSurfaceView;

    private NEPlayer mPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        mSurfaceView = findViewById(R.id.surface_view);
        requestPermission();

        mPlayer = new NEPlayer();
        mPlayer.setSurfaceView(mSurfaceView);
        String path = Environment.getExternalStorageDirectory() + File.separator + "input2.mp4";
        mPlayer.setDataSource(path);
        mPlayer.setOnPrepareListener(new NEPlayer.OnPrepareListener() {
            @Override
            public void onPrepared() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(MainActivity.this, "准备完成", Toast.LENGTH_SHORT).show();
                    }
                });
                mPlayer.start();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();

    }

    public void requestPermission() {
        AndPermission.with(MainActivity.this)
                .runtime()
                .permission(Permission.WRITE_EXTERNAL_STORAGE)
                .onGranted(new Action<List<String>>() {
                    @Override
                    public void onAction(List<String> permissions) {
                        mPlayer.prepare();
                    }
                }).onDenied(new Action<List<String>>() {
            @Override
            public void onAction(List<String> permissions) {
                // TODO what to do
            }
        })
                .start();
    }

    /**
     * 播放
     * @param view
     */
    public void play(View view) {
        requestPermission();
    }
}
