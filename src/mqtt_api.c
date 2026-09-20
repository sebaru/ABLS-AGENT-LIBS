/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/mqtt_api.c Gestion des helpers MQTT API communs à tous les agents                                        */
/* Projet Abls-Habitat                               Gestion d'habitat                                                        */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_api.c
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

 #define _GNU_SOURCE
 #include <stdarg.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Agent_get_mqtt_api_message: Dépile un message de la queue MQTT API (non-bloquant)                                          */
/* Entrée: La structure afférente                                                                                             */
/* Sortie: pointeur vers le JsonNode du message, ou NULL si aucun message disponible                                          */
/******************************************************************************************************************************/
 JsonNode *Agent_get_mqtt_api_message ( struct ABLS_AGENT *agent )
  { if (!agent || !agent->mqtt_api) return(NULL);
    JsonNode *api_message = Mqtt_get_message ( agent->mqtt_api );
    if (api_message)
     { if ( Mqtt_topic_is ( api_message, 4, "+", "AGENT", agent->agent_tech_id, "LOG" ) )
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
       else return ( api_message );                                      /* Transfert directement pour traitement par l'agent */
     }
    Json_unref ( api_message );                       /* Message traité en pre-emption par la librairie, on libère la mémoire */
    return(NULL);
  }

/******************************************************************************************************************************/
/* Agent_send_mqtt_api_message: Envoi MQTT vers l'API en préfixant le topic par le domain_uuid                                */
/* Entrée: Agent, JsonNode, flag retain et topic relatif au domaine (variadique)                                              */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 void Agent_send_mqtt_api_message ( struct ABLS_AGENT *agent, JsonNode *node, gboolean retain, gchar *topic, ... )
  { gchar relative_topic[256], full_topic[512];
    va_list ap;

    if (!agent || !agent->mqtt_api || !agent->domain_uuid || !topic) return;

    va_start ( ap, topic );
    g_vsnprintf ( relative_topic, sizeof(relative_topic), topic, ap );
    va_end ( ap );

    g_snprintf ( full_topic, sizeof(full_topic), "%s/%s", agent->domain_uuid, relative_topic );
    Mqtt_send_message ( agent->mqtt_api, node, retain, "%s", full_topic );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/