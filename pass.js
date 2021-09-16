const { MongoClient, ObjectId } = require("mongodb");
var mqtt = require('mqtt');


var Mongo_username = "admin";
var Mongo_password = "adminpass";
var Mongo_database = "Chimes";
const HiveMQ_username = "WeatherChimes";
const HiveMQ_password = "B1gchime";


//uri used to access OPEnS' MongodB
const uri =
`mongodb+srv://${Mongo_username}:${Mongo_password}@remotetest.cls7o.mongodb.net/${Mongo_database}?retryWrites=true&w=majority`;

//`mongodb+srv://${Mongo_username}:${Mongo_password}>@remotetest.cls7o.mongodb.net/${Mongo_database}?retryWrites=true&w=majority`;

//const mongoclient = new MongoClient(uri);

var options = {
    host: '8acfd6649bcd41f888ba886f128790ae.s1.eu.hivemq.cloud', //OPEnS' HiveMQ broker
    port: 8883, //secure server port
    protocol: 'mqtts',
    username: HiveMQ_username, //HiveMQ_username
    password:  HiveMQ_password//HiveMQ_password
}

//initialize the MQTT client
var mqttclient = mqtt.connect(options);

//setup the callbacks
mqttclient.on('connect', function () {
    console.log('Connected');
});

mqttclient.on('error', function (error) {
  console.log("error \n");
  console.log(error);
});

mqttclient.on('message', function (topic, message) {
  // Called each time a message is received
  console.log('Received message:', topic, message.toString());
  var split_topic = topic.split("/")
  console.log("Device is " + split_topic[1]);
  var device = split_topic[1]
  //if topic starts with Errors, dont send
  database(message, device);
});

// subscribe to topic 'my/test/topic'
//# in MQTT is a multi level subsribe to everything after the defined topics
//"Chimes/+/data" will also work 
mqttclient.subscribe('Chime/#');

function database(message, collection){
    console.log("enter database function");
    MongoClient.connect(uri, function(err, db) {
      console.log("send");
      if (err) throw err;
      var dbo = db.db(Mongo_database);

      // get a timestamp
      var id = ObjectId();
      var time = id.getTimestamp();
      var ts = new Date().getTime();
      message_string = message.toString();
      message_string = message_string.substr(0, message_string.length-1);
      message_string = message_string + `, \"ts\": ${ts} }`;
      let json = JSON.parse(message_string);
      console.log(typeof json);
      console.log(json);
      var myobj = json;
    //dbo.collection(collection)  <-- put this in line below
      dbo.collection(collection).insertOne(myobj, function(err, result) {
        if (err) throw err;
        console.log("1 document inserted");
        //console.log(result.insertedId);
        id = result.insertedId;
        id = id.toString();
        id = id.replace("new ", "");
        console.log(id)
        db.close();
      });
      
    });

}

