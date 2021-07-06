import serial


def get_number(data):
    data.rstrip().lstrip()
    _, number, _, _ = data.split()
    return float(number)

def fetch_data():
    ser_bytes = ser.readline()
    decoded_bytes = ser_bytes.rstrip().decode("utf-8")
    temp, hum, press = decoded_bytes.split(',')
    return temp, hum, press

# open UART connection
ser = serial.Serial('COM3', baudrate=115200)

while True:
    try:
        temp, hum, press = fetch_data()
        temperature = get_number(temp)
        print(temperature)
    except KeyboardInterrupt:
        print("Keyboard Interrupt")
        break