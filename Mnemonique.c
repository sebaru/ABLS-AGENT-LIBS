/******************************************************************************************************************************/
/* ABLS-SATELLITE-LIBS/Mnemonique.c        Déclaration des fonctions pour la gestion des mnemoniques des satellites           */
/* Projet Abls-Habitat                   Gestion d'habitat                                      dim 19 avr 2009 15:15:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/* ©ConnectABLS                                                                                                               */
/******************************************************************************************************************************/
/*
 * Mnemonique.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "abls-satellite-libs.h"

/******************************************************************************************************************************/
/* Mnemo_create_AI: Créer un JSON pour une AI                                                                                 */
/* Entrée: la structure THREAD, les parametres de l'AI                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_AI ( struct SATELLITE *satellite, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "AI" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ai", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add AI %s to API", satellite->thread_tech_id, thread_acronyme );
     }
    Json_unref ( api_result );
    Json_add_bool ( node, "in_range", FALSE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_add_bool ( node, "need_sync", TRUE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_DI: Créé un JSON pour une DI                                                                                  */
/* Entrée: la structure THREAD, les parametres de la DI                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_DI ( struct SATELLITE *satellite, gchar *thread_acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "DI" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/di", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add DI %s to API", satellite->thread_tech_id, thread_acronyme );
     }
    Json_unref ( api_result );
    Json_add_bool ( node, "need_sync", TRUE );       /* Ajoute un flag first turn pour envoyer au master des le 1er tour */
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_CI: Créer un JSON pour une CI                                                                                  */
/* Entrée: la structure THREAD, les parametres de la CI                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_CI ( struct SATELLITE *satellite, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "CI" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ci", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add CI %s to API", satellite->thread_tech_id, thread_acronyme );
     }
    Json_unref ( api_result );
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_DO: Créé un JSON pour une DI                                                                                  */
/* Entrée: la structure SATELLITE, les parametres de la DI                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_DO ( struct SATELLITE *satellite, gchar *thread_acronyme, gchar *libelle, gboolean mono )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "DO" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_bool   ( node, "mono", mono );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/do", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add DO %s to API", satellite->thread_tech_id, thread_acronyme );
     }
    Json_unref ( api_result );
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_AO: Créer un JSON pour une AO                                                                                  */
/* Entrée: la structure THREAD, les parametres de l'AO                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_AO ( struct SATELLITE *satellite, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "AO" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    Json_add_string ( node, "unite", unite );
    Json_add_int    ( node, "archivage", archivage );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/ao", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add AO %s to API", satellite->thread_tech_id, thread_acronyme );
     }
    Json_unref ( api_result );
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_HORLOGE: Créé un JSON pour une Horloge                                                                 */
/* Entrée: la structure THREAD, les parametres de l'HORLOGE                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_HORLOGE ( struct SATELLITE *satellite, gchar *acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "HORLOGE" );
    Json_add_string ( node, "tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "acronyme", acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/add", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add HORLOGE %s to API", satellite->thread_tech_id, acronyme );
     }
    Json_unref ( api_result );
    Json_array_add_element ( Json_get_array ( satellite->IOs, "IOs" ), node );
    return(node);
  }
/******************************************************************************************************************************/
/* Mnemo_create_HORLOGE_tick: Créé un tick sur une horloge donnée                                                              */
/* Entrée: la structure SATELLITE, les parametres de l'HORLOGE                                                                 */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_create_HORLOGE_tick ( struct SATELLITE *satellite, JsonNode *bit, gint heure, gint minute )
  { JsonNode *node = Json_create();
    if (!node) return;
    Json_add_string ( node, "classe", "HORLOGE" );
    Json_add_string ( node, "tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "acronyme", Json_get_string ( bit, "acronyme" ) );
    Json_add_int    ( node, "heure", heure );
    Json_add_int    ( node, "minute", minute );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/add/tick", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add HORLOGE tick %s:%d:%d to API",
                 satellite->thread_tech_id, Json_get_string ( bit, "acronyme" ), heure, minute );
     }
    Json_unref ( api_result );
    Json_unref ( node );
  }
/******************************************************************************************************************************/
/* Mnemo_create_thread_HORLOGE_tick: Créé un tick sur une horloge donnée                                                      */
/* Entrée: la structure THREAD, les parametres de l'HORLOGE                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Mnemo_delete_HORLOGE_tick ( struct SATELLITE *satellite, JsonNode *bit )
  { if (!bit) return;
    Json_add_string ( bit, "classe", "HORLOGE" );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/del/tick", bit );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not DEL HORLOGE tick for '%s'",
                 satellite->thread_tech_id, Json_get_string ( bit, "acronyme" ) );
     }
    Json_unref ( api_result );
  }
/******************************************************************************************************************************/
/* Mnemo_create_WATCHDOG: Créé un watchdog pour un satellite donné                                                            */
/* Entrée: la structure SATELLITE, les parametres du WATCHDOG                                                                 */
/* Sortie: La structure JSON représentant le WATCHDOG                                                                         */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_WATCHDOG ( struct SATELLITE *satellite, gchar *acronyme, gchar *libelle )
  { JsonNode *node = Json_create();
    if (!node) return(NULL);
    Json_add_string ( node, "classe", "WATCHDOG" );
    Json_add_string ( node, "thread_tech_id", satellite->thread_tech_id );
    Json_add_string ( node, "thread_acronyme", satellite->thread_acronyme );
    Json_add_string ( node, "libelle", libelle );
    JsonNode *api_result = Http_Post_to_global_API ( "/run/thread/add/watchdog", node );
    if (!api_result || Json_get_int ( api_result, "http_code" ) != 200)
     { Info( __func__, "mnemo", NULL, LOG_ERR,
                 "%s: Could not add WATCHDOG %s to API", satellite->thread_tech_id, satellite->thread_acronyme );
     }
    Json_unref ( api_result );
    Json_array_add_one_element ( satellite->IOs, "IOs", node );
    return(node);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
