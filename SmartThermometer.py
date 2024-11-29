import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import datetime  # To timestamp saved files

# Connect to the serial port
ser = serial.Serial('COM12', 9600)

# Initialize empty lists to store the data
temperature_data = []
humidity_data = []
gas_level_data = []

# Create figure and axes
fig, ax = plt.subplots(3, 1, figsize=(8, 8))
temperature_line, = ax[0].plot([], [], label="Temperature (°C)", color="red")
humidity_line, = ax[1].plot([], [], label="Humidity (%)", color="blue")
gas_level_line, = ax[2].plot([], [], label="Gas Level", color="green")

# Set axis titles and limits
for i, label in enumerate(["Temperature (°C)", "Humidity (%)", "Gas Level"]):
    ax[i].set_title(label)
    ax[i].set_xlim(0, 50)
    ax[i].set_ylim(0, 100 if i < 2 else 1024)
    ax[i].legend(loc="upper left")
    ax[i].grid()

# Parse a line of data
def parse_data(line):
    try:
        # Strip newline characters and split by commas
        parts = line.strip().split(", ")
        temperature = float(parts[0].split(": ")[1].split(" ")[0])
        humidity = float(parts[1].split(": ")[1].split(" ")[0])
        gas_level = int(parts[2].split(": ")[1])
        return temperature, humidity, gas_level
    except Exception as e:
        print(f"Error parsing line: {line}, Error: {e}")
        return None, None, None

# Update function for animation
def update(frame):
    global temperature_data, humidity_data, gas_level_data

    if ser.in_waiting > 0:
        # Read and decode the data from Arduino
        line = ser.readline().decode('utf-8')
        print(f"Raw data: {line.strip()}")
        temperature, humidity, gas_level = parse_data(line)

        if temperature is not None:
            temperature_data.append(temperature)
            humidity_data.append(humidity)
            gas_level_data.append(gas_level)

            # Update plot data
            temperature_line.set_data(range(len(temperature_data)), temperature_data)
            humidity_line.set_data(range(len(humidity_data)), humidity_data)
            gas_level_line.set_data(range(len(gas_level_data)), gas_level_data)

            # Adjust x-axis limits dynamically
            for i, data in enumerate([temperature_data, humidity_data, gas_level_data]):
                ax[i].set_xlim(0, max(50, len(data)))

# Save function to save the current graph
def save_graph():
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    filename = f"graph_{timestamp}.png"
    plt.savefig(filename)
    print(f"Graph saved as {filename}")

# Set up the animation
ani = FuncAnimation(fig, update, interval=1000)

# Display the plot
plt.tight_layout()

# Add a key event listener to save the graph when pressing "s"
def on_key(event):
    if event.key == "s":
        save_graph()

fig.canvas.mpl_connect("key_press_event", on_key)

plt.show()

