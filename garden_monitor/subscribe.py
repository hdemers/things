import os
import time

from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

root_ca = os.environ['AWS_IOT_ROOT_CA_FILENAME']
private_key = os.environ['AWS_IOT_PRIVATE_KEY_FILENAME']
certificate = os.environ['AWS_IOT_CERTIFICATE_FILENAME']
client_id = os.environ['AWS_IOT_CLIENT_ID']
mqtt_host = os.environ['AWS_IOT_MQTT_HOST']
mqtt_port = os.environ['AWS_IOT_MQTT_PORT']

# For certificate based connection
mqtt = AWSIoTMQTTClient(client_id)

# For Websocket connection
# mqtt = AWSIoTMQTTClient("myClientID", useWebsocket=True)

# Configurations
# For TLS mutual authentication
mqtt.configureEndpoint(mqtt_host, mqtt_port)

# For Websocket
# myShadowClient.configureEndpoint("YOUR.ENDPOINT", 443)
mqtt.configureCredentials(root_ca, private_key, certificate)

mqtt.configureAutoReconnectBackoffTime(1, 32, 20)
mqtt.configureOfflinePublishQueueing(-1)
mqtt.configureDrainingFrequency(2)
mqtt.configureConnectDisconnectTimeout(10)
mqtt.configureMQTTOperationTimeout(5)


def callback(client, userdata, message):
    print("Received a new message: ")
    print(message.payload)
    print("from topic: ")
    print(message.topic)
    print("--------------\n\n")


mqtt.connect()
# mqtt.publish("myTopic", "myPayload", 0)
mqtt.subscribe("topic1", 1, callback)
# mqtt.unsubscribe("myTopic")
# mqtt.disconnect()

loopCount = 0
while True:
    mqtt.publish("topic1", "New Message " + str(loopCount), 1)
    time.sleep(1)
    loopCount += 1
