from flask import Flask, render_template, jsonify
from pymongo import MongoClient
import threading
import paho.mqtt.client as mqtt
from datetime import datetime
from flask_socketio import SocketIO, emit
from pytz import timezone

app = Flask(__name__)
socketio = SocketIO(app)

client = MongoClient("mongodb://localhost:27017/")
db = client['rctank']
hits_collection = db['hits']
temperature_collection = db['temperature']
humidity_collection = db['humidity'] 
local_tz = timezone('Asia/Jakarta')


MQTT_BROKER = "192.168.13.248"
MQTT_PORT = 1884
MQTT_TOPICS = ["controller/temp", "controller/hited", "controller/humidity"]

mqtt_client = mqtt.Client()

def on_message(client, userdata, msg):
    print(f"Pesan diterima di topik {msg.topic}: {msg.payload.decode()}")
    timestamp = datetime.now()  

    if msg.topic == "controller/temp":
        temperature = float(msg.payload.decode())
        temperature_collection.insert_one({
            "temperature": temperature,
            "timestamp": timestamp  
        })
        print("Suhu disimpan ke MongoDB.")
        socketio.emit('new_data', {'type': 'temperature', 'value': temperature, 'timestamp': str(timestamp)})

    elif msg.topic == "controller/hited":
        hits_collection.insert_one({
            "hit": "detected",
            "timestamp": timestamp  
        })
        print("Data getaran disimpan ke MongoDB.")
        socketio.emit('new_data', {'type': 'hit', 'value': 'detected', 'timestamp': str(timestamp)})

    elif msg.topic == "controller/humidity":
        humidity = float(msg.payload.decode())
        humidity_collection.insert_one({
            "humidity": humidity,
            "timestamp": timestamp  
        })
        print("Suhu disimpan ke MongoDB.")
        socketio.emit('new_data', {'type': 'humidity', 'value': humidity, 'timestamp': str(timestamp)})

mqtt_client.on_message = on_message

def run_mqtt():
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    for topic in MQTT_TOPICS:
        mqtt_client.subscribe(topic)
    mqtt_client.loop_forever()

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/dashboard')
def dashboard():
    return render_template('dashboard.html')



@app.route('/get_hits_data')
def get_hits_data():
    hits_data = list(hits_collection.find({}, {'_id': 0})) 
    for data in hits_data:
        if 'timestamp' in data:
            utc_time = data['timestamp']
            local_time = utc_time.astimezone(local_tz)
            data['timestamp'] = local_time.strftime('%Y-%m-%d %H:%M:%S') 
    return jsonify(hits_data)

@app.route('/get_temperature_data')
def get_temperature_data():
    temperature_data = list(temperature_collection.find({}, {'_id': 0}))  
    for data in temperature_data:
        if 'timestamp' in data:
            utc_time = data['timestamp']
            local_time = utc_time.astimezone(local_tz)
            data['timestamp'] = local_time.strftime('%Y-%m-%d %H:%M:%S') 
    return jsonify(temperature_data)

@app.route('/get_humidity_data')
def get_humidity_data():
    humidity_data = list(humidity_collection.find({}, {'_id': 0}))  
    for data in humidity_data:
        if 'timestamp' in data:
            utc_time = data['timestamp']
            local_time = utc_time.astimezone(local_tz)
            data['timestamp'] = local_time.strftime('%Y-%m-%d %H:%M:%S') 
    return jsonify(humidity_data)

if __name__ == '__main__':
   
    mqtt_thread = threading.Thread(target=run_mqtt)
    mqtt_thread.daemon = True
    mqtt_thread.start()
    socketio.run(app, debug=True, host="0.0.0.0")