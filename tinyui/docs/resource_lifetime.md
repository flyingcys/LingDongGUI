# TinyUI 资源生命周期

本文说明 `tinyui_image_source` 与 `tinyui_font` 的所有权与存活规则。用户只通过 TinyUI public API 创建、绑定与清空资源描述符。

**硬约束：**

- core **没有**资源缓存中心。
- core **不对** image/font descriptor 做引用计数。
- 绑定到对象期间，descriptor 与其所借用的底层缓冲必须保持有效。
- 不要手写 descriptor 私有字段，不要在业务代码中依赖后端图片宏或后端类型。

---

## 图像：`tinyui_image_source_t`

### 1. RGB565 借用（`tinyui_image_source_from_rgb565`）

- TinyUI **借用**调用方提供的 `pixels`（及可选 `mask`），**不拷贝**像素。
- `stride` / `mask_stride` 以**字节**计。
- 调用方必须保证：从 `tinyui_image_set_source`（或 props 绑定）成功起，到对象不再使用该 source（更换 source、删除对象）为止，像素缓冲一直有效。
- `tinyui_image_source_deinit` 只清空句柄字段，**不**释放调用方缓冲。

```c
static uint16_t s_pixels[16 * 16];
static tinyui_image_source_t s_src;

void bind_rgb565(tinyui_obj_t *image)
{
    if (tinyui_image_source_from_rgb565(s_pixels,
                                        16,
                                        16,
                                        16 * sizeof(uint16_t),
                                        NULL,
                                        0,
                                        &s_src) != TINYUI_OK) {
        return;
    }
    (void)tinyui_image_set_source(image, &s_src);
}
```

### 2. Builtin（`tinyui_image_source_from_builtin`）

- 使用 `tinyui_builtin_image_t` 枚举取得内置资源描述。
- 静态内置数据由库侧持有；`tinyui_image_source_deinit` 只清空调用方句柄，不释放静态资源。
- 仍须保证 **句柄对象本身**（`tinyui_image_source_t` 存储）在绑定期间存活——推荐静态或页面状态，而不是函数栈上的临时变量长期挂到 widget。

### 3. VRES（`tinyui_image_source_from_vres`）

- 从外部资源地址构造描述符；地址的有效性与映射由集成层/资源打包保证。
- `deinit` 清空句柄；是否释放 vres 底层存储取决于资源系统，**不属于** TinyUI core 的隐式行为。
- 绑定期间 address 指向的数据必须可读。

### 4. 通用图像规则

1. 传给 widget 的 source **对象**必须活得比该次绑定的使用期更久。
2. 不要把栈上临时 `tinyui_image_source_t` 交给长期存在的 widget。
3. 更换 source 后，旧 descriptor 是否仍需保留取决于是否还有其他对象引用；core 不会帮你跟踪。
4. 对空句柄或已 `deinit` 的句柄再次 `deinit` 是安全的。

---

## 字体：`tinyui_font_t`

### Builtin（`tinyui_font_from_builtin`）

- 使用 `tinyui_builtin_font_t`。
- `tinyui_font_deinit` 清空句柄，不释放静态字体数据。

### VRES（`tinyui_font_from_vres`）

- 从 vres 地址构造字体描述符。
- 绑定到 label/button/text 等期间，句柄与 vres 数据必须有效。
- `deinit` 清空句柄；不暗示 core 卸载字体缓存。

### 字体规则

1. 字体句柄必须活得比使用它的控件更久（或在更换字体后确认旧句柄无引用）。
2. 通过 `tinyui_obj_apply_style` 传入的 `style.font` 指针同样是**借用**：apply 之后若对象仍依赖该字体，指针目标必须继续有效。
3. 重复 `deinit` 已清空句柄是安全的。

---

## Style 与临时对象

- **临时 style：** 在栈上构造 `tinyui_style_t`，调用 `tinyui_obj_apply_style` **返回后**即可丢弃 style 结构本身。
- 若 style 内嵌 `font` 指针，则 font 的生命周期仍按上一节——apply 不是“拷贝字体所有权”。
- **禁止**假设 apply 后 core 会复制并持有整份 style。

---

## Demo 与文档约束

- `tinyui/demo/*` 与 current-facing 文档只展示 public factory（builtin / vres / rgb565）。
- 不得展示后端资源字段、后端头文件或暗示存在隐藏缓存。
- 文档可链接示例见 [快速开始](./quick_start.md)。

---

## 反例

```c
/* 错误：栈上 source 在函数返回后失效，但 image 仍可能引用 */
void bad_bind(tinyui_obj_t *image)
{
    tinyui_image_source_t tmp;
    (void)tinyui_image_source_from_builtin(TINYUI_BUILTIN_IMAGE_NOTE, &tmp);
    (void)tinyui_image_set_source(image, &tmp);
}
```

```c
/* 错误：以为 deinit 会等 widget 用完再释放——core 无引用计数 */
void bad_lifetime(tinyui_obj_t *image, tinyui_image_source_t *src)
{
    (void)tinyui_image_set_source(image, src);
    tinyui_image_source_deinit(src); /* 对象仍可能读 src */
}
```
