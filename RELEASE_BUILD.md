# Building Signed Release APK

## Build Command

```bash
bash build_release.sh
```

Enter passwords when prompted (passwords with special characters like `!` are safe).

**Output APKs:**
- `app/build/outputs/apk/playStore/release/app-playStore-release.apk` (Play Store)
- `app/build/outputs/apk/offPlayStore/release/app-offPlayStore-release.apk` (Direct distribution)

---

## Configuration

**Keystore:** `~/android-keystore/upload-keystore.jks` (alias: `key0`)  
**Version:** 0.9.35 (versionCode: 9350)

To update version, edit `versionCode` and `versionName` in `app/build.gradle`.

---

## Verification

```bash
# Verify signature
jarsigner -verify app/build/outputs/apk/playStore/release/app-playStore-release.apk

# View APK info
aapt dump badging app/build/outputs/apk/playStore/release/app-playStore-release.apk | head
```

---

## Troubleshooting

**"Wrong password"**: Passwords are read securely from prompt. Special characters like `!` are safe.

**"Keystore not found"**: Verify `~/android-keystore/upload-keystore.jks` exists.
