from PIL import Image
import os

def convertir_a_rgb565(ruta_imagen, ruta_salida, nombre_array="sprite_lawyer"):
    try:
        img = Image.open(ruta_imagen).convert('RGBA')
        # Redimensionar a 111x200 para que quepa en la pantalla y en la memoria del ESP32
        target_width = 111
        target_height = 200
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
            
            # Recorremos cada píxel
            for y in range(height):
                for x in range(width):
                    r, g, b, a = pixels[x, y]
                    
                    # Detectar transparencia (fondo blanco o canal alfa bajo)
                    is_transparent = (a < 128) or (r > 240 and g > 240 and b > 240)
                    
                    if is_transparent:
                        # Mantener el color de clave cromática (blanco 0xFFFF en memoria)
                        rgb565 = 0xFFFF
                    else:
                        # Conversión estándar RGB888 a RGB565
                        color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                        # Invertir color (para compensar la inversión de la pantalla CYD)
                        color = (~color) & 0xFFFF
                        # Intercambiar bytes (para el almacenamiento little-endian en ESP32)
                        rgb565 = ((color & 0xFF) << 8) | ((color >> 8) & 0xFF)
                    
                    # Escribimos el hex en el archivo
                    f.write(f"0x{rgb565:04X}, ")
                    contador += 1
                    
                    # Salto de línea cada 12 valores
                    if contador % 12 == 0:
                        f.write("\n    ")
                        
            f.write("\n};\n")
            print(f"¡Éxito! Archivo {ruta_salida} generado. ({width}x{height} píxeles)")
            
    except Exception as e:
        print(f"Error procesando la imagen: {e}")

if __name__ == '__main__':
    # Usar lawyer.png de assets/images o raíz
    ruta_png = r"C:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen\assets\images\lawyer.png"
    if not os.path.exists(ruta_png):
        ruta_png = r"C:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen\lawyer.png"
        
    ruta_c = r"C:\Users\l337q\OneDrive\Desktop\PROYECTOS\server-screen\src\icons\lawyer.c"
    convertir_a_rgb565(ruta_png, ruta_c, "sprite_lawyer")
