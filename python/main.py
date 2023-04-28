import firebase_admin
from firebase_admin import db
from datetime import datetime, timedelta

cred = firebase_admin.credentials.Certificate('serviceAccountKey.json')
default_app = firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://localiza-f2007-default-rtdb.firebaseio.com/'
})

beacons_ref = db.reference('localiza/beacons')
scanner_ref = db.reference('localiza/scanner')

current_time = datetime.now()

scanner_ref.delete()

#por beacon
for beacon_key in beacons_ref.get().keys():
    beacon_data = beacons_ref.child(beacon_key).get()
    lowest_rssi_value = int(-100)
    lowest_rssi_time = int()

    #por scanner dentro do beacon
    for device_key in beacon_data.keys():
        device_data = beacon_data[device_key]

        device_time = datetime.fromtimestamp(device_data['time'])

        if current_time - device_time > timedelta(minutes=1):
            beacons_ref.child(beacon_key).child(device_key).delete()
            print(f"Deleted {device_key} from {beacon_key}")

        elif device_data['rssi'] > lowest_rssi_value:
            lowest_rssi_value = device_data['rssi']
            lowest_rssi_device = device_key
            lowest_rssi_time = device_data['time']

    if lowest_rssi_device is not None:
        scanner_ref.child(lowest_rssi_device).child(beacon_key).set({
            'rssi': lowest_rssi_value,
            'time': lowest_rssi_time
        })
        print(f"Added {beacon_key} to {device_key}")

