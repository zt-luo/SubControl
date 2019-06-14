[![HitCount](http://hits.dwyl.io/zt-luo/SubControl.svg)](http://hits.dwyl.io/zt-luo/SubControl)



## Screenshot

![dive_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/dive_page.png)



![analyze_page](https://raw.githubusercontent.com/zt-luo/SubControl/master/doc/img/analyze_page.png)

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
github.com/AlDanial/cloc v 1.82  T=0.08 s (158.0 files/s, 40062.1 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                              4            308             76           1238
Qt                               1              0              0            765
C/C++ Header                     3            102            107            271
SVG                              1              0              1             52
Markdown                         1             19              0             44
ProGuard                         1             16             14             29
Bourne Shell                     1              0              0              1
-------------------------------------------------------------------------------
SUM:                            12            445            198           2400
-------------------------------------------------------------------------------

```

