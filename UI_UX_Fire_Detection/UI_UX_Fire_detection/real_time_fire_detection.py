import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "yolov5"))
import torch
from matplotlib import pyplot as plt
import numpy as np
import cv2
from PIL import ImageGrab, Image
import time
import multiprocessing
from multiprocessing import Pipe

import time
from email.message import EmailMessage
import ssl
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage

import tkinter as tk
from PIL import Image, ImageTk
from tkinter.filedialog import askopenfile
from tkinter import CENTER, filedialog
from tkinter import HORIZONTAL, messagebox
from tkinter import ttk
import tkinter.ttk
import time
import string
from tkinter.filedialog import askopenfile


# # Khai báo biến toàn cục
# Input_ip_text = None
# Input_username_text = None
# Input_password_text = None

def SendEmail(email_receiver):
    count = 0
    email_sender = "mdthienxa@gmail.com"
    email_password = "ubkq irqt hhqj trpm"
    # email_receiver = "vndang00@gmail.com"
    # email_receiver = ["vndang00@gmail.com"] #Email người nhận

    subject = "Cảnh báo phát hiện nguồn cháy"
    # body = """
    # Cảnh báo hiện tại phát hiện đám cháy tại nhà bạn cần xem xét kỹ lưỡng sao đó gọi cho 114
    # """
    # em = EmailMessage()
    # em["From"] = email_sender
    # em["To"] = email_receiver
    # em["subject"] = subject
    # em.set_content(body)

    msg = MIMEMultipart()
    # msg["To"] = email_receiver
    msg["To"] = ", ".join(email_receiver)
    msg["From"] = email_sender
    msg["subject"] = subject
    
    msg_ready = MIMEText('Cảnh báo hiện tại phát hiện đám cháy tại nhà bạn cần xem xét kỹ lưỡng sau đó gọi cho 114')

    image_open = open('fire_detect_image.jpg', 'rb').read()
    image_ready = MIMEImage(image_open,'jpg', name = 'Nguon chay')

    msg.attach(msg_ready)
    msg.attach(image_ready)

    context = ssl.create_default_context()
    with smtplib.SMTP_SSL("smtp.gmail.com", 465, context=context) as smtp:
        smtp.login(email_sender, email_password)
        smtp.sendmail(email_sender, email_receiver, msg.as_string())
                    
def switch_camera(current_camera):
    return cap2 if current_camera == cap1 else cap1

# def fire_detection(url_yolov5, path_weight, current_camera):
def fire_detection(Input_ip_text, Input_username_text, Input_password_text, Input_email_receiver_text):
    # global Input_ip_text, Input_username_text, Input_password_text
    url_yolov5 = "yolov5"
    path_weight = "runs/train/exp7/weights/best.pt"
    img_counter = 0
    model = torch.hub.load(
        url_yolov5, "custom", path=path_weight, source="local", force_reload=True
    )
    IP = Input_ip_text.get()
    USERNAME = Input_username_text.get()
    PASSWORD = Input_password_text.get()
    email_receiver = Input_email_receiver_text.get()
    
    print(IP)
    print(USERNAME)
    print(PASSWORD)
    print(email_receiver)
    
    data_input_save = ',' + IP + ',' + USERNAME + ',' + PASSWORD + ',' + email_receiver
    with open('save_input.txt', 'w') as f:
        f.write(data_input_save)
        
    # USERNAME = "admin"
    # PASSWORD = "dang0802"
    # IP = "192.168.1.100"
    PORT = "554"

    # URL = "rtsp://{}:{}@{}:{}/onvif1".format(USERNAME, PASSWORD, IP, PORT)
    URL = "rtsp://{}:{}@{}:{}/ch1/main".format(USERNAME, PASSWORD, IP, PORT)
    cap1 = cv2.VideoCapture(URL, cv2.CAP_FFMPEG)
    cap2 = cv2.VideoCapture(0)

    current_camera = cap2
    
    CLASSES = ["lua"]
    
    size = 416
    count = 0

    fps_start_time = time.time()
    fps = 0

    while True:
        ret, frame = current_camera.read()
        
        if not ret:
            print("Failed to capture frame")
            current_camera = switch_camera(current_camera)
            ret, frame = current_camera.read()
        
        fps_end_time = time.time()
        fps_diff_time = fps_end_time - fps_start_time
        # fps = 1 / fps_diff_time
        if fps_diff_time != 0:
            fps = 1 / fps_diff_time
        else:
            fps = 0  # hoặc giá trị mặc định bạn muốn
        fps_start_time = fps_end_time
        fps_text = "FPS: {:.2f}ms".format(fps)
        frame = cv2.resize(frame, (600, 500))
        converted = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        converted = Image.fromarray(converted)
        results = model(converted)

        for index, row in results.pandas().xyxy[0].iterrows():
            x1 = int(row["xmin"])
            y1 = int(row["ymin"])
            x2 = int(row["xmax"])
            y2 = int(row["ymax"])
            id = row["class"]
            acuracy = row["confidence"]

            if acuracy * 100 > 70:
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), 2)
                cv2.putText(
                    frame,
                    str(CLASSES[id]) + ": " + str(round(acuracy, 2)),
                    (x1, y1),
                    cv2.FONT_HERSHEY_PLAIN,
                    2,
                    (0, 255, 0),
                    2,
                )
                cv2.putText(
                    frame,
                    fps_text,
                    (5, 30),
                    cv2.FONT_HERSHEY_COMPLEX,
                    1,
                    (255, 255, 0),
                    1,
                )
                # message = "lua"
                try:
                    # img_name = "opencv_frame_{}.png".format(img_counter)
                    img_name = "fire_detect_image.jpg"
                    # cv2.imwrite(img_name, frame)
                    cv2.imwrite(filename=img_name, img=frame)
                    print("{} written!".format(img_name))
                    img_counter += 1
                    SendEmail(email_receiver)
                except:
                    pass

        cv2.imshow("result", frame)
        if cv2.waitKey(10) & 0xFF == ord("q"):
            break
    
    current_camera.release()
    cv2.destroyAllWindows()
    
def UI_UX_Aplication():
    # global Input_ip_text, Input_username_text, Input_password_text
    # app front end
    app = tk.Tk()

    app.title('Smart home CCTV')
    app.geometry('700x420')
    file_path = ""

    # Part Base SW
    frameall = tk.Frame(app)
    frame1 = tk.Frame(frameall)
    frame2 = tk.Frame(frameall)
        
    # Input_path_text = tk.StringVar()
    # Input_path_label = tk.Label(frame1, text='Input path', font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=0, column=0, sticky='w')
    # Input_path_entry = tk.Entry(frame1, textvariable=Input_path_text,
    #                             font='large_font', width=55).grid(row=1, column=0, sticky='w')

    # Output_path_text = tk.StringVar()
    # Output_path_label = tk.Label(frame1, text='Output path', font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=2, column=0, sticky='w')
    # Output_path_entry = tk.Entry(frame1, textvariable=Output_path_text,
    #                             font='large_font', width=55).grid(row=3, column=0, sticky='w')
    
    Input_ip_text = tk.StringVar()
    Input_ip_label = tk.Label(frame1, text='Input ip', font=(
        'bold', 14), bg="#20bebe", fg="black").grid(row=0, column=0, sticky='w')
    Input_ip_entry = tk.Entry(frame1, textvariable=Input_ip_text,
                                font='large_font', width=55).grid(row=1, column=0, sticky='w')
    
    Input_username_text = tk.StringVar()
    Input_username_label = tk.Label(frame1, text='Input username', font=(
        'bold', 14), bg="#20bebe", fg="black").grid(row=2, column=0, sticky='w')
    Input_username_entry = tk.Entry(frame1, textvariable=Input_username_text,
                                font='large_font', width=55).grid(row=3, column=0, sticky='w')
    
    Input_password_text = tk.StringVar()
    Input_password_label = tk.Label(frame1, text='Input password', font=(
        'bold', 14), bg="#20bebe", fg="black").grid(row=4, column=0, sticky='w')
    Input_password_entry = tk.Entry(frame1, textvariable=Input_password_text,
                                font='large_font', width=55).grid(row=5, column=0, sticky='w')
    Input_email_receiver_text = tk.StringVar()
    Input_email_receiver_label = tk.Label(frame1, text='Input email_receiver', font=(
        'bold', 14), bg="#20bebe", fg="black").grid(row=6, column=0, sticky='w')
    Input_email_receiver_entry = tk.Entry(frame1, textvariable=Input_email_receiver_text,
                                font='large_font', width=55).grid(row=7, column=0, sticky='w')
    

    # Output_path_text = tk.StringVar()
    # Output_path_label = tk.Label(frame1, text='Output path', font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=2, column=0, sticky='w')
    # Output_path_entry = tk.Entry(frame1, textvariable=Output_path_text,
    #                             font='large_font', width=55).grid(row=3, column=0, sticky='w')

    # # browse button open file
    # browse_input_path_text = tk.StringVar()
    # browse_btn_input_path = tk.Button(frame1, textvariable=browse_input_path_text, command=lambda: open_file(
    # ), font="bold", width=7, height=1).grid(row=1, column=1, pady=5, padx=10)
    # browse_input_path_text.set("Browse")

    # # browse button save file
    # browse_output_path_text = tk.StringVar()
    # browse_btn_output_path = tk.Button(frame1, textvariable=browse_output_path_text, command=lambda: save_file(
    # ), font="bold", width=7, height=1).grid(row=3, column=1, pady=5, padx=10)
    # browse_output_path_text.set("Browse")

    # frame1.pack()

    # Run program Buttons
    # run_btn_text = tk.StringVar()
    # run_btn = tk.Button(frame1, textvariable=run_btn_text, command=start_program,
    #                     font="bold", width=15).grid(row=4, column=0, columnspan=2, pady=20)
    # run_btn_text.set("RUN")
    run_btn_text = tk.StringVar()
    run_btn = tk.Button(frame1, textvariable=run_btn_text, command=lambda: fire_detection(Input_ip_text, Input_username_text, Input_password_text, Input_email_receiver_text),
                        font="bold", width=15).grid(row=8, column=0, columnspan=2, pady=20)
    run_btn_text.set("RUN")

    # frame1.pack()
    # frame2.pack()

    # noneFill = tk.StringVar()
    # noneLabel = tk.Label(frame1, textvariable=noneFill, font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=5, column=0, columnspan=2)
    # frame3.pack()

    # frame1.pack()

    # progress bar
    # bar = ttk.Progressbar(app, orient='horizontal', length=583, mode='determinate')

    # bar.place(relx=0.5, rely=0.6, anchor=CENTER)

    # frame1.pack()

    # frame4.pack()
    frame1.pack()

    # programing counter / attempt couter step
    # step = ('1', '2', '3', '4', '5', '6', '7', '8', '9', '10')

    # Programing_counter_text = tk.StringVar()
    # Programing_counter_label = tk.Label(frame2, text='Programming_counter_step', font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=0, column=0, padx=30, pady=10, sticky='w')
    # Programing_counter = tk.Spinbox(frame2, textvariable=Programing_counter_text,
    #                                 values=step, width=10, font=('helvetica', 15)).grid(row=1, column=0, padx=30)


    # Programing_Attempt_counter_text = tk.StringVar()
    # Programing_Attempt_counter_label = tk.Label(frame2, text='Programming_Attempt_counter_step', font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=0, column=1, padx=30, pady=10, sticky='w')
    # Programing_Attempt_counter = tk.Spinbox(frame2, textvariable=Programing_Attempt_counter_text,
    #                                         values=step, width=10, font=('helvetica', 15)).grid(row=1, column=1, padx=30)
    # percent = tk.StringVar()
    # percentLabel = tk.Label(frame2, textvariable=percent, font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=5, column=0, columnspan=2)

    # noneFill = tk.StringVar()
    # noneLabel = tk.Label(frame2, textvariable=noneFill, font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=6, column=0, columnspan=2)
    # # frame2.pack()

    # noneFill = tk.StringVar()
    # noneLabel = tk.Label(frame2, textvariable=noneFill, font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=7, column=0, columnspan=2)

    # noneFill = tk.StringVar()
    # noneLabel = tk.Label(frame2, textvariable=noneFill, font=(
    #     'bold', 14), bg="#20bebe", fg="black").grid(row=8, column=0, columnspan=2)



    frame2.pack()



    frameall.place(relx=0.5, rely=0.5, anchor=CENTER)

    # instruction

    instruction = tk.Label(
        app, text="          Welcome to Smart Home CCTV let connect to your camera", font=("helvetica", 14))
    instruction_version = tk.Label(app, text="R1.3.1", font=("helvetica", 14))
    instruction_version.pack(side="right", anchor='s')
    instruction.pack(side="bottom", fill='both', anchor=CENTER)

    frameall.configure(background="#20bebe")
    frame1.configure(background="#20bebe")
    frame2.configure(background="#20bebe")

    app.configure(background="#20bebe")

    try:
        with open('save_input.txt', 'r') as f:
            data_input = f.read()
            # print(data_input)
        
            # ip
            data_input = data_input.partition(",")[2] 
            direct_input = data_input
            direct_input = direct_input.split(',', 1)[0]
            Input_ip_text.set(str(direct_input))
        
            # username
            data_input = data_input.partition(",")[2] 
            direct_output = data_input
            direct_output = direct_output.split(',', 1)[0]
            Input_username_text.set(direct_output)
            
            # password
            data_input = data_input.partition(",")[2] 
            direct_output = data_input
            direct_output = direct_output.split(',', 1)[0]
            Input_password_text.set(direct_output)

            # email_receiver
            data_input = data_input.partition(",")[2] 
            direct_output = data_input
            direct_output = direct_output.split(',', 1)[0]
            Input_email_receiver_text.set(direct_output)
    except:
        print('chua co file data input')

    # Start program
    app.mainloop()

if __name__ == "__main__":
    # url_yolov5 = "yolov5"
    # path_weight = "runs/train/exp7/weights/best.pt"
    # USERNAME = "admin"
    # PASSWORD = "ZSLHOS"
    # IP = "192.168.1.100"
    # PORT = "554"

    # URL = "rtsp://{}:{}@{}:{}/onvif1".format(USERNAME, PASSWORD, IP, PORT)
    # cap1 = cv2.VideoCapture(URL, cv2.CAP_FFMPEG)
    # cap2 = cv2.VideoCapture(0)

    # current_camera = cap1
    # fire_detection(url_yolov5, path_weight, current_camera)
    UI_UX_Aplication()