/**
 * *************** API2_SQL (API 2 de 3) ********************* 
   Esta API toma como argumentos 
        la ZONA y EL NUID de la tarjeta RFId
        e impacta en la base de datos del servidor MariaDB.
 * 
 * 
 * Devuelve los siguientes valores:
   0 Si no encontró ninguna autorización
   1 ó mayor: encontró registros autorizados (deberia ser 1)
   (-1) Si no pudo insertar el registro en la
        tabla de accesos (log de movimientos) 
   (-10) Si no ejecutó (no se le pasaron los param. x cmdline)
   (-11) No conecto al server my_SQL
   (-12) Error en sentencias SQL
 * 
 * 
 * Para compilar, hay que hacerlo con: 
   gcc -o API_SQL  API_SQL.c $(mariadb_config --include --libs) 
 * 
 * 
 * Es necesario tener las librerias de mariadb o mySQL del
   "c Connector" instaladas 
 * 
 * 
 */


/**  LIBRERIAS */ 
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>    // my_SQL o Maria db C_CONNECTOR

/** MAIN */ 

int main(int argc, char *argv[]) {
    /**  Variables pasadas por argumentos en linea de comandos */
    if (argc != 3) {
       printf("ERROR. Uso: %s <ZONA> <NUID>\n", argv[0]);
       return (-10);
    }
    char *zona = argv[1];
    char *NUID = argv[2];
    printf("*** api 2 (MQTT a MySQL) ******\n");
    printf("La ZONA es: %s\n", zona);
    printf("El NUID es: %s\n", NUID);

   /**  Variables a usar por la librería mySQL y QUERYS */  
   MYSQL *conn;
   MYSQL_RES *res;
   MYSQL_ROW row;
  
   char query[255];    /**  El "string" que se va a usar para SQL */
   int resultado = 0;  /**  resultado del SELECT (y de main) */

   /**  Parametros de conexion al server mySQL */
   char *server   = "xx.xx.xx.xx";
   char *user     = "UsuarioBroker";
   char *password = "ClaveBroker";
   char *database = "NombreBaseDeDatos";

   /** Connectar al server y la base de datos */ 
   conn = mysql_init(NULL);
   if (!mysql_real_connect(conn, server,
         user, password, database, 0, NULL, 0)) {
      fprintf(stderr, "Err. en cnx: %s\n", mysql_error(conn));
      exit((-11));
   }

   /** Conectado; "Preguntar" si la tarjeta esta autorizada */
   sprintf(query, "SELECT * FROM t_autorizaciones \
                   WHERE ta_codZona ='%s' \
                   AND ta_NUID = '%s' ;" , zona, NUID);
   if (mysql_query(conn, query)) {
      fprintf(stderr, "Error en SELECT TARJETA: %s\n", mysql_error(conn));
      mysql_close(conn);
      exit((-12));
   }
   res = mysql_use_result(conn);
   while ((row = mysql_fetch_row(res)) != NULL)
         resultado++;
   printf("Resultado autorizaciones: %d \n", resultado);

   /**  Insertar registro en tabla accesos (autorizado o no) */
   sprintf(query, "INSERT INTO t_accesos \
                         (tac_idTarjeta , tac_codZona , \
                          tac_accesoOK, tac_fechaYhora) \
                   VALUES ('%s', '%s', '%d', NOW());", \
                   NUID, zona, resultado);
   if (mysql_query(conn, query)) {
      fprintf(stderr, "Error en INSERT: %s\n", mysql_error(conn));
      mysql_close(conn);
      exit((-12));
   }
   printf("Resultado del INSERT OK !\n");

   /**  Liberar el res y cerrar la conexion */
   mysql_free_result(res);
   mysql_close(conn);

return resultado;
}
