/**
 * **** API1_MS_SRVR (API 1 de 3) *************************
 * 
 * Este MODULO API será el encargado de RECIBIR por
        SUSCRIPCION los mensajes que VAN DESDE un LECTOR
        de zona especificada HACIA EL SERVIDOR MY_SQL
 
 *  TOPICO : "AL_BROKER"
    MENSAJE: "ZZxxxxxxxx" donde:
             ZZ es la ZONA (2 caracteres)
             xxxxxxxx es el NUID de la TARJETA 
             (8 caracteres, 4 digitos en hexa ej: a1b2c3d4) 

 *  Además, al recibir un mensaje LLAMA a la API2_SQL 
    para verificar en la base de datossi la tarjeta está 
    autorizada o no. Luego LLAMA a la API3_MP_ZONA, la
    cual comunica mediante el tópico correspondiente a la
    zona el resultado SQL de acceso de la tarjeta. 
 */


/** LIBRERIAS */
/** Librerias standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/** Librerías para fork() y execl() */
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
/** Libreria MQTT */
#include "MQTTClient.h"

/** Definiciones para Broker MQTT */
#define ADDRESS   "xx.xx.xx.xx:puerto"
#define CLIENTID  "Id_Cliente"  
#define TOPIC     "Topico_IDA_Al_Server" 
#define QOS       1   /** calidad servicio MQTT */
#define TIMEOUT   10000L    
/**   Variables GLOBALES */ 
volatile MQTTClient_deliveryToken tokenEntrega;  // MQTT
/**   Variables del lector RFID y resultado server mySQL  */
char zona[2], NUID[8];

/** FUNCIONES *********************************************/

/**  Ejecuta un proceso HIJO mediante fork() y execl() */
int ejecutaHijo(char progr[], char arg1[], char arg2[]){
    char lineaComandos[64] = "./";   /** arma parametros CLI */
    strcat(lineaComandos,progr);  
    printf("\nLLAMADA A PROCESO CON ARGUMENTOS:\n" );
    printf("<%s>   <%s>   <%s>   <%s>\n\n",lineaComandos, progr, arg1, arg2);

    pid_t child = fork();  /**  arma un proceso HIJO */
    if (child == 0) 
    {   /** en la próxima línea SOLO ENTRA LA LLAMADA AL PROCESO HIJO */
        execl( lineaComandos, progr , arg1 , arg2 , NULL );

        /**  Si no se pudo cargar y ejecutar el hijo se ejecutan las proximas sentencias */
        fprintf( stderr, "ERROR %d CARGANDO proceso %s\n", errno, strerror(errno) );
        return EXIT_FAILURE;
    } 
    else if (child > 0)
    {   /**  Aquí SOLO ENTRA EL PROCESO PADRE*/
        printf("API 1 ESPERA a que %s finalice\n", progr );
        int status, resultado;
        wait( &status );
        resultado = WEXITSTATUS(status);
        printf( "\nValor de RETORNO de %s: %d\n", progr, resultado );
        return resultado;
    }
    else { /**  Aquí solo entra el padre si no pudo crear el hijo */
        fprintf( stderr, "ERROR %d CREANDO proceso %s\n", \
                                errno, strerror(errno) );
        return EXIT_FAILURE;
    }
}

/**  MQTT - Mensaje entregado */ 
void entregado(void *contexto, MQTTClient_deliveryToken dt)
{
    printf("Msg con token: %d - Entrega Confirmada\n", dt);
    tokenEntrega = dt;
}

/** MQTT - Aviso de CONEXION PERDIDA */
void connlost(void *contexto, char *cause)
{
    printf("\nConex. PERDIDA, código: %s\n", cause);
}

/** MQTT - Mensaje RECIBIDO AL TOPICO */ 
int msgSUB(void *contexto, char *nombreTopico,\
            int topicLen, MQTTClient_message *message)
{
    printf("\n\n***NUEVO MENSAJE EN TOPICO %s ***\n", nombreTopico);
 
    /**  Pasa el mensaje recibido a un string formateado (aux) */
    /**    y luego separa los campos zona y NUID */
    char *aux;
    int i = 0;
    sprintf(aux, "%.*s\n", message->payloadlen, (char*)message->payload);
    printf("Mensaje: %s \n", aux);
    for ( i=0 ; i<2 ; i++)
        zona[i] = aux[i];   /**  copia caracter */
    zona[i] = '\0';         /**  agrega ASCII NULO al final */
    printf("ZONA >%s< \n",zona);
    for ( i=0 ; i<8 ; i++)
        NUID[i] = aux[i+2];
    NUID[i]='\0';
    printf("NUID >%s< \n\n",NUID);

    /**  LLAMAR A LA API SQL */
    int acceso, resultado;
    printf("*** CONSULTANDO en tabla aut. zona %s y NUID %s\n",\
              zona, NUID);
    acceso = ejecutaHijo( "api-2-sql" , zona , NUID);
    printf("API SQL devolvió: %d\n\n", acceso);

    /**  LLAMAR A API3_MP_ZONA */
    char topicoSUB[10]="A_ZONA";
    strcat(topicoSUB, zona);
    char *accStr;
    sprintf(accStr, "%d", acceso);
    printf("*** PUBLICANDO en tópico %s resultado %s\n",\
            topicoSUB, accStr);
    resultado = ejecutaHijo( "api-3-zona", topicoSUB, accStr);
    printf("API PUBLICACION ZONA devolvió: %d\n", resultado);

    /**  Libera el mensaje y el topico   */
    MQTTClient_freeMessage(&message);
    MQTTClient_free(nombreTopico);
    return 1;
}

/**   MAIN ****** */
int main(int argc, char* argv[])
{
    /**  1) Creacion del "objeto" cliente MQTT */
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
 
    int rc;  /**  codigo de retorno */

    if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
           MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
        printf("Fallo al crear el cliente, codigo %d\n", rc);
        rc = EXIT_FAILURE;
return rc;
    }

    /**  2) Verif. si CALLBACK (donde LLEGA EL MENSAJE) está OK */
    if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgSUB, entregado)) \
             != MQTTCLIENT_SUCCESS) {
        printf("Rutina Callback INDEFINIDA, codigo %d\n", rc);
        rc = EXIT_FAILURE;
MQTTClient_destroy(&client);
return rc;
    }
 
    /**  3) CONECTA al Broker */
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = "User_MQTT_Broker";
    conn_opts.password = "Pswd_MQTT_Broker";
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Fallo en la conexión, codigo: %d\n", rc);
MQTTClient_destroy(&client);
return rc;
    }

    /**  4) Comienza la escucha al tópico de llegada */
    system("clear"); 
    printf("SUSCRIPCION al Tópico %s\npara Cliente %s con QoS %d\n\n" ,  \
                            TOPIC, CLIENTID, QOS);
    printf("Presione x <Enter> para FINALIZAR\n\n");
    if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    {
        printf("Fallo en la suscripción, codigo: %d\n", rc);
        rc = EXIT_FAILURE;
    }
    else
    {
        int ch;
        do {
           ch = getchar();
        } while (ch != 'x' && ch != 'X');
 
        if ((rc = MQTTClient_unsubscribe(client, TOPIC)) != MQTTCLIENT_SUCCESS) {
           printf("Fallo en la DESUSCRIPCION, código: %d\n", rc);
           rc = EXIT_FAILURE;
        }
    }
 
    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS) {
        printf("Fallo en la DESCONEXION, codigo: %d\n", rc);
        rc = EXIT_FAILURE;
    }

    MQTTClient_destroy(&client);
    return rc;
}
