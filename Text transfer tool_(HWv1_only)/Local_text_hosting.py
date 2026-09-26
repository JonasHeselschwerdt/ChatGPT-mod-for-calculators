import tkinter as tk
from tkinter import ttk

import socket
import threading
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer

httpd = None
server_thread = None
server_running = False

# Window setup

root = tk.Tk()
root.title("Calculator Text - Local network HTTP server")
root.geometry("750x620")

# Information bar on top of window

status_frame = ttk.Frame(root, padding=5)
status_frame.pack(fill="x")

ttk.Label(status_frame, text="Hosting on IPv4:").grid(row=0, column=0, padx=4)

ip_var = tk.StringVar()
ip_entry = ttk.Entry(status_frame, width=15, textvariable=ip_var, state="readonly")
ip_entry.grid(row=0, column=1)

ttk.Label(status_frame, text="Port:").grid(row=0, column=2, padx=4)

port_var = tk.StringVar()
port_entry = ttk.Entry(status_frame, width=6, textvariable=port_var, state="readonly")
port_entry.grid(row=0, column=3)

ttk.Label(status_frame, text="Hosting:").grid(row=0, column=4, padx=4)

status_label = ttk.Label(status_frame, text="Inactive", foreground="red")
status_label.grid(row=0, column=5)

ttk.Label(status_frame, text="ESP32s connected:").grid(row=0, column=6, padx=4)

esp_var = tk.StringVar()
esp_entry = ttk.Entry(status_frame, width=5, textvariable=esp_var, state="readonly")
esp_entry.grid(row=0, column=7)

host_button = ttk.Button(status_frame, text="Host")
host_button.grid(row=0, column=8, padx=10)

# Separation line

ttk.Separator(root, orient="horizontal").pack(fill="x", pady=5)

# Text Frames for user input

text_frame = ttk.Frame(root, padding=10)
text_frame.pack(fill="both", expand=True)

text_widgets = []

for i in range(1, 10):
    ttk.Label(text_frame, text=f"Text {i}").grid(row=i, column=0, sticky="nw", pady=4)

    text_box = tk.Text(
        text_frame,
        height=3,          
        width=85,
        wrap="word"
    )
    text_box.grid(row=i, column=1, pady=4)

    text_widgets.append(text_box)

# Host / Refresh-logic

hosting = False

def toggle_host():
    global hosting
    hosting = not hosting

    if hosting:
        status_label.config(text="Active", foreground="green")
        host_button.config(text="Refresh")
    else:
        status_label.config(text="Inactive", foreground="red")
        host_button.config(text="Host")

# HTML Creation from text

def ascii_filter(text):
    return "".join(c if ord(c) < 128 else "?" for c in text)

def split_fixed_80(text):
    lines = []
    while text:
        chunk = text[:80]
        text = text[80:]
        if len(chunk) < 80:
            chunk = chunk.ljust(80, " ")
        lines.append(chunk)
    return lines

def ascii_filter(text):
    text = text.replace("\r\n", " ").replace("\n", " ").replace("\t", " ")
    text = " ".join(text.split())  # turn multiple space signs into one
    return "".join(c if ord(c) < 128 else "?" for c in text)


def split_80_wordwise(text):
    lines = []
    while len(text) > 80:
        cut = text.rfind(" ", 0, 80)
        if cut == -1 or cut < 40:   # in case of very long word
            cut = 80
        line = text[:cut]
        lines.append(line.ljust(80))
        text = text[cut:].lstrip()
    lines.append(text.ljust(80))
    return lines


def build_text_section(index, raw_text):
    text_num = index + 1
    raw_text = ascii_filter(raw_text)
    lines = split_80_wordwise(raw_text)

    html = []
    html.append(f"<h1>Text {text_num}</h1>")

    for i, line in enumerate(lines):
        html.append(f'<p id="H{text_num}_U{i}">{line}</p>')

    html.append("")  
    return "\n".join(html)


def update_html_from_gui(text_widgets):
    sections = []

    for i in range(9):
        raw_text = text_widgets[i].get("1.0", "end-1c")
        sections.append(build_text_section(i, raw_text))

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Texts 1-9</title>
</head>
<body>

{chr(10).join(sections)}

</body>
</html>
"""

    with open("index.html", "w", encoding="utf-8") as f:
        f.write(html)

    return True, "New HTML created"


# HTTP server creation

def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
    finally:
        s.close()
    return ip

def find_free_port(ip):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((ip, 0))
    port = s.getsockname()[1]
    s.close()
    return port

def start_server():
    global httpd, server_thread, server_running

    ip = get_local_ip()
    port = find_free_port(ip)

    try:
        httpd = ThreadingHTTPServer((ip, port), SimpleHTTPRequestHandler)
    except OSError:
        return False, None, None

    def run():
        global server_running
        server_running = True
        try:
            httpd.serve_forever()
        finally:
            server_running = False

    server_thread = threading.Thread(target=run, daemon=True)
    server_thread.start()

    return True, ip, port

def stop_server():
    global httpd, server_running
    if httpd:
        httpd.shutdown()
        httpd.server_close()
        server_running = False





def host_pressed():
    global server_running

    if not server_running:
        ok, msg = update_html_from_gui(text_widgets)
        if not ok:
            status_label.config(text="Inactive", foreground="red")
            print(msg)
            return

        success, ip, port = start_server()

        if success:
            ip_var.set(ip)
            port_var.set(str(port))
            status_label.config(text="Active", foreground="green")
            host_button.config(text="Refresh")
        else:
            status_label.config(text="Inactive", foreground="red")

    else:
        # Refresh pressed
        update_html_from_gui(text_widgets)



host_button.config(command=host_pressed)

esp_var.set("0")




root.mainloop()


