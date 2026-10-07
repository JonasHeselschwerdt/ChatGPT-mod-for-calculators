'''
ChatGPT Hardware Hack for calculators: Software V2

© 2026 Jonas Heselschwerdt
Licensed under CC BY-NC 4.0

External Equation rendering server for ChatGPT-Mod for calculators
'''

'''
Basic working principle:

1.  ESP32S3 --(Request JSON via HTTP POST)--> Flask HTTP server ---(equations)---> Matplotlib --(rendered equations)--
    --> NumPy & Pillow ----> Equations rendered and converted to XBM (Bitmaps)

2.  XBMs ---> Flask HTTP server ---> Add all XBMs to a bytestream ---> ESP32S3 (---> loads XBMs to flash memory)

Notes: In Request JSON the ESP32S3 instructs the server on how to scale the XBMs (x and y-px) and maximum amount of equations
that can be sent back (limited flash memory on ESP32S3)

A log (xbm_debug.log) is provided, these XBMs can be imported into Lopaka for testing
'''


from flask import Flask, request, Response
import json

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from PIL import Image
import numpy as np


app = Flask(__name__)

# XBM Settings

LOG_FILE = "xbm_debug.log"






# XBM Debugging Log

def log_xbm(xbm, xbm_index):

    with open(LOG_FILE, "a") as f:

        f.write(f"\nconst uint8_t xbm_{xbm_index}[{len(xbm)}] = {{\n")
        for i, byte in enumerate(xbm):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02X}")
            if i < len(xbm) - 1:
                f.write(", ")
            if i % 16 == 15:
                f.write("\n")
        if len(xbm) % 16 != 0:
            f.write("\n")
        f.write("};\n")





# Render Request

@app.route("/render", methods=["POST"])

def render():
    raw_data = request.get_data().decode("utf-8")
    # Make sure '\' are properly escaped in JSON
    raw_data = raw_data.replace("\\", "\\\\")
    try:
        data = json.loads(raw_data)
    except json.JSONDecodeError as e:
        print(f"Invalid JSON: {e}")
        return Response("Invalid JSON",status=400)

    width = data.get("width")
    height = data.get("height")
    max_xbms = data.get("max_xbms")
    equations = data.get("equations", [])
    print(f"Width:     {width}")
    print(f"Height:    {height}")
    print(f"Max XBMs:  {max_xbms}")
    print(f"Equations: {len(equations)}")

    response_data = bytearray()
    xbm_count = 0

    # Rendering using Matplotlib

    for i, equation in enumerate(equations):
        print(f"Equation {i}: {equation}")
        mathtext = f"${equation}$"              # $ signs to denote Mathtext format
        try:
            xbms = convert(mathtext, width, height)
            print(f"Equation {i}: "f"{len(xbms)} XBM(s)")
            for xbm in xbms:
                if xbm_count >= max_xbms:
                    print("Maximum number of XBMs reached.")
                    break
                # Debugging
                log_xbm(xbm,xbm_count)
                response_data.extend(xbm)
                xbm_count += 1
        except Exception as e:
            print(f"ERROR rendering equation {i}: {e}")
        if xbm_count >= max_xbms:
            break

    # Result

    print(f"Returning {xbm_count} XBM(s), "f"{len(response_data)} bytes")

    return Response(bytes(response_data),status=200,mimetype="application/octet-stream")





# Equation to XBM conversion

def convert(equation, width, height):

    fig = plt.figure(figsize=(6, 1),dpi=150)
    fig.text(0,0.5,equation,fontsize=28,va="center",ha="left")
    plt.axis("off")
    fig.canvas.draw()
    image = np.asarray(fig.canvas.buffer_rgba())
    plt.close(fig)

    # Set Bounding Box

    rgb = image[:, :, :3]
    mask = np.any(rgb < 250,axis=2)
    ys, xs = np.where(mask)
    if len(xs) == 0:
        return []

    x_min = xs.min()
    x_max = xs.max() + 1

    y_min = ys.min()
    y_max = ys.max() + 1

    cropped = image[y_min:y_max,x_min:x_max]

    # PIL

    img = Image.fromarray(cropped).convert("L")

    # Scale

    if img.height > height:
        scale = height / img.height
        new_width = round(img.width * scale)
        img = img.resize((new_width, height),Image.Resampling.LANCZOS)

    print(f"Equation after scaling "f"{img.width} × {img.height} px")

    # Turn to 1-Bit

    img = img.point(lambda p: 0 if p < 128 else 255)

    # XBM-Pages

    num_pages = (img.width + width - 1) // width
    xbms = []
    for page in range(num_pages):
        x_start = page * width
        x_end = min(x_start + width,img.width)
        page_img = Image.new("1",(width, height),1)
        crop = img.crop((x_start,0,x_end,img.height))
        page_img.paste(crop,(0, 0))
        pixels = page_img.load()
        xbm_data = bytearray()
        bytes_per_row = (width + 7) // 8
        for y in range(height):
            for byte_x in range(bytes_per_row):
                byte = 0
                for bit in range(8):
                    x = byte_x * 8 + bit
                    if pixels[x, y] == 0:
                        byte |= (1 << bit)
                xbm_data.append(byte)
        xbms.append(bytes(xbm_data))
    return xbms




# Server

if __name__ == "__main__":

    app.run(host="0.0.0.0",port=8080)