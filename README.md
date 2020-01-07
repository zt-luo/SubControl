[![HitCount](http://hits.dwyl.io/zt-luo/SubControl.svg)](http://hits.dwyl.io/zt-luo/SubControl)

based on [ardusub_api](https://github.com/zt-luo/ardusub_api) project.

## Screenshot

![dive_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/video.png)

![analyze_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/control.png)

![analyze_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/setting.png)

## build

### Linux

Dependencies:
QT5, OpenCV, glib-2.0, gstreamer-1.0, libserialport, sqlite3

```shell
git clone https://github.com/zt-luo/SubControl.git SubControl --recursive 

cd SubControl
mkdir build
cd build
cmake ..
make
```

### Windows

not supported yet.


## LOC

```
github.com/AlDanial/cloc v 1.84  T=1.61 s (14.9 files/s, 4337.5 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Qt                               1              0              0           2795
C++                              8            568            174           2340
SVG                              9              2              6            377
C/C++ Header                     3            128            108            349
Markdown                         1             18              0             45
ProGuard                         1             16             14             32
Bourne Shell                     1              0              0              1
-------------------------------------------------------------------------------
SUM:                            24            732            302           5939
-------------------------------------------------------------------------------

```

