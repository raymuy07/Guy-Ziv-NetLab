import os
import numpy as np
import cv2
import matplotlib.pyplot as plt
from typing import Tuple


# ======================
#   Utility functions
# ======================

def ensure_dir(path: str) -> None:
    os.makedirs(path, exist_ok=True)


def load_gray(path: str) -> np.ndarray:
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Could not read image: {path}")
    return img.astype(np.float32) / 255.0


# ======================
#   Part B1: ESF / LSF / MTF
# ======================

def extract_roi(img: np.ndarray,
                roi: Tuple[int, int, int, int]) -> np.ndarray:
    """
    roi = (x0, y0, x1, y1)
    User should pick ROI around the slanted edge.
    """
    x0, y0, x1, y1 = roi
    return img[y0:y1, x0:x1]


def compute_esf_from_slanted_edge(roi: np.ndarray) -> np.ndarray:
    """
    Very simplified ESF estimation:
    - For each row, compute gradient along x.
    - Find edge location (argmax of gradient).
    - Align rows by shifting so that edge centers match.
    - Average aligned rows to get 1D ESF.
    This is not a perfect ISO 12233 implementation but follows the idea.
    """
    h, w = roi.shape
    grad = np.diff(roi, axis=1)  # horizontal gradient (h, w-1)

    # find edge positions per row
    edge_pos = np.argmax(grad, axis=1)  # (h,)

    # reference position = median edge index
    ref_pos = int(np.median(edge_pos))

    aligned_rows = []
    for y in range(h):
        row = roi[y, :]
        shift = ref_pos - edge_pos[y]
        # shift row using np.roll (circular) then crop
        row_shifted = np.roll(row, shift)
        aligned_rows.append(row_shifted)

    aligned_rows = np.stack(aligned_rows, axis=0)  # (h, w)
    esf = np.mean(aligned_rows, axis=0)           # 1D ESF
    # normalize to [0,1]
    esf_min, esf_max = esf.min(), esf.max()
    if esf_max > esf_min:
        esf = (esf - esf_min) / (esf_max - esf_min)
    else:
        esf[:] = 0.0
    return esf


def compute_lsf_from_esf(esf: np.ndarray) -> np.ndarray:
    """
    LSF = derivative of ESF.
    """
    lsf = np.diff(esf)
    return lsf


def compute_mtf_from_lsf(lsf: np.ndarray, sample_spacing: float = 1.0):
    """
    Compute 1D MTF (magnitude of FFT of LSF), normalized s.t. MTF(0)=1.
    Returns (freqs, mtf).
    """
    # zero-pad a bit for smoother spectrum
    n = len(lsf)
    n_fft = int(2 ** np.ceil(np.log2(4 * n)))
    L = np.zeros(n_fft, dtype=np.float32)
    L[:n] = lsf

    L_fft = np.fft.fft(L)
    mtf = np.abs(L_fft)

    # frequencies (cycles per pixel)
    freqs = np.fft.fftfreq(n_fft, d=sample_spacing)

    # keep only non-negative frequencies
    mask = freqs >= 0
    freqs = freqs[mask]
    mtf = mtf[mask]

    # normalize so that MTF(f=0) = 1
    if mtf[0] != 0:
        mtf = mtf / mtf[0]
    return freqs, mtf


def find_mtf50(freqs: np.ndarray, mtf: np.ndarray) -> float:
    """
    Find frequency where MTF falls to 0.5 (MTF50).
    Uses linear interpolation between nearest points.
    """
    # we assume mtf is monotonically decreasing (approx)
    # find first index where mtf < 0.5
    below = np.where(mtf <= 0.5)[0]
    if len(below) == 0:
        # never reaches 0.5
        return float(freqs[-1])

    idx = below[0]
    if idx == 0:
        return float(freqs[0])

    # interpolate between (idx-1, idx)
    x0, y0 = freqs[idx - 1], mtf[idx - 1]
    x1, y1 = freqs[idx], mtf[idx]
    if y1 == y0:
        return float(x1)
    alpha = (0.5 - y0) / (y1 - y0)
    return float(x0 + alpha * (x1 - x0))


# ======================
#   Part B2: Deblurring (Inverse & Wiener)
# ======================

def build_psf_from_lsf(lsf: np.ndarray, length: int) -> np.ndarray:
    """
    Build 1D PSF from 1D LSF by centering it in an array of given length.
    """
    psf = np.zeros(length, dtype=np.float32)
    n = min(len(lsf), length)
    start = (length - n) // 2
    psf[start:start + n] = lsf[:n]
    # normalize energy
    s = psf.sum()
    if s != 0:
        psf /= s
    return psf


def deblur_inverse_filter(img: np.ndarray,
                          psf: np.ndarray,
                          knee_freq_factor: float = 0.5) -> np.ndarray:
    """
    Simple 1D inverse filter along rows with 'knee' frequency.
    knee_freq_factor: fraction of Nyquist at which to start tapering.
    """
    h, w = img.shape

    # FFT of PSF
    H = np.fft.fft(psf, n=w)
    freqs = np.fft.fftfreq(w, d=1.0)
    nyquist = 0.5
    fk = knee_freq_factor * nyquist

    # Build low-pass taper W(f)
    W = np.ones_like(H, dtype=np.float32)
    for i, f in enumerate(freqs):
        af = abs(f)
        if af > fk:
            # simple cosine taper
            t = min(1.0, (af - fk) / (nyquist - fk + 1e-6))
            W[i] = float(0.5 * (1.0 + np.cos(np.pi * t)))  # from 1 to 0

    # Inverse filter with taper
    eps = 1e-4
    H_inv = np.conj(H) / (np.abs(H) ** 2 + eps)
    Filt = H_inv * W

    # Apply rowwise
    out = np.zeros_like(img)
    for y in range(h):
        row = img[y, :]
        R = np.fft.fft(row)
        F_hat = R * Filt
        f_rec = np.fft.ifft(F_hat).real
        out[y, :] = f_rec

    out = np.clip(out, 0.0, 1.0)
    return out


def deblur_wiener(img: np.ndarray,
                  psf: np.ndarray,
                  K: float = 0.01) -> np.ndarray:
    """
    Simple 1D Wiener filter along rows.
    K ~ noise/ signal ratio (tune based on experiment).
    """
    h, w = img.shape

    H = np.fft.fft(psf, n=w)
    eps = 1e-8
    H_conj = np.conj(H)
    H_abs2 = np.abs(H) ** 2

    # Wiener filter
    Filt = H_conj / (H_abs2 + K + eps)

    out = np.zeros_like(img)
    for y in range(h):
        row = img[y, :]
        R = np.fft.fft(row)
        F_hat = R * Filt
        f_rec = np.fft.ifft(F_hat).real
        out[y, :] = f_rec

    out = np.clip(out, 0.0, 1.0)
    return out


# ======================
#   Part B3: Blur metrics B1, B2, B3
# ======================

def blur_metric_B1_from_esf(esf: np.ndarray) -> float:
    """
    B1: 1 / (x90 - x10), where x10, x90 are 10% and 90% points of ESF.
    Larger B1 => sharper.
    """
    # assume esf already normalized [0,1]
    x = np.arange(len(esf))

    def interp_cross(level: float) -> float:
        idx = np.where(esf >= level)[0]
        if len(idx) == 0:
            return float(x[-1])
        i = idx[0]
        if i == 0:
            return float(x[0])
        x0, y0 = x[i - 1], esf[i - 1]
        x1, y1 = x[i], esf[i]
        if y1 == y0:
            return float(x1)
        a = (level - y0) / (y1 - y0)
        return float(x0 + a * (x1 - x0))

    x10 = interp_cross(0.1)
    x90 = interp_cross(0.9)
    width = x90 - x10 if x90 > x10 else 1.0
    B1 = 1.0 / width
    return float(B1)


def blur_metric_B2_from_mtf50(mtf50_freq: float) -> float:
    """
    B2 = MTF50 frequency. Higher => sharper.
    """
    return float(mtf50_freq)


def blur_metric_B3_global_horizontal_gradient(img: np.ndarray) -> float:
    """
    B3: mean absolute horizontal gradient over the (normalized) image.
    """
    grad = np.diff(img, axis=1)
    return float(np.mean(np.abs(grad)))


# ======================
#   Plot helpers
# ======================

def plot_esf_lsf_mtf(esf: np.ndarray,
                     lsf: np.ndarray,
                     freqs: np.ndarray,
                     mtf: np.ndarray,
                     out_dir: str,
                     base_name: str) -> None:
    ensure_dir(out_dir)

    # ESF
    plt.figure()
    plt.plot(esf)
    plt.title(f"ESF - {base_name}")
    plt.xlabel("Position (pixels)")
    plt.ylabel("Normalized intensity")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{base_name}_ESF.png"), dpi=200)
    plt.close()

    # LSF
    plt.figure()
    plt.plot(lsf)
    plt.title(f"LSF - {base_name}")
    plt.xlabel("Position (pixels)")
    plt.ylabel("Derivative")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{base_name}_LSF.png"), dpi=200)
    plt.close()

    # MTF
    plt.figure()
    plt.plot(freqs, mtf)
    plt.title(f"MTF - {base_name}")
    plt.xlabel("Spatial frequency [cycles/pixel]")
    plt.ylabel("MTF")
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, f"{base_name}_MTF.png"), dpi=200)
    plt.close()


# ======================
#   Main runner for Part B
# ======================

def PART_B():
    """
    Practical part:
    - Load drone images with slanted edge.
    - Extract ESF/LSF/MTF and MTF50.
    - Build PSF from LSF.
    - Apply inverse filter and Wiener filter.
    - Compute blur metrics B1, B2, B3 for:
        * original blurred image
        * inverse-filtered image
        * Wiener-filtered image
    """
    # ---- EDIT THESE PATHS ----
    IMAGE_PATHS = [
        r"path\to\drone_image1.png",
        r"path\to\drone_image2.png",
        r"path\to\drone_image3.png",
        r"path\to\drone_image4.png",  # maybe vertical blur example
    ]
    # ROI per image (x0, y0, x1, y1) around the slanted edge
    # >>> YOU MUST TUNE THESE COORDINATES <<<
    ROIs = [
        (100, 100, 400, 400),
        (120, 120, 420, 420),
        (80, 80, 380, 380),
        (90, 90, 390, 390),
    ]
    OUTPUT_DIR = r"path\to\output_folder_part_b"
    # ----------------------------

    ensure_dir(OUTPUT_DIR)

    summary_rows = []

    for img_path, roi in zip(IMAGE_PATHS, ROIs):
        base_name = os.path.splitext(os.path.basename(img_path))[0]
        print(f"\n--- Processing Part B image: {base_name} ---")

        img = load_gray(img_path)
        roi_img = extract_roi(img, roi)

        # ESF / LSF / MTF / MTF50
        esf = compute_esf_from_slanted_edge(roi_img)
        lsf = compute_lsf_from_esf(esf)
        freqs, mtf = compute_mtf_from_lsf(lsf)
        mtf50 = find_mtf50(freqs, mtf)

        # PSF from LSF
        psf = build_psf_from_lsf(lsf, length=img.shape[1])

        # Deblur images
        inv_img = deblur_inverse_filter(img, psf, knee_freq_factor=0.5)
        wiener_img = deblur_wiener(img, psf, K=0.01)

        # Metrics B1/B2/B3
        B1_blur = blur_metric_B1_from_esf(esf)
        B2_blur = blur_metric_B2_from_mtf50(mtf50)
        B3_blur = blur_metric_B3_global_horizontal_gradient(img)

        # For restored images, recompute ESF/LSF/MTF from same ROI
        roi_inv = extract_roi(inv_img, roi)
        roi_wiener = extract_roi(wiener_img, roi)

        esf_inv = compute_esf_from_slanted_edge(roi_inv)
        lsf_inv = compute_lsf_from_esf(esf_inv)
        freqs_inv, mtf_inv = compute_mtf_from_lsf(lsf_inv)
        mtf50_inv = find_mtf50(freqs_inv, mtf_inv)

        esf_w = compute_esf_from_slanted_edge(roi_wiener)
        lsf_w = compute_lsf_from_esf(esf_w)
        freqs_w, mtf_w = compute_mtf_from_lsf(lsf_w)
        mtf50_w = find_mtf50(freqs_w, mtf_w)

        B1_inv = blur_metric_B1_from_esf(esf_inv)
        B2_inv = blur_metric_B2_from_mtf50(mtf50_inv)
        B3_inv = blur_metric_B3_global_horizontal_gradient(inv_img)

        B1_w = blur_metric_B1_from_esf(esf_w)
        B2_w = blur_metric_B2_from_mtf50(mtf50_w)
        B3_w = blur_metric_B3_global_horizontal_gradient(wiener_img)

        # Save plots for original image
        img_out_dir = os.path.join(OUTPUT_DIR, base_name)
        ensure_dir(img_out_dir)

        plot_esf_lsf_mtf(esf, lsf, freqs, mtf, img_out_dir, base_name + "_blurred")

        # Plots for restored images (optional)
        plot_esf_lsf_mtf(esf_inv, lsf_inv, freqs_inv, mtf_inv,
                         img_out_dir, base_name + "_inverse")
        plot_esf_lsf_mtf(esf_w, lsf_w, freqs_w, mtf_w,
                         img_out_dir, base_name + "_wiener")

        # Save images
        cv2.imwrite(os.path.join(img_out_dir, base_name + "_blurred_view.png"),
                    (img * 255).astype(np.uint8))
        cv2.imwrite(os.path.join(img_out_dir, base_name + "_inverse_restored.png"),
                    (inv_img * 255).astype(np.uint8))
        cv2.imwrite(os.path.join(img_out_dir, base_name + "_wiener_restored.png"),
                    (wiener_img * 255).astype(np.uint8))

        # Add to summary table
        summary_rows.append({
            "image": base_name,
            "B1_blur": B1_blur,
            "B2_blur": B2_blur,
            "B3_blur": B3_blur,
            "B1_inverse": B1_inv,
            "B2_inverse": B2_inv,
            "B3_inverse": B3_inv,
            "B1_wiener": B1_w,
            "B2_wiener": B2_w,
            "B3_wiener": B3_w,
        })

    # Save summary as CSV-like text
    summary_path = os.path.join(OUTPUT_DIR, "partB_metrics_summary.txt")
    with open(summary_path, "w") as f:
        header = ["image",
                  "B1_blur", "B2_blur", "B3_blur",
                  "B1_inverse", "B2_inverse", "B3_inverse",
                  "B1_wiener", "B2_wiener", "B3_wiener"]
        f.write("\t".join(header) + "\n")
        for row in summary_rows:
            values = [str(row[h]) for h in header]
            f.write("\t".join(values) + "\n")

    print("\nDone. Check the Part B output folder for ESF/LSF/MTF plots, restored images, and metrics summary.")


if __name__ == "__main__":
    PART_B()
