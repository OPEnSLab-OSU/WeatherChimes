const maxApi = require('max-api');
const { MongoClient} = require("mongodb");

maxApi.post("before connect"); 

maxApi.addHandler('connect', (Mongo_username, Mongo_password, Mongo_unique_cluster_variable, Mongo_database, device) => { 
    maxApi.post("connected");
    const uri = `mongodb+srv://${Mongo_username}:${Mongo_password}@${Mongo_unique_cluster_variable}.mongodb.net/${Mongo_database}?retryWrites=true&w=majority`;
    //const mongoclient = new MongoClient(uri).connect();
    const mongoclient = new MongoClient(uri);
    maxApi.post(Mongo_username);
    maxApi.post(Mongo_password);
    maxApi.post(Mongo_database);
    maxApi.post(device);
    //maxApi.post("bad uri");
    run(mongoclient, Mongo_database, device);

});

async function run(mongoclient, Mongo_database, device) {
    try {
        maxApi.post("In Run function");
        //const uri = `mongodb+srv://${Mongo_username}:${Mongo_password}@remotetest.cls7o.mongodb.net/${Mongo_database}?retryWrites=true&w=majority`;
        //const mongoclient = new MongoClient(uri);
        await mongoclient.connect();
        maxApi.post("Mongo Connect");
        const database = mongoclient.db(Mongo_database);
        maxApi.post(Mongo_database);
        const collection = database.collection(device);
        maxApi.post(device);
        maxApi.outlet('connected');
          
        const changeStream = collection.watch(
        [
            { $match : {"operationType" : "insert" } }
        ]
        );

        changeStream.on("change", next => {
            
            // process any change event
            console.log("received a change to the collection: \t", next.fullDocument);
            console.log(typeof next.fullDocument);
            
            var jsonObj = next.fullDocument;
            delete next.fullDocument._id;
            delete next.fullDocument.ts;
            jsonStr = JSON.stringify(jsonObj);
            maxApi.outlet(jsonStr);
            console.log(jsonStr);
        });
          
        maxApi.post("before grab");
        
        const cursor = collection.find({}, { projection: { _id:0, ts:0 }}).limit(1).sort({$natural:-1});
        cursor.forEach(function(myDoc) {
            
            data = JSON.stringify(myDoc);
            console.log(data);
            maxApi.post(data);
            maxApi.outlet(data);
            
          });

      maxApi.addHandler("grab", () => {
        maxApi.post("grab start");
        //mongoclient.startSession();
        
        maxApi.post("grabbing new");
        const cursor = collection.find({}, { projection: { _id:0, ts:0 }}).limit(1).sort({$natural:-1});
        cursor.forEach(function(myDoc) {
    
            console.log(myDoc);
            data = JSON.stringify(myDoc);
            maxApi.post(data);
            maxApi.outlet(data);
            
          });
      });
      await new Promise(resolve => {
        setTimeout(async () => {
        }, 1000);
      });
  
      
    } 
    catch{
        maxApi.post(`Error with MongoClient`)
    }
    
    finally {
      await mongoclient.close();
    }
  }