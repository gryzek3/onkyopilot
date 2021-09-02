package com.example.tvpilot

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import com.android.volley.RequestQueue
import com.android.volley.toolbox.StringRequest
import com.android.volley.toolbox.Volley

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    fun buttonNetOnClick(view: android.view.View) {
        sendMessage("net")
    }

    private fun sendMessage(url: String){
        var  queue = Volley.newRequestQueue(this)
        val url = "http://192.168.0.106/$url"
// Request a string response from the provided URL.
        val stringRequest = StringRequest(0, url,
            { response ->
                val text = "Sent $url"
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
        sendMessage("tv")
    }
    fun buttonTvOffOnClick(view: android.view.View) {
        sendMessage("tvoff")
    }

    fun buttonChangeTvSourceOnClick(view: android.view.View) {
        sendMessage("source")
    }

    fun buttonAllOffOnClick(view: android.view.View) {
        sendMessage("off")
    }

    fun buttonNetOffClick(view: android.view.View) {
        sendMessage("netoff")
    }

    fun buttonNetVolUpClick(view: android.view.View) {
        sendMessage("netvolup")

    }

    fun buttonNetVolDownClick(view: android.view.View) {
        sendMessage("netvoldown")

    }
}