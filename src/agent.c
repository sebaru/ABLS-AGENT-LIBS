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
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <sys/resource.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <locale.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Agent_send_comm_to_master: Envoi le statut de la comm au master                                                            */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_send_comm_to_master ( struct ABLS_AGENT *agent, gboolean etat )
  { if (agent->comm_status != etat || agent->comm_next_update <= time(NULL))
     { if (agent->mqtt_local == NULL) return;                                              /* Si pas de connexion, on return; */
       Mqtt_Send_WATCHDOG ( agent, "IO_COMM", (etat ? 900 : 0) );

       JsonNode *RootNode = Json_create();
       Json_add_string ( RootNode, "agent_classe",  agent->agent_classe );
       Json_add_string ( RootNode, "agent_tech_id", agent->agent_tech_id );
       Json_add_bool   ( RootNode, "io_comm",       agent->comm_status );
       Json_add_bool   ( RootNode, "mqtt_api_connected", Mqtt_is_connected ( agent->mqtt_api ) );
       Json_add_bool   ( RootNode, "mqtt_local_connected", Mqtt_is_connected ( agent->mqtt_local ) );
       Mqtt_send_message ( agent->mqtt_api, RootNode, TRUE, "HEARTBEAT" );
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
    if ( agent->nbr_tour_next_update <= time(NULL))                                                  /* Toutes les 1 secondes */
     { agent->nbr_tour_par_sec = agent->nbr_tour;
       agent->nbr_tour = 0;
       if(agent->nbr_tour_par_sec > 50) agent->nbr_tour_delai += 50;
       else if(agent->nbr_tour_delai>0) agent->nbr_tour_delai -= 50;
       agent->nbr_tour_next_update = time(NULL) + 1;
     } else agent->nbr_tour++;
    usleep(agent->nbr_tour_delai);

/********************************************************* Toutes les minutes *************************************************/
    if (agent->telemetrie_next_update <= time(NULL))                                                    /* Toutes les minutes */
     { struct rusage conso;
       getrusage ( RUSAGE_SELF, &conso );
       Mqtt_Send_AI ( agent, agent->ai_max_rss, (gdouble)conso.ru_maxrss, TRUE );
       Mqtt_Send_AI ( agent, agent->ai_nbr_tour_par_sec, agent->nbr_tour_par_sec, TRUE );
       Mqtt_Send_AI ( agent, agent->ai_log_par_min, 1.0*Info_reset_nbr_log(), TRUE );
       agent->telemetrie_next_update = time(NULL) + 60;
     }
/******************************************************** Ecoute API **********************************************************/
    JsonNode *api_message = Mqtt_get_message ( agent->mqtt_api );
    if (api_message)
     { if ( Mqtt_topic_is ( api_message, 4, "+", "STOP", "AGENT", agent->agent_tech_id ) )
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is stopping by API request" );
          agent->Agent_run = AGENT_NEED_TO_STOP;
        }
       else if ( Mqtt_topic_is ( api_message, 4, "+", "RESTART", "AGENT", agent->agent_tech_id ) )
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is restarting by API request" );
          agent->Agent_run = AGENT_NEED_TO_RESTART;
        }
       else if ( Mqtt_topic_is ( api_message, 4, "+", "UPGRADE", "AGENT", agent->agent_tech_id )
              || Mqtt_topic_is ( api_message, 4, "+", "UPGRADE", "CLASS", agent->agent_classe ) )
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is upgrading by API request" );
          gint new_pid = fork();
          if (new_pid<0)
           { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_WARNING, "Fils: UPGRADE: Fork Error" ); }
          else if (!new_pid)
           { gchar chaine[256];
             g_snprintf ( chaine, sizeof(chaine), "sudo dnf upgrade abls-agent-%s", agent->agent_classe );
             system(chaine);
             Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_WARNING, "Fils: UPGRADE: done. Restarting." );
             agent->Agent_run = AGENT_NEED_TO_RESTART;                                                  /* Stop old processes */
             exit(0);
           }
        }
       else if ( Mqtt_topic_is ( api_message, 4, "+", "LOG", "AGENT", agent->agent_tech_id ) )
        { if ( Json_has_member ( api_message, "log_level" ) )
           { gint log_level = Json_get_int ( api_message, "log_level" );
             Info_change_log_level ( log_level );
           }
          else if ( Json_has_member ( api_message, "debug" ) )
           { gchar *facility = Json_get_string ( api_message, "debug" );
             if (facility) Info_debug_facility ( agent->agent_tech_id, facility );
           }
          else if ( Json_has_member ( api_message, "undebug" ) )
           { gchar *facility = Json_get_string ( api_message, "undebug" );
             if (facility) Info_undebug_facility ( agent->agent_tech_id, facility );
           }
        }
       Json_unref ( api_message );
     }
  }
/******************************************************************************************************************************/
/* Agent_init: appelé par chaque agent, lors de son démarrage                                                                 */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: pointeur vers la structure initialisée                                                                             */
/******************************************************************************************************************************/
 struct ABLS_AGENT *Agent_init ( gchar *entete, gchar *agent_classe, gchar *agent_version, gint sizeof_vars, int argc, char **argv )
  { gchar chaine[128];
    Info_init ( entete, "agent_tech_id", LOG_INFO );
    Info( __func__, agent_classe, NULL, LOG_INFO, "Agent of class '%s' (version %s) is starting with agent_libs version %s",
          agent_classe, agent_version, ABLS_AGENT_LIBS_VERSION );
    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    struct ABLS_AGENT *agent = g_try_malloc0 ( sizeof(struct ABLS_AGENT) );
    if (!agent)
     { Info( __func__, agent_classe, NULL, LOG_ALERT, "Memory error trying to malloc struct ABLS_AGENT" );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }
    Agent_enable_signals ( agent );
/*------------------------------------------------- Chargement de la config par défaut ---------------------------------------*/
    agent->local_config = Json_create();
    if (!agent->local_config)
     { Info( __func__, agent_classe, NULL, LOG_ALERT, "Memory error trying to malloc local config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }
    Json_add_int ( agent->local_config, "log_level", LOG_INFO );

/*---------------------------------------- apply ENV, FILE and CLI parameters ------------------------------------------------*/
    Config_apply_ENV  ( agent->local_config );                                                        /* Apply ENV parameters */
    Config_apply_FILE ( agent->local_config, ABLS_AGENT_CONFIG_FILE );                               /* Apply file parameters */
    Config_add_parameter ( "domain-uuid",   "UUID",    "UUID du domaine",   CONFIG_STRING );
    Config_add_parameter ( "domain-secret", "SECRET",  "Secret du domaine", CONFIG_STRING );
    Config_add_parameter ( "server-uuid",   "UUID",    "UUID du serveur",   CONFIG_STRING );
    Config_add_parameter ( "agent-tech-id", "TECH_ID", "Agent tech_id",     CONFIG_STRING );
    Config_add_parameter ( "api-url",       "URL",     "URL de l'API",      CONFIG_STRING );
    Config_apply_ARGV ( agent->local_config, argc, argv );                                           /* Apply ARGV parameters */

/*------------------------------------------------- Config control -----------------------------------------------------------*/
    if (!Json_has_member( agent->local_config, "agent_tech_id" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'agent_tech_id', in config, exiting." );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "api_url" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'api_url', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "server_uuid" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'server_uuid', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "domain_uuid" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'domain_uuid', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    if (!Json_has_member( agent->local_config, "domain_secret" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'domain_secret', in config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }

    agent->argc          = argc;
    agent->argv          = argv;
    agent->agent_classe  = agent_classe;
    agent->agent_tech_id = Json_get_string ( agent->local_config, "agent_tech_id" );
    agent->api_url       = Json_get_string ( agent->local_config, "api_url" );
    agent->server_uuid   = Json_get_string ( agent->local_config, "server_uuid" );
    agent->domain_uuid   = Json_get_string ( agent->local_config, "domain_uuid" );
    agent->domain_secret = Json_get_string ( agent->local_config, "domain_secret" );
    Json_to_log ( "local_config", agent->agent_tech_id, agent->local_config );                                /* Print config */

/*------------------------------------------------- Init ---------------------------------------------------------------------*/
    g_snprintf( chaine, sizeof(chaine), "W-%s", agent->agent_tech_id );                            /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

#warning need Drop_priv
/*    mkdir ( agent->agent_classe, S_IRUSR | S_IWUSR | S_IXUSR );*/

    if (sizeof_vars)
     { agent->vars = g_try_malloc0 ( sizeof_vars );
       if (!agent->vars)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory error for vars, exiting." );
          Agent_end ( agent );                                   /* Pas besoin de return : Agent_end fait un exit */
        }
     }
/*----------------------------------------- Connexion API pour récupérer la config distante ----------------------------------*/
    JsonNode *RootNode = Json_create();
    if (RootNode)
     { Json_add_string ( RootNode, "agent_classe",   agent->agent_classe );
       Json_add_string ( RootNode, "agent_tech_id",  agent->agent_tech_id );
       Json_add_string ( RootNode, "version",        agent_version );
       Json_add_int    ( RootNode, "start_time",     time(NULL) );
       agent->api_config = Http_Post_to_global_API ( agent, "/run/agent/config", RootNode );
       Json_unref ( RootNode );
     }
    else
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory error for POST_CONFIG, exiting." );
       Agent_end ( agent );                                   /* Pas besoin de return : Agent_end fait un exit */
     }

    if (agent->api_config && Json_get_int ( agent->api_config, "http_code" ) == 200)
     { Info_change_log_level ( Json_get_int ( agent->api_config, "log_level" ) );
       agent->Agent_run = TRUE;
     }
    else
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_CRIT, "POST_CONFIG from API Failed. Unloading." );
       Agent_end ( agent );
     }

    if (Json_has_member ( agent->api_config, "enable" ) && Json_get_bool ( agent->api_config, "enable" ) == FALSE)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_CRIT, "Agent disabled in API config. Unloading." );
       Agent_end ( agent );
     }

    if (Json_has_member ( agent->api_config, "log_facilities" ))
     { Info_set_facilities ( agent->agent_tech_id, agent->api_config, "log_facilities" ); }

    Json_to_log ( "api_config", agent->agent_tech_id, agent->api_config );                                    /* Print config */

/*------------------------------------------------------ Ecoute du MQTT ------------------------------------------------------*/
    gchar mqtt_username[64];
    g_snprintf ( mqtt_username, sizeof(mqtt_username), "%s-agent", agent->domain_uuid );
    agent->mqtt_api = Mqtt_init( "mqtt_api", agent->agent_tech_id, agent->agent_tech_id,
                                 Json_get_bool ( agent->api_config, "mqtt_over_ssl" ),
                                 Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                 Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                 mqtt_username, Json_get_string ( agent->api_config, "mqtt_password" ),
                                 Json_get_string ( agent->api_config, "mqtt_hostname" ),
                                 Json_get_int ( agent->api_config, "mqtt_port" ),
                                 Json_get_int ( agent->api_config, "mqtt_qos" )
                               );
    Mqtt_subscribe ( agent->mqtt_api, "%s/UPGRADE/AGENT/%s", agent->domain_uuid, agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_api, "%s/UPGRADE/CLASS/%s", agent->domain_uuid, agent->agent_classe );
    Mqtt_subscribe ( agent->mqtt_api, "%s/STOP/AGENT/%s",    agent->domain_uuid, agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_api, "%s/TEST/AGENT/%s",    agent->domain_uuid, agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_api, "%s/LOG/AGENT/%s",     agent->domain_uuid, agent->agent_tech_id );
    Mqtt_last_will ( agent->mqtt_api, "{ \"status\": \"dead\" }", "%s/STATUS/AGENT/%s", agent->domain_uuid, agent->agent_tech_id );
    Mqtt_start ( agent->mqtt_api );

    agent->mqtt_local = Mqtt_init( "mqtt_local", agent->agent_tech_id, agent->agent_tech_id,
                                   Json_get_bool ( agent->local_config, "mqtt_over_ssl" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                   NULL, NULL, /* username/password */
                                   Json_get_string ( agent->api_config, "master_hostname" ),
                                   1883,
                                   1 /* "mqtt_qos */
                                 );
    Mqtt_subscribe ( agent->mqtt_local, "SET_AO/%s/#", agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_local, "SET_DO/%s/#", agent->agent_tech_id );
    Mqtt_start ( agent->mqtt_local );

/* ----------------------------------------- Création du plugin D.L.S de l'agent -------------------------------------------- */
  if (Dls_create_agent_plugin( agent ) == FALSE)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "DLS Create ERROR for '%s'", agent->agent_tech_id ); }

/* ------------------------------------------------ Création des IOs -------------------------------------------------------- */
    agent->IOs = Json_create();
    Json_add_array ( agent->IOs, "IOs" );

/* ------------------------------------------------ Création des IOs -------------------------------------------------------- */
    agent->IOs = Json_create();
    Json_add_array ( agent->IOs, "IOs" );

    agent->ai_nbr_tour_par_sec = Mnemo_create_AI ( agent, "TOUR_PAR_SEC", "Nombre de tour par seconde", "t/s", AGENT_ARCHIVE_5_MIN );
    agent->ai_max_rss          = Mnemo_create_AI ( agent, "MAX_RSS", "Maximum RSS", "kB", AGENT_ARCHIVE_5_MIN );
    agent->ai_log_par_min      = Mnemo_create_AI ( agent, "LOG_PAR_MIN", "Logs par minute", "logs/min", AGENT_ARCHIVE_1_MIN );

    Mnemo_create_WATCHDOG ( agent, "IO_COMM", "Statut de la communication" );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is UP" );
    return ( agent );
  }
/******************************************************************************************************************************/
/* Agent_stop: appelé par chaque agent, lors de son arret                                                                     */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Agent_stop ( struct ABLS_AGENT *agent )
  { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO, "Agent is stopping" );
    Agent_disable_signals();
    Agent_send_comm_to_master ( agent, FALSE );
    Mqtt_stop ( agent->mqtt_api );
    Mqtt_stop ( agent->mqtt_local );
    if (agent->vars) { g_free(agent->vars); }
    Json_unref ( agent->IOs );
  }
/******************************************************************************************************************************/
/* Agent_end: appelé par chaque agent, lors de son arret (public)                                                             */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant, ne revient pas.                                                                                             */
/******************************************************************************************************************************/
 void Agent_end ( struct ABLS_AGENT *agent )
  { if (agent->Agent_run == AGENT_NEED_TO_RESTART) { Agent_restart ( agent ); }       /* ne revient pas, pas besoin de return */
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is DOWN" );
    sleep(1);
    g_free(agent);
    exit(0);
  }
/******************************************************************************************************************************/
/* Agent_restart: appelé pour restarter le meme agent                                                                         */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant, ne revient pas                                                                                              */
/******************************************************************************************************************************/
 void Agent_restart ( struct ABLS_AGENT *agent )
  { Agent_stop ( agent );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is Restarting in 5 seconds." );
    gchar **argv = agent->argv;
    sleep(5);
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is DOWN" );
    g_free(agent);
    execvpe ( argv[0], argv, environ );                                                                      /* Restart agent */
    exit(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
