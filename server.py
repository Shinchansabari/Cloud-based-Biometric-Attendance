from flask import Flask, request, jsonify
from pymongo import MongoClient
from apscheduler.schedulers.background import BackgroundScheduler
import os
from datetime import datetime, timezone
import logging
from functools import wraps
from flask import send_from_directory


app = Flask(__name__)
# Initialize the scheduler
scheduler = BackgroundScheduler()

port = 5500
host = '192.168.137.223'

# Replace with your MongoDB connection details
mongo_url = os.getenv('MONGODB_URL', 'mongodb://shinchan:noharafamily@localhost:27017/?authSource=admin')
db_name = os.getenv('DB_NAME', 'Device_DB')

client = MongoClient(mongo_url)
db = client[db_name]
print('Connected to MongoDB')

# Variable to store fetched data
fetched_data = None
upload_data = None

# Configure logging
logging.basicConfig(level=logging.DEBUG)

@app.route('/')
def home():
    return "Hello, Flask!"

@app.route('/favicon.ico')
def favicon():
    return send_from_directory(os.path.join(app.root_path, 'static'), 'favicon.ico', mimetype='image/vnd.microsoft.icon')

def calculate_Attendance_Summary():
    """
    Function to fetch data from MongoDB every hour.
    This function is scheduled to run periodically.
    """
    global fetched_data
    try:
        # Query the MongoDB collection to get all documents
        collection = db['Attendance_ForStaff']
        documents = list(collection.find())  # Fetch all documents

        # Initialize a dictionary to store class counts
        summary = {}

        for document in documents:

            for batch_key, batch_data in document.items():
                if not batch_key.startswith("Batch_"):
                    continue  # Skip non-batch fields


                for subject_code, subject_data in batch_data.items():
                    class_count = 0

                    for date_key, attendance_list in subject_data.items():
                        if date_key.endswith('//1'):
                            class_count += 1
                        elif date_key.endswith('//2'):
                            class_count += 2

                    for date_key, attendance_list in subject_data.items():
                        for roll_number in attendance_list:
                            # Initialize class count for each roll number

                            roll_number_list = roll_number.split('/')
                            print(roll_number_list)
                            roll_number_id = roll_number_list[0]
                            print(roll_number_id)

                            if roll_number_id not in summary:
                                summary[roll_number_id] = {}
                            if subject_code not in summary[roll_number_id]:
                                summary[roll_number_id][subject_code] = [0,0,0]

                            if roll_number_list[1] == str(1):
                                summary[roll_number_id][subject_code][0] += 1
                            elif roll_number_list[1] == str(2):
                                summary[roll_number_id][subject_code][0] += 2

                            summary[roll_number_id][subject_code][1] = class_count
                            summary[roll_number_id][subject_code][2] = ( ((summary[roll_number_id][subject_code][0]) / class_count) * (100) )
                            

        # Update fetched_data with the calculated class counts
        fetched_data = summary
        print("Fetched and processed data:", fetched_data)

        # Upload the class count data to MongoDB
        upload_Attendance_Summary(summary)

    except Exception as e:
        print(f"Error fetching data from MongoDB: {e}")




def upload_Attendance_Summary(summary):
    """
    Completely replace the existing data in MongoDB with the new summary data.
    """
    try:
        # Define the collection where you want to upload the summary
        collection = db['Attendance_Summary']

        # Define the document structure
        document = {
            'summary': summary,
            'timestamp': datetime.now(timezone.utc) # Add a timestamp to track when the summary was uploaded
        }

        # Replace the document
        result = collection.replace_one(
            {},  # Use a constant ID to identify the document to replace
            document,
            upsert=True  # Create a new document if it doesn't exist
        )

        # Log the result
        logging.debug(f"MongoDB replace result: {result.raw_result}")

        return jsonify({"status": "Summary data replaced successfully"}), 200

    except Exception as e:
        logging.error(f"Error occurred while replacing summary data: {e}")
        return jsonify({"error": "Failed to replace summary data"}), 500
    

@app.route('/fetch_Batch_Details', methods=['POST'])
def fetch_Batch_Details():
    try:
        # Get JSON data from the request body
        data = request.json
        room = data.get('roomNum')

        if not room:
            return jsonify({"status": "error", "message": "No roomNum provided"}), 400

        # Define the query to find documents where the specified initial key exists
        query = {room: {"$exists": True}}

        # Replace 'Batch_Details' with the actual collection name
        collection = db['Batch_Details']
        document = collection.find_one(query)

        if document and room in document:
            batch = document[room]
            # Return only the value associated with the roomNum
            return jsonify({"status": "success", "data": batch}), 200
        else:
            return jsonify({"status": "error", "message": "No documents found with the provided key"}), 404

    except Exception as e:
        logging.error(f"Error occurred while fetching data by key: {e}")
        return jsonify({"error": "Something broke!"}), 500





@app.route('/fetch_Timetable', methods=['POST'])
def fetch_Timetable():
    try:

        data = request.json
        batch = data.get('batch')
        day = data.get('day')

        # Search for the batch in the collection
        collection = db["Timetable"]
        document = collection.find_one({batch: {"$exists": True}})
        if document:
            # Check if day exists
            day_data = document[batch].get(day)
            if day_data:
                return jsonify({"status": "success", "data": day_data}), 200
            else:
                return jsonify({"status": "error", "message": "Day not found"}), 404
        else:
            return jsonify({"status": "error", "message": "Batch not found"}), 404
    except Exception as e:
        logging.error(f"Error occurred: {e}")
        return jsonify({"error": "Something broke!"}), 500


@app.route('/upload_Attendance_ForStudents', methods=['POST'])
def upload_Attendance_ForStudents():
    return upload_Attendance('Attendance_ForStudents')

@app.route('/upload_Attendance_ForStaff', methods=['POST'])
def upload_Attendance_ForStaff():
    return upload_Attendance('Attendance_ForStaff')



@app.route('/upload_Fingerprint', methods=['POST'])
def upload_Fingerprint():
    try:
        # Extract the batch name dynamically from the incoming JSON data
        incoming_data = request.json
        if not incoming_data:
            return jsonify({"error": "No data received"}), 400

        # Get the batch name (it should be the only key in the incoming_data)
        batch_name = next(iter(incoming_data), None)
        if not batch_name:
            return jsonify({"error": "Batch name not found in the data"}), 400
        
        new_data = incoming_data.get(batch_name, {})
        logging.debug(f"Received new data for {batch_name}: {new_data}")

        collection = db['Students_Enrolled']
        existing_doc = collection.find_one({})
        logging.debug(f"Existing document: {existing_doc}")

        existing_data = existing_doc.get('Batch_1', {}) if existing_doc else {}
        conflicting_keys = [key for key in new_data if key in existing_data and existing_data[key] != new_data[key]]
        logging.debug(f"Conflicting keys: {conflicting_keys}")

        if conflicting_keys:
            return jsonify({"error": f"Conflicting keys: {', '.join(conflicting_keys)}"}), 400

        updated_data = {**existing_data, **new_data}
        logging.debug(f"Updated data: {updated_data}")

        result = collection.update_one(
            {},
            {'$set': {'Batch_1': updated_data}},
            upsert=True
        )
        logging.debug(f"MongoDB update result: {result.raw_result}")

        return jsonify({"status": "Data updated successfully"}), 200
    except Exception as e:
        logging.error(f"Error occurred: {e}")
        return jsonify({"error": "Something broke!"}), 500

@app.errorhandler(Exception)
def handle_exception(e):
    print(f"Error: {e}")
    return jsonify({"error": "Something broke!"}), 500



def merge_nested_json(existing_data, new_data):
    for key, value in new_data.items():
        if key in existing_data:
            if isinstance(value, dict) and isinstance(existing_data[key], dict):
                # Recursively merge nested dictionaries
                merge_nested_json(existing_data[key], value)
            elif isinstance(value, list) and isinstance(existing_data[key], list):
                # Avoid duplicate entries in the list
                for item in value:
                    if item not in existing_data[key]:
                        existing_data[key].append(item)
            else:
                existing_data[key] = value
        else:
            existing_data[key] = value



def upload_Attendance(collection_name):
    try:
        incoming_data = request.json
        if not incoming_data:
            return jsonify({"error": "No data received"}), 400

        # Extracting the batch name dynamically
        batch_name = next(iter(incoming_data), None)
        if not batch_name:
            return jsonify({"error": "No batch name found in the data"}), 400
        
        new_data = incoming_data.get(batch_name, {})
        if not new_data:
            return jsonify({"error": f"No data found for {batch_name}"}), 400
        
        logging.debug(f"Received new data for {batch_name}: {new_data}")

        collection = db[collection_name]
        existing_doc = collection.find_one({})
        logging.debug(f"Existing document: {existing_doc}")

        existing_data = existing_doc.get(batch_name, {}) if existing_doc else {}

        # Merge new data with existing data
        merge_nested_json(existing_data, new_data)
        logging.debug(f"Updated data for {batch_name}: {existing_data}")

        result = collection.update_one(
            {},
            {'$set': {batch_name: existing_data}},
            upsert=True
        )
        logging.debug(f"MongoDB update result: {result.raw_result}")

        return jsonify({"status": "Data updated successfully"}), 200
    except Exception as e:
        logging.error(f"Error occurred: {e}")
        return jsonify({"error": "Something broke!"}), 500



# Schedule the job to run every hour
scheduler.add_job(func=calculate_Attendance_Summary, trigger="interval", minutes=1)
# Start the scheduler
scheduler.start()



if __name__ == '__main__':
    try:
        app.run(host=host, port=port, debug=True)
    except (KeyboardInterrupt, SystemExit):
        # Shut down the scheduler gracefully on exit
        scheduler.shutdown()
