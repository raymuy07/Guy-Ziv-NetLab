import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ======== SIMPLE GAUSSIAN NOISE EXPERIMENT ========

# ---------- Basic setup ----------
IMAGE_PATH = r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\Skateboard.png"
SIGMAS = [5, 10, 20, 40, 80]               # Noise std in gray levels (0..255)


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


def add_gaussian_noise(img, sigma_norm):
    """
    Add zero-mean Gaussian noise.
    sigma_norm is std in [0,1] units (e.g. 5/255, 10/255, ...).
    """
    noise = np.random.normal(0.0, sigma_norm, img.shape).astype(np.float32)
    noisy = img + noise
    noisy = np.clip(noisy, 0.0, 1.0)
    return noisy


# ---------- Main experiment ----------

img = load_gray(IMAGE_PATH)

mse_vals, psnr_vals, ssim_vals = [], [], []

for s in SIGMAS:
    sigma_norm = s / 255.0        # convert from gray levels to [0,1]
    noisy = add_gaussian_noise(img, sigma_norm)

    m = mse(img, noisy)
    p = psnr(img, noisy)
    s = ssim(img, noisy, data_range=1.0)

    mse_vals.append(m)
    psnr_vals.append(p)
    ssim_vals.append(s)

# ---------- Plot results ----------

# PSNR vs sigma
plt.figure()
plt.plot(SIGMAS, psnr_vals, 'o-')
plt.xlabel("Sigma (noise std) [gray levels]")
plt.ylabel("PSNR [dB]")
plt.title("PSNR vs Sigma (Gaussian noise)")
plt.grid(True)
plt.show()

# SSIM vs sigma
plt.figure()
plt.plot(SIGMAS, ssim_vals, 'o-')
plt.xlabel("Sigma (noise std) [gray levels]")
plt.ylabel("SSIM")
plt.title("SSIM vs Sigma (Gaussian noise)")
plt.grid(True)
plt.show()

# MSE vs sigma
plt.figure()
plt.plot(SIGMAS, mse_vals, 'o-')
plt.xlabel("Sigma (noise std) [gray levels]")
plt.ylabel("MSE")
plt.title("MSE vs Sigma (Gaussian noise)")
plt.grid(True)
plt.show()
