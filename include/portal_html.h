#ifndef PORTAL_HTML_H
#define PORTAL_HTML_H

#include <Arduino.h>

const char PORTAL_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Agent Screen Setup</title>
    <style>
        :root {
            --primary: #6366f1;
            --primary-hover: #4f46e5;
            --bg-start: #0f172a;
            --bg-end: #1e1b4b;
            --card-bg: rgba(30, 41, 59, 0.85);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --border: rgba(255, 255, 255, 0.12);
            --input-bg: rgba(15, 23, 42, 0.6);
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: linear-gradient(135deg, var(--bg-start), var(--bg-end));
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
            color: var(--text-main);
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            border: 1px solid var(--border);
            border-radius: 20px;
            padding: 32px 28px;
            width: 100%;
            max-width: 420px;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.4);
            animation: fadeIn 0.4s ease-out;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(12px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .header {
            text-align: center;
            margin-bottom: 28px;
        }

        .logo-icon {
            width: 52px;
            height: 52px;
            background: linear-gradient(135deg, #818cf8, #4f46e5);
            border-radius: 14px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            margin-bottom: 12px;
            box-shadow: 0 8px 18px rgba(99, 102, 241, 0.35);
        }

        .logo-icon svg {
            width: 26px;
            height: 26px;
            fill: none;
            stroke: white;
            stroke-width: 2;
            stroke-linecap: round;
            stroke-linejoin: round;
        }

        h1 {
            font-size: 22px;
            font-weight: 700;
            letter-spacing: -0.5px;
            color: #ffffff;
        }

        .subtitle {
            font-size: 13px;
            color: var(--text-muted);
            margin-top: 4px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            font-size: 12px;
            font-weight: 600;
            color: #cbd5e1;
            margin-bottom: 8px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .input-row {
            display: flex;
            gap: 10px;
        }

        input[type="text"], input[type="password"], select {
            width: 100%;
            padding: 12px 14px;
            background: var(--input-bg);
            border: 1px solid var(--border);
            border-radius: 10px;
            font-size: 15px;
            color: #ffffff;
            outline: none;
            transition: all 0.2s ease;
        }

        select option {
            background-color: #1e293b;
            color: #ffffff;
            padding: 10px;
        }

        input[type="text"]:focus, input[type="password"]:focus, select:focus {
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(99, 102, 241, 0.25);
        }

        .btn-icon {
            width: 46px;
            height: 46px;
            background: rgba(255, 255, 255, 0.08);
            border: 1px solid var(--border);
            border-radius: 10px;
            color: var(--text-main);
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            flex-shrink: 0;
            transition: all 0.2s ease;
        }

        .btn-icon:hover {
            background: rgba(255, 255, 255, 0.15);
            border-color: var(--primary);
        }

        .btn-icon.spinning svg {
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }

        .btn-submit {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, var(--primary), var(--primary-hover));
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            box-shadow: 0 4px 14px rgba(99, 102, 241, 0.4);
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            margin-top: 10px;
        }

        .btn-submit:hover {
            transform: translateY(-1px);
            box-shadow: 0 6px 20px rgba(99, 102, 241, 0.5);
        }

        .btn-submit:disabled {
            opacity: 0.7;
            cursor: not-allowed;
            transform: none;
        }

        /* Spinner CSS */
        .spinner {
            width: 18px;
            height: 18px;
            border: 2.5px solid rgba(255, 255, 255, 0.3);
            border-top-color: #ffffff;
            border-radius: 50%;
            animation: spin 0.8s linear infinite;
            display: inline-block;
        }

        .hint {
            font-size: 12px;
            color: var(--text-muted);
            margin-top: 6px;
        }

        .footer {
            margin-top: 24px;
            text-align: center;
            font-size: 12px;
            color: #64748b;
        }

        /* Connecting overlay view */
        .connecting-state {
            display: none;
            text-align: center;
            padding: 30px 10px;
        }

        .large-spinner {
            width: 56px;
            height: 56px;
            border: 4px solid rgba(99, 102, 241, 0.2);
            border-top-color: var(--primary);
            border-radius: 50%;
            animation: spin 0.9s linear infinite;
            margin: 0 auto 24px;
        }

        .connecting-title {
            font-size: 24px;
            font-weight: 700;
            color: #ffffff;
            margin-bottom: 12px;
        }

        .connecting-desc {
            font-size: 16px;
            color: #cbd5e1;
            line-height: 1.5;
        }
    </style>
</head>
<body>
    <div class="card">
        <div id="form-container">
            <div class="header">
                <div class="logo-icon">
                    <svg viewBox="0 0 24 24"><path d="M5 12.55a11 11 0 0 1 14.08 0M1.42 9a16 16 0 0 1 21.16 0M8.53 16.11a6 6 0 0 1 6.95 0M12 20h.01"/></svg>
                </div>
                <h1>Agent Screen Setup</h1>
                <p class="subtitle">Configuración Wi-Fi y Servidor API</p>
            </div>

            <form action="/connect" method="POST" id="setup-form" onsubmit="handleSubmit(event)">
                <div class="form-group">
                    <label for="ssid-select">Red Wi-Fi (SSID)</label>
                    <div class="input-row">
                        <select id="ssid-select">
                            <option value="">Buscando redes cercanas...</option>
                        </select>
                        <button type="button" id="btn-refresh" class="btn-icon" title="Buscar de nuevo" onclick="scanNetworks()">
                            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21.5 2v6h-6M2.13 15.57a10 10 0 0 0 16.92 2.43M2.5 22v-6h6M21.87 8.43a10 10 0 0 0-16.92-2.43"/></svg>
                        </button>
                    </div>
                    <input type="text" id="ssid-manual" style="margin-top: 10px; display: none;" placeholder="Nombre de la red oculta">
                    <input type="hidden" id="ssid" name="ssid" required>
                </div>

                <div class="form-group">
                    <label for="pass">Contraseña Wi-Fi</label>
                    <div class="input-row">
                        <input type="password" id="pass" name="pass" placeholder="Dejar en blanco si es red abierta">
                        <button type="button" id="btn-toggle-pass" class="btn-icon" title="Mostrar/Ocultar contraseña" onclick="togglePassword()">
                            <svg id="eye-icon" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path>
                                <circle cx="12" cy="12" r="3"></circle>
                            </svg>
                        </button>
                    </div>
                </div>

                <div class="form-group">
                    <label for="api_ip">IP Servidor FastAPI (Opcional)</label>
                    <input type="text" id="api_ip" name="api_ip" placeholder="Ej: 192.168.1.100 (Por defecto 192.168.1.2)">
                    <p class="hint">Es la IP de la computadora donde corre el servidor de agentes.</p>
                </div>

                <button type="submit" id="btn-submit" class="btn-submit">
                    <span>Guardar y Conectar</span>
                </button>
            </form>
        </div>

        <div id="connecting-container" class="connecting-state">
            <div class="large-spinner"></div>
            <h2 class="connecting-title">Guardando y Reiniciando...</h2>
            <p class="connecting-desc">El dispositivo está guardando los datos y conectándose a la red Wi-Fi. Puedes cerrar esta página.</p>
        </div>

        <div class="footer">
            ESP32 CYD Agent Screen • v1.0
        </div>
    </div>

    <script>
        function getSignalIcon(rssi) {
            return "📶";
        }

        function togglePassword() {
            const passInput = document.getElementById('pass');
            const eyeIcon = document.getElementById('eye-icon');
            if (passInput.type === 'password') {
                passInput.type = 'text';
                eyeIcon.innerHTML = '<path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line>';
            } else {
                passInput.type = 'password';
                eyeIcon.innerHTML = '<path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle>';
            }
        }

        function scanNetworks() {
            const select = document.getElementById('ssid-select');
            const hiddenSsid = document.getElementById('ssid');
            const manualInput = document.getElementById('ssid-manual');
            const refreshBtn = document.getElementById('btn-refresh');

            refreshBtn.classList.add('spinning');
            select.innerHTML = '<option value="">Buscando redes Wi-Fi...</option>';
            manualInput.style.display = 'none';

            fetch('/scan')
                .then(res => res.json())
                .then(networks => {
                    refreshBtn.classList.remove('spinning');
                    select.innerHTML = '';

                    if (!networks || networks.length === 0) {
                        select.innerHTML = '<option value="">No se encontraron redes</option>';
                    } else {
                        const defaultOpt = document.createElement('option');
                        defaultOpt.value = '';
                        defaultOpt.textContent = '-- Selecciona una red Wi-Fi --';
                        select.appendChild(defaultOpt);

                        networks.forEach(net => {
                            if (!net.ssid) return;
                            const opt = document.createElement('option');
                            opt.value = net.ssid;
                            const signal = getSignalIcon(net.rssi);
                            const lock = net.secure ? ' 🔒' : '';
                            opt.textContent = `${net.ssid} (${signal} ${net.rssi} dBm)${lock}`;
                            select.appendChild(opt);
                        });
                    }

                    const manualOpt = document.createElement('option');
                    manualOpt.value = '__manual__';
                    manualOpt.textContent = '✏️ Ingresar red manualmente (Oculta)...';
                    select.appendChild(manualOpt);
                })
                .catch(err => {
                    refreshBtn.classList.remove('spinning');
                    select.innerHTML = '<option value="">Error al buscar redes</option>';
                    const manualOpt = document.createElement('option');
                    manualOpt.value = '__manual__';
                    manualOpt.textContent = '✏️ Ingresar red manualmente...';
                    select.appendChild(manualOpt);
                });
        }

        document.getElementById('ssid-select').addEventListener('change', function() {
            const hiddenSsid = document.getElementById('ssid');
            const manualInput = document.getElementById('ssid-manual');
            if (this.value === '__manual__') {
                manualInput.style.display = 'block';
                manualInput.value = '';
                manualInput.focus();
                hiddenSsid.value = '';
            } else {
                manualInput.style.display = 'none';
                hiddenSsid.value = this.value;
            }
        });

        document.getElementById('ssid-manual').addEventListener('input', function() {
            document.getElementById('ssid').value = this.value;
        });

        function handleSubmit(e) {
            const hiddenSsid = document.getElementById('ssid');
            if (!hiddenSsid.value || hiddenSsid.value.trim() === '') {
                e.preventDefault();
                alert('Por favor selecciona una red Wi-Fi o ingresa el nombre manualmente.');
                return false;
            }

            const btnSubmit = document.getElementById('btn-submit');
            btnSubmit.disabled = true;
            btnSubmit.innerHTML = '<div class="spinner"></div><span>Guardando...</span>';

            setTimeout(() => {
                document.getElementById('form-container').style.display = 'none';
                document.getElementById('connecting-container').style.display = 'block';
            }, 300);

            return true;
        }

        window.addEventListener('load', scanNetworks);
    </script>
</body>
</html>
)rawhtml";

#endif
