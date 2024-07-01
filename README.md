TRABAJO PRACTICO PROGRAMACION Y COMUNICACION S.E.
=================================================


API en C - Interface SQL (MariaDB) - MQTT (Mosquitto)
-----------------------------------------------------
Esta API esta constituida en tres modulos:

R-api-1-server : Se encarga de "escuchar" al topico "AL_SERVER"
                 Tambien llama al modulo de mySQL (api-2-sql),
                 y luego al modulo MQTT (R-api-3-zona) que PUBLICA
                 el resultado obtenido de mySQL en el topico correspondiente
                 a la zona del lector
Este modulo arranca con el server COMO SERVICIO, y se queda ejecutando
permanentemente en el equipo servidor MQTT y mySQL.

R-api-2-sql: Recibe de "api-1" los datos de zona del lector y NUID de
             la tarjeta y hace la consulta SELECT al server mySQL en la tabla de
             autorizaciones y le devuelve el resultado de esta consulta (0 o 1).

R-api-3-zona: "api 1" le envia los datos de zona del lector
               y el resultado del SELECT. Con esto arma el
               nombre del topico de respuesta (ZONAxx, xx es la zona)
               y envia a ese topico el resultado del SELECT.

---------------------------------------------------------
Instalación de librerias MQTT - MariaDB (mySQL) en LINUX:

MQTT ( Mosquitto )
                  sudo apt-get install libssl-dev
                  sudo apt-get install libpaho-mqtt-dev

MySQL ( MariaDB)
                  sudo apt install libmariadb3 libmariadb-dev
-------------------------------------------------------------

Compilación en LINUX por CLI:

*) MQTT:
        gcc <programa.c> -o <ejecutable> -lpaho-mqtt3c

   Para el caso particular de este trabajo practico se
   compilan R-api-1-server.c  y  R-api-3-zona.c

*) MariaDB:
           gcc -o <ejecutable> <programa.c> $(mariadb_config --include --libs)

   Para el caso particular de este trabajo practico se
   compila R-api-2-sql.c





