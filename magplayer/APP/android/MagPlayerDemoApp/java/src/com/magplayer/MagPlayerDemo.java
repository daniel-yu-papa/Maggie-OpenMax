package com.magplayer;

import android.app.TabActivity;
import android.os.Bundle;
import android.graphics.Color;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.view.LayoutInflater;
import android.util.Log;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.view.View;
import android.widget.Toast;
import android.content.Intent;
import android.widget.Button;
import android.content.Context;
import android.net.Uri;
import android.widget.TextView;
import android.graphics.Typeface;
import android.view.Gravity;
import android.view.Window;
import android.widget.EditText;

public class MagPlayerDemo extends TabActivity
{
    private static final String TAG = "magplayerdemo";
    private TabHost myTabhost;
    
    private Spinner videoCodecList;
    private Spinner audioCodecList;
    private ArrayAdapter<CharSequence> adapter;
    
    private int FileRequestCode;
    private Context context;
    
    private Button selectFile;
    private Button tsplayer_play;
    private Button tsplayer_stop;
    
    private int vpid;
    private int vcodec;
    private int apid;
    private int acodec;
    
    private EditText vpidText;
    private EditText apidText;
    private EditText writeNumTsPacketsText;
    private EditText writeCycleText;
    
    private Uri playFileUri;
// Create an ArrayAdapter using the string array and a default spinner layout


    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        playFileUri = Uri.EMPTY;
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        myTabhost = this.getTabHost();
        LayoutInflater.from(this).inflate(R.layout.main, myTabhost.getTabContentView(), true);
        myTabhost.setBackgroundColor(Color.argb(150, 22, 70, 150));
        
        myTabhost.addTab(myTabhost.newTabSpec("firstTab").setIndicator("tsplayer",getResources().getDrawable(R.drawable.tsplayer)).setContent(R.id.tsplayerID));
        myTabhost.addTab(myTabhost.newTabSpec("secondTab").setIndicator("local playback",getResources().getDrawable(R.drawable.local)).setContent(R.id.localPlayBackID));
        myTabhost.addTab(myTabhost.newTabSpec("thirdTab").setIndicator("network playback",getResources().getDrawable(R.drawable.network)).setContent(R.id.networkPlayBackID));
        
        updateTab(myTabhost);
        
        myTabhost.setOnTabChangedListener(new OnTabChangeListener(){
            @Override
            public void onTabChanged(String tabId) {
                Log.i(TAG, "onTabChanged(" + tabId + ")");
                updateTab(myTabhost);
            }          
        });
        
        videoCodecList = (Spinner) findViewById(R.id.videoCodec);
        adapter = ArrayAdapter.createFromResource(this, R.array.video_codec_array, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        videoCodecList.setAdapter(adapter);
        
        videoCodecList.setOnItemSelectedListener(new OnItemSelectedListener()
        {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) 
            {
                Log.i(TAG, "click video codec:" + position);
                vcodec = position;
            }
            
            @Override
            public void onNothingSelected(AdapterView<?> parentView) 
            {
                Log.i(TAG, "video codec: onNothingSelected");
            }
        });
        
        vpidText = (EditText) findViewById(R.id.videoPID);
        
        audioCodecList = (Spinner) findViewById(R.id.audioCodec);
        adapter = ArrayAdapter.createFromResource(this, R.array.audio_codec_array, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        audioCodecList.setAdapter(adapter);
        
        audioCodecList.setOnItemSelectedListener(new OnItemSelectedListener()
        {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) 
            {
                Log.i(TAG, "click audio codec:" + position);
                acodec = position;
            }
            
            @Override
            public void onNothingSelected(AdapterView<?> parentView) 
            {
                Log.i(TAG, "audio codec: onNothingSelected");
            }
        });
        
        apidText = (EditText) findViewById(R.id.audioPID);
        
        context = getApplicationContext();
        
        selectFile = (Button)findViewById(R.id.tsplayer_select_file);
        selectFile.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v)
            {
               FileRequestCode = 1;
               Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
               intent.setType("*/*");
               intent.addCategory(Intent.CATEGORY_OPENABLE);
               try {
                   startActivityForResult(intent, FileRequestCode);
               } catch (android.content.ActivityNotFoundException ex) {
                   Toast.makeText(context, "please install the file manager", Toast.LENGTH_SHORT).show();
               }
            }
        });
        
        writeNumTsPacketsText = (EditText) findViewById(R.id.writeNumInTSPackets);
        writeCycleText        = (EditText) findViewById(R.id.writeCycleInMS);
        
        tsplayer_play = (Button)findViewById(R.id.tsplayer_play);
        tsplayer_play.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v)
            {
               int numTsPackets;
               int wCycle;
               vpid = Integer.parseInt(vpidText.getText().toString());
               apid = Integer.parseInt(apidText.getText().toString());
               numTsPackets = Integer.parseInt(writeNumTsPacketsText.getText().toString());
               wCycle       = Integer.parseInt(writeCycleText.getText().toString());
               
               if (playFileUri == Uri.EMPTY){
                    Toast.makeText(context, "ERROR: please select the playing file", Toast.LENGTH_SHORT).show();
               }else{
                   Log.i(TAG, "init: vpid=" + vpid + ", vcodec=" + vcodec + ", apid=" + apid + ", acodec=" + acodec + ", url=" + playFileUri.getPath());
                   Log.i(TAG, "tunning param: number of ts packets-" + numTsPackets + "  write cycle-" + wCycle +"ms");
                   
                   nativeTsPlayer_Init(vpid, vcodec, apid, acodec, numTsPackets, wCycle, playFileUri.getPath());
                   nativeTsPlayer_Play();
                }
            }
        });
        
        tsplayer_stop = (Button)findViewById(R.id.tsplayer_stop);
        tsplayer_stop.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v)
            {
               nativeTsPlayer_Stop();
            }
        });
        
        selectFile = (Button)findViewById(R.id.local_select_file);
        selectFile.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View v)
            {
               FileRequestCode = 1;
               Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
               intent.setType("*/*");
               intent.addCategory(Intent.CATEGORY_OPENABLE);
               try {
                   startActivityForResult(intent, FileRequestCode);
               } catch (android.content.ActivityNotFoundException ex) {
                   Toast.makeText(context, "please install the file manager", Toast.LENGTH_SHORT).show();
               }
            }
        });
    }
    
    @Override
    public void onPause(){
        super.onPause();
        nativeTsPlayer_Destroy();
    }
    
    protected void onActivityResult(int requestCode, int resultCode, Intent data) { 
        Log.i(TAG, "request Code: " + requestCode);
        playFileUri = data.getData(); 
        Log.i(TAG, "file name: " + playFileUri.getPath());
        
    }  
    
    private void updateTab(final TabHost tabHost) {  
        for (int i = 0; i < tabHost.getTabWidget().getChildCount(); i++) {  
            View view = tabHost.getTabWidget().getChildAt(i);  
            TextView tv = (TextView) tabHost.getTabWidget().getChildAt(i).findViewById(android.R.id.title);  
            tv.setTextSize(18);  
            tv.setTypeface(Typeface.SERIF, 1);   
            if (tabHost.getCurrentTab() == i) {  
                //view.setBackgroundDrawable(getResources().getDrawable(R.drawable.category_current));
                tv.setTextColor(this.getResources().getColorStateList(  
                        android.R.color.holo_red_dark));  
            } else { 
                //view.setBackgroundDrawable(getResources().getDrawable(R.drawable.category_bg)); 
                tv.setTextColor(this.getResources().getColorStateList(  
                        android.R.color.white));  
            }  
        }  
    }  
    
    static {
        System.loadLibrary("tsplayer_jni");
    }
    
    private static native void nativeTsPlayer_Init(int videoPID, int videoCodec, int audioPID, int audioCodec, int numTSPackets, int wCycle, String url);
    private static native void nativeTsPlayer_Play(); 
    private static native void nativeTsPlayer_Stop(); 
    private static native void nativeTsPlayer_Destroy();
}
