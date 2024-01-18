# Android Floating WebView

A simple app, that I quickly arranged, for an effective way of having a WebView as a constant visual overlay on my screen, but that still allows touches to be directed to any other app running below it.

The UX isn't the best and functionality is not optimized, since I only made this quick and dirty for just my specific use; that is, putting animations on the screen of my old smartphone that I'm upcycling into a PC touchpad, thanks to KDE connect (which I couldn't manage to modify for putting a WebView). I will probably not improve it. See <https://octospacc.altervista.org/2024/01/17/overlay-webview-su-android-a-scopi-ricreativi/>.

Most of the code is taken from <https://github.com/mjlong123123/TestFloaWindow/>, so proper credits go to them, I just did the following things:

* Made the app overlay contain just a WebView, loading a file from private external storage.
* Removed test code and disabled complex window overlay code.
* The floating overlay is made visible but not touchable, and also not movable or resizable.
* Added an ongoing notification for terminating the service.

## Download and Usage

* After [installing AndroidFloatingWebView.apk](dist/AndroidFloatingWebView.apk), start the app a first time, grant overlay permissions, then close it.
* Now, prepare an `index.html` file in the application's private external storage directory (which also appears on the screen if you reopen the app before this file is present); it usually (but not always) is `/sdcard/Android/org.eu.octt.androidfloatingwebview/files/`, and, optionally, any other files that you want to require from your HTML.
* Now, restarting the app will make a WebView appear on your screen, totally functioning except for the fact that you can't touch it; all of your touches will go to whatever app was running before.
* To close this visual overlay, open your notification drawer and click on the service notification ("AndroidFloatingWebView: Click to terminate service").

You can put anything you want in the `index.html`, as long as your system's default WebView implementation can interpret it. You can either put an HTML5 application that will work offline, a document, or put code that redirects the browser to any website. Just keep in mind that you can only see, not touch.

