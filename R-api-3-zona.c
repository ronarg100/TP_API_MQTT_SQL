/**  ************ API3_MP_ZONA (API 3 de 3) ********************
   Este MODULO API será el encargado de publicar
        el resultado que devuelve el modulo SQL
        al lector de la zona especificada  
   
   TOPICO : "A_ZONA<zz>", dode <zz> es la ZONA del lector RFId
   MENSAJE: 0: ACCESO DENEGADO
            1: ACCESO OK
            VALORES NEGATIVOS: ERRORES VARIOS (VER API2_SQL)
*/

/**  Librerias a usar y definiciones */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>  /**  libreria de MQTT instalada */

#define ADDRESS   "xx.xx.xx.xx:pppp" /**  IP y PUERTO broker */
#define CLIENTID  "ClienteMQTT"  
#define TIMEOUT   10000L ax.

int main(int argc, char* argv[]) {
    /**  Mensaje de inicio y configuracion de variables globales */
    printf("****************************\n");
    printf("api 3 - MQTT PUBLICA MENSAJE\n"); 
    if (argc != 3) {
      printf("Uso: %s <TOPICO> <MENSAJE>\n\n\n\n", argv[0]);
      return (-1);
    }
    char *TOPICO = argv[1];     /**  "A_ZONA<zz>" (zz es la ZONA) */
    char *MENSAJE = argv[2];    /** Resultado SQL  */
    printf("Tópico: >%s< \n", TOPICO);
    printf("Mensaje: >%s< \n", MENSAJE);

    /**  Prepara variables y objetos para el cliente y la conexion al broker */
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
 
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = "TPC2";
    conn_opts.password = "paseTPC2";

    int codRet;  /**  Codigo de retorno de este modulo de la API  */
    if ((codRet = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Falla en conexion BROKER, codigo: %d\n", codRet);
        exit(EXIT_FAILURE);
    }
    printf("-------------------\n");
    printf("CONECTADO AL BROKER\n");

    /**  Envia el mensaje MQTT */
    char* payload = MENSAJE;             /**  el mensaje propiamente dicho (cadena) */
    int payloadlen = strlen(payload);    /**  la longitud de la cadena mensaje */
    int qos = 1;                         /**  Calidad de Servicio (QOS) = 1 */
    int retained = 0;                    /**  Retiene el mensaje o nó */
    MQTTClient_deliveryToken dt;
    codRet = MQTTClient_publish(client, TOPICO, payloadlen, payload, qos, retained, &dt);
    printf("**MENSAJE ENVIADO**\n");

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return codRet;
}
