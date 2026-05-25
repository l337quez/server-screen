import serial
import json
import time

# Configuración del puerto serial (Ajusta al puerto de tu CYD en Linux)
PUERTO = '/dev/ttyUSB0'
BAUD_RATE = 115200

def enviar_estado_gateway(conexion_serial):
    # Aquí armas la estructura de datos exactamente como la necesitas
    payload = {
        "agentes": [
            {
                "name": "Dupin",
                "type": "inspector",
                "message": "I found new test in the investigation"
            },
            {
                "name": "Jace",
                "type": "lawyer",
                "message": "We was winning this case"
            }
        ],
        "hardware": {
            "cpu_uso": "42%",
            "ram_libre": "1.2GB",
            "temperatura": "55C"
        }
    }

    # Convertimos el diccionario a un string JSON
    # El '\n' es crucial para que el ESP32 lea hasta ese delimitador
    trama_json = json.dumps(payload) + '\n'

    try:
        # Enviamos los datos codificados en bytes
        conexion_serial.write(trama_json.encode('utf-8'))
        print("Trama enviada con éxito:")
        print(trama_json.strip())
    except Exception as e:
        print(f"Error enviando datos: {e}")

if __name__ == '__main__':
    try:
        # Iniciamos la comunicación serial
        print(f"Abriendo puerto {PUERTO}...")
        esp32_serial = serial.Serial(PUERTO, BAUD_RATE, timeout=1)
        
        # Al abrir el puerto serial, el ESP32 suele reiniciarse. 
        # Le damos 2 segunditos para que despierte y esté listo para escuchar.
        time.sleep(2) 
        
        enviar_estado_gateway(esp32_serial)
        
        esp32_serial.close()
        print("Puerto cerrado.")
        
    except serial.SerialException as e:
        print(f"Error de conexión: {e}")
        print("¿Tienes permisos sobre el puerto? (Prueba con sudo o agregándote al grupo dialout)")