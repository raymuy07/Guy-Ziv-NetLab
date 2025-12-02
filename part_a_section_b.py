import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ======== SIMPLE LINEAR MOTION BLUR EXPERIMENT ========

# ---------- Basic setup ----------
IMAGE_PATH = r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\Skateboard.png"
LENGTHS = [5, 9, 15, 25]                 # Motion blur lengths (pixels)

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
    """Peak signal-to-noise ratio in dB."""
    m = mse(x, y)
    if m == 0:
        return float("inf")
    return 10 * np.log10(1.0 / m)


def motion_blur(img, length):
    """Apply simple horizontal linear motion blur with given length."""
    k = np.zeros((length, length), np.float32)
    k[length // 2, :] = 1.0
    k /= k.sum()
    blurred = cv2.filter2D(img, -1, k)
    return blurred


# ---------- Main experiment ----------

img = load_gray(IMAGE_PATH)

mse_vals, psnr_vals, ssim_vals = [], [], []

for L in LENGTHS:
    blur_img = motion_blur(img, L)

    m = mse(img, blur_img)
    p = psnr(img, blur_img)
    s = ssim(img, blur_img, data_range=1.0)

    mse_vals.append(m)
    psnr_vals.append(p)
    ssim_vals.append(s)

# ---------- Plot results ----------

plt.figure()
plt.plot(LENGTHS, psnr_vals, 'o-')
plt.xlabel("Motion blur length (pixels)")
plt.ylabel("PSNR [dB]")
plt.title("PSNR vs. Motion Blur Length")
plt.grid(True)
plt.show()

plt.figure()
plt.plot(LENGTHS, ssim_vals, 'o-')
plt.xlabel("Motion blur length (pixels)")
plt.ylabel("SSIM")
plt.title("SSIM vs. Motion Blur Length")
plt.grid(True)
plt.show()
