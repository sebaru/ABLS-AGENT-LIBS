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

 #include <sys/time.h>
 #include <glib.h>
 #include <json-glib/json-glib.h>
 #include <abls-libs/abls-libs.h>

 enum { AGENT_ARCHIVE_NONE    = 0,
        AGENT_ARCHIVE_5_SEC   = 50,
        AGENT_ARCHIVE_1_MIN   = 600,
        AGENT_ARCHIVE_5_MIN   = 3000,
        AGENT_ARCHIVE_10_MIN  = 6000,
        AGENT_ARCHIVE_30_MIN  = 18000,
        AGENT_ARCHIVE_1_HEURE = 36000,
        AGENT_ARCHIVE_6_HEURE = 216000,
        AGENT_ARCHIVE_1_JOUR  = 864000
      };

 enum { AGENT_IS_STOPPED,
        AGENT_IS_RUNNING,
        AGENT_NEED_TO_STOP,
        AGENT_NEED_TO_RESTART,
        NBR_AGENT_STATUS,
      };

 struct ABLS_AGENT;         /* Type opaque : le layout est privé à abls-agent-libs, l'accès se fait via les helpers ci-dessous */

 extern struct ABLS_AGENT *Agent_init                 ( gchar *entete, gchar *agent_classe, gchar *agent_version, gint sizeof_vars,
                                                        gint argc, gchar **argv );
 extern void               Agent_is_ready             ( struct ABLS_AGENT *agent );
 extern void               Agent_enable_signals       ( struct ABLS_AGENT *agent );
 extern void               Agent_disable_signals      ( void );
 extern void               Agent_send_comm_to_master  ( struct ABLS_AGENT *agent, gboolean etat );
 extern gpointer           Agent_status_push          ( struct ABLS_AGENT *agent, gchar *format, ... );
 extern void               Agent_status_pop           ( struct ABLS_AGENT *agent, gpointer handle );
 extern void               Agent_loop                 ( struct ABLS_AGENT *agent );
 extern void               Agent_end                  ( struct ABLS_AGENT *agent );
 extern void               Agent_restart              ( struct ABLS_AGENT *agent );
 extern gchar             *Agent_config_get_string    ( struct ABLS_AGENT *agent, gchar *name );
 extern gboolean           Agent_config_get_bool      ( struct ABLS_AGENT *agent, gchar *name );
 extern gint               Agent_config_get_int       ( struct ABLS_AGENT *agent, gchar *name );
 extern JsonArray         *Agent_config_get_array     ( struct ABLS_AGENT *agent, gchar *name );
 extern guint              Agent_config_get_array_length ( struct ABLS_AGENT *agent, gchar *name );
 extern void               Agent_config_foreach_array_element ( struct ABLS_AGENT *agent, gchar *name,
                                                                JsonArrayForeach fonction, gpointer data );
 extern JsonNode          *Agent_get_mqtt_local_message ( struct ABLS_AGENT *agent );

/*********************************************** Accesseurs à la structure opaque *********************************************/
 extern gchar             *Agent_get_tech_id          ( struct ABLS_AGENT *agent );
 extern gchar             *Agent_get_classe           ( struct ABLS_AGENT *agent );
 extern void              *Agent_get_vars             ( struct ABLS_AGENT *agent );
 extern guint              Agent_get_top              ( struct ABLS_AGENT *agent );
 extern gboolean           Agent_is_running           ( struct ABLS_AGENT *agent );
 extern gboolean           Agent_is_apt               ( struct ABLS_AGENT *agent );
 extern gboolean           Agent_is_dnf               ( struct ABLS_AGENT *agent );

#endif /* _ABLS_AGENT_LIBS_AGENT_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/