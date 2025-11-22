import tkinter as tk
from tkinter import scrolledtext
import serial
import threading
import struct, yaml
import os
import queue


def parse_yaml():
    with open("params.yaml", "r") as file:
        data = yaml.safe_load(file)
    return data


def format_struct(data):
    type_map = {
        "uint8_t": "B",
        "int8_t": "b",
        "uint16_t": "H",
        "int16_t": "h",
        "uint32_t": "I",
        "int32_t": "i",
        "float": "f",
        "double": "d"
    }
    fmt = "<" + "".join(type_map[t] for t in data.values())
    return fmt

#def sanatize (value, fmt, idx)


class SerialApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Serial Parameter Monitor")
        self.geometry("650x700")
        self.resizable(False, False)

        self.bg_color = "#404040"
        self.fg_color = "#ffffff"
        self.accent_color = "#ff8800"

        self.configure(bg=self.bg_color)

        self.serial_thread = None
        self.send_thread = None
        self.stop_event = threading.Event()

        # NEW: queue for outgoing serial data
        self.send_queue = queue.Queue()

        # Load parameters
        main_dict = parse_yaml()
        self.params_dict = main_dict.get("params")
        self.fmt = format_struct(self.params_dict)
        self.size = struct.calcsize(self.fmt)
        self.current_params = {key: 0 for key in self.params_dict}

        self.create_widgets()

    def create_widgets(self):
        # ----- Serial Connection -----
        serial_frame = tk.LabelFrame(self, text="Serial Connection", bg=self.bg_color, fg=self.accent_color)
        serial_frame.pack(fill="x", padx=10, pady=10)

        tk.Label(serial_frame, text="Port:", bg=self.bg_color, fg=self.fg_color).grid(row=0, column=0, padx=5, pady=5, sticky="e")
        self.port_entry = tk.Entry(serial_frame, width=14, bg="#303030", fg=self.fg_color, insertbackground=self.accent_color)
        self.port_entry.grid(row=0, column=1, padx=5)
        self.port_entry.insert(0, "/dev/ttyUSB0")

        tk.Label(serial_frame, text="Baud:", bg=self.bg_color, fg=self.fg_color).grid(row=0, column=2, padx=5, pady=5, sticky="e")
        self.baud_entry = tk.Entry(serial_frame, width=10, bg="#303030", fg=self.fg_color, insertbackground=self.accent_color)
        self.baud_entry.grid(row=0, column=3, padx=5)
        self.baud_entry.insert(0, "9600")

        self.connect_btn = tk.Button(serial_frame, text="Connect", command=self.connect_serial, bg="#303030", fg=self.fg_color)
        self.connect_btn.grid(row=0, column=4, padx=10, pady=5)

        # ----- Parameters (scrollable) -----
        param_frame = tk.LabelFrame(self, text="Parameters", bg=self.bg_color, fg=self.accent_color)
        param_frame.pack(fill="both", expand=True, padx=10, pady=10)

        canvas = tk.Canvas(param_frame, bg=self.bg_color, highlightthickness=0)
        scrollbar = tk.Scrollbar(param_frame, orient="vertical", command=canvas.yview)
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        self.scrollable_frame = tk.Frame(canvas, bg=self.bg_color)
        canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw")

        # Store label + entry widgets
        self.param_labels = {}
        self.param_entries = {}

        for name in self.params_dict.keys():
            row_frame = tk.Frame(self.scrollable_frame, bg=self.bg_color)
            row_frame.pack(fill="x", padx=5, pady=2)

            tk.Label(row_frame, text=f"{name}:", bg=self.bg_color, fg=self.fg_color, width=20, anchor="w").pack(side="left")

            val_label = tk.Label(row_frame, text="0", bg=self.bg_color, fg=self.accent_color, width=12)
            val_label.pack(side="left")
            self.param_labels[name] = val_label

            # NEW: editable text box
            if name.lower() != "checksum":
                entry = tk.Entry(row_frame, width=10, bg="#303030", fg=self.fg_color, insertbackground=self.accent_color)
                entry.pack(side="left", padx=5)
            self.param_entries[name] = entry

        # ----- Commands -----
        cmd_frame = tk.LabelFrame(self, text="Commands", bg=self.bg_color, fg=self.accent_color)
        cmd_frame.pack(fill="x", padx=10, pady=5)

        # Original command
        self.send_cmd_button = tk.Button(cmd_frame, text="Send Command", command=self.send_command, bg="#303030", fg=self.fg_color)
        self.send_cmd_button.grid(row=0, column=0, padx=10, pady=5)

        # NEW: Send all parameters
        self.send_all_button = tk.Button(cmd_frame, text="Send All Params", command=self.send_all_params, bg="#303030", fg=self.fg_color)
        self.send_all_button.grid(row=0, column=1, padx=10, pady=5)

        # ----- General Console -----
        self.console = scrolledtext.ScrolledText(
            self,
            height=6,
            wrap="word",
            state="disabled",
            bg="#303030",
            fg=self.fg_color,
            insertbackground=self.accent_color
        )
        self.console.pack(fill="x", padx=10, pady=5)

    def log(self, message):
        self.console.config(state="normal")
        self.console.insert("end", message + "\n")
        self.console.see("end")
        self.console.config(state="disabled")

    # --------------------------
    # SERIAL SEND QUEUE METHODS
    # --------------------------

    def send_command(self):
        """Original single-byte command, now queued."""
        if not hasattr(self, "ser") or not self.ser or not self.ser.is_open:
            self.log("Error: Serial port not connected")
            return

        self.send_queue.put(b'\xA5')  # original command byte
        self.log("Queued single command byte")

    def send_all_params(self):
        """Sends struct binary of ALL parameters, using entry boxes."""
        values = []

        for name, ptype in self.params_dict.items():
            text = self.param_entries[name].get().strip()

            if text == "":
                # Use current live value
                values.append(self.current_params[name])
            else:
                # Convert user input
                try:
                    if ptype in ("float", "double"):
                        values.append(float(text))
                    else:
                        values.append(int(text))
                except:
                    self.log(f"Invalid input for {name}, skipping")
                    return

        try:
            packed_without_checksum = struct.pack(self.fmt[:-1], *values[:-1])

            checksum = 0
            for b in packed_without_checksum:
                checksum ^= b

            values[-1] = checksum

            packed = struct.pack(self.fmt, *values)
            self.send_queue.put(b'\xFF' + packed)
            self.log("Queued parameter packet with checksum")

        except Exception as e:
            self.log(f"Packing error: {e}")


    # Thread that actually writes to serial
    def serial_sender_thread(self):
        while not self.stop_event.is_set():
            try:
                data = self.send_queue.get(timeout=0.1)
                if self.ser and self.ser.is_open:
                    self.ser.write(data)
                self.send_queue.task_done()
            except queue.Empty:
                continue

    # --------------------------
    # SERIAL READING
    # --------------------------

    def connect_serial(self):
        port = self.port_entry.get().strip()

        try:
            baud = int(self.baud_entry.get())
        except ValueError:
            self.log("Error: Non integer baud")
            return

        # Start serial reader
        self.serial_thread = threading.Thread(
            target=self.serial_reader_thread,
            args=(port, baud),
            daemon=True
        )
        self.serial_thread.start()

    def disconnect_serial(self):
        self.stop_event.set()

        if self.serial_thread:
            self.serial_thread.join()

        if self.send_thread:
            self.send_thread.join()

        self.stop_event.clear()
        self.connect_btn.config(text="Connect")
        self.connect_btn.config(command=self.connect_serial)
        self.log("Disconnected COM Port")
        return

    def serial_reader_thread(self, port, baud):
        try:
            self.ser = serial.Serial(port, baud, timeout=.1)
        except Exception:
            self.log("Failed to open COM Port")
            return
        ser = self.ser

        self.log(f"Connected to port {port} at {baud} baud")
        self.connect_btn.config(text="Disconnect")
        self.connect_btn.config(command=self.disconnect_serial)

        # Start sender thread now that port is open
        self.send_thread = threading.Thread(target=self.serial_sender_thread, daemon=True)
        self.send_thread.start()

        while not self.stop_event.is_set():
            try:
                header = ser.read(1)
                if header == b'\xff':
                    data = ser.read(self.size)
                    if len(data) != self.size:
                        continue

                    values = struct.unpack(self.fmt, data)
                    for key, value in zip(self.current_params.keys(), values):
                        self.current_params[key] = value
                    self.update_param_labels()
                    self.log("Received parameters")

                elif header == b'\xa5':
                    print("Success")

            except Exception:
                break

        ser.close()

    def update_param_labels(self):
        for key, label in self.param_labels.items():
            label.config(text=str(self.current_params[key]))

    def on_closing(self):
        self.stop_event.set()
        if self.serial_thread and self.serial_thread.is_alive():
            self.serial_thread.join()
        if self.send_thread and self.send_thread.is_alive():
            self.send_thread.join()
        self.destroy()


if __name__ == "__main__":
    app = SerialApp()
    app.protocol("WM_DELETE_WINDOW", app.on_closing)
    app.mainloop()
