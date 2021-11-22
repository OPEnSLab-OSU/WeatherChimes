import pymongo

MONGO_URI = "mongodb+srv://admin:adminpass@remotetest.cls7o.mongodb.net/"

client = pymongo.MongoClient(MONGO_URI)

collection = client.WeatherChimes.Chime3

with open("output.csv", "w") as f:
    data = list(collection.find())
    f.write("PacketNumber,Moisture\n")
    for i in range(len(data)):
        packetNumber = data[i]["Packet"]["Number"]
        moisture = data[i]["GS3"]["moisture"]
        f.write(f"${packetNumber},${moisture}\n")