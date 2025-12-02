# Quick test - run this to see your ROIs
import cv2
import matplotlib.pyplot as plt

img = cv2.imread(r"Section_2\drone_pic4.png", cv2.IMREAD_GRAYSCALE)
roi = (280, 390, 350, 460)  # x0, y0, x1, y1

# Draw rectangle on image
img_color = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
cv2.rectangle(img_color, (roi[0], roi[1]), (roi[2], roi[3]), (0, 255, 0), 2)

plt.figure(figsize=(10, 8))
plt.imshow(img_color)
plt.title("ROI Selection")
plt.show()