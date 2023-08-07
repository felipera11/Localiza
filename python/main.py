from __future__ import print_function

import firebase_admin
from firebase_admin import db
from datetime import datetime, timedelta

import os.path

from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from googleapiclient.errors import HttpError

SCOPES = ['https://www.googleapis.com/auth/spreadsheets']

# The ID and range of a sample spreadsheet.
SAMPLE_SPREADSHEET_ID = '1IMZrcDvgddjarKStUBvvv3S-6NIrVxBSd_fl4yAMP2g'
SAMPLE_RANGE_NAME = 'scanner_sheet!A2:F'

def sheetConnect():
    
    creds = None
    
    if os.path.exists('token.json'):
        creds = Credentials.from_authorized_user_file('token.json', SCOPES)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                'credentials.json', SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open('token.json', 'w') as token:
            token.write(creds.to_json())

    try:
        service = build('sheets', 'v4', credentials=creds)

        # Call the Sheets API
        sheet = service.spreadsheets()
       
    except HttpError as err:
        print(err)
    
    return sheet

sheet = sheetConnect()

cred = firebase_admin.credentials.Certificate('serviceAccountKey.json')
default_app = firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://projeto-localiza-388217-default-rtdb.firebaseio.com/'
})

beacons_ref = db.reference('localiza/beacons')
scanner_ref = db.reference('localiza/scanner')
dict_ref_scanner = db.reference('localiza/dictionary/scanners')
dict_ref_beacon = db.reference('localiza/dictionary/beacons')

dict_data_scanner = dict_ref_scanner.get()
dict_data_beacon = dict_ref_beacon.get()

current_time = datetime.now()

scanner_ref.delete()

result = sheet.values().clear(spreadsheetId=SAMPLE_SPREADSHEET_ID, range=SAMPLE_RANGE_NAME).execute()

#por beacon
for beacon_key in beacons_ref.get().keys():
    beacon_data = beacons_ref.child(beacon_key).get()
    lowest_rssi_value = int(-100)
    lowest_rssi_time = int()
    lowest_rssi_device = str()
    aux = False

    #por scanner dentro do beacon
    for device_key in beacon_data.keys():
        device_data = beacon_data[device_key]
        valores_adicionar = []
        device_time = datetime.fromtimestamp(device_data['time'])

        try:
            dict_data_beacon[beacon_key]
            if current_time - device_time > timedelta(minutes=1440):
                beacons_ref.child(beacon_key).child(device_key).delete()
                print(f"Deleted {device_key} from {beacon_key}")

            elif device_data['rssi'] > lowest_rssi_value:
                lowest_rssi_value = device_data['rssi']
                lowest_rssi_device = device_key
                lowest_rssi_time = device_data['time']
                aux = True
        except:
            print("no device in dict")
            beacons_ref.child(beacon_key).delete()

    device_time = str(datetime.fromtimestamp(lowest_rssi_time))
    valores_adicionar.append([dict_data_beacon[beacon_key], beacon_key, lowest_rssi_value, device_time, dict_data_scanner[lowest_rssi_device], lowest_rssi_device])
    result = sheet.values().append(spreadsheetId=SAMPLE_SPREADSHEET_ID, range=SAMPLE_RANGE_NAME, valueInputOption="RAW", body={"values": valores_adicionar}).execute() 

    if aux:
        scanner_ref.child(lowest_rssi_device).child(beacon_key).set({
            'rssi': lowest_rssi_value,
            'time': lowest_rssi_time
        })
        print(f"Added {beacon_key} to {lowest_rssi_device}")
        
