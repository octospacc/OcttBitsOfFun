package org.eu.octt.androidfloatingwebview.ui.main

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import org.eu.octt.androidfloatingwebview.R
import kotlinx.android.synthetic.main.fragment_blank.*

class BlankFragment : Fragment() {
    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_blank, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        webview.settings.javaScriptEnabled = true
        //webview.settings.domStorageEnabled = true
        webview.loadUrl("file://" + context?.getExternalFilesDir(null) + "/index.html")
    }
}