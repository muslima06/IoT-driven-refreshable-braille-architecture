from fastapi import FastAPI, UploadFile, File, HTTPException
import uvicorn
from .ocr_service import extract_text_from_image
import requests

FIREBASE_HOST = "https://ocr-smart-braille-book-default-rtdb.firebaseio.com"
FIREBASE_AUTH = "Qf6tCQnZUJgvy9wMaX2L52trv6MwD2tXReOX9W6f"

app = FastAPI(title="Braille OCR Backend")

@app.post("/scan")
async def scan_image(file: UploadFile = File(...)):
    if not file.content_type.startswith('image/'):
        raise HTTPException(status_code=400, detail="File provided is not an image.")

    try:
        contents = await file.read()
        
        # DEBUG: Save the received image to disk to see what the camera is capturing
        with open("debug_capture.jpg", "wb") as f:
            f.write(contents)
            
        text = extract_text_from_image(contents)
        
        # Perform basic text cleaning or validation if necessary
        clean_text = " ".join(text.split())
        
        if not clean_text:
            print("No text found in image.")
            return {"text": ""}
        
        # Push the OCR text to Firebase RTDB
        firebase_url = f"{FIREBASE_HOST}/active_text.json?auth={FIREBASE_AUTH}"
        response = requests.put(firebase_url, json=clean_text)
        print(f"Firebase push status: {response.status_code} - {response.text}")
        
        return {"text": "SENT_TO_FIREBASE"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=8000)
