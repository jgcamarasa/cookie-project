package com.jgcamarasa.cookieproject;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.TextView;

import com.jgcamarasa.cookieproject.messages.IOutMessage;
import com.jgcamarasa.cookieproject.messages.MotionInputOutMsg;
import com.jgcamarasa.cookieproject.messages.ServerExitInMessage;
import com.jgcamarasa.cookieproject.messages.TouchInputOutMsg;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;


public class InputActivity extends Activity implements SensorEventListener {
    private SensorManager mSensorManager;
    private Sensor mSensor;
    private InetAddress mServerAddress;
    private boolean mTouchState = false;
    private boolean mTouchState2 = false;

    private int mPlayerId;
    private int mPlayerId2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_input);

        try {
            mServerAddress = InetAddress.getByName(getIntent().getStringExtra("SERVER_IP"));
        } catch (UnknownHostException e) {
            e.printStackTrace();
            finish();
        }

        mPlayerId = getIntent().getIntExtra("PLAYER_ID", -1);
        ((TextView) this.findViewById(R.id.text_player_id)).setText("Player ID: "+mPlayerId);

        mPlayerId2 = getIntent().getIntExtra("PLAYER_ID2", -1);
        ((TextView) this.findViewById(R.id.text_player_id2)).setText("Player ID: "+mPlayerId2);

        // Set background color
        int[] color = getIntent().getIntArrayExtra("COLOR");
        this.findViewById(R.id.view_input).setBackgroundColor(Color.argb(255, color[0], color[1], color[2]));
        this.findViewById(R.id.view_input).setOnTouchListener(new OnViewTouch());

        // Set background color
        color = getIntent().getIntArrayExtra("COLOR2");
        this.findViewById(R.id.view_input2).setBackgroundColor(Color.argb(255, color[0], color[1], color[2]));
        this.findViewById(R.id.view_input2).setOnTouchListener(new OnViewTouch2());

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        PowerManager.WakeLock wl = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, "");
        wl.acquire();

    }

    @Override
    public void onResume(){
        super.onResume();
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_GRAVITY);
        mSensorManager.registerListener(this,mSensor, SensorManager.SENSOR_DELAY_GAME);
    }

    public void onPause(){
        super.onPause();
        mSensorManager.unregisterListener(this);
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        TextView motionDataText = (TextView) this.findViewById(R.id.text_motion_data);
        motionDataText.setText(""+sensorEvent.values[0] +", "+sensorEvent.values[1] +", "+sensorEvent.values[2] );

        //new SendMsgTask().execute(new MotionInputOutMsg(mPlayerId, sensorEvent.values[0],sensorEvent.values[1],sensorEvent.values[2]));
        if(mTouchState){
            new SendMsgTask().execute(new TouchInputOutMsg(mPlayerId, 1));
            Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
            v.vibrate(25);
        }else{
            new SendMsgTask().execute(new TouchInputOutMsg(mPlayerId, 0));
            Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
        }

        if(mTouchState2){
            new SendMsgTask().execute(new TouchInputOutMsg(mPlayerId2, 1));
            Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
            v.vibrate(25);
        }else{
            new SendMsgTask().execute(new TouchInputOutMsg(mPlayerId2, 0));
            Vibrator v = (Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }

    private class OnViewTouch implements View.OnTouchListener {
        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            if(motionEvent.getAction() == MotionEvent.ACTION_DOWN){
                mTouchState = true;
                Log.i("COOKIE", "Touching");
            }else if(motionEvent.getAction() == MotionEvent.ACTION_CANCEL){
                mTouchState = false;
                Log.i("COOKIE", "Not Touching");
            }else if(motionEvent.getAction() == MotionEvent.ACTION_UP){
                mTouchState = false;
                Log.i("COOKIE", "Not Touching");
            }
            return true;
        }
    }

    private class OnViewTouch2 implements View.OnTouchListener {
        @Override
        public boolean onTouch(View view, MotionEvent motionEvent) {
            if(motionEvent.getAction() == MotionEvent.ACTION_DOWN){
                mTouchState2 = true;
                Log.i("COOKIE", "Touching");
            }else if(motionEvent.getAction() == MotionEvent.ACTION_CANCEL){
                mTouchState2 = false;
                Log.i("COOKIE", "Not Touching");
            }else if(motionEvent.getAction() == MotionEvent.ACTION_UP){
                mTouchState2 = false;
                Log.i("COOKIE", "Not Touching");
            }
            return true;
        }
    }

    private class SendMsgTask extends AsyncTask<IOutMessage, Void, Boolean> {

        @Override
        protected Boolean doInBackground(IOutMessage... msg) {
            DatagramSocket socket = null;
            try {
                // Open the socket
                socket = new DatagramSocket();

                byte[] data = msg[0].getBytes();

                DatagramPacket sendPacket = new DatagramPacket(data, data.length, mServerAddress, 18214);
                socket.send(sendPacket);

                return Boolean.TRUE;

            } catch (SocketTimeoutException e){
                e.printStackTrace();
            } catch (SocketException e) {
                e.printStackTrace();
            } catch (UnknownHostException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if(socket != null){
                    socket.close();
                }
            }

            return Boolean.FALSE;
        }

        protected void onPostExecute(Boolean success) {

        }
    }

    private class ServerResponseTask extends AsyncTask<Void, Void, Boolean>{
        private String mIp;
        private int mPlayerId;
        private int[] mColor;

        @Override
        protected Boolean doInBackground(Void... voids) {
            DatagramSocket socket = null;
            try {
                // Open the socket
                socket = new DatagramSocket();


                byte[] recvBuf = new byte[15000];
                while(true){
                    DatagramPacket receivePacket = new DatagramPacket(recvBuf, recvBuf.length);
                    socket.setSoTimeout(5*1000);
                    socket.receive(receivePacket);

                    int msgType = (int)recvBuf[0];

                    if(msgType==MsgType.MSG_SERVER_EXIT.ordinal()){
                        break;
                    }

                }




                return Boolean.TRUE;

            } catch (SocketTimeoutException e){
                e.printStackTrace();
            } catch (SocketException e) {
                e.printStackTrace();
            } catch (UnknownHostException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if(socket != null){
                    socket.close();
                }
            }

            return Boolean.FALSE;
        }

        protected void onPostExecute(Boolean success) {
            finishActivity(0);
        }

    }


}
