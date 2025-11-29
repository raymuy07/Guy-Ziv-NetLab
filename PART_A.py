import os
import cv2
import json
import imageio
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ======================
#   Utility functions
# ======================

def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def load_image_gray(path):
    """Load image as grayscale float32 in [0,1]."""
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Could not read image: {path}")
    img = img.astype(np.float32) / 255.0
    return img


def mse(x, y):
    return np.mean((x - y) ** 2)


def psnr(x, y, data_range=1.0):
    m = mse(x, y)
    if m == 0:
        return float('inf')
    return 10 * np.log10((data_range ** 2) / m)


def ssim_image(x, y, data_range=1.0):
    return ssim(x, y, data_range=data_range)


# ======================
#   A. Gaussian noise
# ======================

def add_gaussian_noise(img, sigma):
    """Add zero-mean Gaussian noise with std=sigma (in [0,1] scale)."""
    noise = np.random.normal(0.0, sigma, img.shape).astype(np.float32)
    noisy = img + noise
    noisy = np.clip(noisy, 0.0, 1.0)
    return noisy


def experiment_gaussian_noise(img, sigmas, out_dir, img_name="image"):
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
        out_path = os.path.join(out_dir, f"{img_name}_gauss_sigma{sigma:.3f}.png")
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

    # Plot PSNR / SSIM vs sigma
    plt.figure()
    plt.plot(sigmas, psnr_list, marker='o')
    plt.xlabel("Sigma (noise std)")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs Sigma (Gaussian noise) - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_gaussian_psnr_vs_sigma.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(sigmas, ssim_list, marker='o')
    plt.xlabel("Sigma (noise std)")
    plt.ylabel("SSIM")
    plt.title(f"SSIM vs Sigma (Gaussian noise) - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_gaussian_ssim_vs_sigma.png"), dpi=200)
    plt.close()

    return results


# ======================
#   B. Motion blur
# ======================

def motion_blur_kernel(length, angle_deg):
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


def apply_motion_blur(img, length, angle=0.0):
    k = motion_blur_kernel(length, angle)
    blurred = cv2.filter2D(img, -1, k, borderType=cv2.BORDER_REPLICATE)
    return blurred


def experiment_motion_blur(img, lengths, angle_deg, out_dir, img_name="image"):
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
    plt.plot(lengths, psnr_list, marker='o')
    plt.xlabel("Motion blur length (pixels)")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs motion blur length - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_motion_psnr_vs_length.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(lengths, ssim_list, marker='o')
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

def jpeg_compress_gray(img, quality):
    """Compress & decompress using OpenCV JPEG (expects img in [0,1])."""
    img_u8 = (np.clip(img, 0, 1) * 255).astype(np.uint8)
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), int(quality)]
    ok, enc = cv2.imencode(".jpg", img_u8, encode_param)
    if not ok:
        raise RuntimeError("JPEG encoding failed")
    dec = cv2.imdecode(enc, cv2.IMREAD_GRAYSCALE)
    return dec.astype(np.float32) / 255.0


def experiment_jpeg(img, qualities, out_dir, img_name="image"):
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
    plt.plot(qualities, psnr_list, marker='o')
    plt.xlabel("JPEG quality")
    plt.ylabel("PSNR [dB]")
    plt.title(f"PSNR vs JPEG quality - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_jpeg_psnr_vs_quality.png"), dpi=200)
    plt.close()

    plt.figure()
    plt.plot(qualities, ssim_list, marker='o')
    plt.xlabel("JPEG quality")
    plt.ylabel("SSIM")
    plt.title(f"SSIM vs JPEG quality - {img_name}")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{img_name}_jpeg_ssim_vs_quality.png"), dpi=200)
    plt.close()

    return results


# ======================
#   D. Temporal distortions (video)
# ======================

def read_video_gray(path):
    """Read video, return list of grayscale float32 frames in [0,1]."""
    reader = imageio.get_reader(path)
    frames = []
    for f in reader:
        gray = cv2.cvtColor(f, cv2.COLOR_RGB2GRAY)
        gray = gray.astype(np.float32) / 255.0
        frames.append(gray)
    reader.close()
    return frames


def drop_frames(frames, drop_step):
    """
    Drop frames in a periodic pattern.
    drop_step=2 -> keep every 2nd frame.
    """
    if drop_step <= 1:
        return frames[:]
    dropped = [fr for i, fr in enumerate(frames) if (i % drop_step) == 0]
    return dropped


def resample_to_original_length(frames_short, target_len):
    """
    Simple temporal upsampling by frame repetition to match original length.
    """
    if len(frames_short) == 0:
        raise ValueError("frames_short is empty")
    idxs = np.linspace(0, len(frames_short) - 1, num=target_len)
    idxs = np.round(idxs).astype(int)
    return [frames_short[i] for i in idxs]


def video_metrics_per_frame(frames_ref, frames_dist):
    """
    Compute PSNR and SSIM per frame (assuming same length and shape).
    """
    assert len(frames_ref) == len(frames_dist)
    psnr_list, ssim_list = [], []
    for f_ref, f_dist in zip(frames_ref, frames_dist):
        psnr_list.append(float(psnr(f_ref, f_dist)))
        ssim_list.append(float(ssim_image(f_ref, f_dist)))
    return psnr_list, ssim_list


def experiment_temporal_distortions(video_path, drop_steps, out_dir):
    """
    For each drop_step, create a 'distorted' sequence, resample back to
    original length, compute PSNR/SSIM per frame, and plot over time.
    """
    ensure_dir(out_dir)
    frames = read_video_gray(video_path)
    T = len(frames)
    print(f"Loaded video with {T} frames")

    all_results = {}

    for drop_step in drop_steps:
        distorted_short = drop_frames(frames, drop_step)
        distorted = resample_to_original_length(distorted_short, T)

        psnr_list, ssim_list = video_metrics_per_frame(frames, distorted)

        # Save curves
        t = np.arange(T)
        plt.figure()
        plt.plot(t, psnr_list)
        plt.xlabel("Frame index")
        plt.ylabel("PSNR [dB]")
        plt.title(f"Per-frame PSNR, drop_step={drop_step}")
        plt.grid(True)
        plt.savefig(os.path.join(out_dir, f"video_psnr_drop{drop_step}.png"), dpi=200)
        plt.close()

        plt.figure()
        plt.plot(t, ssim_list)
        plt.xlabel("Frame index")
        plt.ylabel("SSIM")
        plt.title(f"Per-frame SSIM, drop_step={drop_step}")
        plt.grid(True)
        plt.savefig(os.path.join(out_dir, f"video_ssim_drop{drop_step}.png"), dpi=200)
        plt.close()

        all_results[drop_step] = {
            "psnr": psnr_list,
            "ssim": ssim_list,
        }

    with open(os.path.join(out_dir, "video_temporal_results.json"), "w") as f:
        json.dump(all_results, f, indent=2)

    return all_results


# ======================
#   E. Subjective test helper (MOS)
# ======================

def save_pair_for_subjective_test(img1, img2, out_dir, pair_name):
    """
    Save a 2x1 concatenated image for subjective comparison.
    The actual 'survey' (which looks better) is done with human viewers.
    """
    ensure_dir(out_dir)
    # both images assumed [0,1] float32
    h, w = img1.shape
    concat = np.zeros((h, 2 * w), dtype=np.float32)
    concat[:, :w] = img1
    concat[:, w:] = img2
    out_path = os.path.join(out_dir, f"pair_{pair_name}.png")
    cv2.imwrite(out_path, (concat * 255).astype(np.uint8))
    print(f"Saved subjective pair: {out_path}")



def PART_A():

    IMAGE_PATHS = [
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\TV.png",
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\WolvesLocations.png",
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\Skateboard.png"
    ]
    VIDEO_PATH = r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\lake.mp4"
    OUTPUT_DIR = r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\output_partA"


    ensure_dir(OUTPUT_DIR)

    # Distortion levels (can be adjusted)
    sigmas = np.linspace(0.01, 0.1, 6)         # Gaussian noise std in [0,1]
    motion_lengths = [1, 3, 5, 9, 13]          # pixels
    motion_angle_deg = 0.0                     # horizontal blur
    jpeg_qualities = [10, 20, 40, 60, 80, 95]
    drop_steps = [1, 2, 3, 4]                  # 1=no drop, 2=drop every second frame, etc.

    # ================
    # Images part (A,B,C,E)
    # ================
    for img_path in IMAGE_PATHS:
        img_name = os.path.splitext(os.path.basename(img_path))[0]
        img = load_image_gray(img_path)

        img_out_dir = os.path.join(OUTPUT_DIR, img_name)
        ensure_dir(img_out_dir)

        print(f"\n--- Processing image: {img_name} ---")

        # A. Gaussian noise
        gauss_dir = os.path.join(img_out_dir, "gaussian_noise")
        gaussian_results = experiment_gaussian_noise(img, sigmas, gauss_dir, img_name)

        # B. Motion blur
        motion_dir = os.path.join(img_out_dir, "motion_blur")
        motion_results = experiment_motion_blur(img, motion_lengths, motion_angle_deg,
                                                motion_dir, img_name)

        # C. JPEG compression
        jpeg_dir = os.path.join(img_out_dir, "jpeg")
        jpeg_results = experiment_jpeg(img, jpeg_qualities, jpeg_dir, img_name)

        # E. Subjective pairs example (choose two distortions with similar PSNR but different SSIM)
        # (Here just a simple example; you should choose meaningfully from your results)
        # Example: noisy image with sigma=0.05 vs blurred with length=5
        noisy_example = add_gaussian_noise(img, sigma=0.05)
        blurred_example = apply_motion_blur(img, length=5, angle=0.0)
        subj_dir = os.path.join(img_out_dir, "subjective_pairs")
        save_pair_for_subjective_test(noisy_example, blurred_example, subj_dir,
                                      pair_name="noise_vs_blur")

        # You can log PSNR/SSIM here and then ask humans which looks better
        print("Example subjective pair metrics:")
        print("Noise sigma=0.05:",
              "PSNR=", psnr(img, noisy_example),
              "SSIM=", ssim_image(img, noisy_example))
        print("Blur length=5:",
              "PSNR=", psnr(img, blurred_example),
              "SSIM=", ssim_image(img, blurred_example))

    # ================
    # Video part (D)
    # ================
    print("\n--- Processing video (temporal distortions) ---")
    video_out_dir = os.path.join(OUTPUT_DIR, "video_temporal")
    temporal_results = experiment_temporal_distortions(VIDEO_PATH, drop_steps, video_out_dir)

    print("\nDone. Check the 'output' folder for images, graphs, and JSON files.")


