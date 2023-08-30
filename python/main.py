import firebase_admin
from firebase_admin import db

from datetime import datetime, timedelta

import sheetConnect
#sensitive data in a separate file for better security
import sensitiveData

SPREADSHEET_ID = sensitiveData.SPREADSHEET_ID
RANGE_NAME = sensitiveData.RANGE_NAME

serviceAccountKey = sensitiveData.serviceAccountKey

databaseURL = sensitiveData.databaseURL

#connect to google sheets
sheet = sheetConnect.sheetConnect()

#connect to firebase
cred = firebase_admin.credentials.Certificate(serviceAccountKey)
default_app = firebase_admin.initialize_app(cred, {
    'databaseURL': databaseURL
})

#set the references to the database
beacons_ref = db.reference('localiza/beacons')
scanner_ref = db.reference('localiza/scanner')
dict_ref_scanner = db.reference('localiza/dictionary/scanners')
dict_ref_beacon = db.reference('localiza/dictionary/beacons')
update_ref = db.reference('localiza/update_status')

#set the dictionaries for the beacons and scanners
dict_data_scanner = dict_ref_scanner.get()
dict_data_beacon = dict_ref_beacon.get()

#clear the database
scanner_ref.delete()

#clear the spreadsheet
result = sheet.values().clear(spreadsheetId=SPREADSHEET_ID, range=RANGE_NAME).execute()

last_filter = datetime.now()
status_check = False

#main loop
while(1):
    #get the current time
    current_time = datetime.now()

    #check if the last filter was more than 1 minute ago
    if current_time - last_filter > timedelta(minutes=1):

        #check if the status check was already done 
        if status_check == False:
            #status check
            for scanner_status in update_ref.get().keys():
                status_data = update_ref.child(scanner_status).get()
            
                if status_data == False:
                    print(f"{scanner_status} not available")
                    last_filter = datetime.now()
    
            status_check = True

        #if the status check was already done
        else:

            #iterating through the beacons
            for beacon_key in beacons_ref.get().keys():
                beacon_data = beacons_ref.child(beacon_key).get()
                lowest_rssi_value = int(-100)
                lowest_rssi_time = int()
                lowest_rssi_device = str()
                aux = False

                #iterating through the scanners
                for device_key in beacon_data.keys():
                    device_data = beacon_data[device_key]
                    valores_adicionar = []
                    device_time = datetime.fromtimestamp(device_data['time'])

                    #check if the device is in the dictionary
                    try:
                        dict_data_beacon[beacon_key]
                        if current_time - device_time > timedelta(minutes=131400):
                            beacons_ref.child(beacon_key).child(device_key).delete()
                            print(f"Deleted {device_key} from {beacon_key}")

                        elif device_data['rssi'] > lowest_rssi_value:
                            lowest_rssi_value = device_data['rssi']
                            lowest_rssi_device = device_key
                            lowest_rssi_time = device_data['time']
                            aux = True
                    #if the device is not in the dictionary, delete it
                    except:
                        print("no device in dict")
                        beacons_ref.child(beacon_key).delete()

                #add the data to the spreadsheet
                device_time = str(datetime.fromtimestamp(lowest_rssi_time))
                valores_adicionar.append([dict_data_beacon[beacon_key], beacon_key, lowest_rssi_value, device_time, dict_data_scanner[lowest_rssi_device], lowest_rssi_device])
                result = sheet.values().append(spreadsheetId=SPREADSHEET_ID, range=RANGE_NAME, valueInputOption="RAW", body={"values": valores_adicionar}).execute() 

                #add the data to the database
                if aux:
                    scanner_ref.child(lowest_rssi_device).child(beacon_key).set({
                        'rssi': lowest_rssi_value,
                        'time': lowest_rssi_time
                    })
                    print(f"Added {beacon_key} to {lowest_rssi_device}")
                    
            #reset the status check
            for scanner_status in update_ref.get().keys():
                update_ref.child(scanner_status).set(False)
            status_check = False
            last_filter = datetime.now()