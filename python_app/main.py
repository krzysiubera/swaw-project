import serial


uart_handler = serial.Serial(port='COM12', baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE,
                             stopbits=serial.STOPBITS_ONE)

data_to_write = "get temp\r\n".encode()
uart_handler.write(data_to_write)
result = uart_handler.readline().decode().strip()
print(result)

data_to_write = "get press\r\n".encode()
uart_handler.write(data_to_write)
result = uart_handler.readline().decode().strip()
print(result)

data_to_write = "get hum\r\n".encode()
uart_handler.write(data_to_write)
result = uart_handler.readline().decode().strip()
print(result)
