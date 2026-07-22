import serial
import json
import time

# Configuración del puerto serial
PUERTO = 'COM3'  # Cambia a tu puerto COM asignado
BAUD_RATE = 115200

def enviar_estado_gateway(conexion_serial):
    # Estructura de datos con el estado actual
    payload = {
        "agentes": [
            {
                "name": "Dupin",
                "role": "inspector",
                "message": "I found new test in the investigation. ..",
                "is_alarm": False,
                "is_active": True
            },
            {
                "name": "Jace",
                "role": "lawyer",
                "message": "We was winning this case mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm m m mmm  m m mmmm mm m m mmmmmmmmmmmmmmmmmmmmmmmmmmmmmm m m m mm  aaa a sas a   ada da d ad a",
                "is_alarm": False,
                "is_active": True
            }
        ],
        "hardware": {
            "cpu_uso": "42%",
            "ram_libre": "1.2GB",
            "temperatura": "55C",
            "is_alarm": False
        }
    }

    # Convertimos el diccionario a un string JSON con salto de línea
    trama_json = json.dumps(payload) + '\n'

    try:
        # Enviamos los datos codificados en bytes
        conexion_serial.write(trama_json.encode('utf-8'))
        print("Trama enviada con éxito:")
        print(trama_json.strip())
        return True
    except Exception as e:
        print(f"Error enviando datos: {e}")
        return False

if __name__ == '__main__':
    esp32_serial = None
    try:
        print("Iniciando bucle de envío (Presiona Ctrl+C para salir)...")
        
        # Bucle infinito para mantener la conexión viva y actualizando
        while True:
            # Si el puerto no está abierto, intentamos abrirlo
            if esp32_serial is None or not esp32_serial.is_open:
                try:
                    print(f"Intentando abrir puerto {PUERTO}...")
                    esp32_serial = serial.Serial(PUERTO, BAUD_RATE, timeout=1)
                    print("Esperando 2 segundos para la estabilización del hardware...")
                    time.sleep(2)
                    print("Conexión establecida.")
                except Exception as e:
                    print(f"Error de conexión: {e}")
                    esp32_serial = None
                    print("Reintentando conectar en 5 segundos...")
                    time.sleep(5)
                    continue

            # Enviamos el estado
            exito = enviar_estado_gateway(esp32_serial)
            if not exito:
                print("Error detectado al enviar datos. Cerrando puerto para forzar reconexión...")
                try:
                    esp32_serial.close()
                except:
                    pass
                esp32_serial = None
            
            # Espera 5 segundos antes de enviar la siguiente actualización
            time.sleep(5)
        
    except KeyboardInterrupt:
        print("\nMonitoreo cancelado por el usuario.")
    finally:
        # Bloque de seguridad: si el puerto quedó abierto al salir con Ctrl+C, se cierra limpiamente
        if 'esp32_serial' in locals() and esp32_serial is not None and esp32_serial.is_open:
            esp32_serial.close()
            print("Puerto cerrado correctamente.")