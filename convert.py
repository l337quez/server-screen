import os
from PIL import Image

def convertir_a_rgb565(ruta_imagen, ruta_salida, nombre_array):
    try:
        img = Image.open(ruta_imagen).convert('RGBA')
        target_width = 70
        target_height = 70
        img = img.resize((target_width, target_height), Image.Resampling.LANCZOS)
        width, height = img.size

        with open(ruta_salida, 'w') as f:
            f.write(f"/* Archivo generado para ESP32 CYD (TFT_eSPI) */\n")
            f.write(f"#include <stdint.h>\n")
            f.write(f"#ifdef __AVR__\n")
            f.write(f"#include <avr/pgmspace.h>\n")
            f.write(f"#else\n")
            f.write(f"#define PROGMEM\n")
            f.write(f"#endif\n\n")
            f.write(f"const uint16_t {nombre_array}_width = {width};\n")
            f.write(f"const uint16_t {nombre_array}_height = {height};\n\n")
            f.write(f"const uint16_t {nombre_array}[{width * height}] PROGMEM = {{\n")

            pixels = img.load()
            contador = 0
            
            for y in range(height):
                for x in range(width):
                    r, g, b, a = pixels[x, y]
                    is_transparent = (a < 128) or (r > 240 and g > 240 and b > 240)
                    
                    if is_transparent:
                        rgb565 = 0xFFFF
                    else:
                        color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                        color = (~color) & 0xFFFF
                        rgb565 = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF)
                    
                    f.write(f"0x{rgb565:04X}, ")
                    contador += 1
                    
                    if contador % 12 == 0:
                        f.write("\n    ")
                        
            f.write("\n};\n")
            print(f"¡Éxito! Archivo {ruta_salida} generado. ({width}x{height} píxeles)")
            
    except Exception as e:
        print(f"Error procesando la imagen {ruta_imagen}: {e}")

base_dir = r"C:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen"
img_dir = os.path.join(base_dir, "assets", "images")
out_dir = os.path.join(base_dir, "src", "icons")

images = {
    "1-Dupin-frontal.png": "sprite_dupin_frontal",
    "3-Ronaldofrontal.png": "sprite_ronaldo_frontal"
}

for img_name, array_name in images.items():
    in_path = os.path.join(img_dir, img_name)
    out_path = os.path.join(out_dir, img_name.replace(".png", ".c"))
    convertir_a_rgb565(in_path, out_path, array_name)
