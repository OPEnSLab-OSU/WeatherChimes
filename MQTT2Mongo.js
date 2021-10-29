/**
 * Recieves and parses MQTT data from different sensing "Nodes" and stores them into a MongoDB database
 * 
 * @author Will Richards
 */

// Import MQTT and MongoDB libraries
var mqtt = require('mqtt');
const { MongoClient } = require("mongodb");

// Mongo username and password
const Mongo_username = "";
const Mongo_password = "";

const MQTT_username = "";
const MQTT_password = "";
const MQTT_broker = "mqtt://192.168.43.49:1883";

// Set the mongoURI to any ip on port 27017
const mongoURI = "mongodb://0.0.0.0:27017/"

// Create a new MongoClient pointing to the previously specified server location
var mongoClient = new MongoClient(mongoURI);

// Connect to the MQTT broker running on our machine, should probably change to 0.0.0.0 but don't know if that would work as phone/hotspot died so I cant check
var client = mqtt.connect(MQTT_broker);

// Parse out the JSON and send it to the mongo database
function parse_and_send_to_database(jsonInput, topic){

    console.log(topic);

    // Split the topic to represent the database and collection name, eg. SitkaNet/Node1 {SitkaNet is the database, Node1 is the collection}
    dbName = topic.split('/')[0];
    collectionName = topic.split('/')[1];

    // Parse the JSON input
    const jsonObj = JSON.parse(jsonInput);

    // Set the database and collection to write to
    const db = mongoClient.db(dbName);
    const collection = db.collection(collectionName);

    // Get sensor objects
    contents = jsonObj[0]["contents"];

    // Set containg all data recieved from the current poll
    var set = {};

    // Check if an RTC timestamp is present if so add it to the set
    if(jsonObj[0].hasOwnProperty("timestamp"))
        set["Timestamp"] = jsonObj[0]["timestamp"];

    // Loop over componets/sensors in the JSON
    for (let j in contents)
    {
        // Name of the sensor moudle
        moduleName = contents[j]["module"];

        // Set containing data key with number value
        var dataSet = {};

        // Add each data piece to a set with a the coresponding name as the key
        for(let dataOutput in contents[j]["data"])
        {
            // Pair the actual data with the matchin key
            dataSet[dataOutput] = contents[j]["data"][dataOutput];
        }
    
        // Add the data to the overall sensor name, sort the data keys alphabetically
        set[moduleName] = sort_set(dataSet);
    }

    console.log(set);
     // Sort the sensors alphabetically and add them into the collection
     const insertResult = collection.insertMany([sort_set(set)]);
}

/**
 * Connect to the mongo database to store data
 */
async function connect_to_mongo(){
    await mongoClient.connect();
    console.log("Connected to MongoDB Server!")
}

/**
 * Sort a set alphabetically and return the sorted set
 */
function sort_set(set){

    // Key names in the set
    keyNames = []

    // New sorted set
    sortedSet = {};

    // Get the key names of the set, ignoring the timestamp key name as that should always be first and add them to the key names array
    for(let module in set){
        if(module != "Timestamp"){
            keyNames.push(module);
        }
    }

    // Sort the keynames array
    keyNames.sort();

    if(set.hasOwnProperty("Timestamp")){
        keyNames.unshift("Timestamp");
    }
    

    // Construct the new set in the proper sorted order
    keyNames.forEach(element => {
        sortedSet[element] = set[element];
    });   

    // Return the sorted set
    return sortedSet;
}

/**
 * Connect to the local MQTT Broker
 */
function connect_to_broker(){
    // Subscribe to any topic formated as such <Something>/<SomethingElse>
    client.subscribe("+/+", function(err){
        
        if(err){
            console.log("An error occurred: "+ err.message);
        }
        else{
            console.log("Connected to broker!");
        }
    });

    // When a message is published to that topic send it to be parsed and put into the database
    client.on('message', function (topic, message){
        parse_and_send_to_database(message, topic);
    });
}

// Finally begin the entire script by connecting to the mongo database waiting until that is complete then connecting to the MQTT broker and beginning data proccessing
connect_to_mongo()
    .then(() => connect_to_broker())
    .catch(console.error);