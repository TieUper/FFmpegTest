package com.test.ffmpegtest;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class NEPlayer implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    //直播地址或媒体文件路径
    private String mSource;
    private SurfaceHolder mSurfaceHolder;

    private OnPrepareListener mOnPrepareListener;

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        mOnPrepareListener = onPrepareListener;
    }

    public void setDataSource(String source) {
        mSource = source;
    }

    public void prepare() {
        prepareNative(mSource);
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.removeCallback(this);
        }
        mSurfaceHolder = surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    /**
     * 画布创建回调
     * @param holder
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布改变
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    /**
     * 画布销毁
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    /**
     * 播放
     */
    public void start() {
        startNative();
    }

    interface OnPrepareListener{

        void onPrepared();
    }

    /**
     * JNI回调，准备完成
     */
    public void onPrepared() {
        if (mOnPrepareListener != null) {
            mOnPrepareListener.onPrepared();
        }
    }

    private native void prepareNative(String source);

    private native void startNative();

    private native void setSurfaceNative(Surface surface);
}
