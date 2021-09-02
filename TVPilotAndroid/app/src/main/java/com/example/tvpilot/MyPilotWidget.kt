package com.example.tvpilot

import android.app.PendingIntent
import android.appwidget.AppWidgetManager
import android.appwidget.AppWidgetProvider
import android.content.Context
import android.widget.RemoteViews

import android.content.Intent
import android.util.Log
import android.widget.Toast


/**
 * Implementation of App Widget functionality.
 */
class MyPilotWidget : AppWidgetProvider() {
    override fun onUpdate(
        context: Context,
        appWidgetManager: AppWidgetManager,
        appWidgetIds: IntArray
    ) {
        // There may be multiple widgets active, so update all of them
        for (appWidgetId in appWidgetIds) {
            updateAppWidget(context, appWidgetManager, appWidgetId)
        }
    }

    override fun onEnabled(context: Context) {
        // Enter relevant functionality for when the first widget is created
    }

    override fun onDisabled(context: Context) {
        // Enter relevant functionality for when the last widget is disabled
    }
    private fun updateAppWidget(
        context: Context,
        appWidgetManager: AppWidgetManager,
        appWidgetId: Int
    ) {
        // Construct the RemoteViews object
        val views = RemoteViews(context.packageName, R.layout.my_pilot_widget)
        views.setOnClickPendingIntent(R.id.buttonNet, buttonnetOnclick(context))
//    views.setOnClickPendingIntent(R.id.buttonNetOff, buttonNetOff_OnClick())
//    views.setOnClickPendingIntent(R.id.buttonTV, buttonTv_OnClick())
//    views.setOnClickPendingIntent(R.id.buttonTVOff, buttonTvOff_OnClick(context))

        // Instruct the widget manager to update the widget
        appWidgetManager.updateAppWidget(appWidgetId, views);
    }

//fun buttonTvOff_OnClick(  context: Context): PendingIntent? {
//    val intent =  Intent(context, Class.forName(MyPilotWidget));
//    intent.setAction(action)
//    return PendingIntent.getBroadcast(context, 0, intent, 0);
//}

    private fun buttonnetOnclick(context: Context): PendingIntent? {
        val text = "Hello toast!"
        val duration = Toast.LENGTH_SHORT

        val toast = Toast.makeText(context, text, duration)
        toast.show()
        val intent =  Intent("ButtonNetClicked");
        intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP;
   return PendingIntent.getBroadcast(context, 0, intent, 0);
    }
}



