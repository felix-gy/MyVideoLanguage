//
// Created by felixg on 25/06/2025.
//

#ifndef ARCHIVOS_H
#define ARCHIVOS_H

std::unordered_map<std::string, std::string> comandos_a_python = {
    {"++", "concatenate_videoclips"},
    {">>", "composite_transition"},
    {"load", "VideoFileClip"},
    {"export", "write_videofile"},
    {"duration", "clip.duration"},
    {"speed", "clip.fx(vfx.speedx, factor)"},
    {"WResolution", "clip.w"},
    {"HResolution", "clip.h"},
    {"Format", "clip.filename.split('.')[-1]"},
    {"clip", "clip.subclip(start, end)"},
    {"Append", "playlist.append(clip)"},
    {"Remove", "playlist.remove(clip)"},
    {"+", "+"},
    {"-", "-"},
    {"eq", "=="},
    {"neq", "!="},
    {"<", "<"},
    {">", ">"},
    {"replay_list", "for item in playlist:"},
    {"replay_range", "for i in range(n):"},
    {"if", "if"},
    {"then", ":"},
    {"end", ""},
    {"print", "print"},
    {"Int", ""},
    {"Time", ""},
    {"String", ""},
    {"Bool", ""},
    {"Video", "VideoFileClip"},
    {"Playlist", "list of VideoFileClip"}
};



#endif //ARCHIVOS_H
