
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

```
//置于三种默认配置之下
    
/*注意!此代码应区别!*/ -> GetGlyphRangesChineseSimplifiedCommon()//会导致部分中文不能正常显示
GetGlyphRangesChineseFull()//应用此段代码
    
//实时渲染方法（从硬盘中读取）
ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SIMYOU.TTF", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

//内存字体数组：baidu_font_data
//ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)baidu_font_data, baidu_font_size,15.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
//发现采用上方代码退出时会报错

//字体配置（基于上面优化）
//从内存中读取
ImFontConfig f_cfg;
f_cfg.FontDataOwnedByAtlas = false;
ImFont* font = io.Fonts->AddFontFromMemoryTTF((void*)baidu_font_data, baidu_font_size, 15.0f, &f_cfg, io.Fonts->GetGlyphRangesChineseFull());
```

### git 大文件处理
git lfs track "font_diy.h"
track 之后 在进行 git add/commit/push
