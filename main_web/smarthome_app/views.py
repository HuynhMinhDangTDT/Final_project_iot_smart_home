from django.shortcuts import render , redirect, get_object_or_404
from .models import information
from django.contrib import messages
# from django.contrib.auth import authenticate,login,logout
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.http import JsonResponse
from django.http import HttpResponse, HttpResponseRedirect
from django.template import loader
from django.urls import reverse
import pyrebase
from django.contrib.auth.models import User
from rest_framework import authentication
from rest_framework import exceptions
import firebase_admin as admin
import firebase_admin.auth as auth
from django.contrib import auth
from datetime import datetime

# from smarthome_app.middleware import FirebaseAuthMiddleware
# from server_smart_skylight import  server
# from real_time_fire_detection import fire_detection

import firebase_admin
from firebase_admin import credentials, db

config = {
    'apiKey': "AIzaSyCSTpQEqVcR-lW5DsgE5_Kqn7MrV6OZSx8",
    'authDomain': "iot-smart-home-a7c95.firebaseapp.com",
    'databaseURL': "https://iot-smart-home-a7c95-default-rtdb.asia-southeast1.firebasedatabase.app",
    'projectId': "iot-smart-home-a7c95",
    'storageBucket': "iot-smart-home-a7c95.appspot.com",
    'messagingSenderId': "327764472304",
    'appId': "1:327764472304:web:9a54ddafe8094f88686d35",
    'measurementId': "G-3YJXZXNBEV"
}


cred = credentials.Certificate("static/iot-smart-home-a7c95-firebase-adminsdk-3ky17-f1b7fd681d.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': "https://iot-smart-home-a7c95-default-rtdb.asia-southeast1.firebasedatabase.app/"
})
firebase = pyrebase.initialize_app(config)
authe = firebase.auth()
DataBase = firebase.database()

# Khởi tạo biến lưu thời gian bật nút
toggle_start_times = {
    0: None,
    1: None,
    2: None,
    3: None,
    4: None,
    5: None,
}

# Khởi tạo biến lưu thời gian đã trôi qua (tính bằng giây)
toggle_durations = {
    0: 0,
    1: 0,
    2: 0,
    3: 0,
    4: 0,
    5: 0,
}

def LoginPage(request):
    # if request.method == 'POST':
    #     # username=request.POST.get('username')
    #     email = request.POST.get('email')
    #     password = request.POST.get('password')
    #     try:
    #         # if there is no error then signin the user with given email and password
    #         user = authe.sign_in_with_email_and_password(email, password)
    #         # print(user['idToken'])
    #         id_token = user['idToken']
    #         # print(id_token)
    #         # session_id = user['idToken']
    #         # request.session['uid'] = str(session_id)
    #         idtoken = request.session['uid']
    #         a = authe.get_account_info(idtoken)
    #         # print(a)
    #         if id_token != None:
    #             FirebaseAuthMiddleware.process_request(self, auth_header=id_token)
    #         # print("id" +" " + ' : ' + str(idtoken))
    #         # a = authe.get_account_info(idtoken)
    #         # print(a)
    #         # print(request.session['uid'])
            
    #         # return HttpResponseRedirect(reverse('dashboard'))
    #         return redirect(f'/dashboard/?token={id_token}')
    #     except:
    #         messages.error(request, ('userID or Password is incorrect!!!!'))
    #         return render(request, "login.html")
    #     # user=authenticate(request,username=username,password=pass1)
    #     # if user is not None:
    #     #     login(request,user)
    #     #     # return redirect(request.GET.get('next'))
    #     #     return HttpResponseRedirect(reverse('dashboard'))
    #     # else:
    #     #     messages.success(request, ('userID or Password is incorrect!!!!'))
    #     #     return redirect('login')
    #     print(user['idToken'])
    #     session_id = user['idToken']
    #     request.session['uid'] = str(session_id)

    #     idtoken = request.session['uid']
    #     print("id" +" " + ' : ' + str(idtoken))
    #     a = authe.get_account_info(idtoken)
    #     print(a)
    #     a = a['Users']
    #     # a = a[0]
    #     # a = a['localId']
    # else:
    #     return render(request, 'login.html')
    if request.method == 'POST':
        # response = HttpResponse("Cookies Cleared")
        # response.delete_cookie('sessionid')  # Delete specific cookie
        email = request.POST.get('email')
        password = request.POST.get('password')
        
        try:
            # Attempt to sign in with Firebase
            user = authe.sign_in_with_email_and_password(email, password)
            id_token = user['idToken']
            
            # Store the id_token in session
            request.session['uid'] = id_token
            
            # Redirect to the dashboard or any page after successful login
            response = redirect('/dashboard/')
            # response.set_cookie('idToken', id_token)
            response.set_cookie('idToken', id_token, httponly=True, max_age = 3600)
            return response
            # return redirect(f'/dashboard/?token={id_token}')
        
        except:
            messages.error(request, "Email or password is incorrect!")
            return render(request, "login.html")
    else:
        return render(request, "login.html")


def LogoutPage(request):
    # logout(request)
    # return redirect('login')
    # try:
    #     del request.session['uid']
    # except:
    #     pass
    auth.logout(request)
    # Clear the session
    request.session.flush()  # Clear all session data
    response = redirect('/login/')
    response.delete_cookie('idToken')  # Delete the token cookie
    # Redirect to login page
    # return redirect('login')
    return response
    # return render(request, "Login.html")

# def signUp(request):
#     return render(request, "Registration.html")

# def postsignUp(request):
#      email = request.POST.get('email')
#      passs = request.POST.get('pass')
#      name = request.POST.get('name')
#      try:
#         # creating a user with the given email and password
#         user=authe.create_user_with_email_and_password(email,passs)
#         uid = user['localId']
#         idtoken = request.session['uid']
#         print(uid)
#      except:
#         return render(request, "Registration.html")
#      return render(request,"Login.html")

# @login_required(login_url='login')
# @login_required

def dashboard(request):
    # Check if the user is logged in
    id_token = request.session.get('uid')
    
    if not id_token:
        return redirect('login')  # If no token, redirect to login
    
    try:
        user_info = authe.get_account_info(id_token)
        email_verified = user_info['users'][0]['emailVerified']
        print(email_verified)
        email = user_info['users'][0]['email']
        print(email)
        if not email_verified:
            messages.error(request, "Please verify your email before logging in.")
            return redirect('login')
    except Exception as e:
        messages.error(request, f"Error retrieving account info: {e}")
        return redirect('login')

    data = DataBase.child("Users").get()
    # print(data)
    # print(user_info)
    
    users_ref = db.reference("Users")
    users_data = users_ref.get()
    # print(users_data)
    # Tìm user dựa trên email đăng nhập
    humidity = 0
    temperature = 0
    total_energy_use = 0
    toggle_state0 = True
    toggle_state1 = True
    toggle_state2 = True
    toggle_state3 = True
    toggle_state4 = True
    toggle_state5 = True
    
    power_device0 = 0.0
    power_device1 = 0.0
    power_device2 = 0.0
    power_device3 = 0.0
    power_device4 = 0.0
    power_device5 = 0.0
    
    Energy_device0 = 0.0
    Energy_device1 = 0.0
    Energy_device2 = 0.0
    Energy_device3 = 0.0
    Energy_device4 = 0.0
    Energy_device5 = 0.0
    
    # toggle_status0 = "false"
    # toggle_status1 = "false"
    # toggle_status2 = "false"
    # toggle_status3 = "false"
    # toggle_status4 = "false"
    # toggle_status5 = "false"
    
    name_device0 = ""
    name_device1 = ""
    name_device2 = ""
    name_device3 = ""
    name_device4 = ""
    name_device5 = ""
    if users_data:
        for user_id, user_info in users_data.items():
            if user_info.get("email") == email:
                
                devices = user_info.get("device", [])  # Lấy danh sách thiết bị
                # Đảm bảo có đủ thiết bị trước khi truy cập chỉ mục
                toggle_state0 = devices[0].get('status', 'N/A') if len(devices) > 0 else 'N/A'
                toggle_state1 = devices[1].get('status', 'N/A') if len(devices) > 1 else 'N/A'
                toggle_state2 = devices[2].get('status', 'N/A') if len(devices) > 2 else 'N/A'
                toggle_state3 = devices[3].get('status', 'N/A') if len(devices) > 3 else 'N/A'
                toggle_state4 = devices[4].get('status', 'N/A') if len(devices) > 4 else 'N/A'
                toggle_state5 = devices[5].get('status', 'N/A') if len(devices) > 5 else 'N/A'
                for device in devices:
                    print(f"Device ID: {device.get('id')}, Name: {device.get('name')}, Status: {device.get('status')}, Power: {device.get('power')}, EnergyConsumption: {device.get('energyConsumption')}")
                try:
                    name_device0 = devices[0]['name']
                    power_device0 = devices[0].get('power')
                    Energy_device0 = devices[0].get('energyConsumption')
                    power_device0 = round(power_device0 / 1000, 2)
                    Energy_device0 = round(Energy_device0 / 1000, 2)
                except:
                    name_device0 = "Chua duoc them vao"
                try:
                    name_device1 = devices[1]['name']
                    power_device1 = devices[1].get('power')
                    Energy_device1 = devices[1].get('energyConsumption')
                    power_device1 = round(power_device1 / 1000, 2)
                    Energy_device1 = round(Energy_device1 / 1000, 2)
                except:
                    name_device1 = "Chua duoc them vao"
                try:
                    name_device2 = devices[2]['name']
                    power_device2 = devices[2].get('power')
                    Energy_device2 = devices[2].get('energyConsumption')
                    power_device2 = round(power_device2 / 1000, 2)
                    Energy_device2 = round(Energy_device2 / 1000, 2)
                except:
                    name_device2 = "Chua duoc them vao"
                try:
                    name_device3 = devices[3]['name']
                    power_device3 = devices[3].get('power')
                    Energy_device3 = devices[3].get('energyConsumption')
                    power_device3 = round(power_device3 / 1000, 2)
                    Energy_device3 = round(Energy_device3 / 1000, 2)
                except:
                    name_device3 = "Chua duoc them vao"
                try:
                    name_device4 = devices[4]['name']
                    power_device4 = devices[4].get('power')
                    Energy_device4 = devices[4].get('energyConsumption')
                    power_device4 = round(power_device4 / 1000, 2)
                    Energy_device4 = round(Energy_device4 / 1000, 2)
                except:
                    name_device4 = "Chua duoc them vao"
                try:
                    name_device5 = devices[5]['name']
                    power_device5 = devices[5].get('power')
                    Energy_device5 = devices[5].get('energyConsumption')
                    power_device5 = round(power_device5 / 1000, 2)
                    Energy_device5 = round(Energy_device5 / 1000, 2)
                except:
                    name_device5 = "Chua duoc them vao"
            

            dht11_data = user_info.get("DHT11", {})
            if dht11_data:
                humidity = dht11_data.get('doam', 'N/A')
                temperature = dht11_data.get('nhietdo', 'N/A')
                print(f"DHT11 - Do am: {humidity}")
                print(f"DHT11 - Nhiet do: {temperature}")

            # Truy xuất thông tin công suất tiêu thụ
            power_consumption = user_info.get("capacity", {})
            if power_consumption:
                total_energy_use = power_consumption.get('CongSuatTieuThu', 'N/A')
                total_energy_use = round(total_energy_use / 1000, 2)
                print(f"Công suất tiêu thụ: {total_energy_use}")
                total_Power = power_consumption.get('TongCongSuat', 'N/A')
                total_Power = round(total_Power / 1000, 4)
                print(f"Công suất tiêu thụ: {total_Power}")
                
                
            # devices = user_info.get("device", {})
            # for device in devices:
            #     print(f"Device ID: {device.get('id')}, Name: {device.get('name')}, Status: {device.get('status')}")
            
    # Cập nhật thời gian bật cho từng toggle_state nếu nó được bật (True)
    if toggle_state0 == True:
        if toggle_start_times[0] is None:  # Nếu chưa lưu thời gian bật
            toggle_start_times[0] = datetime.now()
    else:
        toggle_start_times[0] = None  # Nếu tắt, reset thời gian

    if toggle_state1 == True:
        if toggle_start_times[1] is None:
            toggle_start_times[1] = datetime.now()
    else:
        toggle_start_times[1] = None

    if toggle_state2 == True:
        if toggle_start_times[2] is None:
            toggle_start_times[2] = datetime.now()
    else:
        toggle_start_times[2] = None

    if toggle_state3 == True:
        if toggle_start_times[3] is None:
            toggle_start_times[3] = datetime.now()
    else:
        toggle_start_times[3] = None

    if toggle_state4 == True:
        if toggle_start_times[4] is None:
            toggle_start_times[4] = datetime.now()
    else:
        toggle_start_times[4] = None

    if toggle_state5 == True:
        if toggle_start_times[5] is None:
            toggle_start_times[5] = datetime.now()
    else:
        toggle_start_times[5] = None

    # Cập nhật thời gian đã trôi qua
    toggle_durations[0] = get_duration(toggle_start_times[0])
    toggle_durations[1] = get_duration(toggle_start_times[1])
    toggle_durations[2] = get_duration(toggle_start_times[2])
    toggle_durations[3] = get_duration(toggle_start_times[3])
    toggle_durations[4] = get_duration(toggle_start_times[4])
    toggle_durations[5] = get_duration(toggle_start_times[5])
    # print(toggle_durations[0])
    
    formatted_date = get_formatted_date()
    
    # Lấy thời gian hiện tại
    current_time = datetime.now()

    # Định dạng thời gian theo "hh:mm"
    formatted_time = current_time.strftime("%H:%M")
                
    if request.method == "POST":
        # Cập nhật trạng thái toggle
        toggle_state0 = request.POST.get("toggle0")
        toggle_state1 = request.POST.get("toggle1")
        toggle_state2 = request.POST.get("toggle2")
        toggle_state3 = request.POST.get("toggle3")
        toggle_state4 = request.POST.get("toggle4")
        toggle_state5 = request.POST.get("toggle5")
        
        if users_data:
            for user_id, user_info in users_data.items():
                if user_info.get("email") == email:
                    print(f"User ID: {user_id}")
                    print(f"Name: {user_info.get('name')}")
                    print(f"Phone: {user_info.get('phone')}")
                    # Truy xuất thông tin DHT11
                    # dht11_data = user_info.get("DHT11", {})
                    # if dht11_data:
                    #     humidity = dht11_data.get('doam', 'N/A')
                    #     temperature = dht11_data.get('nhietdo', 'N/A')
                    #     print(f"DHT11 - Do am: {humidity}")
                    #     print(f"DHT11 - Nhiet do: {temperature}")

                    # # Truy xuất thông tin công suất tiêu thụ
                    # power_consumption = user_info.get("capacity", {})
                    # if power_consumption:
                    #     total_energy_use = power_consumption.get('CongSuatTieuThu', 'N/A')
                    #     print(f"Công suất tiêu thụ: {total_energy_use}")

                    # Truy xuất thông tin thiết bị
                    devices = user_info.get("device", [])
                    for device in devices:
                        print(f"Device ID: {device.get('id')}, Name: {device.get('name')}, Status: {device.get('status')}")
                    
                    try:
                        name_device0 = devices[0]['name']
                    except:
                        name_device0 = "Chua duoc them vao"
                    try:
                        name_device1 = devices[1]['name']
                    except:
                        name_device1 = "Chua duoc them vao"
                    try:
                        name_device2 = devices[2]['name']
                    except:
                        name_device2 = "Chua duoc them vao"
                    try:
                        name_device3 = devices[3]['name']
                    except:
                        name_device3 = "Chua duoc them vao"
                    try:
                        name_device4 = devices[4]['name']
                    except:
                        name_device4 = "Chua duoc them vao"
                    try:
                        name_device5 = devices[5]['name']
                    except:
                        name_device5 = "Chua duoc them vao"
                    
                    # print(name_device0)
                    # print(name_device1)
                    # print(name_device2)
                    # print(name_device3)
                    # print(name_device4)
                    # print(name_device5)
                    
                    if name_device0 != "Chua duoc them vao":
                        if toggle_state0 is None:
                            toggle_status0 = False
                            device_ref = users_ref.child(user_id).child("device").child("0")
                            device_ref.update({
                                "status": bool(toggle_status0)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("0").update({"status": toggle_status0})
                        else:
                            toggle_status0 = toggle_state0
                            device_ref = users_ref.child(user_id).child("device").child("0")
                            device_ref.update({
                                "status": bool(toggle_status0)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("0").update({"status": toggle_status0})
                    if name_device1 != "Chua duoc them vao":
                        if toggle_state1 is None:
                            toggle_status1 = False
                            device_ref = users_ref.child(user_id).child("device").child("1")
                            device_ref.update({
                                "status": bool(toggle_status1)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("1").update({"status": toggle_status1})
                        else:
                            toggle_status1 = toggle_state1
                            device_ref = users_ref.child(user_id).child("device").child("1")
                            device_ref.update({
                                "status": bool(toggle_status1)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("1").update({"status": toggle_status1})
                    if name_device2 != "Chua duoc them vao":
                        if toggle_state2 is None:
                            toggle_status2 = False
                            device_ref = users_ref.child(user_id).child("device").child("2")
                            device_ref.update({
                                "status": bool(toggle_status2)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("2").update({"status": toggle_status2})
                        else:
                            toggle_status2 = toggle_state2
                            device_ref = users_ref.child(user_id).child("device").child("2")
                            device_ref.update({
                                "status": bool(toggle_status2)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("2").update({"status": toggle_status2})
                    if name_device3 != "Chua duoc them vao":
                        if toggle_state3 is None:
                            toggle_status3 = False
                            device_ref = users_ref.child(user_id).child("device").child("3")
                            device_ref.update({
                                "status": bool(toggle_status3)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("3").update({"status": toggle_status3})
                        else:
                            toggle_status3 = toggle_state3
                            device_ref = users_ref.child(user_id).child("device").child("3")
                            device_ref.update({
                                "status": bool(toggle_status3)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("3").update({"status": toggle_status3})
                    if name_device4 != "Chua duoc them vao":
                        if toggle_state4 is None:
                            toggle_status4 = False
                            device_ref = users_ref.child(user_id).child("device").child("4")
                            device_ref.update({
                                "status": bool(toggle_status4)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("4").update({"status": toggle_status4})
                        else:
                            toggle_status4 = toggle_state4
                            device_ref = users_ref.child(user_id).child("device").child("4")
                            device_ref.update({
                                "status": bool(toggle_status4)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("4").update({"status": toggle_status4})
                    if name_device5 != "Chua duoc them vao":
                        if toggle_state5 is None:
                            toggle_status5 = False
                            device_ref = users_ref.child(user_id).child("device").child("5")
                            device_ref.update({
                                "status": bool(toggle_status5)
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("5").update({"status": toggle_status5})
                        else:
                            toggle_status5 = toggle_state5
                            device_ref = users_ref.child(user_id).child("device").child("5")
                            device_ref.update({
                                "status": bool(toggle_status5)
                            })
                    print("toggle0: ", toggle_status0)
                    print("toggle1: ", toggle_status1)
                    print("toggle2: ", toggle_status2)
                    print("toggle3: ", toggle_status3)
                    print("toggle4: ", toggle_status4)
                    print("toggle5: ", toggle_status5)
                # user_ref = DataBase.child("Users").child(user.key())
                # user_ref.child("device").child("5").update({"status": toggle_status5})
    # localid = user_info['users'][0]['localId']
    # print(localid)
    
    # print(data.child("localid").get("temperature",'N/A'))
    # user_ref = DataBase.child("Users").child(localid)
    # data2 =user_ref.child("device").child("0")
    # print(user_ref)
    # if data.each():
    #     current_data = {}
    #     for user in data.each():
    #         current_data[user.key()] = user.val()  # Lưu dữ liệu hiện tại
    #         # print(f"User ID: {user.key()}")  # In ra ID của người dùng
    #         if current_data[user.key()].get("email") == email:
    #             # Truy xuất thông tin DHT11 và các thiết bị
    #             dht11_data = user.val().get("DHT11", {})
    #             if dht11_data:
    #                 # print("DHT11 Data:")
    #                 # print(f"  - Do am: {dht11_data.get('doam', 'N/A')}")
    #                 humidity = dht11_data.get('doam', 'N/A')
                    
    #                 # print(f"  - Nhiet do: {dht11_data.get('nhietdo', 'N/A')}")
    #                 temperature = dht11_data.get('nhietdo', 'N/A')
                
    #             powerconsumtion = user.val().get("capacity", {})
                
    #             if powerconsumtion:
    #                 # print(f"  - Cong Suat Tieu Thu: {powerconsumtion.get('CongSuatTieuThu', 'N/A')}")

    #                 Total_Energy_Use = powerconsumtion.get('CongSuatTieuThu', 'N/A')
    
    # device = user.val().get("device", {})
    # user_ref = DataBase.child("Users").child(user.key())
    # print(user_ref)
                
    # device0_status = device[0]['status']
    # print(device0_status)
    
    
    
    
    
        
    return render(request, "dashboard2.html", {
                    'hum': humidity,
                    'temp': temperature,
                    'capacity': total_energy_use,
                    'namedv0': name_device0,
                    'namedv1': name_device1,
                    'namedv2': name_device2,
                    'namedv3': name_device3,
                    'namedv4': name_device4,
                    'namedv5': name_device5,
                    'power_device0' : power_device0,
                    'Energy_device0': Energy_device0,
                    'power_device1' : power_device1,
                    'Energy_device1': Energy_device1,
                    'power_device2' : power_device2,
                    'Energy_device2': Energy_device2,
                    'power_device3' : power_device3,
                    'Energy_device3': Energy_device3,
                    'power_device4' : power_device4,
                    'Energy_device4': Energy_device4,
                    'power_device5' : power_device5,
                    'Energy_device5': Energy_device5,
                    'toggles0': toggle_state0,
                    'toggles1': toggle_state1,
                    'toggles2': toggle_state2,
                    'toggles3': toggle_state3,
                    'toggles4': toggle_state4,
                    'toggles5': toggle_state5,
                    'toggle_durations_0': toggle_durations[0],
                    'toggle_durations_1': toggle_durations[1],
                    'toggle_durations_2': toggle_durations[2],
                    'toggle_durations_3': toggle_durations[3],
                    'toggle_durations_4': toggle_durations[4],
                    'toggle_durations_5': toggle_durations[5],
                    'formatted_date': formatted_date,
                    'formatted_time': formatted_time,
                    'total_Power': total_Power,
    })

# Hàm để tính thời gian đã trôi qua
def get_duration(start_time):
    if start_time:
        # Tính số giây đã trôi qua
        duration_in_seconds = (datetime.now() - start_time).total_seconds()
        return format_duration(duration_in_seconds)
    return "00:00:00"  # Nếu không có thời gian bắt đầu, trả về "00:00:00"

def format_duration(seconds):
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    seconds = int(seconds % 60)
    return f"{hours:02}:{minutes:02}:{seconds:02}"

def get_formatted_date():
    # Lấy ngày và giờ hiện tại
    current_date = datetime.now()
    # Định dạng ngày theo yêu cầu: Ví dụ: Saturday January 20, 2025
    formatted_date = current_date.strftime("%A %B %d, %Y")
    return formatted_date
