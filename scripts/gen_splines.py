import cv2
import numpy as np
from matplotlib import pyplot as plt

def draw_spline(image : np.array, points) -> np.array:
  t = np.arange(0.01,1,0.01)
  curve_points = np.empty((t.shape[0] + 1,2), dtype=np.uint8) 
  curve_points[0] = points[-1]
  for i in range(len(t)):
    curve_points[i + 1] = t[i]*t[i]*points[0] + 2*t[i]*(1-t[i])*points[1] + (1-t[i])*(1-t[i])*points[2]  

  curve_points = np.append(curve_points, [points[0]], axis=0) 
  for i in range(curve_points.shape[0] - 1):
    image = cv2.line(image, curve_points[i], curve_points[i+1], color=[255,255,255])
  return image

if __name__ == "__main__":
  image = np.zeros((200,200,3), dtype=np.uint8)
  points = np.array([[20,20], [70,80],[120,20]], dtype=np.uint8)
  spline_image = draw_spline(image, points)
  plt.imshow(image)
  plt.show()