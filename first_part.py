import os
import cv2
import json
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ======================
#   Utility functions
# ======================

def ensure_dir(path: str) -> None:
    """Create directory if it does not exist."""
    os.makedirs(path, exist_ok=True)


def load_image_gray(path: str) -> np.ndarray:
    """Load image as grayscale float32 in [0,1]."""
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Could not read image: {path}")
    img = img.astype(np.float32) / 255.0
    return img


def mse(x: np.ndarray, y: np.ndarray) -> float:
    return float(np.mean((x - y) ** 2))


def psnr(x: np.ndarray, y: np.ndarray, data_range: float = 1.0) -> float:
    m = mse(x, y)
    if m == 0:
        return float("inf")
    return float(10 * np.log10((data_range ** 2) / m))


def ssim_image(x: np.ndarray, y: np.ndarray, data_range: float = 1.0) -> float:
    return float(ssim(x, y, data_range=data_range))


# ======================
#   A. Gaussian noise
# ======================

def add_gaussian_noise(img: np.ndarray, sigma: float) -> np.ndarray:
    """Add zero-mean Gaussian noise with std=sigma (in [0,1] scale)."""
    noise = np.random.normal(0.0, sigma, img.shape).astype(np.float32)
    noisy = img + noise
    noisy = np.clip(noisy, 0.0, 1.0)
    return noisy


def experiment_gaussian_noise(img: np.ndarray,
                              sigma1,
                              out_dir: str,
                              img_name: str = "image") -> dict:
    sigmas = np.sqrt(sigma1) / 255.0
    ensure_dir(out_dir)
    mse_list, psnr_list, ssim_list = [], [], []

    for sigma in sigmas:
        noisy = add_gaussian_noise(img, sigma)
        m = mse(img, noisy)
        p = psnr(img, noisy)
        s = ssim_image(img, noisy)

        mse_list.append(float(m))
        psnr_list.append(float(p))
        ssim_list.append(float(s))

        # Save example noisy image
        out_path = os.path.join(out_dir, f"{img_name}_gauss_sigma{sigma}.png") #origginal sigma
        cv2.imwrite(out_path, (noisy * 255).astype(np.uint8))

    # Save numeric results
    results = {
        "sigma": list(map(float, sigmas)),
        "mse": mse_list,
        "psnr": psnr_list,
        "ssim": ssim_list,
    }
    with open(os.path.join(out_dir, f"{img_name}_gaussian_noise_results.json"), "w") as f:
        json.dump(results, f, indent=2)

    # Plot PSNR / SSIM / MSE vs sigma
    plt.figure()
    plt.plot(sigma1, psnr_list, marker="o")
    plt.xlabel("Sigma (noise std)")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs Sigma (Gaussian noise) - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_gaussian_psnr_vs_sigma.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(sigma1, ssim_list, marker="o")
    plt.xlabel("Sigma (noise std)")
    plt.ylabel("SSIM")
    plt.title(f"SSIM vs Sigma (Gaussian noise) - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_gaussian_ssim_vs_sigma.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(sigma1, mse_list, marker="o")
    plt.xlabel("Sigma (noise std)")
    plt.ylabel("MSE")
    plt.title(f"MSE vs Sigma (Gaussian noise) - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_gaussian_mse_vs_sigma.png"), dpi=200)
    plt.close()

    return results


# ======================
#   B. Motion blur
# ======================

def motion_blur_kernel(length: float, angle_deg: float) -> np.ndarray:
    """Create a linear motion blur PSF kernel."""
    eps = 1e-8
    size = int(length)
    if size < 1:
        size = 1

    kernel = np.zeros((size, size), dtype=np.float32)
    kernel[size // 2, :] = 1.0

    # Rotate
    M = cv2.getRotationMatrix2D((size / 2 - 0.5, size / 2 - 0.5), angle_deg, 1.0)
    kernel = cv2.warpAffine(kernel, M, (size, size))
    kernel = kernel / (kernel.sum() + eps)
    return kernel


def apply_motion_blur(img: np.ndarray, length: float, angle: float = 0.0) -> np.ndarray:
    k = motion_blur_kernel(length, angle)
    blurred = cv2.filter2D(img, -1, k, borderType=cv2.BORDER_REPLICATE)
    return blurred


def experiment_motion_blur(img: np.ndarray,
                           lengths,
                           angle_deg: float,
                           out_dir: str,
                           img_name: str = "image") -> dict:
    ensure_dir(out_dir)
    mse_list, psnr_list, ssim_list = [], [], []

    for L in lengths:
        blurred = apply_motion_blur(img, L, angle_deg)
        m = mse(img, blurred)
        p = psnr(img, blurred)
        s = ssim_image(img, blurred)

        mse_list.append(float(m))
        psnr_list.append(float(p))
        ssim_list.append(float(s))

        out_path = os.path.join(out_dir, f"{img_name}_motion_L{L:.1f}_angle{angle_deg}.png")
        cv2.imwrite(out_path, (blurred * 255).astype(np.uint8))

    results = {
        "length": list(map(float, lengths)),
        "mse": mse_list,
        "psnr": psnr_list,
        "ssim": ssim_list,
    }
    with open(os.path.join(out_dir, f"{img_name}_motion_results.json"), "w") as f:
        json.dump(results, f, indent=2)

    # Plot
    plt.figure()
    plt.plot(lengths, psnr_list, marker="o")
    plt.xlabel("Motion blur length (pixels)")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs motion blur length - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_motion_psnr_vs_length.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(lengths, ssim_list, marker="o")
    plt.xlabel("Motion blur length (pixels)")
    plt.ylabel("SSIM")
    plt.title(f"SSIM vs motion blur length - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_motion_ssim_vs_length.png"), dpi=200)
    plt.close()

    return results


# ======================
#   C. JPEG compression
# ======================

def jpeg_compress_gray(img: np.ndarray, quality: int) -> np.ndarray:
    """Compress & decompress using OpenCV JPEG (expects img in [0,1])."""
    img_u8 = (np.clip(img, 0, 1) * 255).astype(np.uint8)
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), int(quality)]
    ok, enc = cv2.imencode(".jpg", img_u8, encode_param)
    if not ok:
        raise RuntimeError("JPEG encoding failed")
    dec = cv2.imdecode(enc, cv2.IMREAD_GRAYSCALE)
    return dec.astype(np.float32) / 255.0


def experiment_jpeg(img: np.ndarray,
                    qualities,
                    out_dir: str,
                    img_name: str = "image") -> dict:
    ensure_dir(out_dir)
    mse_list, psnr_list, ssim_list = [], [], []

    for q in qualities:
        jpg_img = jpeg_compress_gray(img, q)
        m = mse(img, jpg_img)
        p = psnr(img, jpg_img)
        s = ssim_image(img, jpg_img)

        mse_list.append(float(m))
        psnr_list.append(float(p))
        ssim_list.append(float(s))

        out_path = os.path.join(out_dir, f"{img_name}_jpeg_Q{q}.png")
        cv2.imwrite(out_path, (jpg_img * 255).astype(np.uint8))

    results = {
        "quality": list(map(int, qualities)),
        "mse": mse_list,
        "psnr": psnr_list,
        "ssim": ssim_list,
    }
    with open(os.path.join(out_dir, f"{img_name}_jpeg_results.json"), "w") as f:
        json.dump(results, f, indent=2)

    # Plot
    plt.figure()
    plt.plot(qualities, psnr_list, marker="o")
    plt.xlabel("JPEG quality")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs JPEG quality - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_jpeg_psnr_vs_quality.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(qualities, ssim_list, marker="o")
    plt.xlabel("JPEG quality")
    plt.ylabel("SSIM")
    plt.title(f"SSIM vs JPEG quality - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_jpeg_ssim_vs_quality.png"), dpi=200)
    plt.close()

    return results


# ======================
#   E. Subjective test helper (MOS)
# ======================

def save_pair_for_subjective_test(img1: np.ndarray,
                                  img2: np.ndarray,
                                  out_dir: str,
                                  pair_name: str) -> None:
    """
    Save a 2x1 concatenated image for subjective comparison.
    The actual 'survey' (which looks better) is done with human viewers.
    """
    ensure_dir(out_dir)
    h, w = img1.shape
    concat = np.zeros((h, 2 * w), dtype=np.float32)
    concat[:, :w] = img1
    concat[:, w:] = img2
    out_path = os.path.join(out_dir, f"pair_{pair_name}.png")
    cv2.imwrite(out_path, (concat * 255).astype(np.uint8))
    print(f"Saved subjective pair: {out_path}")


# ======================
#   Main runner for Part A
# ======================

def PART_A():
    """
    Run all experiments for Part A:
    - Gaussian noise
    - Motion blur
    - JPEG compression
    - Subjective pair generation
    """
    # ---- EDIT THESE PATHS ----
    IMAGE_PATHS = [
        r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\Skateboard.png",
        r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\TV.png",
        r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\WolvesLocations.png",
    ]
    OUTPUT_DIR = r"C:\Users\zivza\Ziv\English\multymedia\output_part_a"
    # --------------------------

    ensure_dir(OUTPUT_DIR)

    # Distortion levels (can be adjusted)
    sigma1 = np.array ([5,10,20,40,80])          # Sigma givven values in wuestion
    sigmas = np.sqrt(sigma1) / 255.0
    motion_lengths = [1, 3, 5, 9, 13]          # pixels
    motion_angle_deg = 0.0                     # horizontal blur
    jpeg_qualities = [10, 20, 40, 60, 80, 95]

    for img_path in IMAGE_PATHS:
        img_name = os.path.splitext(os.path.basename(img_path))[0]
        img = load_image_gray(img_path)

        img_out_dir = os.path.join(OUTPUT_DIR, img_name)
        ensure_dir(img_out_dir)

        print(f"\n--- Processing image: {img_name} ---")

        # A. Gaussian noise
        gauss_dir = os.path.join(img_out_dir, "gaussian_noise")
        experiment_gaussian_noise(img, sigma1, gauss_dir, img_name)

        # B. Motion blur
        motion_dir = os.path.join(img_out_dir, "motion_blur")
        experiment_motion_blur(img, motion_lengths, motion_angle_deg,
                               motion_dir, img_name)

        # C. JPEG compression
        jpeg_dir = os.path.join(img_out_dir, "jpeg")
        experiment_jpeg(img, jpeg_qualities, jpeg_dir, img_name)

        # E. Subjective pairs example
        noisy_example = add_gaussian_noise(img, sigma=0.05)
        blurred_example = apply_motion_blur(img, length=5, angle=0.0)
        subj_dir = os.path.join(img_out_dir, "subjective_pairs")
        save_pair_for_subjective_test(
            noisy_example,
            blurred_example,
            subj_dir,
            pair_name="noise_vs_blur"
        )

    print("\nDone. Check the Part A output folder for images, graphs, and JSON files.")


if __name__ == "__main__":
    PART_A()
