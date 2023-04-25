import cv2
import numpy as np

if __name__ == '__main__':
  cap = cv2.VideoCapture(0)
  while True:
    ret, frame = cap.read()
    name = cap.getBackendName() + '_' + str(cap.get(cv2.CAP_PROP_BACKEND)) + '_' + str(cap.get(cv2.CAP_PROP_GUID)) 
    print(name)
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('c'):
      print('Changing the camera')
      cap.release()
      cap = cv2.VideoCapture(1)
    if cv2.waitKey(1) & 0xFF == ord('q'):
      break
  cap.release()
  cv2.destroyAllWindows()