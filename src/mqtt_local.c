/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/mqtt_local.c        Fonctions communes de gestion des requetes MQTT locales                                */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                17.08.2024 12:31:26 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mqtt_local.c
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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "abls-agent-libs.h"

/******************************************************************************************************************************/
/* Mqtt_Send_AI: Envoie le bit AI au master                                                                                   */
/* Entrée: la structure ABLS_AGENT, l'AI, la valeur et le range                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mqtt_Send_AI ( struct ABLS_AGENT *agent, JsonNode *agent_ai, gdouble valeur, gboolean in_range )
  { if (! (agent && agent_ai)) return;
    gdouble  old_valeur   = Json_get_double ( agent_ai, "valeur" );
    gboolean old_in_range = Json_get_bool   ( agent_ai, "in_range" );
    gboolean need_sync    = Json_get_bool   ( agent_ai, "need_sync" );

    if ( need_sync || old_valeur != valeur || old_in_range != in_range )
     { Json_add_double( agent_ai, "valeur", valeur );
       Json_add_bool( agent_ai, "in_range", in_range );
       Json_add_bool( agent_ai, "need_sync", FALSE );
       gchar *agent_tech_id = Json_get_string ( agent_ai, "agent_tech_id" );
       gchar *agent_acronyme = Json_get_string ( agent_ai, "agent_acronyme" );
       Info( __func__, "mqtt_local", agent->agent_tech_id, LOG_DEBUG, "'%s:%s' = %f (in_range=%d)", agent_tech_id, agent_acronyme, valeur, in_range );
       JsonNode *RootNode = Json_create();
       if (!RootNode) return;
       Json_add_double( RootNode, "valeur", valeur );
       Json_add_bool( RootNode, "in_range", in_range );
       Mqtt_send_message ( agent->mqtt_local, RootNode, TRUE, "SET_AI/%s/%s", agent_tech_id, agent_acronyme );
       Json_unref( RootNode );
     }
  }
/******************************************************************************************************************************/
/* Mqtt_Send_DI: Envoie le bit DI au master                                                                                   */
/* Entrée: la structure ABLS_AGENT, la DI, la valeur                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mqtt_Send_DI ( struct ABLS_AGENT *agent, JsonNode *agent_di, gboolean etat )
  { if (! (agent && agent_di)) return;
    gboolean old_etat   = Json_get_bool ( agent_di, "etat" );
    gboolean need_sync  = Json_get_bool ( agent_di, "need_sync" );

    if ( need_sync || (old_etat != etat) )
     { Json_add_bool( agent_di, "etat", (etat ? TRUE : FALSE) );
       Json_add_bool( agent_di, "need_sync", FALSE );
       gchar *agent_tech_id = Json_get_string ( agent_di, "agent_tech_id" );
       gchar *agent_acronyme = Json_get_string ( agent_di, "agent_acronyme" );
       Info( __func__, "mqtt_local", agent->agent_tech_id, LOG_DEBUG, "'%s:%s' = %d", agent_tech_id, agent_acronyme, etat );
       JsonNode *RootNode = Json_create();
       if (!RootNode) return;
       Json_add_bool( RootNode, "etat", etat );
       Mqtt_send_message ( agent->mqtt_local, RootNode, TRUE, "SET_DI/%s/%s", agent_tech_id, agent_acronyme );
       Json_unref( RootNode );
     }
  }
/******************************************************************************************************************************/
/* Mqtt_Send_DI_pulse: Envoie le bit DI au master, au format pulse                                                            */
/* Entrée: la structure ABLS_AGENT, la DI, la valeur                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mqtt_Send_DI_pulse ( struct ABLS_AGENT *agent, gchar *tech_id, gchar *acronyme )
  { if (! (agent && tech_id && acronyme)) return;
    JsonNode *thread_di = Json_create();
    if (!thread_di) return;
    Json_add_string( thread_di, "from_thread_tech_id", agent->agent_tech_id );
    Info( __func__, "mqtt_local", agent->agent_tech_id, LOG_DEBUG, "'%s:%s' = PULSE", tech_id, acronyme );
    Mqtt_send_message ( agent->mqtt_local, thread_di, FALSE, "SET_DI_PULSE/%s/%s", tech_id, acronyme );
    Json_unref( thread_di );
  }
/******************************************************************************************************************************/
/* Mqtt_Send_CI_pulse: Envoie une impulsion CI au master                                                                      */
/* Entrée: la structure ABLS_AGENT, le noeud CI                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mqtt_Send_CI_pulse ( struct ABLS_AGENT *agent, JsonNode *thread_ci )
  { if (! (agent && thread_ci)) return;
    gchar *thread_acronyme = Json_get_string ( thread_ci, "thread_acronyme" );
    Info( __func__, "mqtt_local", agent->agent_tech_id, LOG_DEBUG, "'%s:%s' = PULSE", agent->agent_tech_id, thread_acronyme );
    JsonNode *RootNode = Json_create();
    if (!RootNode) return;
    Json_add_string( RootNode, "from_thread_tech_id", agent->agent_tech_id );
    Mqtt_send_message ( agent->mqtt_local, RootNode, FALSE, "SET_CI_PULSE/%s/%s", agent->agent_tech_id, thread_acronyme );
    Json_unref( RootNode );
  }
/******************************************************************************************************************************/
/* Mqtt_Send_WATCHDOG: Envoie le WATCHDOG au master                                                                           */
/* Entrée: la structure ABLS_AGENT, le watchdog, la consigne                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mqtt_Send_WATCHDOG ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gint consigne )
  { if (! (agent && agent_acronyme)) return;
    JsonNode *agent_watchdog = Json_create();
    if(!agent_watchdog) return;
    Json_add_int( agent_watchdog, "consigne", consigne );

    Info( __func__, "mqtt_local", agent->agent_tech_id, LOG_DEBUG, "'%s:%s' = %d", agent->agent_tech_id, agent_acronyme, consigne );
    Mqtt_send_message ( agent->mqtt_local, agent_watchdog, TRUE, "SET_WATCHDOG/%s/%s", agent->agent_tech_id, agent_acronyme );
    Json_unref( agent_watchdog );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
