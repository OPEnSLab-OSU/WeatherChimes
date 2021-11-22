/**
 * Parses data between Max and MongoDB
 * 
 * @author Will Richards, Rij Dorfman
 */

// Import the max-api library and mongodb library for interfacing respectfully
const maxAPI = require('max-api');
const {MongoClient} = require('mongodb');

// Print "Before Connect" to the console
maxAPI.post("Before Connect")

// Called when the connect button is pressed on the max patch and all of the mongoDB information is passed to it
maxAPI.addHandler('connect', (MONGO_USERNAME, MONGO_PASSWORD, CLUSTER_VARIABLE, MONGO_DATABASE, DEVICE_NAME) => {
    maxAPI.post("Connection Request Received... ");
    const uri = `mongodb+srv://${MONGO_USERNAME}:${MONGO_PASSWORD}@${CLUSTER_VARIABLE}.mongodb.net/?retryWrites=true&w=majority`;
    maxAPI.post(uri);
    const mongoClient = new MongoClient(uri);

    // Print out all the MongoDB connection data
    maxAPI.post(`Username: ${MONGO_USERNAME}`);
    maxAPI.post(`Password: ${MONGO_PASSWORD}`);
    maxAPI.post(`Database: ${MONGO_DATABASE}`);
    maxAPI.post(`Device Name: ${DEVICE_NAME}`);

    // Called after we have setup the connnection to the mongo database
    connect(mongoClient, MONGO_DATABASE, DEVICE_NAME);

});


/**
 * Called after we have connected Max to our MongoDB database
 * @param {MongoClient} mongoClient 
 * @param {String} MONGO_DATABASE 
 * @param {String} DEVICE_NAME
 */
async function connect(mongoClient, MONGO_DATABASE, DEVICE_NAME){
    try{

        // Connect to the database
        maxAPI.post("Connecting to MongoDB...");
        await mongoClient.connect();
        

        // Get references to the database and collection of the specified device
        const database = mongoClient.db(MONGO_DATABASE);
        const collection = database.collection(DEVICE_NAME);
        maxAPI.post("Connected to Database!");

        maxAPI.post("Watching for changes...");

        const changeStream = collection.watch(
            [
                { $match : {"operationType" : "insert" } }
            ]
        );

        // When there is a change to the collection 
        changeStream.on("change", addedDocument => {

            // Print out the contents of the added document
            console.log("A new document was added to the collection: \n", addedDocument.fullDocument);

            jsonStr = JSON.stringify(next.fullDocument);
            maxAPI.post(`Added Document (JSON): ${jsonStr}`);
            console.log(`Added Document (JSON): ${jsonStr}`);
            maxApi.outlet(jsonStr);
        });

        maxApi.post("before grab");
        
        const cursor = collection.find({}, { projection: { _id:0, ts:0 }}).limit(1).sort({$natural:-1});
        cursor.forEach(function(myDoc) {
            
            data = JSON.stringify(myDoc);
            console.log(data);
            maxApi.post(data);
            maxApi.outlet(data);
            
          });

        // Called when we want to grab data
        maxAPI.addHandler("grab", () => {
            maxAPI.post("Retrieving Data...");

            // Get the newest document in the collection
            const cursor = collection.find({}, { projection: { _id:0, Timestamp:0 }}).limit(1).sort({$natural:-1});

            // For each part of the document log it, convert it to a string and then push it to max
            cursor.forEach(function(myDoc){
                console.log(myDoc);

                data = JSON.stringify(myDoc);
                maxAPI.post(`Sensor Data: ${data}`);
                maxAPI.outlet(data);
            })
        });

        // Set a connection timeout so we are not stuck here forever
        await new Promise(resolve => {
            setTimeout(async () => {
            }, 1000);
          });
    }catch{
        maxAPI.post(`An Error with MongoClient has Occurred`);
    }


    // In the end close the MongoDB connection
   // finally {
    //    await mongoClient.close();
    //}
}