import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ======== SIMPLE JPEG COMPRESSION EXPERIMENT ========

# ---------- Basic setup ----------
IMAGE_PATH = r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\Skateboard.png"
QUALITIES = [10, 30, 50, 75, 95]


# ---------- Utility functions ----------

def load_gray(path):
    """Load image as grayscale float32 in [0,1]."""
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(path)
    return img.astype(np.float32) / 255.0


def mse(x, y):
    """Mean squared error."""
    return np.mean((x - y) ** 2)


def psnr(x, y):
    """Peak signal-to-noise ratio in dB (for range [0,1])."""
    m = mse(x, y)
    if m == 0:
        return float("inf")
    return 10 * np.log10(1.0 / m)


def jpeg_compress_gray(img, quality):
    """
    Compress + decompress a grayscale image using JPEG.
    'img' is in [0,1]; 'quality' is JPEG quality (0–100).
    """
    img_u8 = (np.clip(img, 0, 1) * 255).astype(np.uint8)
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), int(quality)]
    ok, enc = cv2.imencode(".jpg", img_u8, encode_param)
    if not ok:
        raise RuntimeError("JPEG encoding failed")
    dec = cv2.imdecode(enc, cv2.IMREAD_GRAYSCALE)
    return dec.astype(np.float32) / 255.0


# ---------- Main experiment ----------

img = load_gray(IMAGE_PATH)

mse_vals, psnr_vals, ssim_vals = [], [], []

for q in QUALITIES:
    jpg_img = jpeg_compress_gray(img, q)

    m = mse(img, jpg_img)
    p = psnr(img, jpg_img)
    s = ssim(img, jpg_img, data_range=1.0)

    mse_vals.append(m)
    psnr_vals.append(p)
    ssim_vals.append(s)

# ---------- Plot results ----------

# PSNR vs JPEG quality
plt.figure()
plt.plot(QUALITIES, psnr_vals, 'o-')
plt.xlabel("JPEG quality")
plt.ylabel("PSNR [dB]")
plt.title("PSNR vs JPEG quality")
plt.grid(True)
plt.show()

# SSIM vs JPEG quality
plt.figure()
plt.plot(QUALITIES, ssim_vals, 'o-')
plt.xlabel("JPEG quality")
plt.ylabel("SSIM")
plt.title("SSIM vs JPEG quality")
plt.grid(True)
plt.show()

# MSE vs JPEG quality
plt.figure()
plt.plot(QUALITIES, mse_vals, 'o-')
plt.xlabel("JPEG quality")
plt.ylabel("MSE")
plt.title("MSE vs JPEG quality")
plt.grid(True)
plt.show()
