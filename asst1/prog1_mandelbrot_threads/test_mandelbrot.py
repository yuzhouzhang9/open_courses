import numpy as np
import matplotlib.pyplot as plt

# 定义Mandelbrot集合的计算函数
def mandelbrot(c, max_iter):
    z = 0
    n = 0
    while abs(z) <= 2 and n < max_iter:
        z = z*z + c
        n += 1
    return n

# 设置图像的尺寸和分辨率
width, height = 800, 800
x_min, x_max = -2.0, 1.0
y_min, y_max = -1.5, 1.5
max_iter = 256  # 最大迭代次数

# 创建一个二维数组来保存每个像素点的迭代结果
mandelbrot_set = np.zeros((width, height))

# 对图像的每个像素点进行迭代计算
for x in range(width):
    for y in range(height):
        # 将像素点的坐标映射到复平面
        real = x_min + (x / width) * (x_max - x_min)
        imag = y_min + (y / height) * (y_max - y_min)
        c = complex(real, imag)
        # 计算该点属于Mandelbrot集合的程度
        mandelbrot_set[x, y] = mandelbrot(c, max_iter)

# 可视化结果
plt.imshow(mandelbrot_set.T, cmap='inferno', extent=[x_min, x_max, y_min, y_max])
plt.colorbar()  # 添加颜色条
plt.title("Mandelbrot Set")
plt.show()
plt.savefig("mandelbrot.png")  # 保存为 PNG 格式的图片
