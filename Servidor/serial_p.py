from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import serial
import threading

# Criação do aplicativo Flask
app = Flask(__name__)
socketio = SocketIO(app)

# Função para ler dados da porta serial
def read_serial():
    # Abre a porta COM4 com as configurações adequadas
    with serial.Serial('COM4', 115200, timeout=1) as ser:
        while True:
            # Lê uma linha de dados da porta serial
            data = ser.readline().decode('utf-8').strip()
            if data:
                print("=====================================")
                print(data)
                data = data.split(" ")

                # Emite os dados para todos os clientes conectados via WebSocket
                socketio.emit('novo_dado', {'dados': data})

# Rota Flask para exibir a interface web
@app.route('/')
def index():
    return render_template('index.html')

# Função para rodar o servidor Flask
def run_flask():
    socketio.run(app, host='0.0.0.0', port=5000)

# Função para rodar a thread de leitura serial
def enviar_serial(valor):
    with serial.Serial('COM4', 9600, timeout=1) as ser:
        ser.write(str(valor).encode())  # Envia o valor 1 para a Raspberry Pi Pico

# Manipulador para o evento de enviar dado via WebSocket
@socketio.on('enviar_dado')
def handle_enviar_dado(data):
    valor = data['valor']
    print(f"Enviando valor {valor} para a Raspberry Pi Pico...")
    enviar_serial(valor)  # Envia o valor via serial

# Função principal para iniciar o servidor e a leitura serial
def main():
    # Inicia a leitura da serial em uma thread separada
    serial_thread = threading.Thread(target=read_serial)
    serial_thread.daemon = True
    serial_thread.start()
    
    # Inicia o servidor Flask
    run_flask()

if __name__ == "__main__":
    main()
