import os
from PIL import Image

def convert_ppm_to_png():
    # 获取当前路径
    current_path = os.getcwd()
    
    # 遍历当前路径下的所有文件
    for filename in os.listdir(current_path):
        if filename.endswith('.ppm'):
            # 构建完整的文件路径
            ppm_file = os.path.join(current_path, filename)
            # 定义输出文件名
            png_file = os.path.splitext(ppm_file)[0] + '.png'
            
            # 打开 PPM 文件并转换为 PNG
            with Image.open(ppm_file) as img:
                img.save(png_file, 'PNG')
                print(f"Converted '{filename}' to '{os.path.basename(png_file)}'")

if __name__ == "__main__":
    convert_ppm_to_png()
