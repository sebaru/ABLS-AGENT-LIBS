/******************************************************************************************************************************/
/* ABLS-SATELLITE-LIBS/satellite.c     Gestion de la couche commune à tous les satellites                                     */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * satellite.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * ABLS-SATELLITE-LIBS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-SATELLITE-LIBS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-SATELLITE-LIBS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #define _GNU_SOURCE
 #include <sys/resource.h>
 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>
 #include <locale.h>

 #include <sys/wait.h>
 #include <fcntl.h>
 #include <errno.h>
 #include <dlfcn.h>
 #include <link.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls_satellite_libs.h"

/******************************************************************************************************************************/
/* Satellite_send_comm_to_master: Envoi le statut de la comm au master                                                        */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Satellite_send_comm_to_master ( struct SATELLITE *module, gboolean etat )
  { if (module->mqtt_local == NULL) return;                                                /* Si pas de connexion, on return; */
    if (module->comm_status != etat || module->comm_next_update <= time(NULL))
     { MQTT_Send_WATCHDOG ( module, "IO_COMM", (etat ? 900 : 0) );

       JsonNode *RootNode = Json_node_create();
       Json_add_string ( RootNode, "thread_classe",  Json_get_string ( module->config, "thread_classe"  ) );
       Json_add_string ( RootNode, "thread_tech_id", Json_get_string ( module->config, "thread_tech_id" ) );
       Json_add_bool   ( RootNode, "io_comm",        module->comm_status );
       Json_add_bool   ( RootNode, "mqtt_connected", (etat ? module->MQTT_connected : FALSE) );
       MQTT_Send_to_API ( RootNode, "HEARTBEAT" );
       Json_unref ( RootNode );
      
       module->comm_next_update = time(NULL) + 60;                                                      /* Toutes les minutes */
       module->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Satellite_loop: S'occupe de la telemetrie, de la comm périodique, de la vitesse de rotation                                */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Satellite_loop ( struct SATELLITE *module )
  { Satellite_send_comm_to_master ( module, module->comm_status );

/********************************************************* tour par secondes **************************************************/
    if ( module->nbr_tour_next_update <= time(NULL))                                                 /* Toutes les 1 secondes */
     { module->nbr_tour_par_sec = module->nbr_tour;
       module->nbr_tour = 0;
       if(module->nbr_tour_par_sec > 50) module->nbr_tour_delai += 50;
       else if(module->nbr_tour_delai>0) module->nbr_tour_delai -= 50;
       module->nbr_tour_next_update = time(NULL) + 1;
     } else module->nbr_tour++;
    usleep(module->nbr_tour_delai);

/********************************************************* Toutes les minutes *************************************************/
    if (module->telemetrie_next_update <= time(NULL))                                                   /* Toutes les minutes */
     { MQTT_Send_AI ( module, module->ai_nbr_tour_par_sec, module->nbr_tour_par_sec, TRUE );
       module->telemetrie_next_update = time(NULL) + 60;
     }
  }
/******************************************************************************************************************************/
/* Satellite_init: appelé par chaque satellite, lors de son démarrage                                                               */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Satellite_init ( gchar *thread_classe, gint sizeof_vars )
  { gchar chaine[128];
    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    struct SATELLITE *satellite = g_try_malloc0 ( sizeof(struct SATELLITE) );
    if (!satellite)
     { Info( __func__, thread_classe, NULL, LOG_ERR, "Memory error trying to malloc struct SATELLITE" );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    satellite->local_config = Json_create();
    if (!satellite->local_config)
     { Info( __func__, thread_classe, NULL, LOG_ERR, "Memory error trying to malloc local config, exiting." );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    Json_read_from_file ( ABLS_SATELLITE_CONFIG_FILE, satellite->local_config );

    if (!Json_has_member( satellite->local_config, "thread_tech_id" ))
     { Info( __func__, thread_classe, NULL, LOG_ERR, "There is no 'thread_tech_id', in config, exiting." );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    if (!Json_has_member( satellite->local_config, "api_url" ))
     { Info( __func__, thread_classe, NULL, LOG_ERR, "There is no 'api_url', in config, exiting." );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    if (!Json_has_member( satellite->local_config, "domain_uuid" ))
     { Info( __func__, thread_classe, NULL, LOG_ERR, "There is no 'domain_uuid', in config, exiting." );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    if (!Json_has_member( satellite->local_config, "domain_secret" ))
     { Info( __func__, thread_classe, NULL, LOG_ERR, "There is no 'domain_secret', in config, exiting." );
       Satellite_end ( satellite );                                      /* Pas besoin de return : Satellite_end fait un exit */
     }

    gchar *thread_tech_id = Json_get_string ( satellite->local_config, "thread_tech_id" );
    Json_to_log ( thread_classe, thread_tech_id, satellite->local_config );

    g_snprintf( chaine, sizeof(chaine), "W-%s", thread_tech_id );                                  /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    mkdir ( thread_classe, S_IRUSR | S_IWUSR | S_IXUSR );

    if (sizeof_vars)
     { satellite->vars = g_try_malloc0 ( sizeof_vars );
       if (!satellite->vars)
        { Info( __func__, thread_classe, thread_tech_id, LOG_ERR, "Memory error for vars, exiting." );
          Satellite_end ( satellite );                                   /* Pas besoin de return : Satellite_end fait un exit */
        }
     }
/* ----------------------------------------------------- Connexion API ------------------------------------------------------ */
 
/* ----------------------------------------------------- Ecoute du MQTT ----------------------------------------------------- */
    satellite->mqtt_api = 
    satellite->mqtt_local = Mqtt_init( thread_classe, thread_tech_id, thread_tech_id,
                                       Json_get_bool ( satellite->local_config, "mqtt_over_ssl" ),
                                       Json_get_string ( satellite->local_config, "mqtt_ca_file" ),
                                       Json_get_string ( satellite->local_config, "mqtt_ca_path" ),
                                       thread_tech_id,
                                       Json_get_string ( satellite->local_config, "mqtt_password" ) );
    Mqtt_subscribe ( satellite->mqtt_local, sss"sssWATCHDOG" );
    Mqtt_Start ( satellite->mqtt_local );

/* ------------------------------------------- Création du plugin dans l'api ------------------------------------------------ */
    JsonNode *RootNode = Json_create();
    if (RootNode)
     { Json_add_string ( RootNode, "tech_id", thread_tech_id );
       Json_add_string ( RootNode, "thread_classe", thread_classe ); 
       Json_add_int    ( RootNode, "syn_id", 2 ); 
       gchar *name = Json_get_string ( module->config, "description" );
       Json_add_string ( RootNode, "name", name );
       Json_add_string ( RootNode, "shortname", name );
       gchar package[128];
       g_snprintf ( package, sizeof(package), "Thread_%s", thread_classe );
       Json_add_string ( RootNode, "package", package );
       if (Dls_auto_create_plugin( RootNode ) == FALSE)
        { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: DLS Create ERROR (%s)\n", thread_tech_id, name ); }
       Json_node_unref ( RootNode );
     }
    else Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Memory error while creating DLS RootNode", thread_tech_id );

/* ------------------------------------------------ Création des IOs --------------------------------------------------------- */
    module->IOs = Json_create();
    Json_add_array ( module->IOs, "IOs" );

    module->ai_nbr_tour_par_sec = Mnemo_create_thread_AI ( module, "THREAD_TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", ARCHIVE_5_MIN );
    Mnemo_create_thread_WATCHDOG ( module, "IO_COMM", "Statut de la communication" );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Satellite '%s' is UP", thread_tech_id );
  }
/******************************************************************************************************************************/
/* Satellite_end: appelé par chaque satellite, lors de son arret                                                                    */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Satellite_end ( struct THREAD *module )
  { Satellite_send_comm_to_master ( module, FALSE );
    mosquitto_disconnect( module->MQTT_session );
    mosquitto_loop_stop( module->MQTT_session, FALSE );
    mosquitto_destroy( module->MQTT_session );
    g_slist_foreach ( module->MQTT_messages, (GFunc) Json_node_unref, NULL );
    g_slist_free    ( module->MQTT_messages );   module->MQTT_messages = NULL;
    if (module->vars) { g_free(module->vars);  module->vars   = NULL; }
    Json_unref ( module->IOs );           module->IOs    = NULL;
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s' is DOWN", Json_get_string ( module->config, "thread_tech_id") );
    sleep(1);                       /* le temps d'un appel libsoup a Thread_ws_on_master_connected si Operation was cancelled */
    pthread_exit(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
