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

 #include <glib.h>
 #include <json-glib/json-glib.h>
 #include <signal.h>

 #include <abls-libs/abls-libs.h>

 #define ABLS_AGENT_CONFIG_FILE "/etc/abls-agent.conf"

 struct ABLS_AGENT
  { gboolean Agent_run;                                     /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    struct ABLS_MQTT *mqtt_local;
    struct ABLS_MQTT *mqtt_api;
    JsonNode *local_config;                                                      /* Pointeur vers la config locale de l'agent */
    JsonNode *api_config;                                                           /* Pointeur vers la config API de l'agent */
    gchar *agent_tech_id;                                                                 /* Identifiant technique de l'agent */
    gchar *agent_classe;                                                                                 /* Classe de l'agent */
    gchar *domain_uuid;                                                                                    /* UUID du domaine */
    gchar *domain_secret;                                                                                /* Secret du domaine */
    gchar *api_url;                                                                                           /* URL de l'API */
    gint     comm_status;                                                       /* Report local du status de la communication */
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

 extern struct ABLS_AGENT *Agent_init                 ( gchar *agent_classe, gint sizeof_vars );
 extern void               Agent_send_comm_to_master  ( struct ABLS_AGENT *agent, gboolean etat );
 extern void               Agent_loop                 ( struct ABLS_AGENT *agent );
 extern void               Agent_end                  ( struct ABLS_AGENT *agent );

#endif /* _ABLS_AGENT_LIBS_AGENT_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/