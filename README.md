
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
.\binary_to_compressed_c.exe -nocompress msyh.ttc font_diy > font_diy.h 

.\binary_to_compressed_c.exe -base85 JetBrainsMonoNL-Light.ttf font_diy > font_diy.h

