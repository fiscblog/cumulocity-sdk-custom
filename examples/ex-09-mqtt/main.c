#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"

#define ADDRESS     "bbv-ch.cumulocity.com"
#define CLIENTID    "d6361eb0-5634-48ff-a92d-db90c52f29d5" // randomly chosen

void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received operation %s\n", payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = "t573330530/christian.fischer@bbv.ch";
    conn_opts.password = "";

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    //create device: Name, Revision
    publish(client, "s/us", "100,C MQTT,c8y_MQTTDevice");
    //add Configuration, Shell and LogfileRequest for device
    publish(client, "s/us", "114,c8y_Command,c8y_Configuration,c8y_LogfileRequest");
    //set hardware information
    publish(client, "s/us", "110,S123456789,MQTT test model,Rev0.1");
    //set required update interval to 5min
    publish(client, "s/us", "117,5");
    //set available logfiles
    publish(client, "s/us", "118,ntcagent,dmesg,logread");
    // use SR Template GUI instead
    //publish(client, "s/ut", "10,999,POST,MEASUREMENT,,c8y_MyMeasurement,,c8y_MyMeasurement.M.value,NUMBER,");

    //listen for operation
    MQTTClient_subscribe(client, "s/ds", 0);

    for (;;) {
        // send signal strength meas in db
        publish(client, "s/us", "210,-87");
        
        //send temperature measurement
        publish(client, "s/us", "211,25");

        //send battery level meas in %
        publish(client, "s/us", "212,95");

        publish(client, "s/uc/149", "999,,34");

        // create event with altitude & latitude & longitude
        publish(client, "s/us", "401,51.227741,6.773456");

        // update the position on the device
        publish(client, "s/us", "402,51.227741,6.773456");

        sleep(3);
    }
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}
