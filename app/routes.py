from flask import Blueprint, render_template, request, jsonify
from pythonosc.udp_client import SimpleUDPClient

bp = Blueprint('main', __name__)

# OSC client setup
client = SimpleUDPClient("127.0.0.1", 57120)

# Function to send effect selection
def select_effect(effect_name):
    if effect_name == "stop":
        client.send_message("/effect", 0)
    elif effect_name == "passthrough":
        client.send_message("/effect", 1)
    elif effect_name == "pitchShift":
        client.send_message("/effect", 2)
    elif effect_name == "reverb":
        client.send_message("/effect", 3)
    elif effect_name == "distortion":
        client.send_message("/effect", 4)

# Function to update parameters
def update_param(type, value):
    type_id = 0
    if type == "volume":
        type_id = 1
    if type == "pitch":
        type_id = 2
    if type == "reverb_mix":
        type_id = 3
    if type == "reverb_room":
        type_id = 4
    if type == "distortion":
        type_id = 5
    client.send_message("/param", [type_id, float(value)])
    print(f"Param '{type}' updated to {value}")

@bp.route('/')
def index():
    return render_template('index.html')

@bp.route('/select_effect', methods=['POST'])
def select_effect_route():
    effect_name = request.json['effect']
    select_effect(effect_name)
    return jsonify({"status": "success"})

@bp.route('/update_param', methods=['POST'])
def update_param_route():
    type = request.json['type']
    value = request.json['value']
    update_param(type, value)
    return jsonify({"status": "success"})
