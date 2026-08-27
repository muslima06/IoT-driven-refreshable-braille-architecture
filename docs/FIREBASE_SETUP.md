# Firebase Setup Guide

Follow these steps to set up a free Firebase project and get the database URL and credentials needed to connect both the ESP32-S3 firmware and the Android App.

---

## 1. Create a Firebase Project

1. Go to the [Firebase Console](https://console.firebase.google.com/).
2. Click **Add project** (or **Create a project**).
3. Enter a project name (e.g. `Braille-OCR-Display`).
4. Click **Continue**.
5. Disable Google Analytics (unnecessary for this project) and click **Create project**.
6. Wait for the project to provision and click **Continue**.

---

## 2. Create the Realtime Database

1. In the left-side navigation panel, scroll to the **Build** section and click **Realtime Database**.
2. Click the **Create Database** button.
3. Select your Database Location (choose the default location closest to you) and click **Next**.
4. Start in **Locked mode** and click **Enable**.
5. Once created, copy the **Database URL** from the header of your database view. It will look like this:
   `https://your-project-id-default-rtdb.firebaseio.com/` (or similar).

---

## 3. Configure Database Security Rules

To allow both the Android app and the ESP32 to read and write without complex OAuth authentication, configure open permissions for testing:

1. Click on the **Rules** tab at the top of the Realtime Database page.
2. Edit the JSON rules to read:
   ```json
   {
     "rules": {
       ".read": "true",
       ".write": "true"
     }
   }
   ```
3. Click **Publish**.

> [!WARNING]
> Setting rules to `true` allows anyone with the URL to read and write data. This is suitable for development and testing. For a production deployment, replace with restricted user authentication rules.

---

## 4. Get the Database secret (for ESP32)

To connect the ESP32 to Firebase, we will use a database secret token:

1. Click the **Gear icon (Project settings)** in the top-left menu next to "Project Overview".
2. Select **Project settings**.
3. Go to the **Service accounts** tab.
4. Click on **Database secrets** in the sub-menu.
5. Hover over the secret key and click **Show**.
6. Copy the secret key string. This will be used as `FIREBASE_AUTH` in the ESP32 config.

---

## 5. Get the App config (for Android App)

To connect Android Studio to your Firebase project, generate a config file:

1. In the **Project settings** page, go to the **General** tab.
2. Scroll down to the "Your apps" section and click the **Android icon**.
3. Enter an Android package name: `com.braille.ocr`.
4. Click **Register app**.
5. Download the `google-services.json` file.
6. Place this `google-services.json` file inside the `app/` folder of your Android Studio project.
