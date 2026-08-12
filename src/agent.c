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
 #include <stdarg.h>
 #include <grp.h>
 #include <pwd.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Agent_send_comm_to_master: Envoi le statut de la comm au master                                                            */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_send_comm_to_master ( struct ABLS_AGENT *agent, gboolean etat )
  { if (agent->comm_status != etat || agent->comm_next_update <= time(NULL))
     { Mqtt_Send_WATCHDOG ( agent, "IO_COMM", (etat ? 900 : 0) );
       agent->comm_status = etat;
       agent->comm_next_update = time(NULL) + 60;                                                       /* Toutes les minutes */
     }
  }
/******************************************************************************************************************************/
/* Agent_set_status: Publie un status texte de l'agent vers l'API via MQTT                                                    */
/* Entrée: La structure afférente et une chaine formatée variadique                                                           */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_set_status ( struct ABLS_AGENT *agent, gchar *format, ... )
  { gchar status[256];
    va_list ap;

    if (!agent || !agent->mqtt_api || !format) return;

    va_start ( ap, format );
    g_vsnprintf ( status, sizeof(status), format, ap );
    va_end ( ap );

    JsonNode *RootNode = Json_create();
    if (!RootNode) return;
    Json_add_string ( RootNode, "status", status );
    Agent_send_mqtt_api_message ( agent, RootNode, TRUE, "AGENT/%s/STATUS", agent->agent_tech_id );
    Json_unref ( RootNode );
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "%s", status );
  }
/******************************************************************************************************************************/
/* Agent_loop: S'occupe de la telemetrie, de la comm périodique, de la vitesse de rotation                                    */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_loop ( struct ABLS_AGENT *agent )
  { static guint  tps_nbr_tour = 0;
    static time_t tps_next_update = 0;                                       /* Délai de calcul du nombre de tour par seconde */
    static guint  tps_delai = 1000;                  /* délai inhérent a l'atteinte de la cible du nombre de tour par seconde */

    Agent_send_comm_to_master ( agent, agent->comm_status );

    guint now = agent->Top;
/********************************************************* tour par secondes **************************************************/
    if ( tps_next_update <= now )                                                                    /* Toutes les 1 secondes */
     { agent->tps_value = tps_nbr_tour;
       tps_nbr_tour = 0;
       if(agent->tps_value > agent->tps_consigne)
        { tps_delai += 50; } else if(tps_delai>0) { tps_delai -= 50; }                        /* Ajustemet du délai d'attente */
       tps_next_update = now + 10;
     } else tps_nbr_tour++;
    usleep(tps_delai);

/********************************************************* Toutes les minutes *************************************************/
    if (agent->telemetrie_next_update == 0 || agent->telemetrie_next_update <= now )             /* Toutes les minutes + Init */
     { agent->telemetrie_next_update = now + 600;
       struct rusage conso;
       getrusage ( RUSAGE_SELF, &conso );
       Mqtt_Send_AI ( agent, agent->ai_max_rss, (gdouble)conso.ru_maxrss, TRUE );
       Mqtt_Send_AI ( agent, agent->ai_nbr_tour_par_sec, 1.0*agent->tps_value, TRUE );
       Mqtt_Send_AI ( agent, agent->ai_log_par_min, 1.0*Info_reset_nbr_log(), TRUE );
       JsonNode *RootNode = Json_create();
       Json_add_bool ( RootNode, "io_comm", agent->comm_status );
       Json_add_bool ( RootNode, "mqtt_local_connected", Mqtt_is_connected ( agent->mqtt_local ) );
       Agent_send_mqtt_api_message ( agent, RootNode, TRUE, "AGENT/%s/HEARTBEAT", agent->agent_tech_id );
       Json_unref ( RootNode );
     }
  }
/******************************************************************************************************************************/
/* Agent_is_ready: appelé au demarrage lorsque l'agent est pret                                                               */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Agent_is_ready ( struct ABLS_AGENT *agent )
  { Mqtt_start ( agent->mqtt_local );
    Mqtt_start ( agent->mqtt_api );
    Agent_set_status ( agent, "Agent is UP" );
  }
/******************************************************************************************************************************/
/* Agent_init: appelé par chaque agent, lors de son démarrage                                                                 */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: pointeur vers la structure initialisée                                                                             */
/******************************************************************************************************************************/
 struct ABLS_AGENT *Agent_init ( gchar *entete, gchar *agent_classe, gchar *agent_version, gint sizeof_vars, int argc, char **argv )
  { gchar chaine[128];
    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */

    Info_init ( entete, "agent_tech_id", LOG_INFO );
    Info( __func__, agent_classe, NULL, LOG_INFO, "Agent of class '%s' (version %s) is starting with ABLS_AGENT_LIBS_VERSION=%s",
          agent_classe, agent_version, ABLS_AGENT_LIBS_VERSION );
    Info( __func__, agent_classe, NULL, LOG_INFO, "User='%s', Group='%s'",
          getpwuid(getuid())->pw_name, getgrgid(getgid())->gr_name );
    Info( __func__, agent_classe, NULL, LOG_INFO, "Using directory '%s'", g_get_current_dir() );

    struct ABLS_AGENT *agent = g_try_malloc0 ( sizeof(struct ABLS_AGENT) );
    if (!agent)
     { Info( __func__, agent_classe, NULL, LOG_ALERT, "Memory error trying to malloc struct ABLS_AGENT" );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }
    agent->argc          = argc;
    agent->argv          = argv;
    agent->agent_classe  = agent_classe;

    Agent_enable_signals ( agent );
    Http_Init ( agent );

/*----------------------------------------------- Check de l'OS sous jacent --------------------------------------------------*/
    gchar *path_dnf = g_find_program_in_path ( "dnf" );
    agent->is_dnf = (path_dnf != NULL);
    g_free ( path_dnf );

    gchar *path_apt = g_find_program_in_path ( "apt" );
    agent->is_apt = (path_apt != NULL);
    g_free ( path_apt );

/*------------------------------------------------- Chargement de la config par défaut ---------------------------------------*/
    agent->local_config = Json_create();
    if (!agent->local_config)
     { Info( __func__, agent_classe, NULL, LOG_ALERT, "Memory error trying to malloc local config, exiting." );
       Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
     }
    Json_add_int  ( agent->local_config, "log_level", LOG_INFO );                     /* Mise en place des valeurs par défaut */
    Json_add_bool ( agent->local_config, "dry_run", FALSE );
    Json_add_int  ( agent->local_config, "tps", 50 );                                       /* 50 tour par seconde par défaut */

/*---------------------------------------- apply ENV, FILE and CLI parameters ------------------------------------------------*/
    Config_apply_ENV  ( agent->local_config );                                                        /* Apply ENV parameters */
    Config_apply_FILE ( agent->local_config, ABLS_AGENT_CONFIG_FILE );                               /* Apply file parameters */
    Config_add_parameter ( "domain-uuid",   "UUID",    "UUID du domaine",   CONFIG_STRING );
    Config_add_parameter ( "domain-secret", "SECRET",  "Secret du domaine", CONFIG_STRING );
    Config_add_parameter ( "server-uuid",   "UUID",    "UUID du serveur",   CONFIG_STRING );
    Config_add_parameter ( "agent-tech-id", "TECH_ID", "Agent tech_id",     CONFIG_STRING );
    Config_add_parameter ( "api-url",       "URL",     "URL de l'API",      CONFIG_STRING );
    Config_add_parameter ( "tps",           "TPS",     "Tour par seconde",  CONFIG_INT );
    Config_add_parameter ( "dry-run",       NULL,      "Do not really send Inputs or outputs", CONFIG_FLAG );
    Config_add_parameter ( "standalone",    NULL,      "Standalone mode, API disabled", CONFIG_FLAG );
    Config_add_parameter ( "master_hostname", "HOSTNAME", "Master hostname in standalone mode", CONFIG_STRING );
    Config_add_parameter ( "save",          NULL,      "Save local configuration to default config file", CONFIG_FLAG );
    Config_apply_ARGV ( agent->local_config, argc, argv );                                           /* Apply ARGV parameters */

/*------------------------------------------------- Config control -----------------------------------------------------------*/
    if (!Json_has_member( agent->local_config, "agent_tech_id" ))
     { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'agent_tech_id', in config, exiting." );
       Agent_end ( agent );                                      /* Pas besoin de return : Agent_end fait un exit */
     }

    agent->standalone = Json_get_bool ( agent->local_config, "standalone" );
    if ( agent->standalone == FALSE )
     { if (!Json_has_member( agent->local_config, "api_url" ))
        { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'api_url', in config, exiting." );
         Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
        }

       if (!Json_has_member( agent->local_config, "server_uuid" ))
        { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'server_uuid', creating one." );
          gchar server_uuid[37];  /* UUID is 36 characters + null terminator */
          UUID_New ( (gchar *)&server_uuid );
          Json_add_string ( agent->local_config, "server_uuid", server_uuid );
        }

       if (!Json_has_member( agent->local_config, "domain_uuid" ))
        { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'domain_uuid', in config, exiting." );
          Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
        }

       if (!Json_has_member( agent->local_config, "domain_secret" ))
        { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'domain_secret', in config, exiting." );
          Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
        }
      }

    if ( agent->standalone == TRUE )
     { if (!Json_has_member( agent->local_config, "master_hostname" ))
        { Info( __func__, agent_classe, NULL, LOG_CRIT, "There is no 'master_hostname', in config, exiting." );
          Agent_end ( agent );                                                  /* Pas besoin de return : Agent_end fait un exit */
        }
     }
/*--------------------------------------------------- Sauvegarde de la conf --------------------------------------------------*/
    if (Json_get_bool ( agent->local_config, "save" ))
     { Json_remove ( agent->local_config, "save" );

       if (!Json_write_to_file ( ABLS_AGENT_CONFIG_FILE, agent->local_config ))
        { Info( __func__, agent_classe, NULL, LOG_ERR, "Unable to save local config to '%s'", ABLS_AGENT_CONFIG_FILE ); }
       else
        { Info( __func__, agent_classe, NULL, LOG_NOTICE, "Local config saved to '%s'", ABLS_AGENT_CONFIG_FILE ); }
     }

    agent->agent_tech_id = Json_get_string ( agent->local_config, "agent_tech_id" );
    agent->api_url       = Json_get_string ( agent->local_config, "api_url" );
    agent->server_uuid   = Json_get_string ( agent->local_config, "server_uuid" );
    agent->domain_uuid   = Json_get_string ( agent->local_config, "domain_uuid" );
    agent->domain_secret = Json_get_string ( agent->local_config, "domain_secret" );
    agent->dry_run       = Json_get_bool   ( agent->local_config, "dry_run" );
    agent->tps_consigne  = Json_get_int    ( agent->local_config, "tps" );
    Json_to_log ( "local_config", agent->agent_tech_id, agent->local_config );                                /* Print config */

    if (agent->dry_run) Info( __func__, agent_classe, agent->agent_tech_id, LOG_NOTICE, "Dry-run mode enabled." );

/*------------------------------------------------- Init ---------------------------------------------------------------------*/
    g_snprintf( chaine, sizeof(chaine), "W-%s", agent->agent_tech_id );                            /* Positionne le nom noyau */
    gchar *upper_name = g_ascii_strup ( chaine, -1 );
    prctl(PR_SET_NAME, upper_name, 0, 0, 0 );
    g_free(upper_name);

    if (sizeof_vars)
     { agent->vars = g_try_malloc0 ( sizeof_vars );
       if (!agent->vars)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory error for vars, exiting." );
          Agent_end ( agent );                                   /* Pas besoin de return : Agent_end fait un exit */
        }
     }
/*----------------------------------------- Connexion API pour récupérer la config distante ----------------------------------*/
    if ( agent->standalone == FALSE )
     { JsonNode *RootNode = Json_create();
       if (!RootNode)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory error for POST_CONFIG, exiting." );
          Agent_end ( agent );                                   /* Pas besoin de return : Agent_end fait un exit */
        }
       Json_add_string ( RootNode, "agent_classe",   agent->agent_classe );
       Json_add_string ( RootNode, "agent_tech_id",  agent->agent_tech_id );
       Json_add_string ( RootNode, "version",        agent_version );
       Json_add_int    ( RootNode, "start_time",     time(NULL) );
       agent->api_config = Http_Post_to_global_API ( agent, "/run/agent/config", RootNode );
       Json_unref ( RootNode );

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
     }
    Json_to_log ( "api_config", agent->agent_tech_id, agent->api_config );                                    /* Print config */

/*------------------------------------------------------ Ecoute du MQTT ------------------------------------------------------*/
    gchar mqtt_username[64];
    if (agent->standalone == FALSE)
     { g_snprintf ( mqtt_username, sizeof(mqtt_username), "%s-agent", agent->domain_uuid );
       agent->mqtt_api = Mqtt_init( "mqtt_api", agent->agent_tech_id, agent->agent_tech_id,
                                    Json_get_bool ( agent->api_config, "mqtt_over_ssl" ),
                                    Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                    Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                    mqtt_username, Json_get_string ( agent->api_config, "mqtt_password" ),
                                    Json_get_string ( agent->api_config, "mqtt_hostname" ),
                                    Json_get_int ( agent->api_config, "mqtt_port" ),
                                    Json_get_int ( agent->api_config, "mqtt_qos" )
                                  );
       Mqtt_subscribe ( agent->mqtt_api, "%s/AGENT/%s/TEST", agent->domain_uuid, agent->agent_tech_id );
       Mqtt_subscribe ( agent->mqtt_api, "%s/AGENT/%s/UPGRADE", agent->domain_uuid, agent->agent_tech_id );
       Mqtt_subscribe ( agent->mqtt_api, "%s/AGENT/%s/RESTART", agent->domain_uuid, agent->agent_tech_id );
       Mqtt_subscribe ( agent->mqtt_api, "%s/AGENT/%s/STOP", agent->domain_uuid, agent->agent_tech_id );
       Mqtt_subscribe ( agent->mqtt_api, "%s/AGENT/%s/LOG",  agent->domain_uuid, agent->agent_tech_id );
       Mqtt_last_will ( agent->mqtt_api, "{ \"status\": \"dead\" }", "%s/AGENT/%s/STATUS", agent->domain_uuid, agent->agent_tech_id );
     }

    agent->mqtt_local = Mqtt_init( "mqtt_local", agent->agent_tech_id, agent->agent_tech_id,
                                   Json_get_bool ( agent->local_config, "mqtt_over_ssl" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_file" ),
                                   Json_get_string ( agent->local_config, "mqtt_ca_path" ),
                                   NULL, NULL, /* username/password */
                                   (agent->standalone ? Json_get_string ( agent->local_config, "master_hostname" )
                                                      : Json_get_string ( agent->api_config,   "master_hostname" ) ),
                                   1883,
                                   1 /* "mqtt_qos */
                                 );
    Mqtt_subscribe ( agent->mqtt_local, "SET_AO/%s/#", agent->agent_tech_id );
    Mqtt_subscribe ( agent->mqtt_local, "SET_DO/%s/#", agent->agent_tech_id );

/* ----------------------------------------- Création du plugin D.L.S de l'agent -------------------------------------------- */
  if ( agent->standalone == FALSE && Dls_create_agent_plugin( agent ) == FALSE )
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
    return ( agent );
  }
/******************************************************************************************************************************/
/* Agent_stop: appelé par chaque agent, lors de son arret                                                                     */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Agent_stop ( struct ABLS_AGENT *agent )
  { Agent_disable_signals();
    Agent_send_comm_to_master ( agent, FALSE );
    Mqtt_stop ( agent->mqtt_api );
    Mqtt_stop ( agent->mqtt_local );
    if (agent->vars) { g_free(agent->vars); }
    Http_End ( agent );
    Json_unref ( agent->IOs );
  }
/******************************************************************************************************************************/
/* Agent_end: appelé par chaque agent, lors de son arret (public)                                                             */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant, ne revient pas.                                                                                             */
/******************************************************************************************************************************/
 void Agent_end ( struct ABLS_AGENT *agent )
  { if (agent->Agent_run == AGENT_NEED_TO_RESTART) { Agent_restart ( agent ); }       /* ne revient pas, pas besoin de return */
    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is stopping." );
    Agent_set_status ( agent, "Agent is stopped" );
    sleep(1);
    Agent_stop ( agent );
    g_free(agent);
    exit(0);
  }
/******************************************************************************************************************************/
/* Agent_restart: appelé pour restarter le meme agent                                                                         */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: néant, ne revient pas                                                                                              */
/******************************************************************************************************************************/
 void Agent_restart ( struct ABLS_AGENT *agent )
  { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_NOTICE, "Agent is restarting." );
    Agent_set_status ( agent, "Agent is restarting" );
    sleep(1);
    Agent_stop ( agent );
    gchar **argv = agent->argv;
    g_free(agent);
    execvpe ( argv[0], argv, environ );                                                                      /* Restart agent */
    exit(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
