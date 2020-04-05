# Andrew Valenzuela
# CSCI-246
# Jin Park
from tempimage import TempImage
# libs made for image processing
from picamera.array import PiRGBArray
from picamera import PiCamera
from imutils.video import VideoStream

import imutils
import argparse
import datetime
import json
import time
import cv2
 
# construct the argument parser and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-c", "--conf", required=True,
  help="path to the JSON configuration file")
args = vars(ap.parse_args())
 
# loads my config file
conf = json.load(open(args["conf"]))
client = None

camera = PiCamera()
camera.resolution = tuple(conf["resolution"])
camera.framerate = conf["fps"]
rawCapture = PiRGBArray(camera, size=tuple(conf["resolution"]))

print("[INFO] warming up...")
time.sleep(conf["camera_warmup_time"])
avg = None
lastUploaded = datetime.datetime.now()
motionCounter = 0

for f in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
  frame = f.array
  timestamp = datetime.datetime.now()
  text = "No movement detected"

# set up the captured frame
  frame = imutils.resize(frame, width=1280)
# resize the image
  gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
# blurs the image

  gray = cv2.GaussianBlur(gray, (21, 21), 0)

  # if the average frame is None, initialize it
  if avg is None:
    avg = gray.copy().astype("float")
    rawCapture.truncate(0)
    continue
        # average the previous frames to get a base line image
  cv2.accumulateWeighted(gray, avg, 0.5)
  frameDelta = cv2.absdiff(gray, cv2.convertScaleAbs(avg))
        
        # make the image b/w 
  thresh = cv2.threshold(frameDelta, conf["delta_thresh"], 255,
    cv2.THRESH_BINARY)[1]
        # makes the white area bigger
  thresh = cv2.dilate(thresh, None, iterations=2)
        # outlines similar intensity pixels
  cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
    cv2.CHAIN_APPROX_SIMPLE)
  cnts = imutils.grab_contours(cnts)

  # loop over the contours
  for c in cnts:
    # if the contour is too small, ignore it
    if cv2.contourArea(c) < conf["min_area"]:
      continue

    # compute the bounding box for the contour, draw it on the frame,
    (x, y, w, h) = cv2.boundingRect(c)
    cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
    text = "Movement detected"

  # draw the text and timestamp on the frame
  ts = timestamp.strftime("%A %d %B %Y %I:%M:%S%p")
  cv2.putText(frame, "{}".format(text), (10, 20),
    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
  cv2.putText(frame, ts, (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX,
    0.35, (0, 0, 255), 1)

        # check to see if the room is occupied
  if text == "Movement detected":
    # check to see if enough time has passed between uploads
    if (timestamp - lastUploaded).seconds >= conf["min_upload_seconds"]:
      motionCounter += 1

      # check for min number of movement frames to save image
      if motionCounter >= conf["min_motion_frames"]:
        # write the image to temporary file
        t = TempImage()
        cv2.imwrite(t.path, frame)

        # update the timestamp and reset the motion counter
        lastUploaded = timestamp
        motionCounter = 0

  # else no motion
  else:
    motionCounter = 0

# check to see if the frames should be displayed to screen
  if conf["show_video"]:
    # display the video stream
    cv2.imshow("LIVE FEED", frame)
    # quit on 'q' press
    if cv2.waitKey(1) & 0xFF == ord("q"):
      break

  # clear the stream
  rawCapture.truncate(0)
