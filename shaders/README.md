# Upscaling filters (VIDEO -> FILTER)

GLSL shaders in the libretro/RetroArch format, loaded by `sources/postfx.cpp`.

| File | Filter | Author | License | Source |
|---|---|---|---|---|
| `scale2x.glsl` | Scale2x (AdvMAME2x) | Andrea Mazzoleni | GPL | libretro/glsl-shaders `scalenx/shaders/scale2x.glsl` |
| `2xsai.glsl` | 2xSaI | Derek Liauw Kie Fa (Kreed), algorithm; port written for F-1 Spirit | GPL-2.0-or-later | this project |
| `hqx-pass1.glsl`, `hqx-pass2.glsl`, `hq2x-lut.png` | hq2x | Maxim Stepin, Cameron Zemek, Jules Blok | LGPL-2.1-or-later | libretro/glsl-shaders `hqx/shader-files/`, `hqx/resources/hq2x.png` |
| `xbr-lv2.glsl` | xBR level 2 | Hyllian | MIT | libretro/glsl-shaders `xbr/shaders/xbr-lv2.glsl` |

The libretro files are unmodified copies, taken from
https://github.com/libretro/glsl-shaders at commit
`435612f`. Their license headers are kept in each file.
