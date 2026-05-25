import os
from PIL import Image

png_path = r"c:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen\assets\images\inspector.png"
out_path = r"c:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen\src\icons\inspector.c"

if not os.path.exists(png_path):
    print(f"Error: {png_path} does not exist!")
    exit(1)

print(f"Opening {png_path}...")
img = Image.open(png_path)
target_width = 140
target_height = 186
img_resized = img.resize((target_width, target_height), Image.LANCZOS).convert("RGBA")

def swap_bytes(val16):
    return ((val16 & 0xFF) << 8) | ((val16 >> 8) & 0xFF)

def swap_red_blue(val565):
    r = (val565 >> 11) & 0x1F
    g = (val565 >> 5) & 0x3F
    b = val565 & 0x1F
    return (b << 11) | (g << 5) | r

def rgb_to_rgb565(r, g, b):
    r5 = (r >> 3) & 0x1F
    g6 = (g >> 2) & 0x3F
    b5 = (b >> 3) & 0x1F
    return (r5 << 11) | (g6 << 5) | b5

# Generate all 4 combinations
maps = {
    'swap_invert': [], # BGR Swapped + Inverted (Current)
    'swap_only': [],   # BGR Swapped + NOT Inverted
    'invert_only': [], # NOT Swapped + Inverted
    'raw': []          # Standard RGB (NOT Swapped + NOT Inverted)
}

for y in range(target_height):
    for x in range(target_width):
        r, g, b, a = img_resized.getpixel((x, y))
        is_transparent = (a < 128) or (r > 240 and g > 240 and b > 240)
        
        # Base RGB565
        color = rgb_to_rgb565(r, g, b)
        
        # 1. swap_invert
        if is_transparent:
            val = 0xFFFF
        else:
            val = swap_red_blue((~color) & 0xFFFF)
        maps['swap_invert'].extend([(val >> 8) & 0xFF, val & 0xFF])
        
        # 2. swap_only
        if is_transparent:
            val = 0xFFFF
        else:
            val = swap_red_blue(color)
        maps['swap_only'].extend([(val >> 8) & 0xFF, val & 0xFF])
        
        # 3. invert_only
        if is_transparent:
            val = 0xFFFF
        else:
            val = (~color) & 0xFFFF
        maps['invert_only'].extend([(val >> 8) & 0xFF, val & 0xFF])
        
        # 4. raw
        if is_transparent:
            val = 0xFFFF
        else:
            val = color
        maps['raw'].extend([(val >> 8) & 0xFF, val & 0xFF])

# Format C code
header_lines = [
    '#ifdef __has_include',
    '    #if __has_include("lvgl.h")',
    '        #ifndef LV_LVGL_H_INCLUDE_SIMPLE',
    '            #define LV_LVGL_H_INCLUDE_SIMPLE',
    '        #endif',
    '    #endif',
    '#endif',
    '',
    '#if defined(LV_LVGL_H_INCLUDE_SIMPLE)',
    '    #include "lvgl.h"',
    '#else',
    '    #include "lvgl/lvgl.h"',
    '#endif',
    '',
    '#ifndef LV_ATTRIBUTE_MEM_ALIGN',
    '#define LV_ATTRIBUTE_MEM_ALIGN',
    '#endif',
    ''
]

def format_map(name, data):
    lines = [f'const LV_ATTRIBUTE_MEM_ALIGN uint8_t inspector_{name}_map[] = {{']
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        lines.append("  " + ", ".join(f"0x{b:02x}" for b in chunk) + ",")
    lines.append('};')
    lines.append('')
    lines.append(f'const lv_img_dsc_t inspector_{name} = {{')
    lines.append('  .header.cf = LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED,')
    lines.append('  .header.always_zero = 0,')
    lines.append('  .header.w = 140,')
    lines.append('  .header.h = 186,')
    lines.append('  .data_size = 26040 * LV_COLOR_SIZE / 8,')
    lines.append(f'  .data = inspector_{name}_map,')
    lines.append('};')
    lines.append('')
    return '\n'.join(lines)

print("Writing C file...")
os.makedirs(os.path.dirname(out_path), exist_ok=True)
with open(out_path, "w") as f:
    f.write("\n".join(header_lines) + "\n\n")
    
    # Write the default main structure under the name 'inspector_pixel' for compilation backwards compatibility
    f.write(format_map("pixel", maps['swap_invert']))
    
    # Write the alternate configurations
    f.write(format_map("pixel_swap_only", maps['swap_only']))
    f.write(format_map("pixel_invert_only", maps['invert_only']))
    f.write(format_map("pixel_raw", maps['raw']))

print(f"Successfully generated all options in: {out_path}")
