import os
import numpy as np

# Import only what we need from PART_A
from PART_A import ensure_dir, load_image_gray, experiment_motion_blur

def PART_B():
    # ---- PATHS ----
    IMAGE_PATHS = [
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\TV.png",
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\WolvesLocations.png",
        r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\Zivspics\Skateboard.png",
    ]
    OUTPUT_DIR = r"C:\Users\zivza\Ziv\English\MMcompression\EXE1\output_partB"
    # ----------------

    ensure_dir(OUTPUT_DIR)

    # Blur configuration
    motion_lengths = [1, 3, 5, 9, 13]   # pixels
    motion_angle_deg = 0.0              # horizontal blur

    for img_path in IMAGE_PATHS:
        img_name = os.path.splitext(os.path.basename(img_path))[0]
        print(f"\n--- PART B: Motion Blur analysis for image: {img_name} ---")

        img = load_image_gray(img_path)

        img_out_dir = os.path.join(OUTPUT_DIR, img_name, "motion_blur")
        ensure_dir(img_out_dir)

        # Run motion blur experiment
        motion_results = experiment_motion_blur(
            img,
            motion_lengths,
            motion_angle_deg,
            img_out_dir,
            img_name
        )

        print("Results:", motion_results)

    print("\nPART B done. Check the 'output_partB' folder for images, graphs, and JSON files.")


if __name__ == "__main__":
    PART_B()
