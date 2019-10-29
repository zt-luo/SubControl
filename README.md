[![HitCount](http://hits.dwyl.io/zt-luo/SubControl.svg)](http://hits.dwyl.io/zt-luo/SubControl)



## Screenshot

![dive_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/video.png)

![analyze_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/control.png)

![analyze_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/setting.png)

## build

### Linux

```shell
git clone https://github.com/zt-luo/SubControl.git
git submodule init
git submodule update

cd ardusub_api
mkdir build
cd build
cmake ..
make

cd ../../vlc-qt
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
sudo make install

cd ../..
mkdir build
cd build
qmake ../SubControl.pro
make
```



## LOC

```
github.com/AlDanial/cloc v 1.82  T=0.55 s (29.2 files/s, 6587.1 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                              4            342             78           1422
Qt                               1              0              0            896
C/C++ Header                     3            108            108            281
SVG                              5              1              3            244
Markdown                         1             19              0             44
ProGuard                         1             16             14             29
Bourne Shell                     1              0              0              1
-------------------------------------------------------------------------------
SUM:                            16            486            203           2917
-------------------------------------------------------------------------------

```

