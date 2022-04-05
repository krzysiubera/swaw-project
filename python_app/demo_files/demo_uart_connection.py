from uart_handler import UartHandler


uart_handler = UartHandler()

uart_handler.send_command("get temp")
print(f"Temperature: {uart_handler.read_last_data()} deg C")

uart_handler.send_command("get press")
print(f"Pressure: {uart_handler.read_last_data()} hPa")

uart_handler.send_command("get hum")
print(f"Humidity: {uart_handler.read_last_data()} %")

uart_handler.send_command("get meas temp")
print("Temperature measurement: ")
for idx in range(180):
    print(uart_handler.read_last_data(), end=" ")
print()


uart_handler.send_command("get meas hum")
print("Humidity measurement: ")
for idx in range(180):
    print(uart_handler.read_last_data(), end=" ")
print()

uart_handler.send_command("get meas press")
print("Pressure measurement: ")
for idx in range(180):
    print(uart_handler.read_last_data(), end=" ")
print()


