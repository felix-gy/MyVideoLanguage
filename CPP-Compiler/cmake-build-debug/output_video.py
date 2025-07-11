# Código generado automáticamente desde lenguaje de compilador
from moviepy.editor import *
from moviepy.video.fx import *
import os 

video1 = VideoFileClip("video.mp4")
video2 = VideoFileClip("video_22.mp4")
timepo1 = (1, 0)
timepo2clip = (5, 0)
first = 5
second = 10
if first != second:
    print("Son diferentes")

videoRecortado = video1.subclip(timepo1, timepo2clip)
videoConcatenado = concatenate_videoclips([videoRecortado, video2])
videoFinal = videoConcatenado.speedx(2)
for i in range(3): 
    print("Iteración")

durationTime = str(videoFinal.duration)
videoFinal.write_videofile("videoFinal.mp4", codec='libx264', audio_codec='aac')

