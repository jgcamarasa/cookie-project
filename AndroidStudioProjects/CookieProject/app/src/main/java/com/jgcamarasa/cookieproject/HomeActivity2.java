package com.jgcamarasa.cookieproject;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.jgcamarasa.cookieproject.messages.DiscoverOutMessage;
import com.jgcamarasa.cookieproject.messages.ServerDiscoverResponseInMessage;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;


public class HomeActivity2 extends Activity {

    private int mPlayerId;
    private int[] mColor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

        this.findViewById(R.id.connect_button).setOnClickListener(new ConnectButtonClicked());

        mPlayerId = getIntent().getIntExtra("PLAYER_ID", -1);
        mColor = getIntent().getIntArrayExtra("COLOR");
    }


    private class ConnectButtonClicked implements View.OnClickListener {
        @Override
        public void onClick(View view) {
            new FindServerTask().execute();
            Button connectButton = ((Button) view);
            connectButton.setEnabled(false);
            connectButton.setText("Connecting...");
        }
    }

    // Tries to find the server ip
    private class FindServerTask extends AsyncTask<Void, Void, Boolean>{
        private String mIp;
        private int mPlayerId;
        private int[] mColor;

        @Override
        protected Boolean doInBackground(Void... voids) {
            DatagramSocket socket = null;
            try {
                // Open the socket
                socket = new DatagramSocket();

                DiscoverOutMessage msg = new DiscoverOutMessage();

                byte[] data = msg.getBytes();

                // Send the broadcast msg
                // UPDATE: Broadcast over wifi is soooo slow.
                // We do this little dirty thing right there...

                /*for(int i = 0; i <= 10; i++){
                    for(int j = 0; j <= 255; j++) {
                        DatagramPacket sendPacket = new DatagramPacket(data, data.length, InetAddress.getByName("192.168."+i+"." + j), 1821);
                        socket.send(sendPacket);
                    }
                }*/

                DatagramPacket sendPacket = new DatagramPacket(data, data.length, InetAddress.getByName("255.255.255.255"), 18214);
                socket.send(sendPacket);

                byte[] recvBuf = new byte[15000];
                DatagramPacket receivePacket = new DatagramPacket(recvBuf, recvBuf.length);
                socket.setSoTimeout(20*1000);
                socket.receive(receivePacket);

                ServerDiscoverResponseInMessage response = new ServerDiscoverResponseInMessage();
                response.setFromBytes(receivePacket.getData());

                mPlayerId = response.getPlayerId();
                mColor = new int[3];
                mColor[0] = response.getR();
                mColor[1] = response.getG();
                mColor[2] = response.getB();
                mIp = receivePacket.getAddress().getHostAddress();

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
            if(success){
                Toast.makeText(HomeActivity2.this, "Server found at: "+mIp, Toast.LENGTH_LONG).show();
                Intent intent = new Intent(HomeActivity2.this, InputActivity.class);
                intent.putExtra("SERVER_IP", mIp);
                intent.putExtra("PLAYER_ID", HomeActivity2.this.mPlayerId);
                intent.putExtra("COLOR", HomeActivity2.this.mColor);
                intent.putExtra("PLAYER_ID2", mPlayerId);
                intent.putExtra("COLOR2", mColor);
                startActivity(intent);
            } else {
                Toast.makeText(HomeActivity2.this, "Error connecting", Toast.LENGTH_SHORT).show();
            }

            Button connectButton = ((Button) findViewById(R.id.connect_button));
            connectButton.setEnabled(true);
            connectButton.setText("CONNECT!");
        }

    }

}
