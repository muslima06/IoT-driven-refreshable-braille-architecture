package com.braille.ocr

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageCapture
import androidx.camera.core.ImageCaptureException
import androidx.camera.core.ExperimentalGetImage
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.google.mlkit.vision.common.InputImage
import com.google.mlkit.vision.text.TextRecognition
import com.google.mlkit.vision.text.latin.TextRecognizerOptions
import com.google.firebase.database.ktx.database
import com.google.firebase.ktx.Firebase
import com.braille.ocr.databinding.ActivityMainBinding
import android.view.View
import android.view.animation.Animation
import android.view.animation.TranslateAnimation
import android.speech.RecognitionListener
import android.speech.RecognizerIntent
import android.speech.SpeechRecognizer
import android.content.Intent
import android.graphics.Rect
import androidx.camera.core.ImageProxy
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {
    private lateinit var binding: ActivityMainBinding
    private var imageCapture: ImageCapture? = null
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var speechRecognizer: SpeechRecognizer
    private val firebaseRef = Firebase.database("https://ocr-smart-braille-book-default-rtdb.firebaseio.com/").reference.child("active_text")

    companion object {
        private const val TAG = "MainActivity"
        private const val REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS = arrayOf(
            Manifest.permission.CAMERA,
            Manifest.permission.RECORD_AUDIO
        )
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS)
        }

        binding.captureButton.setOnClickListener { 
            it.animate().scaleX(0.9f).scaleY(0.9f).setDuration(100).withEndAction {
                it.animate().scaleX(1f).scaleY(1f).setDuration(100).start()
                takePhoto()
            }.start()
        }

        binding.micButton.setOnClickListener {
            it.animate().scaleX(0.9f).scaleY(0.9f).setDuration(100).withEndAction {
                it.animate().scaleX(1f).scaleY(1f).setDuration(100).start()
                startVoiceRecognition()
            }.start()
        }

        initSpeechRecognizer()
        cameraExecutor = Executors.newSingleThreadExecutor()
    }

    private fun initSpeechRecognizer() {
        speechRecognizer = SpeechRecognizer.createSpeechRecognizer(this)
        speechRecognizer.setRecognitionListener(object : RecognitionListener {
            override fun onReadyForSpeech(params: Bundle?) {
                Log.d(TAG, "Ready for speech")
                startScanAnimation() // Reuse the scan animation for visual feedback
            }
            override fun onBeginningOfSpeech() {}
            override fun onRmsChanged(rmsdB: Float) {}
            override fun onBufferReceived(buffer: ByteArray?) {}
            override fun onEndOfSpeech() {
                stopScanAnimation()
            }
            override fun onError(error: Int) {
                stopScanAnimation()
                Log.e(TAG, "Speech Error: $error")
            }
            override fun onResults(results: Bundle?) {
                val matches = results?.getStringArrayList(SpeechRecognizer.RESULTS_RECOGNITION)
                if (!matches.isNullOrEmpty()) {
                    val spokenText = matches[0]
                    Log.d(TAG, "Speech Result: $spokenText")
                    showResult(spokenText)
                    uploadToFirebase(spokenText)
                }
            }
            override fun onPartialResults(partialResults: Bundle?) {}
            override fun onEvent(eventType: Int, params: Bundle?) {}
        })
    }

    private fun startVoiceRecognition() {
        val intent = Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH).apply {
            putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL, RecognizerIntent.LANGUAGE_MODEL_FREE_FORM)
            putExtra(RecognizerIntent.EXTRA_PROMPT, "Speak now...")
        }
        speechRecognizer.startListening(intent)
    }

    private fun uploadToFirebase(text: String) {
        firebaseRef.setValue(text)
            .addOnSuccessListener { Log.d(TAG, "Voice text uploaded to Firebase") }
            .addOnFailureListener { e -> Log.e(TAG, "Firebase upload failed", e) }
    }

    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(baseContext, it) == PackageManager.PERMISSION_GRANTED
    }

    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)
        cameraProviderFuture.addListener({
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()
            val preview = Preview.Builder().build().also {
                it.setSurfaceProvider(binding.previewView.surfaceProvider)
            }
            imageCapture = ImageCapture.Builder().build()
            val cameraSelector = CameraSelector.DEFAULT_BACK_CAMERA
            try {
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(
                    this, cameraSelector, preview, imageCapture
                )
            } catch (exc: Exception) {
                Log.e(TAG, "Binding failed", exc)
            }
        }, ContextCompat.getMainExecutor(this))
    }

    private fun takePhoto() {
        val imageCapture = imageCapture ?: return
        imageCapture.takePicture(ContextCompat.getMainExecutor(this), object : ImageCapture.OnImageCapturedCallback() {
            @androidx.annotation.OptIn(ExperimentalGetImage::class)
            override fun onCaptureSuccess(imageProxy: ImageProxy) {
                val mediaImage = imageProxy.image
                if (mediaImage != null) {
                    val image = InputImage.fromMediaImage(mediaImage, imageProxy.imageInfo.rotationDegrees)
                    processImage(image, imageProxy)
                }
                imageProxy.close()
            }

            override fun onError(exception: ImageCaptureException) {
                Log.e(TAG, "Photo capture failed: ${exception.message}", exception)
            }
        })
    }

    private fun processImage(image: InputImage, imageProxy: ImageProxy) {
        startScanAnimation()
        val recognizer = TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS)
        recognizer.process(image)
            .addOnSuccessListener { visionText ->
                stopScanAnimation()
                
                // Get image dimensions after rotation
                val rotation = imageProxy.imageInfo.rotationDegrees
                val isSwapped = rotation == 90 || rotation == 270
                val imageWidth = if (isSwapped) imageProxy.height else imageProxy.width
                val imageHeight = if (isSwapped) imageProxy.width else imageProxy.height

                // Get boundary and preview positions
                val boundaryRect = Rect()
                binding.recognitionBoundary.getGlobalVisibleRect(boundaryRect)
                val previewRect = Rect()
                binding.previewView.getGlobalVisibleRect(previewRect)

                // Calculate boundary ratios relative to preview
                val leftRatio = (boundaryRect.left - previewRect.left).toFloat() / previewRect.width()
                val topRatio = (boundaryRect.top - previewRect.top).toFloat() / previewRect.height()
                val rightRatio = (boundaryRect.right - previewRect.left).toFloat() / previewRect.width()
                val bottomRatio = (boundaryRect.bottom - previewRect.top).toFloat() / previewRect.height()

                val resultBuilder = StringBuilder()
                for (block in visionText.textBlocks) {
                    for (line in block.lines) {
                        val box = line.boundingBox ?: continue
                        
                        // Check if line center is within the boundary
                        val centerX = (box.left + box.right) / 2f
                        val centerY = (box.top + box.bottom) / 2f
                        
                        val relX = centerX / imageWidth
                        val relY = centerY / imageHeight
                        
                        if (relX in leftRatio..rightRatio && relY in topRatio..bottomRatio) {
                            resultBuilder.append(line.text).append("\n")
                        }
                    }
                }

                val extracted = resultBuilder.toString().trim()
                Log.d(TAG, "Filtered OCR result: $extracted")
                showResult(extracted)
                uploadToFirebase(extracted)
            }
            .addOnFailureListener { e ->
                stopScanAnimation()
                Log.e(TAG, "OCR failed", e)
            }
    }

    private fun startScanAnimation() {
        binding.scanLine.visibility = View.VISIBLE
        val animation = TranslateAnimation(
            0f, 0f, 
            0f, binding.recognitionBoundary.height.toFloat()
        ).apply {
            duration = 1500
            repeatCount = Animation.INFINITE
            repeatMode = Animation.RESTART
        }
        binding.scanLine.startAnimation(animation)
    }

    private fun stopScanAnimation() {
        binding.scanLine.clearAnimation()
        binding.scanLine.visibility = View.GONE
    }

    private fun showResult(text: String) {
        binding.resultText.text = text
        if (text.isNotEmpty()) {
            binding.resultCard.visibility = View.VISIBLE
            binding.resultCard.alpha = 0f
            binding.resultCard.translationY = 50f
            binding.resultCard.animate()
                .alpha(1f)
                .translationY(0f)
                .setDuration(300)
                .start()
        } else {
            binding.resultCard.visibility = View.INVISIBLE
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        cameraExecutor.shutdown()
        speechRecognizer.destroy()
    }
}
