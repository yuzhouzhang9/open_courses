#!/bin/bash

# 确保脚本遇到错误时立即退出
set -e

# 定义要测试的 --v 参数值列表
V_VALUES=(1 2 3 4 5 6)

# 遍历并顺序执行每个参数组合
for v in "${V_VALUES[@]}"; do
  echo "Running ./mandelbrot_ispc --v $v..."

  # 检查可执行文件是否存在
  if [ ! -x "./mandelbrot_ispc" ]; then
    echo "Error: ./mandelbrot_ispc not found or not executable."
    exit 1
  fi

  # 运行程序并传递 --v 参数
  ./mandelbrot_ispc --v "$v"

  echo "Completed ./mandelbrot_ispc --v $v."
  echo "----------------------------"

  # 调用 Python 脚本转换生成的 ppm 图片为 png
  echo "Converting mandelbrot-serial.ppm to PNG..."
  python3 ../convert_ppm_to_png.py mandelbrot-serial.ppm

  # 重命名生成的 PNG 文件
  mv mandelbrot-serial.png "mandelbrot-serial-v$v.png"
  mv mandelbrot-ispc.png "mandelbrot-ispc-v$v.png"
  echo "Saved mandelbrot-serial-v$v.png."
  echo "----------------------------"
done

echo "All executions and conversions completed successfully!"
