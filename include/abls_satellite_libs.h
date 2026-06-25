#ifndef ABLS_SATELLITE_LIBS_H
#define ABLS_SATELLITE_LIBS_H

#include <glib.h>
#include <json-glib/json-glib.h>
#include <signal.h>

#include <abls-libs/abls-libs.h>

#define ABLS_SATELLITE_CONFIG_FILE "/etc/abls-habitat-agent.conf"

 struct SATELLITE
  { pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    gboolean Satellite_run;                                 /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    struct ABLS_MQTT *mqtt_local;
    struct ABLS_MQTT *mqtt_api;
    JsonNode *local_config;                                                    /* Pointeur vers la config locale du satellite */
    JsonNode *api_config;                                                         /* Pointeur vers la config API du satellite */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    JsonNode *ai_nbr_tour_par_sec;                                                                        /* Tour par seconde */
    JsonNode *IOs;
    gint nbr_tour;
    gint nbr_tour_par_sec;
    gint nbr_tour_next_update;
    gint nbr_tour_delai;
    gint telemetrie_next_update;
    void *vars;                                                               /* Pointeur vers les variables de run du module */
  };

#endif
