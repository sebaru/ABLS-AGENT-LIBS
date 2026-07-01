/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/agent.c     Gestion de la couche commune à tous les agents                                                 */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * ABLS-AGENT-LIBS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-LIBS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-LIBS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #define _GNU_SOURCE
 #include <glib.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <locale.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls_agent_libs.h"

/******************************************************************************************************************************/
/* Agent_send_comm_to_master: Envoi le statut de la comm au master                                                            */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_send_comm_to_master ( struct ABLS_AGENT *agent, gboolean etat )
  { if (agent->comm_status != etat || agent->comm_next_update <= time(NULL))
     { if (agent->mqtt_local == NULL) return;                                              /* Si pas de connexion, on return; */
       MQTT_Send_WATCHDOG ( agent, "IO_COMM", (etat ? 900 : 0) );

       JsonNode *RootNode = Json_create();
       Json_add_string ( RootNode, "agent_classe",  agent->agent_classe );
       Json_add_string ( RootNode, "agent_tech_id", agent->agent_tech_id );
       Json_add_bool   ( RootNode, "io_comm",       agent->comm_status );
       Json_add_bool   ( RootNode, "mqtt_api_connected", Mqtt_is_connected ( agent->mqtt_api ) );
       Json_add_bool   ( RootNode, "mqtt_local_connected", Mqtt_is_connected ( agent->mqtt_local ) );
       MQTT_Send_to_API ( RootNode, "HEARTBEAT" );
       Json_unref ( RootNode );

       agent->comm_next_update = time(NULL) + 60;                                                       /* Toutes les minutes */
       agent->comm_status = etat;
     }
  }
/******************************************************************************************************************************/
/* Agent_loop: S'occupe de la telemetrie, de la comm périodique, de la vitesse de rotation                                    */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_loop ( struct ABLS_AGENT *agent )
  { Agent_send_comm_to_master ( agent, agent->comm_status );

/********************************************************* tour par secondes **************************************************/
    if ( agent->nbr_tour_next_update <= time(NULL))                                                 /* Toutes les 1 secondes */
     { agent->nbr_tour_par_sec = agent->nbr_tour;
       agent->nbr_tour = 0;
       if(agent->nbr_tour_par_sec > 50) agent->nbr_tour_delai += 50;
       else if(agent->nbr_tour_delai>0) agent->nbr_tour_delai -= 50;
       agent->nbr_tour_next_update = time(NULL) + 1;
     } else agent->nbr_tour++;
    usleep(agent->nbr_tour_delai);

/********************************************************* Toutes les minutes *************************************************/
    if (agent->telemetrie_next_update <= time(NULL))                                                   /* Toutes les minutes */
     { MQTT_Send_AI ( agent, agent->ai_nbr_tour_par_sec, agent->nbr_tour_par_sec, TRUE );
       agent->telemetrie_next_update = time(NULL) + 60;
     }
  }
/******************************************************************************************************************************/
/* Agent_init: appelé par chaque agent, lors de son démarrage                                                                 */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: pointeur vers la structure initialisée                                                                             */
/******************************************************************************************************************************/
 struct ABLS_AGENT *Agent_init ( gchar *agent_classe, gint sizeof_vars )
  { gchar chaine[128];
    Info_init ( "Agent", "agent_tech_id", LOG_INFO );
    Info( __func__, agent_classe, NULL, LOG_INFO, "Agent of class '%s' is starting", agent_classe );
    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    struct ABLS_AGENT *agent = g_try_malloc0 ( sizeof(struct ABLS_AGENT) );
    if (!agent)
     { Info( __func__, agent_classe, NULL, LOG_ERR, "Memory error trying to malloc struct ABLS_AGENT" );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }

/*------------------------------------------------- Chargement de la config par défaut ---------------------------------------*/
    agent->local_config = Json_create();
    if (!agent->local_config)
     { Info( __func__, agent_classe, NULL, LOG_ERR, "Memory error trying to malloc local config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }
    Json_add_int ( agent->local_config, "log_level", LOG_INFO );

/*------------------------------------------------- apply file and ENV config ------------------------------------------------*/
    Json_read_config ( ABLS_AGENT_CONFIG_FILE, agent->local_config );                       /* Apply file then ENV parameters */

/*------------------------------------------------- Config control -----------------------------------------------------------*/
    if (!Json_has_member( agent->local_config, "agent_tech_id" ))
     { Info( __func__, agent_classe, NULL, LOG_ERR, "There is no 'agent_tech_id', in config, exiting." );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "api_url" ))
     { Info( __func__, agent_classe, NULL, LOG_ERR, "There is no 'api_url', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "domain_uuid" ))
     { Info( __func__, agent_classe, NULL, LOG_ERR, "There is no 'domain_uuid', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "domain_secret" ))
     { Info( __func__, agent_classe, NULL, LOG_ERR, "There is no 'domain_secret', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    agent->agent_tech_id = Json_get_string ( agent->local_config, "agent_tech_id" );
    agent->agent_classe  = Json_get_string ( agent->local_config, "agent_classe" );
    agent->api_url       = Json_get_string ( agent->local_config, "api_url" );
    agent->domain_uuid   = Json_get_string ( agent->local_config, "domain_uuid" );
    agent->domain_secret = Json_get_string ( agent->local_config, "domain_secret" );
    Json_to_log ( agent->agent_classe, agent->agent_tech_id, agent->local_config );                           /* Print config */

/*------------------------------------------------- Init ---------------------------------------------------------------------*/
    g_snprintf( chaine, sizeof(chaine), "W-%s", agent->agent_tech_id );                            /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    mkdir ( agent->agent_classe, S_IRUSR | S_IWUSR | S_IXUSR );

    if (sizeof_vars)
     { agent->vars = g_try_malloc0 ( sizeof_vars );
       if (!agent->vars)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Memory error for vars, exiting." );
          Agent_end ( agent );                                   /* Pas besoin de return : Agent_end fait un exit */
        }
     }
/*----------------------------------------- Connexion API pour récupérer la config distante ----------------------------------*/
    agent->api_config = Http_Get_from_global_API ( "/run/agent/config", "agent_tech_id=%s", agent->agent_tech_id );
    if (agent->api_config && Json_get_int ( agent->api_config, "http_code" ) == 200)
     { agent->Agent_run = TRUE;
     }
    else
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "GET_CONFIG from API Failed. Unloading." );
       Agent_end ( agent );
     }


/*------------------------------------------------------ Ecoute du MQTT ------------------------------------------------------*/
    agent->mqtt_api = Mqtt_init( agent->agent_classe, agent->agent_tech_id, agent->agent_tech_id,
                                 Json_get_bool ( agent->api_config, "mqtt_over_ssl" ),
                                 Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                 Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                 agent->agent_tech_id, Json_get_string ( agent->api_config, "mqtt_password" ),
                                 Json_get_string ( agent->api_config, "mqtt_hostname" ),
                                 Json_get_int ( agent->api_config, "mqtt_port" ),
                                 Json_get_int ( agent->api_config, "mqtt_qos" )
                               );
    Mqtt_subscribe ( agent->mqtt_api, "%s/agent/%s/%s", agent->domain_uuid, agent->agent_classe, agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_api, "%s/agents/%s", agent->domain_uuid, agent->agent_classe );
    Mqtt_subscribe ( agent->mqtt_api, "%s/agents", agent->domain_uuid );
    Mqtt_start ( agent->mqtt_api );

    agent->mqtt_local = Mqtt_init( agent->agent_classe, agent->agent_tech_id, agent->agent_tech_id,
                                   Json_get_bool ( agent->local_config, "mqtt_over_ssl" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                   agent->agent_tech_id, Json_get_string ( agent->local_config, "mqtt_password" ),
                                   Json_get_string ( agent->local_config, "mqtt_hostname" ),
                                   Json_get_int ( agent->local_config, "mqtt_port" ),
                                   Json_get_int ( agent->local_config, "mqtt_qos" )
                                 );
    Mqtt_subscribe ( agent->mqtt_local, "agent/%s/%s", agent->agent_classe, agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_local, "agents/%s", agent->agent_classe );
    Mqtt_subscribe ( agent->mqtt_local, "agents" );
    Mqtt_start ( agent->mqtt_local );

/* ----------------------------------------- Création du plugin D.L.S de l'agent -------------------------------------------- */
    JsonNode *PluginNode = Json_create();
    if (PluginNode)
     { Json_add_string ( PluginNode, "tech_id", agent->agent_tech_id );
       Json_add_int    ( PluginNode, "syn_id", 2 );                             /* Raccroché au synoptique Système (pas HOME) */
       gchar *name = Json_get_string ( agent->api_config, "description" );
       Json_add_string ( PluginNode, "name", name );
       Json_add_string ( PluginNode, "shortname", name );
       gchar package[128];
       g_snprintf ( package, sizeof(package), "Agent_%s", agent->agent_classe );
       Json_add_string ( PluginNode, "package", package );
       if (Dls_auto_create_plugin( PluginNode ) == FALSE)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "DLS Create ERROR for '%s'", name ); }
       Json_unref ( PluginNode );
     }
    else Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Memory error while creating DLS pluginNode" );

/* ------------------------------------------------ Création des IOs -------------------------------------------------------- */
    agent->IOs = Json_create();
    Json_add_array ( agent->IOs, "IOs" );

    agent->ai_nbr_tour_par_sec = Mnemo_create_AI ( agent, "THREAD_TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", ARCHIVE_5_MIN );
    Mnemo_create_WATCHDOG ( agent, "IO_COMM", "Statut de la communication" );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is UP" );
    return ( agent );
  }
/******************************************************************************************************************************/
/* Agent_end: appelé par chaque agent, lors de son arret                                                                      */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_end ( struct ABLS_AGENT *agent )
  { Agent_send_comm_to_master ( agent, FALSE );
    Mqtt_stop ( agent->mqtt_local );
    Mqtt_stop ( agent->mqtt_api );
    if (agent->vars) { g_free(agent->vars); }
    Json_unref ( agent->IOs );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is DOWN" );
    sleep(1);
    exit(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
