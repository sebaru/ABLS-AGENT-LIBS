/******************************************************************************************************************************/
/* ABLS-AGENT-LIBS/mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques des agents                   */
/* Projet Abls-Habitat                   Gestion d'habitat                                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemonique.c
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
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "abls_agent_libs.h"

/******************************************************************************************************************************/
/* Mnemo_create_AI: Créer un JSON pour une AI                                                                                 */
/* Entrée: la structure ABLS_AGENT, les parametres de l'AI                                                                    */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_AI ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "AI" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/add/ai", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add AI %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_add_bool ( node, "in_range", FALSE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_add_bool ( node, "need_sync", TRUE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_DI: Créé un JSON pour une DI                                                                                  */
/* Entrée: la structure AGENT, les parametres de la DI                                                                        */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_DI ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "DI" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/add/di", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add DI %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_add_bool ( node, "need_sync", TRUE );            /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_CI: Créer un JSON pour une CI                                                                                 */
/* Entrée: la structure ABLS_AGENT, les parametres de la CI                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_CI ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "CI" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/add/ci", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add CI %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_DO: Créé un JSON pour une DI                                                                                  */
/* Entrée: la structure ABLS_AGENT, les parametres de la DI                                                                   */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_DO ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gboolean mono )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "DO" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_bool   ( node, "mono", mono );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/do", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add DO %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_AO: Créer un JSON pour une AO                                                                                 */
/* Entrée: la structure AGENT, les parametres de l'AO                                                                         */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_AO ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "AO" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/add/ao", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add AO %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_HORLOGE: Créé un JSON pour une Horloge                                                                        */
/* Entrée: la structure ABLS_AGENT, les parametres de l'HORLOGE                                                               */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_HORLOGE ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "HORLOGE" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/add/horloge", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add HORLOGE %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_HORLOGE_tick: Créé un tick sur une horloge donnée                                                             */
/* Entrée: la structure ABLS_AGENT, les parametres de l'HORLOGE                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_create_HORLOGE_tick ( struct ABLS_AGENT *agent, JsonNode *bit, gint heure, gint minute )
  { JsonNode *node = Json_create();
    if (!node) return;
    Json_add_string ( node, "classe", "HORLOGE" );
    Json_add_string ( node, "tech_id", Json_get_string ( bit, "tech_id" ) );
    Json_add_string ( node, "acronyme", Json_get_string ( bit, "acronyme" ) );
    Json_add_int    ( node, "heure", heure );
    Json_add_int    ( node, "minute", minute );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/add/tick", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
                 "Could not add HORLOGE tick %s:%d:%d to API",
                 Json_get_string ( bit, "acronyme" ), heure, minute );
     }
    Json_unref ( api_result );
    Json_unref ( node );
  }
/******************************************************************************************************************************/
/* Mnemo_delete_thread_HORLOGE_tick: Supprime un tick sur une horloge donnée                                                  */
/* Entrée: la structure ABLS_AGENT, les parametres de l'HORLOGE                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_delete_thread_HORLOGE_tick ( struct ABLS_AGENT *agent, JsonNode *bit )
  { if (!bit) return;
    Json_add_string ( bit, "classe", "HORLOGE" );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/del/tick", bit );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR,
                 "Could not DEL HORLOGE tick for '%s'", Json_get_string ( bit, "acronyme" ) );
     }
    Json_unref ( api_result );
  }
/******************************************************************************************************************************/
/* Mnemo_create_WATCHDOG: Créer un JSON pour un WATCHDOG                                                                      */
/* Entrée: la structure ABLS_AGENT, les parametres du WATCHDOG                                                                */
/* Sortie: le JsonNode représentant le bit interne                                                                            */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_WATCHDOG ( struct ABLS_AGENT *agent, gchar *agent_acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "WATCHDOG" );
    Json_add_string ( node, "agent_tech_id", agent->agent_tech_id );
    Json_add_string ( node, "agent_acronyme", agent_acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/watchdog", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Could not add WATCHDOG %s to API", agent_acronyme ); }
    Json_unref ( api_result );
    Json_array_add_one_element ( agent->IOs, "IOs", node );
    return(node);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
