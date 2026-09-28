/******************************************************************************************************************************/
/* src/agent_private.h Définition interne de la structure ABLS_AGENT — abls-agent-libs                                        */
/* Projet Abls-Habitat                               Gestion d'habitat                                       03.07.2026       */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * agent_private.h
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

/* Header PRIVE : il n'est PAS installé. Les agents ne doivent jamais connaitre le layout de struct ABLS_AGENT, afin qu'un    */
/* changement de ce layout ne casse pas les binaires compilés contre une autre version de la librairie.                       */

#ifndef _ABLS_AGENT_LIBS_AGENT_PRIVATE_H_
 #define _ABLS_AGENT_LIBS_AGENT_PRIVATE_H_

 #include <signal.h>
 #include <sys/time.h>

 #include "abls-agent-libs.h"

 struct ABLS_AGENT
  { gboolean Agent_run;                                     /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean is_dnf;                                                             /* TRUE if the underlying OS is Debian-based */
    gboolean is_apt;                                                             /* TRUE if the underlying OS is Debian-based */
    gboolean standalone;                                                   /* TRUE if the agent is running in standalone mode */
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
    gboolean dry_run;                                                                 /* Do not really send Inputs or outputs */
    gint     comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    JsonNode *IOs;

    guint tps_consigne;                                                                   /* nombre de tour par seconde cible */
    guint tps_value;                                                                     /* nombre de tour par seconde actuel */

    guint telemetrie_next_update;
    JsonNode *ai_nbr_tour_par_sec;                                                                        /* Tour par seconde */
    JsonNode *ai_rss_mem;                                                                                      /* RSS */
    JsonNode *ai_virt_mem;                                                                                    /* Mémoire virtuelle */
    JsonNode *ai_log_par_min;                                                                              /* Logs par minute */

    GSList  *status_stack;                                   /* Pile LIFO des status de l'agent. La tete est le status publié */
    GRWLock  status_stack_lock;

    struct itimerval timer;
    guint Top;                                                                                          /* dixième de seconde */
    void *vars;                                                               /* Pointeur vers les variables de run du module */
  };

#endif /* _ABLS_AGENT_LIBS_AGENT_PRIVATE_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
