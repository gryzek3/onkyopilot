package com.example.tvpilot

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import com.android.volley.toolbox.StringRequest
import com.android.volley.toolbox.Volley

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    fun buttonNetOnClick(view: android.view.View) {
        sendMessageToKitchen("net")
    }

    private fun sendMessageToKitchen(query: String) {
        val url = "http://192.168.0.106/$query"
        sendMessage(url)
    }
    private fun sendMessageToBedroom(query: String) {
        val url = "http://192.168.0.230/$query"
        sendMessage(url, true)
    }

    private  fun sendMessage(url: String, readResponse: Boolean = false) {
// Request a string response from the provided URL.
        var  queue = Volley.newRequestQueue(this)
        val stringRequest = StringRequest(0, url,
            { response ->
                var text = "OK";
                if(readResponse) {
                    text = response;
                }
                val duration = Toast.LENGTH_SHORT
                val toast = Toast.makeText(this, text, duration)
                toast.show()
            },
            { it ->
                val text = "Error: $url ${it.message}"
                val duration = Toast.LENGTH_LONG
                val toast = Toast.makeText(this, text, duration)
                toast.show()
            })
        queue.add(stringRequest)
    }

    fun buttonTvOnClick(view: android.view.View) {
        sendMessageToKitchen("tv")
    }
    fun buttonTvOffOnClick(view: android.view.View) {
        sendMessageToKitchen("tvoff")
    }

    fun buttonChangeTvSourceOnClick(view: android.view.View) {
        sendMessageToKitchen("source")
    }

    fun buttonAllOffOnClick(view: android.view.View) {
        sendMessageToKitchen("off")
    }

    fun buttonNetOffClick(view: android.view.View) {
        sendMessageToKitchen("netoff")
    }

    fun buttonNetVolUpClick(view: android.view.View) {
        sendMessageToKitchen("netvolup")
    }

    fun buttonNetVolDownClick(view: android.view.View) {
        sendMessageToKitchen("netvoldown")
    }
    fun buttonLightClick(view: android.view.View) {
        sendMessageToBedroom("light")
    }
    fun buttonTempClick(view: android.view.View) {
        sendMessageToBedroom("temperature")
    }
}