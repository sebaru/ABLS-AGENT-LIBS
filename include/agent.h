/******************************************************************************************************************************/
/* include/agent.h     Déclaration de la structure et des fonctions Agent — abls-agent-libs                                   */
/* Projet Abls-Habitat                               Gestion d'habitat                                       03.07.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
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

#ifndef _ABLS_AGENT_LIBS_AGENT_H_
 #define _ABLS_AGENT_LIBS_AGENT_H_

 #include <signal.h>

 #define ABLS_AGENT_CONFIG_FILE "/etc/abls-agent.conf"

 enum { AGENT_ARCHIVE_NONE,
        AGENT_ARCHIVE_5_SEC,
        AGENT_ARCHIVE_1_MIN,
        AGENT_ARCHIVE_5_MIN,
        AGENT_ARCHIVE_10_MIN,
        AGENT_ARCHIVE_30_MIN,
        AGENT_ARCHIVE_1_HEURE,
        AGENT_ARCHIVE_6_HEURE,
        AGENT_ARCHIVE_1_JOUR,
        NBR_AGENT_ARCHIVE
      };

 enum { AGENT_IS_STOPPED,
        AGENT_IS_RUNNING,
        AGENT_NEED_TO_STOP,
        AGENT_NEED_TO_RESTART,
        NBR_AGENT_STATUS,
      };

 struct ABLS_AGENT
  { gboolean Agent_run;                                     /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gint argc;                                                        /* Report des argc, argv pour permettre l'Agent_Restart */
    gchar **argv;
    struct ABLS_MQTT *mqtt_local;
    struct ABLS_MQTT *mqtt_api;
    JsonNode *local_config;                                                      /* Pointeur vers la config locale de l'agent */
    JsonNode *api_config;                                                           /* Pointeur vers la config API de l'agent */
    gchar *agent_tech_id;                                                                 /* Identifiant technique de l'agent */
    gchar *agent_classe;                                                                                 /* Classe de l'agent */
    gchar *server_uuid;                                                                                    /* UUID du serveur */
    gchar *domain_uuid;                                                                                    /* UUID du domaine */
    gchar *domain_secret;                                                                                /* Secret du domaine */
    gchar *api_url;                                                                                           /* URL de l'API */
    gint     comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    JsonNode *IOs;
    gint nbr_tour;
    gint nbr_tour_par_sec;
    gint nbr_tour_next_update;
    gint nbr_tour_delai;

    gint telemetrie_next_update;
    JsonNode *ai_nbr_tour_par_sec;                                                                        /* Tour par seconde */
    JsonNode *ai_max_rss;                                                                                      /* Maximum RSS */
    JsonNode *ai_log_par_min;                                                                              /* Logs par minute */

    void *vars;                                                               /* Pointeur vers les variables de run du module */
  };

 extern struct ABLS_AGENT *Agent_init                 ( gchar *entete, gchar *agent_classe, gchar *agent_version, gint sizeof_vars,
                                                        gint argc, gchar **argv );
 extern void               Agent_enable_signals       ( struct ABLS_AGENT *agent );
 extern void               Agent_disable_signals      ( void );
 extern void               Agent_send_comm_to_master  ( struct ABLS_AGENT *agent, gboolean etat );
 extern void               Agent_loop                 ( struct ABLS_AGENT *agent );
 extern void               Agent_end                  ( struct ABLS_AGENT *agent );
extern void               Agent_restart              ( struct ABLS_AGENT *agent );

#endif /* _ABLS_AGENT_LIBS_AGENT_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/