import serial
import json
import time
import os

# Configuración del puerto serial
PUERTO = 'COM3'  # Cambia al número de tu puerto COM asignado
BAUD_RATE = 115200

def limpiar_pantalla():
    os.system('cls' if os.name == 'nt' else 'clear')

def conectar_serial():
    try:
        print(f"Abriendo puerto {PUERTO}...")
        conn = serial.Serial(PUERTO, BAUD_RATE, timeout=1)
        print("Esperando 2 segundos para la estabilización del hardware...")
        time.sleep(2)
        return conn
    except Exception as e:
        print(f"\n[ERROR DE CONEXIÓN] No se pudo abrir el puerto {PUERTO}: {e}")
        return None

def enviar_trama_dinamica(conexion_serial, nombre, rol, mensaje, is_alarm=False, is_active=True):
    """
    Empaqueta los datos introducidos por el usuario en la estructura JSON
    que el ESP32 espera y los envía por el puerto serie.
    """
    payload = {
        "agentes": [
            {
                "name": nombre,
                "role": rol,
                "message": mensaje,
                "is_alarm": is_alarm,
                "is_active": is_active
            }
        ],
        "hardware": {
            "cpu_uso": "50%",
            "ram_libre": "1.0GB",
            "temperatura": "48C",
            "is_alarm": is_alarm
        }
    }

    # El '\n' al final es obligatorio para que el ESP32 sepa dónde termina la línea
    trama_json = json.dumps(payload) + '\n'

    try:
        conexion_serial.write(trama_json.encode('utf-8'))
        print("\n[OK] ¡Trama enviada con éxito al ESP32!")
        return True
    except Exception as e:
        print(f"\n[ERROR] No se pudieron enviar los datos: {e}")
        return False

if __name__ == '__main__':
    limpiar_pantalla()
    print("====================================================")
    print("   CONSOLA DE PRUEBAS INTERACTIVAS - ESP32 CYD     ")
    print("====================================================")
    
    esp32_serial = conectar_serial()
    
    try:
        while True:
            print("\n====================================================")
            print(" ESCRIBE EL MENSAJE PARA TU PANTALLA RETRO  ")
            print("====================================================")
            print("Escribe 'salir' en cualquier campo para cerrar el script.\n")
            
            # Verificar y reconectar si la conexión se ha perdido
            if esp32_serial is None or not esp32_serial.is_open:
                print("[!] Puerto serial no conectado o desconectado. Intentando conectar...")
                esp32_serial = conectar_serial()
                if esp32_serial is None:
                    print("[!] No se pudo establecer conexión. Presiona ENTER para reintentar o escribe 'salir' para cerrar.")
                    opcion = input().strip().lower()
                    if opcion == 'salir':
                        break
                    limpiar_pantalla()
                    continue
            
            # 1. Captura de datos por consola
            nombre = input("1. Nombre del agente (ej. Dupin / Jace): ").strip()
            if nombre.lower() == 'salir': break
            
            rol = input("2. Rol/Tipo (escribe 'inspector' o 'lawyer'): ").strip().lower()
            if rol.lower() == 'salir': break
            if rol not in ['inspector', 'lawyer']:
                print("[!] Rol no reconocido (Usa 'inspector' o 'lawyer'). Reintentando...")
                time.sleep(2)
                limpiar_pantalla()
                continue
                
            mensaje = input("3. Mensaje para el diálogo: ").strip()
            if mensaje.lower() == 'salir': break

            alarma_input = input("4. ¿Es una alarma? (s/n, por defecto 'n'): ").strip().lower()
            if alarma_input == 'salir': break
            is_alarm = alarma_input in ['s', 'si', 'y', 'yes', 'true', '1']

            active_input = input("5. ¿Está activo el agente? (s/n, por defecto 's'): ").strip().lower()
            if active_input == 'salir': break
            is_active = active_input not in ['n', 'no', 'false', '0']

            # 2. Enviar los datos editados manteniendo el puerto abierto
            print("\nEnviando datos...")
            exito = enviar_trama_dinamica(esp32_serial, nombre, rol, mensaje, is_alarm, is_active)
            
            if not exito:
                print("[!] Error detectado al enviar datos. Cerrando conexión serial para forzar reconexión...")
                try:
                    esp32_serial.close()
                except:
                    pass
                esp32_serial = None
            
            print("\nPresiona ENTER para enviar otro mensaje...")
            input()
            limpiar_pantalla()

    except KeyboardInterrupt:
        print("\n\nConsola de pruebas cerrada por el usuario.")
    finally:
        # Aseguramos el cierre limpio al terminar el bucle
        if 'esp32_serial' in locals() and esp32_serial is not None and esp32_serial.is_open:
            esp32_serial.close()
            print("Puerto serial cerrado correctamente.")