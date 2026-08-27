import pytesseract
import cv2
import numpy as np
import os

# Point pytesseract to the standard Windows installation path
if os.name == 'nt':
    pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'


def extract_text_from_image(image_bytes: bytes) -> str:
    """
    Extracts text from an image using Tesseract OCR.
    Ensure tesseract is installed on the system.
    """
    try:
        # Decode image bytes directly into an OpenCV grayscale matrix
        nparr = np.frombuffer(image_bytes, np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_GRAYSCALE)
        
        if img is None:
            return ""

        # Adaptive Thresholding: Automatically handles uneven lighting and shadows.
        # This prevents Tesseract from hallucinating garbage characters from dark patches.
        thresh = cv2.adaptiveThreshold(img, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 51, 15)
        
        # Use PSM 6 (Assume a single uniform block of text) to avoid scattered hallucinations
        text = pytesseract.image_to_string(thresh, config='--psm 6')
        return text.strip()
    except Exception as e:
        print(f"OCR Error: {e}")
        return ""
