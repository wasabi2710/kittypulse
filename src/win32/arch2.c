#include <windows.h>
#include <stdio.h>
#include <gdiplus.h>
#include <gdiplus/gdiplusflat.h>

// Function to initialize GDI+
GpStatus InitGDIPlus(ULONG_PTR* gdiplusToken) {
    GdiplusStartupInput gdiplusStartupInput;
    gdiplusStartupInput.GdiplusVersion = 1;
    gdiplusStartupInput.DebugEventCallback = NULL;
    gdiplusStartupInput.SuppressBackgroundThread = FALSE;
    gdiplusStartupInput.SuppressExternalCodecs = FALSE;

    return GdiplusStartup(gdiplusToken, &gdiplusStartupInput, NULL);
}

// Function to clean up GDI+
void cleanupGDIPlus(ULONG_PTR gdiplusToken) {
    GdiplusShutdown(gdiplusToken);
}

// Function to get the dominant color
void getDominantColor(const wchar_t* imagePath) {
    GpImage* image = NULL;
    BitmapData bitmapData;
    unsigned long long totalRed = 0, totalGreen = 0, totalBlue = 0;
    unsigned int width, height;
    unsigned char* pixelData;
    int stride;
    unsigned long long pixelCount = 0;

    // Load image
    GpStatus status = GdipLoadImageFromFile(imagePath, &image);
    if (status != Ok || image == NULL) {
        printf("Failed to load image. GDI+ error code: %d\n", status);
        return;
    }
    printf("Image loaded successfully: %ls\n", imagePath);

    // Get dimensions
    REAL realWidth, realHeight;
    status = GdipGetImageDimension(image, &realWidth, &realHeight);
    if (status != Ok) {
        printf("Failed to get image dimensions. GDI+ error code: %d\n", status);
        GdipDisposeImage(image);
        return;
    }
    width = (unsigned int)realWidth;
    height = (unsigned int)realHeight;
    printf("Image dimensions: width=%u, height=%u\n", width, height);

    // Create a GpRect for the entire image
    GpRect rect = {0, 0, (INT)width, (INT)height};

    // Convert image to bitmap
    GpBitmap* bitmap = (GpBitmap*)image;

    // Lock the bitmap bits
    status = GdipBitmapLockBits(bitmap, &rect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);
    if (status != Ok) {
        printf("Failed to lock bitmap bits. GDI+ error code: %d\n", status);
        GdipDisposeImage(bitmap);
        GdipDisposeImage(image);
        return;
    }

    // Analyze colors
    pixelData = (unsigned char*)bitmapData.Scan0;
    stride = bitmapData.Stride;

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int offset = (y * stride) + (x * 4);
            totalRed += pixelData[offset + 2];
            totalGreen += pixelData[offset + 1];
            totalBlue += pixelData[offset];
            pixelCount++;
        }
    }

    // Unlock the bitmap
    GdipBitmapUnlockBits(bitmap, &bitmapData);

    // Cleanup
    GdipDisposeImage(bitmap);
    GdipDisposeImage(image);

    // Calculate average colors
    unsigned long long avgRed = totalRed / pixelCount;
    unsigned long long avgGreen = totalGreen / pixelCount;
    unsigned long long avgBlue = totalBlue / pixelCount;

    // Determine the dominant color
    if (avgRed > avgGreen && avgRed > avgBlue) {
        printf("Dominant color: Red (Avg Intensity = %llu)\n", avgRed);
    } else if (avgGreen > avgRed && avgGreen > avgBlue) {
        printf("Dominant color: Green (Avg Intensity = %llu)\n", avgGreen);
    } else {
        printf("Dominant color: Blue (Avg Intensity = %llu)\n", avgBlue);
    }
}

int main() {
    ULONG_PTR gdiplusToken;

    // Initialize GDI+
    if (InitGDIPlus(&gdiplusToken) != Ok) {
        printf("Failed to initialize GDI+.\n");
        return 1;
    }

    // Specify the image path
    wchar_t imagePath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, "C:\\Users\\Administrator\\Documents\\DEV\\cpp\\kittypulse\\src\\bg.jpg", -1, imagePath, MAX_PATH);

    // Get the dominant color
    getDominantColor(imagePath);

    // Cleanup GDI+
    cleanupGDIPlus(gdiplusToken);

    return 0;
}
