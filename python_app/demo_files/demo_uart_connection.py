from uart_handler import UartHandler


uart_handler = UartHandler()

uart_handler.send_command("get temp")
print(f"Temperature: {uart_handler.read_last_data()} deg C")

uart_handler.send_command("get press")
print(f"Pressure: {uart_handler.read_last_data()} hPa")

uart_handler.send_command("get hum")
print(f"Humidity: {uart_handler.read_last_data()} %")

uart_handler.send_command("get meas temp")
MAX_NUM_MEAS = 180
collected_meas_num = 0
measurements = []

while collected_meas_num < MAX_NUM_MEAS:
    received_msg = uart_handler.read_last_data()
    print(received_msg, end=" ")
    if received_msg == "End buffer":
        break
    measurements.append(received_msg)
    collected_meas_num = collected_meas_num + 1

print(f"Num meas: {len(measurements)}")



