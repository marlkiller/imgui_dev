
## 集成 ImGui

1. 安装DX SDK
2. 将IMGUI的依赖项 复制到项目下
3. 找到 example 下的 main.cpp, 复制源码 

![example](https://raw.githubusercontent.com/marlkiller/imgui_dev/main/md/1651486811198.jpg)


## 内存加载自定义字体
### 编译程序
cd font  
cl binary_to_compressed_c.cpp

### 打包
eg:(mono 不支持中文)
- ./binary_to_compressed_c -nocompress 方正兰亭细黑_GBK.TTF font_diy > fzlt_yz.h
- ./binary_to_compressed_c -base85 方正兰亭细黑_GBK.TTF font_diy > fzlt_yz.h

### git 大文件处理
git lfs track "font_diy.h"
track 之后 在进行 git add/commit/push
