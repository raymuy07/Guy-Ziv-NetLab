import cv2
import numpy as np
import matplotlib.pyplot as plt
from skimage.metrics import structural_similarity as ssim


# ========== CONFIGURATION ==========

VIDEO_PATH = r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\lake.mp4"
DISTORTED_VIDEO_PATH = r"C:\Users\zivza\Ziv\English\multymedia\Zivspics\distorted_fd.mp4"
MAX_DROP_PROB = 0.4                               # max frame drop probability (towards end)


# ========== BASIC METRIC FUNCTIONS ==========

def mse(x: np.ndarray, y: np.ndarray) -> float:
    """Mean Squared Error between two gray frames in [0,1]."""
    return float(np.mean((x - y) ** 2))


def psnr(x: np.ndarray, y: np.ndarray) -> float:
    """PSNR in dB for images in [0,1]."""
    m = mse(x, y)
    if m == 0:
        return float("inf")
    return float(10 * np.log10(1.0 / m))


def to_gray_01(frame_bgr: np.ndarray) -> np.ndarray:
    """Convert BGR frame (0..255) to grayscale float32 in [0,1]."""
    gray = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2GRAY)
    return gray.astype(np.float32) / 255.0


# ========== VIDEO IO HELPERS ==========

def load_video_gray(path: str):
    """
    Load all frames from a video, return:
    - frames_gray: list of 2D arrays (H,W) in [0,1]
    - fps: frames per second
    - size: (width, height)
    """
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        raise FileNotFoundError(f"Cannot open video: {path}")

    fps = cap.get(cv2.CAP_PROP_FPS)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    frames_gray = []
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        frames_gray.append(to_gray_01(frame))

    cap.release()
    return frames_gray, fps, (width, height)


def save_video_gray(frames_gray, path: str, fps: float, size):
    """
    Save a list of gray frames [0,1] as an MP4 video (converted to BGR).
    """
    width, height = size
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    out = cv2.VideoWriter(path, fourcc, fps, (width, height), isColor=True)

    for g in frames_gray:
        g_clipped = np.clip(g, 0.0, 1.0)
        g_u8 = (g_clipped * 255).astype(np.uint8)
        bgr = cv2.cvtColor(g_u8, cv2.COLOR_GRAY2BGR)
        out.write(bgr)

    out.release()


# ========== FRAME DROPPING (GRADED) ==========

def apply_graded_frame_dropping(frames_gray, max_drop_prob: float):
    """
    Create a temporally distorted sequence using graded frame dropping.

    Idea:
    - We keep the same number of frames as the original.
    - For each time index i, we compute a drop probability that grows over time.
    - If we 'drop' a frame, we repeat the previous distorted frame (freeze effect).
    """
    num_frames = len(frames_gray)
    if num_frames == 0:
        raise RuntimeError("No frames in input video.")

    distorted = []
    # First frame is always kept
    distorted.append(frames_gray[0])

    for i in range(1, num_frames):
        # Linearly increasing drop probability from 0 to max_drop_prob
        p_drop = max_drop_prob * (i / (num_frames - 1))

        if np.random.rand() < p_drop:
            # Drop: repeat previous distorted frame (temporal freeze)
            distorted.append(distorted[-1])
        else:
            # Keep current original frame
            distorted.append(frames_gray[i])

    return distorted


# ========== PER-FRAME METRICS ==========

def compute_metrics_per_frame(orig_frames, dist_frames):
    """
    Compute PSNR and SSIM per frame between original and distorted sequences.
    Assumes same length and same spatial size.
    """
    if len(orig_frames) != len(dist_frames):
        raise ValueError("Original and distorted sequences must have same number of frames.")

    psnr_vals = []
    ssim_vals = []

    for ref, dis in zip(orig_frames, dist_frames):
        p = psnr(ref, dis)
        s = float(ssim(ref, dis, data_range=1.0))
        psnr_vals.append(p)
        ssim_vals.append(s)

    return psnr_vals, ssim_vals


# ========== PLOTTING ==========

def plot_metrics_over_time(psnr_vals, ssim_vals):
    """Plot PSNR and SSIM per frame as a function of frame index."""
    num_frames = len(psnr_vals)
    idx = np.arange(num_frames)

    # PSNR over time
    plt.figure()
    plt.plot(idx, psnr_vals, "-")
    plt.xlabel("Frame index")
    plt.ylabel("PSNR [dB]")
    plt.title("Per-frame PSNR over time (graded frame dropping)")
    plt.grid(True)
    plt.show()

    # SSIM over time
    plt.figure()
    plt.plot(idx, ssim_vals, "-")
    plt.xlabel("Frame index")
    plt.ylabel("SSIM")
    plt.title("Per-frame SSIM over time (graded frame dropping)")
    plt.grid(True)
    plt.show()


# ========== MAIN RUNNER FOR PART D ==========

def PART_D():
    """
    Part D: Video Temporal Distortions with graded frame dropping.

    1. Load original clip.
    2. Create distorted sequence with graded frame dropping (and keep same length).
    3. Optionally save distorted video.
    4. Compute per-frame PSNR/SSIM.
    5. Plot metrics over time.
    """
    # 1. Load original video
    orig_frames, fps, size = load_video_gray(VIDEO_PATH)
    print(f"Loaded {len(orig_frames)} frames, fps={fps}, size={size}")

    # 2. Apply graded frame dropping
    dist_frames = apply_graded_frame_dropping(orig_frames, MAX_DROP_PROB)
    print("Created distorted sequence with graded frame dropping.")

    # 3. Save distorted video (optional)
    save_video_gray(dist_frames, DISTORTED_VIDEO_PATH, fps, size)
    print(f"Distorted video saved to: {DISTORTED_VIDEO_PATH}")

    # 4. Compute per-frame metrics
    psnr_vals, ssim_vals = compute_metrics_per_frame(orig_frames, dist_frames)
    print("Computed per-frame PSNR and SSIM.")

    # 5. Plot metrics over time
    plot_metrics_over_time(psnr_vals, ssim_vals)
    print("Done. Check the plots and distorted video file.")


if __name__ == "__main__":
    PART_D()
