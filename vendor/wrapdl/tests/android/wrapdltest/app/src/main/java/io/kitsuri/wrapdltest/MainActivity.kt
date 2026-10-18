package io.kitsuri.wrapdltest

import android.os.Bundle
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import io.kitsuri.wrapdltest.ui.theme.WrapdltestTheme

class MainActivity : ComponentActivity() {

    external fun nativeRunTests(libDir: String, selfPath: String): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        System.loadLibrary("wrapdltest")
        enableEdgeToEdge()

        val libDir = applicationInfo.nativeLibraryDir
        val selfPath = libDir + "/libwrapdltest.so"

        setContent {
            WrapdltestTheme {
                TestScreen(libDir, selfPath, ::nativeRunTests)
            }
        }
    }
}

@Composable
private fun TestScreen(
    libDir: String,
    selfPath: String,
    run: (String, String) -> String,
) {
    val text = remember { mutableStateOf("running tests...") }

    LaunchedEffect(libDir) {
        val result = run(libDir, selfPath)
        Log.i("WrapDLTest", result)
        text.value = result
    }

    Scaffold(modifier = Modifier.fillMaxSize()) { pad ->
        Text(
            text = text.value,
            modifier = Modifier
                .padding(pad)
                .padding(12.dp)
                .fillMaxSize()
                .verticalScroll(rememberScrollState()),
            style = MaterialTheme.typography.bodySmall,
            fontFamily = FontFamily.Monospace,
        )
    }
}
