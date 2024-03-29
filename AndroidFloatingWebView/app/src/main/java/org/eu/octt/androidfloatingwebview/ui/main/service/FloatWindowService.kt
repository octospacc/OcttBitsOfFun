package org.eu.octt.androidfloatingwebview.ui.main.service

import android.animation.LayoutTransition
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.graphics.PixelFormat
//import android.graphics.Rect
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.*
import androidx.core.app.NotificationCompat
import androidx.fragment.app.FragmentController
import androidx.fragment.app.FragmentHostCallback
import androidx.fragment.app.FragmentManager
import org.eu.octt.androidfloatingwebview.R
import org.eu.octt.androidfloatingwebview.ui.main.BlankFragment
import kotlin.system.exitProcess
//import com.dragon.testfloatwindow.ui.main.widgets.TouchContainer
//import kotlin.math.absoluteValue

class FloatWindowService : Service() {

    //companion object {
        //var gNotificationBuilder : NotificationCompat.Builder? = null
    //}

    private val windowManager: WindowManager by lazy {
        getSystemService(Context.WINDOW_SERVICE) as WindowManager
    }

    private val rootContainer: ViewGroup by lazy {
        val view = LayoutInflater.from(this).inflate(R.layout.root_layout, null, false) as ViewGroup
        view.layoutTransition = LayoutTransition()
        /*
        val closeView = view.findViewById(R.id.closeView) as View
        closeView.setOnClickListener {
            stopSelf()
        }
        val dragView = view.findViewById(R.id.dragView) as TouchContainer
        dragView.listener = object : TouchContainer.Listener {
            override fun move(dx: Int, dy: Int) {
                updateWindowPosition(dx, dy)
            }
        }
        */
        view
    }

    private val layoutParameter: WindowManager.LayoutParams by lazy {
        val params = WindowManager.LayoutParams()
        //params.width = 400
        //params.height = 400
        params.type = when {
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.O -> WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY
            Build.VERSION.SDK_INT == Build.VERSION_CODES.N_MR1 -> WindowManager.LayoutParams.TYPE_SYSTEM_ALERT
            else -> WindowManager.LayoutParams.TYPE_TOAST
        }
        params.flags = (
                WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                        or WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        or WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                        or WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
                        or WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                        or WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED
                        or WindowManager.LayoutParams.FLAG_FULLSCREEN)
        params.format = PixelFormat.TRANSLUCENT
        params.gravity = Gravity.START or Gravity.TOP
        params
    }

    /*
    private val displayRect: Rect by lazy {
        val rect = Rect()
        windowManager.defaultDisplay.getRectSize(rect)
        rect
    }
    */

    private val mFragments =
        FragmentController.createController(object : FragmentHostCallback<Service>(
            this,
            Handler(Looper.getMainLooper()),
            0
        ) {
            override fun onGetHost(): Service? {
                return this@FloatWindowService
            }

            override fun onFindViewById(id: Int): View? {
                return rootContainer.findViewById(id)
            }

            override fun onGetLayoutInflater(): LayoutInflater {
                return super.onGetLayoutInflater().cloneInContext(this@FloatWindowService)
            }
        })

    override fun onBind(intent: Intent?): IBinder? {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.let {
            when (it.action) {
                "SHOW" -> {
                    getFragmentManager().beginTransaction()
                        .replace(R.id.window_content_id, BlankFragment())
                        .commit()
                }
                else -> {}
            }
        }
        return super.onStartCommand(intent, flags, startId)
    }

    override fun onCreate() {
        super.onCreate()
        windowManager.addView(rootContainer, layoutParameter)
        mFragments.attachHost(null)
        mFragments.dispatchResume()

        val notificationIntentFilter = IntentFilter("android.intent.CLOSE_ACTIVITY")
        registerReceiver(terminateReceiver, notificationIntentFilter)
        val notificationIntent = Intent("android.intent.CLOSE_ACTIVITY")
        val notificationPendingIntent = PendingIntent.getBroadcast(this, 0, notificationIntent, 0)
        val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val notificationBuilder = NotificationCompat.Builder(applicationContext)
            .setContentTitle("AndroidFloatingWebView")
            .setContentText("Click to terminate service")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentIntent(notificationPendingIntent)
            .setOngoing(true)
            //.setAutoCancel(true) // doesn't seem to work!
        val notification = notificationBuilder.build()
        notificationManager.notify(0, notification)
        //gNotificationBuilder = notificationBuilder
    }

    override fun onDestroy() {
        super.onDestroy()
        mFragments.dispatchDestroy()
        windowManager.removeView(rootContainer)
    }

    private fun getFragmentManager(): FragmentManager {
        return mFragments.supportFragmentManager
    }

    /*
    fun updateWindowSize(width: Int, height: Int) {
        if (layoutParameter.width != width || layoutParameter.height != height) {
            layoutParameter.width = width
            layoutParameter.height = height
            checkRange()
            windowManager.updateViewLayout(rootContainer, layoutParameter)
        }
    }

    private fun updateWindowPosition(dx: Int, dy: Int) {
        if (dx.absoluteValue > 0 || dy.absoluteValue > 0) {
            layoutParameter.x = layoutParameter.x + dx
            layoutParameter.y = layoutParameter.y + dy
            checkRange()
            windowManager.updateViewLayout(rootContainer, layoutParameter)
        }
    }

    private fun checkRange() {
        when {
            layoutParameter.x < 0 -> layoutParameter.x = 0
            layoutParameter.x + layoutParameter.width > displayRect.width() -> {
                layoutParameter.x = displayRect.width() - layoutParameter.width
            }
        }
        when {
            layoutParameter.y < 0 -> layoutParameter.y = 0
            layoutParameter.y + layoutParameter.height > displayRect.height() -> {
                layoutParameter.y = displayRect.height() - layoutParameter.height
            }
        }
    }
    */

    val terminateReceiver = object: BroadcastReceiver() {
        override fun onReceive(p0: Context?, p1: Intent?) {
            //gNotificationBuilder?.setOngoing(false)
            exitProcess(0)
        }
    }
}