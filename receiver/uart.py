import serial
import datetime
import csv
import matplotlib.pyplot as plt
import numpy as np
import matplotlib
matplotlib.use("tkAgg")


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

# initialize plot
plot_window = 20
y_var = np.array(np.zeros([plot_window]))

plt.ion()
fig, ax = plt.subplots()
ax.set_title('Temperature')
line, = ax.plot(y_var)

while True:
    try:
        temp, hum, press = fetch_data()
        try:
            temperature = get_number(temp)
        except:
            continue
        with open("test_data.csv", "a") as f:
            writer = csv.writer(f, delimiter=',')
            writer.writerow([datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"), temperature])
        y_var = np.append(y_var, temperature)
        y_var = y_var[1:plot_window+1]
        line.set_ydata(y_var)
        ax.relim()
        ax.autoscale_view()
        fig.canvas.draw()
        fig.canvas.flush_events()
    except Exception as e:
        print(e)
        break
    
