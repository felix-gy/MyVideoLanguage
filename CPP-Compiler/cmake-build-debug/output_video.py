# Código del Compilador C++
from moviepy import *
from moviepy.video.fx import *
import math
import os 

video1 = VideoFileClip("video1.mp4")
video2 = VideoFileClip("video2.mp4")
timepo1 = (0, 0)
timepo2clip = (2, 0)
first = 5
second = 5 + 3 + 2 + 10
if first != second:
    print("Son diferentes")

videoRecortado = video1.subclipped(timepo1, timepo2clip)
video2 = video2.subclipped(timepo1, timepo2clip)
video3 = video2.with_effects([vfx.MultiplySpeed(2)]) 

videoFinal = CompositeVideoClip([
    videoRecortado.with_effects([vfx.SlideOut(3, side='left')]),
    video3.with_start(videoRecortado.duration - 3).with_effects([vfx.SlideIn(3, side='right')]),
])
for i in range(3): 
    print("Iteración")

videoFinal.write_videofile("videoFinal.mp4")

