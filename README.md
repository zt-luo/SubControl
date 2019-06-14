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

