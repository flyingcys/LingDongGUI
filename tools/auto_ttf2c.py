import os
import yaml
import subprocess
from pathlib import Path
import sys

def read_font_yaml(file_path):
    if not os.path.exists(file_path):
        create_default_font_config(file_path)
    with open(file_path, 'r', encoding='utf-8') as file:
        yamlData = yaml.safe_load(file)
    return yamlData

def create_default_font_config(file_path):
    default_config = """- font:
    family: Arial
    size: 12
    bold: false
    italic: false
    underline: false
    strikeOut: false
    path: arial.ttf
    codeName: Arial
    bitWidth: 8
    text: " !\\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~±"
    index: 0
- font:
    family: Arial
    size: 16
    bold: false
    italic: false
    underline: false
    strikeOut: false
    path: arial.ttf
    codeName: Arial
    text: '012345678'
    index: 0
"""
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(default_config)

header_content_start = f"""#ifndef __UI_FONTS_H__
#define __UI_FONTS_H__

#ifdef __cplusplus
extern "C" {{
#endif

#include "arm_2d_helper.h"
"""

header_content_end = f"""
#ifdef __cplusplus
}}
#endif

#endif
"""

def generate_font_data_for_each_config(fontYaml, output_dir):
    print('')
    header_content=""
    for config in fontYaml:
        font_config = config.get('font', {})
        font_index = font_config.get('index', 0)
        ttf_path = font_config.get('path', 'Default.ttf')
        pixel_size = font_config.get('size', 12)
        font_bit_width = font_config.get('bitWidth', None)
        text = font_config.get('text', " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~±")
        text = str(text)
        base_font_name = font_config.get('codeName', 'Default')

        font_name = ttf_path
        # 判断ttf path是文件
        if not os.path.isfile(ttf_path):
            ttf_path = f"{output_dir}/{font_name}"
            if not os.path.exists(ttf_path):
                if os.name == 'nt':  # Windows
                    ttf_path = f"C:/Windows/Fonts/{font_name}"
                    if not os.path.exists(ttf_path):
                        ttf_path = f"{Path.home()}/AppData/Local/Microsoft/Windows/Fonts/{font_name}"

        print('['+base_font_name+']\n    '+ttf_path)

        base_font_name = base_font_name.replace('-', '_')
        base_font_name = base_font_name.replace(' ', '_')
        base_font_name = base_font_name.replace('.', '_')
        output_c = f"{output_dir}/{base_font_name}_{pixel_size}.c"
        print('    size:',pixel_size)

        font_txt_path=f'{output_dir}/font.txt'
        if font_bit_width is not None:
            print('    bitWidth:',font_bit_width)
        print('')
        with open(font_txt_path, 'w', encoding='utf-8') as file:
            pass

        with open(font_txt_path, 'a', encoding='utf-8') as file:
            file.write(text + '\n')
        
        thisPyPath = os.path.dirname(os.path.abspath(__file__))
        command_args = [
            'python', f"{thisPyPath}/ttf2c.py", '-i', ttf_path, '-t', font_txt_path, '-o', output_c,
            '-p', str(pixel_size), '-n', f'{base_font_name}_{pixel_size}'
        ]
        if font_bit_width is not None:
            command_args.extend(['-s', str(font_bit_width)])

        if ttf_path.lower().endswith('.ttc'):
            command_args.extend(['--index', str(font_index)])
        subprocess.run(command_args)

        if font_bit_width is not None:
            header_content += f"""
extern const
struct {{
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table;
}} ARM_2D_FONT_{base_font_name}_{str(pixel_size)}_A{font_bit_width};

#define FONT_{base_font_name.upper()}_{str(pixel_size)}          (arm_2d_font_t*)&ARM_2D_FONT_{base_font_name}_{str(pixel_size)}_A{font_bit_width}
"""
        else:
            for i in range(4):
                header_content += f"""
extern const
struct {{
    implement(arm_2d_user_font_t);
    arm_2d_char_idx_t tUTF8Table;
}} ARM_2D_FONT_{base_font_name}_{str(pixel_size)}_A{2**i};

#define FONT_{base_font_name.upper()}_{str(pixel_size)}_A{2**i}          (arm_2d_font_t*)&ARM_2D_FONT_{base_font_name}_{str(pixel_size)}_A{2**i}
"""

    

    with open(f"{output_dir}/uiFonts.h", 'w', encoding='utf-8') as file:
        file.write(header_content_start)
        file.flush()
        file.write(header_content)
        file.flush()
        file.write(header_content_end)
        file.flush()



def main(argv):
    if not argv:
        output_dir = os.path.dirname(os.path.abspath(__file__))
    else:
        output_dir = argv[0]
    if output_dir[-1] == '\\' or output_dir[-1] == '/':
        output_dir = output_dir[:-1]

    fontYamlData = read_font_yaml(f'{output_dir}/font.yaml')

    font_txt_path=f"{output_dir}/font.txt"
    if not os.path.exists(font_txt_path):
        with open(font_txt_path, 'w', encoding='utf-8') as file:
            pass

    generate_font_data_for_each_config(fontYamlData,output_dir)
    os.remove(font_txt_path)

if __name__ == "__main__":
    main(sys.argv[1:])